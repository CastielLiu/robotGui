#include "mainwindow.h"
#include <thread>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_udpReceiver(nullptr),
    m_udpSender(nullptr),
    m_configFile("RobotGui"),
    m_configFileType("hm"),
    m_imageLabel(nullptr),
    m_radar(nullptr),
    mNavigation(nullptr),
    mKeyBorad(nullptr)
{
    ui->setupUi(this);

    //客户端模式选择
    if(!this->clientModeSelection())
        exit(0);
    this->loadPerformance();//载入用户参数

    this->setWindowTitle(mWindowTitle);
    this->setWindowIcon(QIcon(":/images/app_icon"));
    this->setFixedSize(this->width(),this->height());

    ui->stackedWidget->setCurrentIndex(stackWidget_HomePage);//切换到主页面
    ui->pushButton_home->hide();//隐藏home键
    m_clockDisplayTimer = this->startTimer(59000); //启动时钟更新定时器
    updateClockDisplay(); //定时器需要１分钟后才会触发，先手动更新一次时钟

    ui->label_registerStatus->setOpenClickEvent(true);
    connect(ui->label_registerStatus,SIGNAL(clicked()),this,SLOT(onLableRegisterStatusClicked()));
    ui->label_registerStatus->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter); //设置label文字居中
    ui->checkBox_audio->setCheckState(Qt::CheckState::Checked);
    ui->lineEdit_calledId->setText(QString::number(g_calledId));
    //ui->checkBox_audio->hide();

    ui->label_log->setAlignment(Qt::AlignHCenter); //居中对齐

    //连接action信号槽
    connect(ui->action_user_id,SIGNAL(triggered()),this,SLOT(onActionUserId()));
    connect(ui->action_auto_login,SIGNAL(triggered()),this,SLOT(onActionAutoLogin()));
    connect(ui->action_robotCall_id,SIGNAL(triggered()),this,SLOT(onActionRobotCallId()));
    connect(ui->action_robotControl_id,SIGNAL(triggered()),this,SLOT(onActionRobotControlId()));
    connect(ui->action_cameraConfig,SIGNAL(triggered()),this,SLOT(onActionCameraConfig()));
    connect(ui->action_audioConfig,SIGNAL(triggered()),this,SLOT(onActionAudioConfig()));
    connect(ui->action_about,SIGNAL(triggered()),this,SLOT(onActionAbout()));
    connect(ui->action_debugConfig,SIGNAL(triggered()),this,SLOT(onActionDebugConfig()));
    connect(ui->action_biologicalRadar,SIGNAL(triggered()),this,SLOT(onActionBiologicalRadar()));
    connect(ui->action_workLog,SIGNAL(triggered()),this,SLOT(onActionWorkLog()));
    connect(ui->action_serverConfig,SIGNAL(triggered()),this,SLOT(onActionServerConfig()));

    //实例化接收器
    m_udpReceiver = new UdpReceiver;
    connect(m_udpReceiver,SIGNAL(startChatSignal(uint16_t,bool)),this,SLOT(startChat(uint16_t,bool)));
    connect(m_udpReceiver,SIGNAL(stopChatSignal(bool)),this,SLOT(stopChat(bool)));
    connect(m_udpReceiver,SIGNAL(logoutSignal()),this,SLOT(logout()));
    connect(m_udpReceiver,SIGNAL(calledBusy()),this,SLOT(onCalledBusy()));
    connect(m_udpReceiver,SIGNAL(connectAcceptted()),this,SLOT(onConnectAcceptted()));

    connect(m_udpReceiver,SIGNAL(calledOffline()),this,SLOT(onCalledOffline()));
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
    {
        delete m_udpReceiver;
        m_udpSender = nullptr;
    }
    if(m_udpSender != nullptr)
    {
        delete m_udpSender;
        m_udpSender = nullptr;
    }
    if(mNavigation != nullptr)
    {
        delete mNavigation;
        mNavigation = nullptr;
    }
    if(mKeyBorad != nullptr)
    {
        delete mKeyBorad;
        mKeyBorad = nullptr;
    }

    delete ui;
}

