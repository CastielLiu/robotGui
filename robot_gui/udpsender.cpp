#include "udpsender.h"

UdpSender::UdpSender():
    m_udpSocket(nullptr),
    m_audioRecorder(nullptr),
    m_vedioCaptor(nullptr),
    m_remoteControler(nullptr),
    m_bioRadar(nullptr)
{
    //std::cout << "create UdpSender in thread: " << QThread::currentThreadId() << std::endl;
}

UdpSender::~UdpSender()
{
    stopSend();
}

/*开始发送
 * dstId @目标客户端的id
 */
bool UdpSender::startSend(uint16_t dstId)
{
    m_dstId = dstId;

    //语音
    m_audioRecorder = new AudioHandler;
    if(g_isOpenAudio) //受语音复选框控制
        m_audioRecorder->init(AudioHandler::AudioMode_Record);

    //视频
    m_vedioCaptor = new VedioHandler;
    if(g_isOpenVedio)//受视频复选框控制
        m_vedioCaptor->init(VedioHandler::VedioMode_Capture);

    //远程端
    if(g_isRemoteTerminal)
    {
        //远程控制
        m_remoteControler = new RemoteControl;
    }
    //本地端
    else
    {
        //生物雷达
        m_bioRadar = new BiologicalRadar;
    }

    this->start();
    return true;
}

void UdpSender::stopSend()
{
    //停止发送
    if(this->isRunning())
    {
        this->requestInterruption();
        this->quit();
        this->wait();
    }

    //语音
    if(m_audioRecorder!= nullptr )
    {
        delete m_audioRecorder;
        m_audioRecorder = nullptr;
    }

    //视频
    if(m_vedioCaptor != nullptr)
    {
        delete m_vedioCaptor;
        m_vedioCaptor = nullptr;
    }

    //远程控制
    if(m_remoteControler != nullptr)
    {
        delete m_remoteControler;
        m_remoteControler = nullptr;
    }

    //生物雷达
    if(m_bioRadar != nullptr)
    {
        delete m_bioRadar;
        m_bioRadar = nullptr;
    }
}

void UdpSender::run()
{
    //此处分配内存不传递this指针，否则由于this所在线程与当前线程不匹配而报错
    //在run函数内部分配内存以保证socket使用线程与socket所在线程一致
    m_udpSocket = new QUdpSocket();
    //std::cout << "UdpSender: create QUdpSocket in thread: " << QThread::currentThreadId() << std::endl;
    uint32_t cnt = 0;
    while (!this->isInterruptionRequested())
    {
        //正在连接，发送请求连接指令
        if(g_transferStatus == transferStatus_Starting)
        {
            pkgHeader_t requestConnectPkg(PkgType_RequestConnect);
            requestConnectPkg.senderId = g_myId;
            requestConnectPkg.receiverId = g_calledId;

            m_udpSocket->writeDatagram((char *)&requestConnectPkg,
                      sizeof(requestConnectPkg), g_serverIp,g_msgPort);
            QThread::msleep(300);
            continue;
        }
        //未连接
        else if(g_transferStatus != transferStatus_Ing)
        {
            QThread::msleep(50);
            continue;
        }

        if(g_isRemoteTerminal)
        {
            m_remoteControler->sendControlCmd(m_udpSocket,g_robotControlId);
        }
        else
        {
            //发送生物雷达数据
            m_bioRadar->sendData(m_udpSocket,g_calledId);
        }

        if(++cnt%2==0)
           m_vedioCaptor->sendImage(m_udpSocket,m_dstId);

        m_audioRecorder->sendAudio(m_udpSocket,m_dstId);
        QThread::msleep(50);
    }
    m_udpSocket->close();
    delete m_udpSocket;
    m_udpSocket = nullptr;
}

//动态开启视频
void UdpSender::openVedio()
{
    if(m_vedioCaptor)
        m_vedioCaptor->init(VedioHandler::VedioMode_Capture);
}

//动态关闭视频
void UdpSender::closeVedio()
{
    if(m_vedioCaptor)
        m_vedioCaptor->stop(VedioHandler::VedioMode_Capture);
}

void UdpSender::openAudio()
{
    if(m_audioRecorder)
        m_audioRecorder->init(AudioHandler::AudioMode_Record);
}

void UdpSender::closeAudio()
{
    if(m_audioRecorder)
        m_audioRecorder->stop(AudioHandler::AudioMode_Record);
}
