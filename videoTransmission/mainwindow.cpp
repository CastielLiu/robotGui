#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <thread>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_udpReceiver(nullptr),
    m_udpSender(nullptr),
    m_configFile("config")
{
    std::cout << "create MainWindow in thread: " << QThread::currentThreadId() << std::endl;

    ui->setupUi(this);
    g_ui = ui; //初始化全局ui指针便于在外部调用ui
    this->setWindowTitle("机器人远程工具");
    this->setWindowIcon(QIcon(":/images/icon"));
    this->setFixedSize(this->width(),this->height());

    ui->label_registerStatus->setOpenClickEvent(true);
    connect(ui->label_registerStatus,SIGNAL(clicked()),this,SLOT(onLableRegisterStatusClicked()));
    ui->label_registerStatus->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter); //设置label文字居中
    ui->checkBox_audio->setCheckState(Qt::CheckState::Checked);

    //连接action信号槽
    connect(ui->action_user_id,SIGNAL(triggered()),this,SLOT(onActionUserId()));
    connect(ui->action_auto_login,SIGNAL(triggered()),this,SLOT(onActionAutoLogin()));
    connect(ui->action_robotCall_id,SIGNAL(triggered()),this,SLOT(onActionRobotCallId()));
    connect(ui->action_robotControl_id,SIGNAL(triggered()),this,SLOT(onActionRobotControlId()));
    connect(ui->action_about,SIGNAL(triggered()),this,SLOT(onActionAbout()));

    this->loadPerformance();//载入用户参数

    //实例化接收器
    m_udpReceiver = new UdpReceiver;
    connect(m_udpReceiver,SIGNAL(startChatSignal(uint16_t)),this,SLOT(startChat(uint16_t)));
    connect(m_udpReceiver,SIGNAL(stopChatSignal()),this,SLOT(stopChat()));
    connect(m_udpReceiver,SIGNAL(logoutSignal()),this,SLOT(logout()));
    connect(m_udpReceiver,SIGNAL(calledBusy()),this,SLOT(onCalledBusy()));
    connect(m_udpReceiver,SIGNAL(updateRegisterStatus(int)),this,SLOT(updateRegisterStatus(int)));
    connect(m_udpReceiver,SIGNAL(showMsgInStatusBar(const QString&,int)),
            this,SLOT(showMsgInStatusBar(const QString&,int)));

    if(m_autoRegister)
        this->login();

    //debug
}

MainWindow::~MainWindow()
{
    if(m_udpReceiver != nullptr)
        delete m_udpReceiver;
    if(m_udpSender != nullptr)
        delete m_udpSender;

    delete ui;
}

//开始通话
void MainWindow::startChat(uint16_t id)
{
    ui->pushButton_call->setChecked(true);
    ui->pushButton_call->setText("disconnect");

    m_udpSender = new UdpSender;
    m_udpSender->startSend(id);
    m_udpReceiver->startPlayMv();
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

void MainWindow::on_pushButton_call_clicked()
{
    if(ui->pushButton_call->isChecked())
    {
        uint16_t dstId = g_robotCallId;

        if(g_registerStatus != RegisterStatus_Ok )
            ui->statusBar->showMessage("Please login firstly!",5000);
        else if(g_robotCallId == 0 )
            ui->statusBar->showMessage("Please set robot communicate id",5000);
        else if(g_robotControlId == 0 )
            ui->statusBar->showMessage("Please set robot control id",5000);
        else if(g_systemStatus == SystemOnThePhone)
            ui->statusBar->showMessage("Call in progress!",5000);
        else if(dstId == g_myId)
            ui->statusBar->showMessage(QString("Can not call yourself!"),5000);
        else
        {
            this->startChat(dstId);
            return;
        }
        ui->pushButton_call->setChecked(false);
    }
    else
        this->stopChat();
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
        event->accept();  //接受退出信号，程序退出
        QThread::sleep(1); //等待清理线程完毕
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

    //config->beginGroup("");
    config->setValue("userId", g_myId);
    config->setValue("autoLogin", QString::number(m_autoRegister));
    config->setValue("robotCallId", g_robotCallId);
    config->setValue("robotCotrolId", g_robotControlId);
    //config->endGroup();
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

        QString ip = config->value(QString("server/ip")).toString();
        quint16 port = config->value(QString("server/port")).toUInt();
        g_registerPort = port;
        g_serverIp.setAddress(ip);
    }
    delete config;
}

void MainWindow::onActionUserId()
{
    QInputDialog dia(this);
    dia.setWindowTitle("Set User Id");
    dia.setLabelText("Please input id: ");
    dia.setInputMode(QInputDialog::TextInput);//可选参数：DoubleInput  TextInput
    dia.setTextValue(QString::number(g_myId));
    if(dia.exec() == QInputDialog::Accepted)
    {
       g_myId = dia.textValue().toUInt();
       if(g_myId == 0)
       {
           ui->statusBar->showMessage("error, please input again",1000);
           onActionUserId(); //输入错误后再次调用本函数
       }
       QString msg = QString("Set User Id ") + QString::number(g_myId) + " ok.";
       ui->statusBar->showMessage(msg, 3000);
    }
}

void MainWindow::onActionAutoLogin()
{
    int button = QMessageBox::question(this, tr("自动登录设置"),
                                   QString(tr("下次自动登录？")),
                                   QMessageBox::Yes | QMessageBox::No);
    if (button == QMessageBox::Yes)
        m_autoRegister = true;
    else
        m_autoRegister = false;
}

void MainWindow::onActionRobotCallId()
{
    QInputDialog dialog(this);
    dialog.setWindowTitle(tr("设置机器人通话ID")); //窗口名称
    dialog.setLabelText("请输入: "); //输入提示
    dialog.setInputMode(QInputDialog::TextInput);//可选参数：DoubleInput  TextInput IntInput
    dialog.setTextValue(QString::number(g_robotCallId));

    if(dialog.exec() == QInputDialog::Accepted)
    {
        bool ok;
        uint16_t id = dialog.textValue().toUInt(&ok);
        if(ok)
            g_robotCallId = id;
        else
            onActionRobotCallId();
    }
}

void MainWindow::onActionRobotControlId()
{
    QInputDialog dialog(this);
    dialog.setWindowTitle(tr("设置机器人控制ID")); //窗口名称
    dialog.setLabelText("请输入: "); //输入提示
    dialog.setInputMode(QInputDialog::TextInput);//可选参数：DoubleInput  TextInput IntInput
    dialog.setTextValue(QString::number(g_robotControlId));

    if(dialog.exec() == QInputDialog::Accepted)
    {
        bool ok;
        uint16_t id = dialog.textValue().toUInt(&ok);
        if(ok)
            g_robotControlId = id;
        else
            onActionRobotControlId();
    }
}

void MainWindow::onActionAbout()
{
    QMessageBox::about(this,tr("关于"),tr("castiel_liu@outlook.com"));
}

void MainWindow::on_checkBox_vedio_stateChanged(int arg1)
{
    if(!m_udpSender) return;

    if(arg1)
        m_udpSender->openVedio();
    else
        m_udpSender->closeVedio();
}

void MainWindow::on_checkBox_audio_stateChanged(int arg1)
{
    if(!m_udpSender) return;

    if(arg1)
        m_udpSender->openAudio();
    else
        m_udpSender->closeAudio();
}

void MainWindow::updateRobotStatus(const QString& qstr)
{
    ui->label_robotStatus->setText(qstr);
}
