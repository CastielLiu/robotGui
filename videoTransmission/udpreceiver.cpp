#include "udpreceiver.h"

UdpReceiver::UdpReceiver():
    m_udpSocket(nullptr),
    m_audioPlayer(nullptr),
    m_vedioPlayer(nullptr)
{
    //利用每帧图片的数据量确定缓冲区的大小
    m_maxBufLength = 640*480*3+sizeof(transPack_t);
    m_dataBuf = new char[m_maxBufLength];

    m_udpSocket = new QUdpSocket();
    m_udpSocket->bind(QHostAddress::Any);
    //连接信号槽，udp收到消息后触发槽函数
    connect(m_udpSocket,SIGNAL(readyRead()),this,SLOT(onReadyRead()),Qt::DirectConnection);
}

UdpReceiver::~UdpReceiver()
{
    if(m_udpSocket != nullptr)
    {
        //对象析构后与之绑定的所有信号自动断开，无需手动断开
        //disconnect(m_udpSocket,SIGNAL(readyRead()),this,SLOT(onReadyRead()));
        m_udpSocket->close();
        delete m_udpSocket;
        m_udpSocket = nullptr;
    }

    stopPlayMv();
    delete [] m_dataBuf;
}

//注册到服务器
//此处只需启动一个注册线程
void UdpReceiver::registerToServer()
{
    std::thread t(&UdpReceiver::registerToServerThread,this);
    t.detach();
}

//注册到服务器线程函数
//此线程循环发送请求注册信息到服务器，直到注册成功或超时
void UdpReceiver::registerToServerThread()
{
    qDebug() << "registerToServerThread in: " << QThread::currentThread() ;
    transPack_t package;
    package.type = RequestRegister;
    package.senderId = g_myId;
    int cnt = 0;
    g_registerStatus = 1;
    while(g_registerStatus == 1) //登陆中(循环登录)
    {
        qDebug() << "send register msg...";
        m_udpSocket->writeDatagram((char *)&package,
                   sizeof(package), g_serverIp,g_registerPort);

        QThread::msleep(1000);
        if(++cnt > 10)
        {
            g_registerStatus = 0;
            g_ui->label_registerStatus->setText("login");
            qDebug() << "login time out!";
            break;
        }
    }
    if(g_registerStatus == 2) //登录成功
    {
        g_ui->label_registerStatus->setText("logout");
    }

}

//确认注册
//当收到服务器的注册回应时调用此函数
//此函数首先向serverPort再次发送注册信息，以与服务器的消息转发socket建立连接
void UdpReceiver::confirmRegister(quint16 serverPort)
{
    qDebug() << "confirmRegister... port:" <<  serverPort;
    transPack_t package;
    package.type = RequestRegister;
    package.senderId = g_myId;
    for(int i=0; i<3; ++i)
    {
        qDebug() << "try to send msg to new port:" <<  serverPort;
        m_udpSocket->writeDatagram((char *)&package,
                   sizeof(package), g_serverIp,serverPort);
        QThread::msleep(50);
    }
}

//退出登陆 等待完善...
void UdpReceiver::logout()
{
    //发送退出登陆信号，服务器收到后将客户端从列表中删除

    g_registerStatus = 0;
    stopPlayMv();
}

//  开始播放视频和声音
void UdpReceiver::startPlayMv()
{
    m_audioPlayer = new AudioHandler;
    m_audioPlayer->init("play");

    m_vedioPlayer = new VedioHandler;
    m_vedioPlayer->init("play");

    this->start(); //启动音频视频播放Qthread
}

void UdpReceiver::stopPlayMv()
{
    if(this->isRunning())
    {
        this->requestInterruption();
        this->quit();
        this->wait();
    }

    if(m_audioPlayer != nullptr)
    {
        delete m_audioPlayer;
        m_audioPlayer = nullptr;
    }
    if(m_vedioPlayer != nullptr)
    {
        delete m_vedioPlayer;
        m_vedioPlayer = nullptr;
    }
}

//udp 数据读取函数
void UdpReceiver::onReadyRead()
{
    //std::cout << "UdpReceiver::onReadyRead!!!" << std::endl;
    while(m_udpSocket->hasPendingDatagrams())
    {
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
        else if((Video == package->type || Audio == package->type) && //收到语音或视频
                (!g_isCaller && g_otherId==0)) //客户端被叫，且主叫id为默认
        {
            //如果为主叫方，下次直接处理消息
            //如果为被叫方，应将对方id保存并启动udpSender
            qDebug() << "called! .....";
            this->startPlayMv(); //开始播放
            g_otherId = package->senderId;
            emit startSendSignal();
        }
        else if(Video == package->type)
        {
            handleVedioMsg(m_dataBuf);
        }
        else if(Audio == package->type)
        {
            handleAudioMsg(m_dataBuf);
        }
        else if(ResponseRegister == package->type)
        {
            quint16 serverPort =
                m_dataBuf[sizeof(transPack_t)]+m_dataBuf[sizeof(transPack_t)+1]*256;

            confirmRegister(serverPort);
        }

        else if(RegisterOK == package->type)
        {
            g_msgPort = senderport; //记录服务器数据端口号
            g_registerStatus = 2;
            //启动心跳线程
            std::thread t(&UdpReceiver::heartBeatThread,this);
            t.detach();
        }
        else if(RegisterFail == package->type)
        {
            qDebug() << "register failed!" ;
            g_registerStatus = 0;
        }
        else if(HeartBeat == package->type)
        {
            m_heartBeatMutex.lock();
            m_serverLastHeartBeatTime = time(0);
            m_heartBeatMutex.unlock();
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

    //m_udpSocket->writeDatagram(data,sizeof(package)+package.length,g_serverIp,g_serverPort);
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
        QThread::msleep(30);
    }

}

void UdpReceiver::heartBeatThread()
{
    int heartBeatInterval = 5;
    transPack_t heartBeatPkg(HeartBeat);
    while(g_registerStatus == 2)
    {
        m_heartBeatMutex.lock();
        if(m_serverLastHeartBeatTime !=0 && m_serverLastHeartBeatTime-time(0) >heartBeatInterval+1)
        {
            qDebug() << "server is shutdown" ;
        }
        m_heartBeatMutex.unlock();

        m_udpSocket->writeDatagram((char *)&heartBeatPkg,
                   sizeof(heartBeatPkg), g_serverIp,g_msgPort);
        QThread::sleep(heartBeatInterval);
    }
}
