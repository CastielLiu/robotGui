#include "udpsender.h"

UdpSender::UdpSender():
    m_udpSocket(nullptr),
    m_audioRecorder(nullptr),
    m_vedioCaptor(nullptr),
    m_remoteControler(nullptr)
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

    m_audioRecorder = new AudioHandler;
    if(g_isOpenAudio)
        m_audioRecorder->init(AudioHandler::AudioMode_Record);

    m_vedioCaptor = new VedioHandler;
    if(g_isOpenVedio)
        m_vedioCaptor->init(VedioHandler::VedioMode_Capture);

    if(g_isRemoteTerminal)
    {
        m_remoteControler = new RemoteControl;
  /*      connect(g_ui->widget_control1,SIGNAL(dirKeyPressed(int)),
                m_remoteControler,SLOT(onDirKeyPressed(int)));

        connect(g_ui->widget_control1,SIGNAL(dirKeyReleased(int)),
                m_remoteControler,SLOT(onDirKeyReleased(int)));*/
    }
    this->start();
    return true;
}

void UdpSender::stopSend()
{
    if(this->isRunning())
    {
        this->requestInterruption();
        this->quit();
        this->wait();
    }

    if(m_audioRecorder!= nullptr )
    {
        delete m_audioRecorder;
        m_audioRecorder = nullptr;
    }

    if(m_vedioCaptor != nullptr)
    {
        delete m_vedioCaptor;
        m_vedioCaptor = nullptr;
    }

    if(m_remoteControler != nullptr)
    {
        delete m_remoteControler;
        m_remoteControler = nullptr;
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
        QThread::msleep(50);
        if(g_isRemoteTerminal)
            m_remoteControler->sendControlCmd(m_udpSocket,g_robotControlId);

       if(++cnt%2==0)
           m_vedioCaptor->sendImage(m_udpSocket,m_dstId);

       m_audioRecorder->sendAudio(m_udpSocket,m_dstId);
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

const RemoteControl * UdpSender::getRemoteCtrler()
{
    return m_remoteControler;
}