bool MainWindow::clientModeSelection()
{
    QMessageBox msgBox(QMessageBox::Question, tr("机器人操作界面"), tr("客户端模式选择"),
                       QMessageBox::YesAll|QMessageBox::Yes|QMessageBox::Cancel);
    msgBox.button(QMessageBox::YesAll)->setText(tr("远程端"));
    msgBox.button(QMessageBox::Yes)->setText(tr("本地端"));
    msgBox.button(QMessageBox::Cancel)->setText(tr("退出"));

    msgBox.setDefaultButton(QMessageBox::Yes);

    int button = msgBox.exec();

    if(button == QMessageBox::Cancel)
        return false;
    else if(button == QMessageBox::YesAll)
    {
        g_isRemoteTerminal = true;
        mWindowTitle = tr("用户界面: 远程端");
        ui->widget_control2->hide();
        //ui->widget_control2->setHidden(true);
        //ui->widget_control2->setVisible(false);
        //ui->widget_control2->setEnabled(false); //失能上面所有控件，但不隐藏

    }
    else if(button == QMessageBox::Yes)
    {
        g_isRemoteTerminal = false;
        mWindowTitle = tr("用户界面: 本地端");
    }
    return true;
}

/* @brief    开始信息交互
 * @param id 对方ID
 * @param is_called 是否为被叫
 */
void MainWindow::startChat(uint16_t id, bool is_called)
{
    if(g_transferStatus != transferStatus_Idle)
        return;

    if(!is_called) //主叫
    {
        //此处将状态置为启动中，待连接成功后置为正在传输
        g_transferStatus = transferStatus_Starting;
        //ui->pushButton_call->setChecked(true);
        ui->pushButton_call->setStyleSheet("border-image: url(:/images/calling_icon);");
        //ui->pushButton_call->setText("connecting");
    }
    else
    {
        g_transferStatus = transferStatus_Ing;
        ui->pushButton_call->setChecked(true);
        //ui->pushButton_call->setText("disconnect");
        ui->pushButton_call->setStyleSheet("border-image: url(:/images/hungup_icon);");
    }

    //实例化udp发送器
    m_udpSender = new UdpSender;
    m_udpSender->startSend(id);
    if(g_isRemoteTerminal) //远程端
    {
        //绑定控制面板键盘捕获事件
        connect(ui->widget_control1,SIGNAL(dirKeyPressed(int)),
                m_udpSender->getRemoteCtrler(),SLOT(onDirKeyPressed(int)));

        connect(ui->widget_control1,SIGNAL(dirKeyReleased(int)),
                m_udpSender->getRemoteCtrler(),SLOT(onDirKeyReleased(int)));
    }
    //启动所有消息处理器
    m_udpReceiver->startAllmsgHandler();
    if(g_isRemoteTerminal) //作为远程端时,连接生物雷达数据接收与数据更新信号槽
        connect(m_udpReceiver->bioRadar(),SIGNAL(updateData(bioRadarData_t)),
                this,SLOT(onBioRadarUpdateData(bioRadarData_t)));

    ui->lineEdit_calledId->setText(QString::number(id));
}

/* @brief 停止消息传输
 * @param is_auto 是否为自动断开
 *        自动断开:对方请求断开，我方自动断开
 *        手动断开:我方手动请求断开
 * 此函数可能会被系统重复调用，务必严格条件限定，防止内存重复释放
 * 若此函数被不同线程同步调用，极可能发生内存重复释放
 */
