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
    qDebug() << "MainWindow in thread" << QThread::currentThread() ;

    ui->setupUi(this);
    g_ui = ui; //初始化全局ui指针便于在外部调用ui
    this->setWindowTitle("视频聊天工具");
    this->setWindowIcon(QIcon(":/images/icon"));

    ui->label_registerStatus->setOpenClickEvent(true);
    connect(ui->label_registerStatus,SIGNAL(clicked()),this,SLOT(onLableRegisterStatusClicked()));
    ui->label_registerStatus->setAlignment(Qt::AlignHCenter); //设置label文字居中
    connect(ui->action_user_id,SIGNAL(triggered()),this,SLOT(setUserId()));
    connect(ui->action_auto_login,SIGNAL(triggered()),this,SLOT(setAutoLogin()));

    this->loadPerformance();//载入用户参数

    //实例化接收器
    m_udpReceiver = new UdpReceiver;
    connect(m_udpReceiver,SIGNAL(startChatSignal(uint16_t)),this,SLOT(startChat(uint16_t)));
    connect(m_udpReceiver,SIGNAL(stopChatSignal()),this,SLOT(stopChat()));
    connect(m_udpReceiver,SIGNAL(logoutSignal()),this,SLOT(logout()));
    connect(m_udpReceiver,SIGNAL(calledBusy()),this,SLOT(onCalledBusy()));

    if(m_autoRegister)
    {
        m_udpReceiver->registerToServer();
        ui->label_registerStatus->setText("logging in");
    }
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
    ui->pushButton_call->setText("stop");

    m_udpSender = new UdpSender();
    m_udpSender->startSend(id);
    m_udpReceiver->startPlayMv();
}

void MainWindow::stopChat()
{
    ui->pushButton_call->setChecked(false);
    ui->pushButton_call->setText("start");
    m_udpReceiver->stopPlayMv();
    //务必首先判断m_udpSender是否已经被实例化
    if(m_udpSender != nullptr)
    {
        m_udpSender->stopSend();
        delete m_udpSender;
        m_udpSender = nullptr;
    }
    m_udpReceiver->sendCmd(DisConnect);
    ui->label_showImageMain->clear();
}

//用户退出登陆，主动退出，
//与服务器失去连接被动退出
void MainWindow::logout()
{
    g_registerStatus = 0;
    this->stopChat();
    m_udpReceiver->logout();

    ui->label_registerStatus->setText("login");
}

void MainWindow::login()
{
    g_registerStatus =1;
    ui->label_registerStatus->setText("logging in, click cancel");
    //qDebug() << "click event in thread" << QThread::currentThread() ;
    m_udpReceiver->registerToServer();
}


void MainWindow::on_pushButton_call_clicked()
{
    if(ui->pushButton_call->isChecked())
    {
        bool ok;
        uint16_t dstId = ui->lineEdit_dstId->text().toUShort(&ok);

        if(g_registerStatus != 2 )
            ui->statusBar->showMessage("please login firstly!",3000);
        else if(g_systemStatus == SystemOnThePhone)
            ui->statusBar->showMessage("Call in progress, No Call!",3000);
        else if(!ok)
            ui->statusBar->showMessage(QString("input id error, please input again"),3000);
        else if(dstId == g_myId)
            ui->statusBar->showMessage(QString("can not call yourself!"),3000);
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
    this->stopChat();
    ui->label_showImageMain->clear();

    ui->pushButton_call->setChecked(false);
    ui->pushButton_call->setText("start");
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

void MainWindow::setUserId()
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
           setUserId();
       }
       QString msg = QString("Set User Id ") + QString::number(g_myId) + " ok.";
       ui->statusBar->showMessage(msg, 3000);
    }
}

void MainWindow::onLableRegisterStatusClicked()
{
    //0未登陆，1登陆中，2已登录
    if(g_registerStatus == 0)
        this->login();
    else if(g_registerStatus == 1)
        this->logout();
    else if(g_registerStatus == 2)
        this->logout();

}

void MainWindow::setAutoLogin()
{
    int button = QMessageBox::question(this, tr("自动登录设置"),
                                   QString(tr("下次自动登录？")),
                                   QMessageBox::Yes | QMessageBox::No);
    if (button == QMessageBox::No)
        m_autoRegister = true;
    else
        m_autoRegister = false;
}

void MainWindow::savePerformance()
{
    //创建配置文件操作对象
    QSettings *config = new QSettings(m_configFile, QSettings::IniFormat);

    //config->beginGroup("");
    config->setValue("user id", g_myId);
    config->setValue("auto login", QString::number(m_autoRegister));
    //config->endGroup();
    delete config;
}

void MainWindow::loadPerformance()
{
    QSettings *config = new QSettings(m_configFile, QSettings::IniFormat);
    if(config)
    {
        uint16_t id = config->value(QString("user id")).toUInt();
        if(id != 0 )
            g_myId = id;
        m_autoRegister = config->value("auto login").toBool();
    }
    delete config;
}

