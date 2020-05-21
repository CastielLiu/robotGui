#include "mainwindow.h"
#include <thread>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_udpReceiver(nullptr),
    m_udpSender(nullptr),
    m_configFile("config"),
    m_imageLabel(nullptr),
    m_radar(nullptr)
{
    ui->setupUi(this);

    this->clientModeSelection();//客户端模式选择
    this->loadPerformance();//载入用户参数

    this->setWindowTitle(mWindowTitle);
    this->setWindowIcon(QIcon(":/images/icon"));
    this->setFixedSize(this->width(),this->height());

    ui->label_registerStatus->setOpenClickEvent(true);
    connect(ui->label_registerStatus,SIGNAL(clicked()),this,SLOT(onLableRegisterStatusClicked()));
    ui->label_registerStatus->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter); //设置label文字居中
    ui->checkBox_audio->setCheckState(Qt::CheckState::Checked);
    ui->lineEdit_calledId->setText(QString::number(g_robotCallId));
    //ui->checkBox_audio->hide();

    ui->label_log->setAlignment(Qt::AlignHCenter); //居中对齐

    //连接action信号槽
    connect(ui->action_user_id,SIGNAL(triggered()),this,SLOT(onActionUserId()));
    connect(ui->action_auto_login,SIGNAL(triggered()),this,SLOT(onActionAutoLogin()));
    connect(ui->action_robotCall_id,SIGNAL(triggered()),this,SLOT(onActionRobotCallId()));
    connect(ui->action_robotControl_id,SIGNAL(triggered()),this,SLOT(onActionRobotControlId()));
    connect(ui->action_about,SIGNAL(triggered()),this,SLOT(onActionAbout()));
    connect(ui->action_debugConfig,SIGNAL(triggered()),this,SLOT(onActionDebugConfig()));
    connect(ui->action_biologicalRadar,SIGNAL(triggered()),this,SLOT(onActionBiologicalRadar()));
    connect(ui->action_workLog,SIGNAL(triggered()),this,SLOT(onActionWorkLog()));

    //实例化接收器
    m_udpReceiver = new UdpReceiver;
    connect(m_udpReceiver,SIGNAL(startChatSignal(uint16_t)),this,SLOT(startChat(uint16_t)));
    connect(m_udpReceiver,SIGNAL(stopChatSignal()),this,SLOT(stopChat()));
    connect(m_udpReceiver,SIGNAL(logoutSignal()),this,SLOT(logout()));
    connect(m_udpReceiver,SIGNAL(calledBusy()),this,SLOT(onCalledBusy()));
    connect(m_udpReceiver,SIGNAL(updateRegisterStatus(int)),this,SLOT(updateRegisterStatus(int)));
    connect(m_udpReceiver,SIGNAL(showMsgInStatusBar(const QString&,int)),
            this,SLOT(showMsgInStatusBar(const QString&,int)));
    connect(m_udpReceiver,SIGNAL(enableMyImageLabel(bool)),this,SLOT(enableMyImageLabel(bool)));
    connect(m_udpReceiver,SIGNAL(enableImageDisplay(bool)),this,SLOT(enableImageDisplay(bool)));
    connect(m_udpReceiver,SIGNAL(addWorkLog(const QString&,bool)),this,SLOT(addWorkLog(const QString&,bool)));

    ui->widget_control1->setFocus(); //设置焦点,方向控制按钮

    if(m_autoRegister)  this->login();

}

MainWindow::~MainWindow()
{
    if(m_udpReceiver != nullptr)
        delete m_udpReceiver;
    if(m_udpSender != nullptr)
        delete m_udpSender;

    delete ui;
}

bool MainWindow::clientModeSelection()
{
    int button = QMessageBox::question(this, tr("客户端模式选择"),
                                   QString(tr("远程端 : Yes\n本地端 : No")),
                                   QMessageBox::Yes | QMessageBox::No);
    if (button == QMessageBox::Yes)
    {
        g_isRemoteTerminal = true;
        mWindowTitle = tr("用户界面: 远程端");
        ui->widget_control2->hide();
    }
    else if(button == QMessageBox::No)
    {
        g_isRemoteTerminal = false;
        mWindowTitle = tr("用户界面: 本地端");
    }
    return true;
}

//开始通话
void MainWindow::startChat(uint16_t id)
{
    ui->pushButton_call->setChecked(true);
    ui->pushButton_call->setText("disconnect");

    m_udpSender = new UdpSender;
    m_udpSender->startSend(id);
    if(g_isRemoteTerminal)
    {
        connect(ui->widget_control1,SIGNAL(dirKeyPressed(int)),
                m_udpSender->getRemoteCtrler(),SLOT(onDirKeyPressed(int)));

        connect(ui->widget_control1,SIGNAL(dirKeyReleased(int)),
                m_udpSender->getRemoteCtrler(),SLOT(onDirKeyReleased(int)));
    }

    m_udpReceiver->startPlayMv();
    ui->lineEdit_calledId->setText(QString::number(id));
}