void MainWindow::stopChat(bool is_auto)
{
    //qDebug() << "stopChat :" << (is_auto? "auto" : "manual" );
    if(g_transferStatus == transferStatus_Idle || //空闲，无需再次退出
       g_transferStatus == transferStatus_Stoping)//正在退出，禁止再次退出
         return;

    g_transferStatus = transferStatus_Stoping;

    if(!is_auto) //手动结束会话，发送中断指令通知对方
    {
        m_udpReceiver->sendInstructions(PkgType_DisConnect,g_calledId);
    }
    if(m_udpReceiver) //不删除，需保持监听
        m_udpReceiver->stopAllmsgHandler();

    //务必首先判断m_udpSender是否已经被实例化
    if(m_udpSender != nullptr)
    {
        m_udpSender->stopSend();
        delete m_udpSender;
        m_udpSender = nullptr;
    }

    ui->label_showImageMain->clear();

    ui->pushButton_call->setChecked(false);
    //ui->pushButton_call->setText("connect");
    ui->pushButton_call->setStyleSheet("border-image: url(:/images/call_icon);");
    g_transferStatus = transferStatus_Idle;
}

//用户退出登陆，主动退出，
//与服务器失去连接被动退出
void MainWindow::logout()
{
    this->updateRegisterStatus(RegisterStatus_None); //状态更新
    this->stopChat();         // 中断通话
    m_udpReceiver->logout();  //退出登录
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
    {
        ui->label_registerStatus->setText("login");
        ui->label_registerStatus->setStyleSheet("border-image: url(:/images/login1);");
    }
    else if(RegisterStatus_Ing == g_registerStatus)
    {
        ui->label_registerStatus->setText("logging in, click cancel");
    }
    else if(RegisterStatus_Ok == g_registerStatus)
    {

        ui->label_registerStatus->setText("logout");
        ui->label_registerStatus->setStyleSheet("border-image: url(:/images/login2);");
    }
}

void MainWindow::showMsgInStatusBar(const QString& msg,int timeout)
{
    ui->statusBar->showMessage(msg, timeout);
}

//状态栏永久信息
void MainWindow::updateStatusBarPemanentMsg()
{
    /*
    static QLabel *permanent=new QLabel(this);
    //permanent->setFrameStyle(QFrame::Box|QFrame::Sunken);

    QString str = QString("host id:")+QString::number(g_myId);
    if(g_isRemoteTerminal)
        str = QString("user id:")+QString::number(g_myId);
    permanent->setText(str);

    ui->statusBar->addPermanentWidget(permanent);//显示永久信息
    */
}

//被叫忙
void MainWindow::onCalledBusy()
{
    showMsgInStatusBar("The subscriber you dialed is busy now!",3000);
    this->stopChat();
}

//被叫接受连接请求
void MainWindow::onConnectAcceptted()
{
    g_transferStatus = transferStatus_Ing;
    ui->pushButton_call->setChecked(true);
    //ui->pushButton_call->setText("disconnect");
    ui->pushButton_call->setStyleSheet("border-image: url(:/images/hungup_icon);");
}

//被叫不在线
void MainWindow::onCalledOffline()
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
    QSettings *config = new QSettings(m_configFile, m_configFileType);
    //qDebug() << config->fileName() ;

    config->setValue("autoLogin", QString::number(m_autoRegister));
    config->setValue("userId", g_myId);
    config->setValue("robotCallId", g_calledId);
    config->setValue("robotCotrolId", g_robotControlId);
    config->setValue("cameraId", g_cameraId);

    config->beginGroup("server");
    config->setValue("ip", g_serverIp.toString());
    config->setValue("port",g_registerPort);
    config->endGroup();

    config->beginGroup("camera");
    config->setValue("resolution", g_cameraResolution);
    config->setValue("image_scale",g_sendVideoScale);
    config->setValue("description", g_cameraDescription);
    config->endGroup();

    delete config;
}

