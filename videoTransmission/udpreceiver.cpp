#include "udpreceiver.h"

UdpReceiver::UdpReceiver():
    m_udpSocket(nullptr)
{
    //利用每帧图片的数据量确定缓冲区的大小
    m_maxBufLength = 640*480*3+sizeof(transPack_t);
    m_dataBuf = new char[m_maxBufLength];
    m_audioPlayer = new AudioHandler;
    m_audioPlayer->init("play");

    m_vedioPlayer = new VedioHandler;
    m_vedioPlayer->init("play");
}

UdpReceiver::~UdpReceiver()
{
    stopReceive();
    delete [] m_dataBuf;
    delete m_audioPlayer;
    delete m_vedioPlayer;

    if(m_udpSocket!=nullptr)
        delete m_udpSocket;
}

//注册到服务器
bool UdpReceiver::registerToServer()
{
    transPack_t package;
    package.head[0] = 0x66;
    package.head[1] = 0xcc;
    package.type = Register;
    package.length = 0;
    package.checkNum = 0;
    package.senderId = g_myId;
    package.receiverId = 0;

    size_t dataLength = sizeof(package)+package.length;
    for(int i=0; i<5; ++i)
        m_udpSocket->writeDatagram((char *)&package,dataLength, g_serverIp,g_serverPort);
    return true;
}

//返回本地端口号
uint16_t UdpReceiver::startReceive()
{
    m_udpSocket = new QUdpSocket(this);
    m_udpSocket->bind(QHostAddress::Any);

    registerToServer();

    //连接信号槽，udp收到消息后触发槽函数
    connect(m_udpSocket,SIGNAL(readyRead()),this,SLOT(onReadyRead()));
    this->start();

    return m_udpSocket->localPort();
}

void UdpReceiver::stopReceive()
{
    //判断指针是否为空，然后断开信号
    //否则，若指针为空将报错
    if(m_udpSocket != nullptr)
        disconnect(m_udpSocket,SIGNAL(readyRead()),this,SLOT(onReadyRead()));

    if(this->isRunning())
    {
        this->requestInterruption();
        this->quit();
        this->wait();
    }

    if(m_udpSocket != nullptr)
    {
        m_udpSocket->close();
        delete m_udpSocket;
        m_udpSocket = nullptr;
    }
}

//udp 数据读取函数
void UdpReceiver::onReadyRead()
{
    //std::cout << "UdpReceiver::onReadyRead!!!" << std::endl;
    while(m_udpSocket->hasPendingDatagrams())
    {
        //应为服务器ip
        QHostAddress senderip;
        quint16 senderport;

        int len = m_udpSocket->readDatagram(m_dataBuf,m_maxBufLength,&senderip,&senderport);
        //qDebug() << "(" <<senderip.toIPv4Address() << ") -> receive msg. len: " << len ;

        if( g_serverIp.toIPv4Address() != g_serverIp.toIPv4Address()) //消息不是来自服务器
            continue;
        if(len <=0 )
            continue;


        transPack_t* package = (transPack_t*)m_dataBuf;
        //std::cout << "package->type:" << int(package->type) << std::endl;

        if(RequestConnect == package->type)
        {
            //std::cout <<  "RequestConnect" << std::endl;
            if(g_systemStatus!= SystemIdle)
                handleRequestConnect(package->senderId, false);
            else
                handleRequestConnect(package->senderId ,true);
        }
        //应答被接受、拒绝、挂断 是否来自正在通话的ip？ 需增加判断条件
        else if(AcceptConnect == package->type)
        {
            //std::cout <<  "AcceptConnect" << std::endl;
            g_systemStatus = SystemAccepted;
        }
        else if(RequestConnect == package->type)
        {
            //std::cout <<  "RequestConnect" << std::endl;
            g_systemStatus = SystemRefused;
        }
        else if(DisConnect == package->type)
        {
            //std::cout <<  "DisConnect" << std::endl;
        }
        else if(Vedio == package->type)
        {
            //std::cout <<  "Vedio" << std::endl;
            handleVedioMsg(m_dataBuf);
        }
        else if(Audio == package->type)
        {
            //std::cout <<  "Audio" << std::endl;
            handleAudioMsg(m_dataBuf);
        }
        else
        {
            std::cout << "unknown type " << std::endl;
        }

    }
}

void UdpReceiver::handleRequestConnect(uint16_t id, bool flag)
{
    transPack_t package;
    package.head[0] = 0x66;
    package.head[1] = 0xcc;
    if(flag)
        package.type = AcceptConnect;
    else
        package.type = RefuseConnect;
    package.length = 0;
    package.checkNum = 0;
    package.senderId = g_myId;
    package.receiverId = id;
    const char *data = (const char *)&package;

    m_udpSocket->writeDatagram(data,sizeof(package)+package.length,g_serverIp,g_serverPort);
}

void UdpReceiver::handleVedioMsg(char* const buf)
{
    transPack_t* package = (transPack_t*)buf;
    int len = package->length;
    m_vedioPlayer->appendData(buf+sizeof(transPack_t),len);
}

void UdpReceiver::handleAudioMsg(char* const buf)
{
    transPack_t* package = (transPack_t*)buf;
    int len = package->length;
    //std::cout << len << " bytes add to audioBuf" << std::endl;
    m_audioPlayer->appendData(buf+sizeof(transPack_t),len);
}

//音频和视频播放
void UdpReceiver::run(void)
{
    while (!this->isInterruptionRequested())
    {
        m_audioPlayer->playAudio();
        m_vedioPlayer->playVedio();
        QThread::msleep(50);
    }

}
