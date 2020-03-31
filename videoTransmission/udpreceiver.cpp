#include "udpreceiver.h"

UdpReceiver::UdpReceiver():
    m_udpSocket(nullptr),
    m_audioPlayer(nullptr),
    m_vedioPlayer(nullptr)
{
    std::cout << "create UdpReceiver in thread: " << QThread::currentThreadId() << std::endl;

    //利用每帧图片的数据量确定缓冲区的大小
    m_maxBufLength = 640*480*3+sizeof(pkgHeader_t);
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
    //std::thread t(&UdpReceiver::registerToServerThread,this);
    //t.detach();
    registerToServerInMainThread();
}

//注册到服务器线程函数
//此线程循环发送请求注册信息到服务器，直到注册成功或超时
//经测试，在新线程中发送注册消息可实现用户注册，
//但当同时执行两个应用时，第二个无法注册,提示无法在子线程blabla
void UdpReceiver::registerToServerThread()
{
    emit this->updateRegisterStatus(RegisterStatus_Ing);

    std::cout << "registerToServerThread in: " << QThread::currentThread() << std::endl;
    pkgHeader_t package;
    package.type = PkgType_RequestRegister;
    package.senderId = g_myId;
    package.receiverId = 0;
    int cnt = 0;

    qDebug() << "registering to server...";
    while(g_registerStatus == RegisterStatus_Ing) //登陆中(循环登录)
    {
        m_udpSocket->writeDatagram((char *)&package,
                   sizeof(package), g_serverIp,g_registerPort);

        QThread::msleep(1000);
        //qDebug() << "bytesAvailable::" << m_udpSocket->bytesAvailable();
        if(++cnt > 10)
        {
            emit this->updateRegisterStatus(RegisterStatus_None);

            qDebug() << "login time out!";
            break;
        }
    }
}

void UdpReceiver::registerToServerInMainThread()
{
    emit updateRegisterStatus(RegisterStatus_Ing);

    pkgHeader_t package(PkgType_RequestRegister);
    package.senderId = g_myId;
    package.receiverId = 0;

    m_udpSocket->writeDatagram((char *)&package,
               sizeof(package), g_serverIp,g_registerPort);
    m_registerTimerId = this->startTimer(10000); //启动10s定时器
}

void UdpReceiver::timerEvent(QTimerEvent *event)
{
    if(event->timerId() == m_registerTimerId)
    {
        killTimer(m_registerTimerId);//一定要关掉定时器，否则将被循环触发
        if(g_registerStatus != RegisterStatus_Ok)
        {
            qDebug() << "register overtime!";
            emit this->updateRegisterStatus(RegisterStatus_None);
        }
    }
}

//确认注册
//当收到服务器的注册回应时调用此函数
//此函数首先向serverPort再次发送注册信息，以与服务器的消息转发socket建立连接
void UdpReceiver::confirmRegister(quint16 serverPort)
{
    qDebug() << "confirmRegister... port:" <<  serverPort;
    pkgHeader_t package(PkgType_RequestRegister);
    package.senderId = g_myId;
    package.receiverId = 0;
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
    pkgHeader_t LogOutPkg(PkgType_LogOut);
    m_udpSocket->writeDatagram((char *)&LogOutPkg,
               sizeof(LogOutPkg), g_serverIp,g_msgPort);
    stopPlayMv();
}

//  开始播放视频和声音
void UdpReceiver::startPlayMv()
{
    g_systemStatus = SystemOnThePhone;
    m_audioPlayer = new AudioHandler;
    m_audioPlayer->init("play");

    m_vedioPlayer = new VedioHandler;
    m_vedioPlayer->init("play");

    this->start(); //启动音频视频播放Qthread
}