//载入参数
void MainWindow::loadPerformance()
{
    QSettings *config = new QSettings(m_configFile, m_configFileType);
    if(config)
    {
        uint16_t id = config->value(QString("userId")).toUInt();
        if(id != 0 )  g_myId = id; //?参数文件中没有参数怎么办？

        m_autoRegister = config->value("autoLogin").toBool();
        g_calledId = config->value("robotCallId").toInt();
        g_robotControlId = config->value("robotCotrolId").toInt();
        if(g_robotControlId == 0) g_robotControlId = 5050;

        g_cameraId = config->value("cameraId").toInt();

        QString ip = config->value(QString("server/ip")).toString();
        quint16 port = config->value(QString("server/port")).toUInt();

        if(ip.isEmpty())
        {
            ip = QString("62.234.114.48");
            port = 8617;
        }
        g_registerPort = port;
        g_serverIp.setAddress(ip);

        g_cameraResolution = config->value("camera/resolution").toSize();
        g_sendVideoScale = config->value("camera/image_scale").toFloat();
        if(g_sendVideoScale == 0) g_sendVideoScale = 1.0;
        g_cameraDescription = config->value("camera/description").toString();
    }
    delete config;
    updateStatusBarPemanentMsg();
    ui->lineEdit_hostId->setText(QString::number(g_myId));
}

void MainWindow::on_checkBox_vedio_stateChanged(int arg1)
{
    g_isOpenVedio = bool(arg1);
    if(!m_udpSender) return;

    if(arg1)
    {
        m_udpSender->openVedio();
    }
    else
    {
        m_udpSender->closeVedio();

        g_myImageMutex.lock();
        g_myImage = nullptr;

        if(m_myImageBig)
            ui->label_showImageMain->clear();
        else
            m_imageLabel->clear();

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
    QMutexLocker my_image_locker(&g_myImageMutex);
    if(g_myImage!= nullptr && !g_myImage->isNull())
    {
        if(m_myImageBig)
            ui->label_showImageMain->setPixmap(
                    QPixmap::fromImage(g_myImage->scaled(ui->label_showImageMain->size())));
        else
            m_imageLabel->setPixmap(QPixmap::fromImage(g_myImage->scaled(m_imageLabel->size())));
        g_myImage = nullptr;
    }
    my_image_locker.unlock();

    QMutexLocker other_image_locker(&g_otherImageMutex);
    if(g_otherImage!= nullptr && !g_otherImage->isNull())
    {
        if(!m_myImageBig)
            ui->label_showImageMain->setPixmap(
                    QPixmap::fromImage(g_otherImage->scaled(ui->label_showImageMain->size())));
        else
            m_imageLabel->setPixmap(QPixmap::fromImage(g_otherImage->scaled(m_imageLabel->size())));
        g_otherImage = nullptr;

    }
    other_image_locker.unlock();
}

void MainWindow::updateClockDisplay()
{
    QTime time = QTime::currentTime();
    QString textTime = time.toString("hh:mm"); //hh:mm:ss
    ui->lcdNumber_clock->display(textTime);
}

//定时器溢出事件函数
void MainWindow::timerEvent(QTimerEvent *event)
{
    if(event->timerId() == m_imageDisplayTimer)
        displayImage();
    else if(event->timerId() == m_clockDisplayTimer)
        updateClockDisplay();

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
    ui->lineEdit_bloodO2->setText(QString::number(data.bloodOxygen));
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
    char time_buf[30] = "[";
    strftime(time_buf+1,21,"%Y-%m-%d %H:%M:%S.",ttime);
    sprintf(time_buf+21,"%03d] ",msec);
    ui->textBrowser_log->append(QString(time_buf) + str);
}

void MainWindow::on_stackedWidget_currentChanged(int arg1)
{
    if(arg1 == stackWidget_HomePage)
        ui->pushButton_home->hide();
    else
        ui->pushButton_home->show();
}

/*@brief 槽函数，接收导航目标点更新信号
 *@param goalsInfo 导航目标点信息
 */
void MainWindow::onUpdateNavGoalsInfo(const std::vector<goalInfo_t>& goalsInfo)
{
    mNavgoalsInfo = goalsInfo;
    ui->comboBox_navGoalsInfo->clear();
    for(const goalInfo_t& goalInfo:goalsInfo)
    {
        //std::cout << goalInfo.name << std::endl;;
        ui->comboBox_navGoalsInfo->addItem(QString::fromStdString(goalInfo.name));
    }
}

