#include "udpsender.h"

UdpSender::UdpSender():
    m_udpSocket(nullptr),
    m_audioRecorder(nullptr),
    m_vedioCaptor(nullptr),
    m_remoteControler(nullptr)
{
    std::cout << "create UdpSender in thread: " << QThread::currentThreadId() << std::endl;
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
    if(!m_audioRecorder->init("record"))
        return false;

    m_vedioCaptor = new VedioHandler;
    if(!m_vedioCaptor->init("record"))
        return false;

    if(g_openRemoteControl)
    {
        m_remoteControler = new RemoteControl;
        connect(g_ui->widget_control1,SIGNAL(dirKeyPressed(int)),
                m_remoteControler,SLOT(onDirKeyPressed(int)));

        connect(g_ui->widget_control1,SIGNAL(dirKeyReleased(int)),
                m_remoteControler,SLOT(onDirKeyReleased(int)));
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
    std::cout << "UdpSender: create QUdpSocket in thread: " << QThread::currentThreadId() << std::endl;

    while (!this->isInterruptionRequested())
    {
       //发送消息
       m_audioRecorder->sendAudio(m_udpSocket,m_dstId);
       m_vedioCaptor->sendImage(m_udpSocket,m_dstId);
       if(g_openRemoteControl)
           m_remoteControler->sendControlCmd(m_udpSocket,g_robotControlId);
       QThread::msleep(20);
       //QThread::sleep(2);
    }
    m_udpSocket->close();
    delete m_udpSocket;
    m_udpSocket = nullptr;
}