void UdpReceiver::stopPlayMv()
{
    g_systemStatus = SystemIdle;

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

void UdpReceiver::sendCmd(PkgType cmdType)
{
    pkgHeader_t pkg(cmdType);
    m_udpSocket->writeDatagram((char*)&pkg,sizeof(pkgHeader_t),g_serverIp,g_msgPort);
}

//udp 数据读取函数
void UdpReceiver::onReadyRead()
{
    //std::cout << "UdpReceiver::onReadyRead in thread: " << QThread::currentThreadId() << std::endl;
    while(m_udpSocket->hasPendingDatagrams())
    {
        QHostAddress senderip;
        quint16 senderport;

        int len = m_udpSocket->readDatagram(m_dataBuf,m_maxBufLength,&senderip,&senderport);

        if( g_serverIp.toIPv4Address() != g_serverIp.toIPv4Address()) //消息不是来自服务器
            continue;
        if(len <=0 ) continue;

        pkgHeader_t* package = (pkgHeader_t*)m_dataBuf;
        //std::cout << "package->type:" << int(package->type) << std::endl;

        if(PkgType_ResponseRegister == package->type) //服务器回应注册,包含新端口号
        {
            quint16 serverPort =
                m_dataBuf[sizeof(pkgHeader_t)]+m_dataBuf[sizeof(pkgHeader_t)+1]*256;

            confirmRegister(serverPort);
        }
        else if(PkgType_RegisterOK == package->type) //服务器回应注册成功
        {
            g_msgPort = senderport; //记录服务器数据端口号
            emit this->updateRegisterStatus(RegisterStatus_Ok);
            //启动心跳线程
            std::thread t(&UdpReceiver::heartBeatThread,this);
            t.detach();
        }
        else if(PkgType_RegisterFail == package->type) //服务器回应注册失败
        {
            qDebug() << "register failed!" ;
            emit this->updateRegisterStatus(RegisterStatus_None);
        }
        else if(PkgType_RequestConnect == package->type) //请求连接
        {
            //请求连接一般有很多条，确保只接受处理一次
            if(SystemOnThePhone == g_systemStatus)
                continue;
            uint16_t callerId = package->senderId;
            QString qstr = QString("you have a new call. ID: ") + QString::number(callerId);
            g_ui->statusBar->showMessage(qstr, 3000);
            //可以考虑设置一个弹窗线程，用户选择是否接听
            pkgHeader_t pkg;
            memcpy(&pkg, m_dataBuf, sizeof(pkgHeader_t)); //拷贝接收到的消息！
            pkg.senderId = g_myId;
            pkg.receiverId = callerId;
            pkg.length = 0;
            if(g_canCalled)
            {
                pkg.type =  PkgType_AcceptConnect;
                //启动发送数据
                emit startChatSignal(callerId);
            }
            else
                pkg.type =  PkgType_RefuseConnect;
            m_udpSocket->writeDatagram((char*)&pkg,sizeof(pkgHeader_t),senderip,senderport);
        }
        else if(PkgType_RefuseConnect == package->type) // 拒绝连接，请求被拒
        {
            emit calledBusy();
        }
        else if(PkgType_CalledOffline == package->type)
        {
            if(package->receiverId == g_robotControlId) //机器人受控端不在线
                g_ui->statusBar->showMessage(QString("Robot control receiver offline!"),2000);
            else if(g_ignoreCalledOffline) //视频语音被叫端不在线
                g_ui->statusBar->showMessage(QString("Called offline, but you ignore it"),2000);
            else
                emit calledBusy();
        }
        else if(PkgType_DisConnect == package->type)
        {
            g_ui->statusBar->showMessage(QString("Call disconnected"), 3000);
            emit stopChatSignal();
            //std::cout <<  "PkgType_DisConnect" << std::endl;
        }
        else if(PkgType_HeartBeat == package->type)
        {
            m_heartBeatMutex.lock();
            m_serverLastHeartBeatTime = time(nullptr);
            m_heartBeatMutex.unlock();
        }
        //以上消息类型为指令消息
        //以下消息类型为通话消息
        else if(PkgType_Video == package->type)
        {
            if(SystemOnThePhone == g_systemStatus)
                handleVedioMsg(m_dataBuf);
        }
        else if(PkgType_Audio == package->type)
        {
            if(SystemOnThePhone == g_systemStatus)
                handleAudioMsg(m_dataBuf);
        }
        else
            std::cout << "unknown type " << std::endl;
    }
}

void UdpReceiver::handleVedioMsg(char* const buf)
{
    pkgHeader_t* package = (pkgHeader_t*)buf;
    int len = package->length;
    m_vedioPlayer->appendData(buf+sizeof(pkgHeader_t),len);
}

void UdpReceiver::handleAudioMsg(char* const buf)
{
    pkgHeader_t* package = (pkgHeader_t*)buf;
    int len = package->length;
    //std::cout << len << " bytes add to audioBuf" << std::endl;
    m_audioPlayer->appendData(buf+sizeof(pkgHeader_t),len);
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
    m_serverLastHeartBeatTime = time(nullptr); //此处必须初始化
    pkgHeader_t heartBeatPkg(PkgType_HeartBeat);
    heartBeatPkg.senderId = g_myId;

    while(g_registerStatus == RegisterStatus_Ok)
    {
        m_heartBeatMutex.lock();
        //qDebug() << m_serverLastHeartBeatTime << "\t" << m_serverLastHeartBeatTime-time(0) ;
        bool disconnect = time(0) - m_serverLastHeartBeatTime >g_heartBeatInterval + g_maxHeartBeatDelay;
        m_heartBeatMutex.unlock();
        if(disconnect)
        {
            emit logoutSignal();
            qDebug() << "server is shutdown" ;
            return;
        }

        m_udpSocket->writeDatagram((char *)&heartBeatPkg,
                   sizeof(heartBeatPkg), g_serverIp,g_msgPort);
        QThread::sleep(g_heartBeatInterval);
    }
}