void MainWindow::stopChat()
{
    ui->pushButton_call->setChecked(false);
    ui->pushButton_call->setText("connect");
    m_udpReceiver->stopPlayMv();
    //务必首先判断m_udpSender是否已经被实例化
    if(m_udpSender != nullptr)
    {
        m_udpSender->stopSend();
        delete m_udpSender;
        m_udpSender = nullptr;
    }
    m_udpReceiver->sendCmd(PkgType_DisConnect);
    ui->label_showImageMain->clear();
}

//用户退出登陆，主动退出，
//与服务器失去连接被动退出
void MainWindow::logout()
{
    this->updateRegisterStatus(RegisterStatus_None);
    this->stopChat();
    m_udpReceiver->logout();
}

void MainWindow::login()
{
    this->updateRegisterStatus(RegisterStatus_Ing);
    m_udpReceiver->registerToServer();
}

void MainWindow::updateRegisterStatus(int status)
{
    g_registerStatus = status;
    if(RegisterStatus_None == g_registerStatus)
        ui->label_registerStatus->setText("login");
    else if(RegisterStatus_Ing == g_registerStatus)
        ui->label_registerStatus->setText("logging in, click cancel");
    else if(RegisterStatus_Ok == g_registerStatus)
        ui->label_registerStatus->setText("logout");
}

void MainWindow::showMsgInStatusBar(const QString& msg,int timeout)
{
    ui->statusBar->showMessage(msg, timeout);
}

void MainWindow::updateStatusBarPemanentMsg()
{
    static QLabel *permanent=new QLabel(this);
    //permanent->setFrameStyle(QFrame::Box|QFrame::Sunken);

    QString str = QString("host id:")+QString::number(g_myId);
    if(g_isRemoteTerminal)
        str = QString("user id:")+QString::number(g_myId);
    permanent->setText(str);

    ui->statusBar->addPermanentWidget(permanent);//显示永久信息
}

//被叫忙
void MainWindow::onCalledBusy()
{
    showMsgInStatusBar("The subscriber you dialed is offline!",3000);
    this->stopChat();
}

//点击叉号时的提示弹窗
void MainWindow::closeEvent(QCloseEvent *event)
{
    int button = QMessageBox::question(this, tr("退出程序"),
                                   QString(tr("确认退出程序?")),
                                   QMessageBox::Yes | QMessageBox::No);
    if (button == QMessageBox::No)
    {
        event->ignore();  //忽略退出信号，程序继续运行
    }
    else if (button == QMessageBox::Yes)
    {
        savePerformance();
        if(g_registerStatus != RegisterStatus_None)
            this->logout();
        QThread::sleep(1); //等待清理线程完毕
        event->accept();  //接受退出信号，程序退出

        std::cout << "exit ok" << std::endl;
    }
}

void MainWindow::onLableRegisterStatusClicked()
{
    if(g_registerStatus == RegisterStatus_None)
        this->login();
    else if(g_registerStatus != RegisterStatus_None)
        this->logout();
}

void MainWindow::savePerformance()
{
    //创建配置文件操作对象
    QSettings *config = new QSettings(m_configFile, QSettings::IniFormat);

    config->setValue("autoLogin", QString::number(m_autoRegister));
    config->setValue("userId", g_myId);
    config->setValue("robotCallId", g_robotCallId);
    config->setValue("robotCotrolId", g_robotControlId);

    config->beginGroup("server");
    config->setValue("ip", g_serverIp.toString());
    config->setValue("port",g_registerPort);

    config->endGroup();
    delete config;
}

//载入参数
void MainWindow::loadPerformance()
{
    QSettings *config = new QSettings(m_configFile, QSettings::IniFormat);
    if(config)
    {
        uint16_t id = config->value(QString("userId")).toUInt();
        if(id != 0 )  g_myId = id; //?参数文件中没有参数怎么办？

        m_autoRegister = config->value("autoLogin").toBool();
        g_robotCallId = config->value("robotCallId").toInt();
        g_robotControlId = config->value("robotCotrolId").toInt();
        if(g_robotControlId == 0) g_robotControlId = 5050;

        QString ip = config->value(QString("server/ip")).toString();
        quint16 port = config->value(QString("server/port")).toUInt();

        if(ip.isEmpty())
        {
            ip = QString("62.234.114.48");
            port = 8617;
        }
        g_registerPort = port;
        g_serverIp.setAddress(ip);
    }
    delete config;
    updateStatusBarPemanentMsg();
}

void MainWindow::on_checkBox_vedio_stateChanged(int arg1)
{
    g_isOpenVedio = bool(arg1);
    if(!m_udpSender) return;
    if(arg1)
    {
        //enableImageDisplay(true); //启动所有视频显示
        m_udpSender->openVedio();
    }
    else
    {
        m_udpSender->closeVedio();
        //enableImageDisplay(false); //关闭所有视频显示
        g_myImageMutex.lock();
        g_myImage = nullptr;
        g_myImageMutex.unlock();
    }
}

