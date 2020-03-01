#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <thread>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_udpReceiver(nullptr),
    m_udpSender(nullptr)
{
    ui->setupUi(this);
    g_ui = ui; //初始化全局ui指针便于在外部调用ui
    this->setWindowTitle("视频聊天工具");
    this->setWindowIcon(QIcon(":/images/icon"));
    qDebug() << "MainWindow in thread" << QThread::currentThread() ;
    ui->label_registerStatus->setOpenClickEvent(true);
    connect(ui->label_registerStatus,SIGNAL(clicked()),this,SLOT(onLableRegisterStatusClicked()));
    ui->label_registerStatus->setAlignment(Qt::AlignHCenter);

    //实例化接收器
    m_udpReceiver = new UdpReceiver;
    connect(m_udpReceiver,SIGNAL(startSendSignal()),this,SLOT(startSendSlot()));
    m_udpReceiver->registerToServer();
    ui->label_registerStatus->setText("logging in");
}

MainWindow::~MainWindow()
{
    if(m_udpReceiver != nullptr)
        delete m_udpReceiver;
    if(m_udpSender != nullptr)
        delete m_udpSender;

    delete ui;
}

//开始发送数据 槽函数，信号来自于udpReceiver
//客户端被叫，udpReceiver保存主叫id后发出信号
void MainWindow::startSendSlot()
{
    m_udpSender = new UdpSender();
    m_udpSender->startSend(g_otherId);
}

void MainWindow::on_pushButton_call_clicked()
{
    if(ui->pushButton_call->isChecked())
    {
        if(g_registerStatus != 2 )
        {
            ui->statusBar->showMessage("please login firstly!",3000);
            return;
        }

        g_isCaller = true;

        m_udpSender = new UdpSender();
        uint16_t dstId = ui->lineEdit_dstId->text().toUShort();
        bool ok = m_udpSender->startSend(dstId);
        if(!ok)
            return;

        ui->pushButton_call->setChecked(true);
        ui->pushButton_call->setText("stop");
    }
    else
    {
        m_udpSender->stopSend();
        m_udpReceiver->stopReceive();
        ui->label_showImageMain->clear();

        ui->pushButton_call->setChecked(false);
        ui->pushButton_call->setText("start");
        g_isCaller = false;
    }
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
        std::cout << "exit " << std::endl;
        event->accept();  //接受退出信号，程序退出
        QThread::sleep(1); //等待清理线程完毕
        std::cout << "exit ok" << std::endl;
    }
}

void MainWindow::on_pushButton_setMyId_clicked()
{
    if(ui->lineEdit_myId->text().isEmpty())
    {
        ui->statusBar->showMessage("empty!!!",3000);
    }
    else
    {
        g_myId = ui->lineEdit_myId->text().toUInt();
        ui->statusBar->showMessage("my id set ok.",3000);
    }
}

void MainWindow::onLableRegisterStatusClicked()
{
    //0未登陆，1登陆中，2已登录
    if(g_registerStatus == 0)
    {
        g_registerStatus =1;
        ui->label_registerStatus->setText("logging in, click cancel");
        m_udpReceiver->registerToServer();
    }
    else if(g_registerStatus == 1)
    {
        g_registerStatus = 0;
        ui->label_registerStatus->setText("login");
    }
    else if(g_registerStatus == 2)
    {
        g_registerStatus = 0;
        m_udpReceiver->logout();
        ui->label_registerStatus->setText("login");
    }
}