void MainWindow::on_checkBox_audio_stateChanged(int arg1)
{
    g_isOpenAudio = bool(arg1);
    if(!m_udpSender) return;

    if(arg1)
        m_udpSender->openAudio();
    else
        m_udpSender->closeAudio();
}

void MainWindow::updateRobotStatus(const QString& qstr)
{

    //ui->label_robotStatus->setText(qstr);
}

//启动、关闭图片显示小窗口
void MainWindow::enableMyImageLabel(bool open)
{
    m_myImageBig = false;
    if(open)
    {
        if(m_imageLabel != nullptr)
            return;
        m_imageLabel = new MyQLabel(ui->label_showImageMain);
        connect(m_imageLabel,SIGNAL(clicked()),this,SLOT(switchImagePosition()));
        m_imageLabel->setGeometry(0,0,128,96);
        m_imageLabel->setOpenMoveEvent(true);
        m_imageLabel->setOpenClickEvent(true);
        m_imageLabel->show();
    }
    else if(m_imageLabel != nullptr)
    {
        delete m_imageLabel;
        m_imageLabel = nullptr;
    }
}

//切换我方图片与对方图片位置
void MainWindow::switchImagePosition()
{
    m_myImageBig = !m_myImageBig;

    m_imageLabel->clear();
    ui->label_showImageMain->clear();
}

void MainWindow::enableImageDisplay(bool status)
{
    //判断m_imageDisplayTimer是否为0，防止重复启动
    if(status && m_imageDisplayTimer==0)
    {
        enableMyImageLabel(true);
        //启动定时器并设置循环时间ms
        m_imageDisplayTimer = startTimer(50);
    }
    else
    {
        killTimer(m_imageDisplayTimer);
        m_imageDisplayTimer = 0;
        QThread::msleep(50); //等待myImageLabel空闲后再失能
        enableMyImageLabel(false);
    }
}

void MainWindow::displayImage()
{
   // qDebug() << "enableImageDisplay" ;
    QMutexLocker locker1(&g_myImageMutex);
    if(g_myImage!= nullptr && !g_myImage->isNull())
    {
        if(m_myImageBig)
            ui->label_showImageMain->setPixmap(
                    QPixmap::fromImage(g_myImage->scaled(ui->label_showImageMain->size())));
        else
            m_imageLabel->setPixmap(QPixmap::fromImage(g_myImage->scaled(m_imageLabel->size())));
    }
   /* else if(g_myImage == nullptr) //本地图片指针为空，表明本地摄像头未启动
    {
        if(m_myImageBig) ui->label_showImageMain->clear();
        else m_imageLabel->clear();
    }*/

    locker1.unlock();

    QMutexLocker locker2(&g_otherImageMutex);
    if(g_otherImage!= nullptr && !g_otherImage->isNull())
    {
        if(!m_myImageBig)
            ui->label_showImageMain->setPixmap(
                    QPixmap::fromImage(g_otherImage->scaled(ui->label_showImageMain->size())));
        else
            m_imageLabel->setPixmap(QPixmap::fromImage(g_otherImage->scaled(m_imageLabel->size())));

       //static int testCnt = 0;
       //ui->statusBar->showMessage(QString::number(testCnt++));
    }
    locker2.unlock();
}

//定时器溢出事件函数
void MainWindow::timerEvent(QTimerEvent *event)
{
    if(event->timerId() == m_imageDisplayTimer)
       displayImage();
}


void MainWindow::updateAvailaleSerial()
{
    ui->comboBox_availaleSerial->clear();
    QStringList list;
    foreach(const QSerialPortInfo& info, QSerialPortInfo::availablePorts())
        list << info.portName();
    qSort(list.begin(),list.end());
    ui->comboBox_availaleSerial->addItems(list);
}

void MainWindow::onBioRadarUpdateData(bioRadarData_t data)
{
    ui->lineEdit_breathRate->setText(QString::number(data.breathRate));
    ui->lineEdit_temperature->setText(QString::number(data.temperature));
    QString bloodPressure = QString::number(data.bloodPressureH)+" / "+QString::number(data.bloodPressureL);
    ui->lineEdit_bloodPressure->setText(bloodPressure);
    ui->lineEdit_heartBeatReat->setText(QString::number(data.heartBeatRate));
}

void MainWindow::addWorkLog(const QString& str, bool vip)
{
    if((!ui->checkBox_showLog->isChecked()) && (!vip))
        return;
    std::chrono::milliseconds timeNow=
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());

    uint64_t time = timeNow.count(); //时间戳
    time_t sec = time/1000;
    int msec = time%1000;

    struct tm *ttime = localtime(&sec);
    char time_buf[26] = "[";
    strftime(time_buf+1,21,"%Y-%m-%d %H:%M:%S.",ttime);
    sprintf(time_buf+21,"%03d] ",msec);
    ui->textBrowser_log->append(QString(time_buf) + str);
}
