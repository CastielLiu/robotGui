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

    //std::thread t = std::thread(&MainWindow::requestConnect,this,dstIp,dstPort);
    //t.detach();
    //requestConnect(dstId);
}

MainWindow::~MainWindow()
{
    if(m_udpReceiver != nullptr)
        delete m_udpReceiver;
    if(m_udpSender != nullptr)
        delete m_udpSender;

    delete ui;
}


void MainWindow::requestConnect(uint16_t id)
{
    QUdpSocket *udpSocket = new QUdpSocket(this);

    transPack_t package;
    package.head[0] = 0x66;
    package.head[1] = 0xcc;
    package.type = RequestConnect;
    package.length = 0;
    package.checkNum = 0;
    package.senderId = g_myId;
    package.receiverId = id;

    size_t dataLength = sizeof(package)+package.length;

    g_systemStatus = SystemBusy;
    int cnt = 200;
    while(g_systemStatus == SystemBusy)
    {
        //发送请求数据
        qint64 len =
        udpSocket->writeDatagram((char *)&package,dataLength, g_serverIp,g_serverPort);

        qDebug() <<g_serverIp.toString() << "\t"
                 << g_serverPort <<"\t len:"<<  len << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        if(cnt-- == 0)
        {
            g_systemStatus = SystemIdle;
            break;
        }
    }

    if(g_systemStatus == SystemRefused) //请求被拒绝
        g_systemStatus = SystemIdle;
    else if(g_systemStatus == SystemAccepted) //请求被接受
    {
        g_systemStatus = SystemRunning;
        m_udpSender->startSend(id);
    }
    else if(g_systemStatus == SystemIdle)
    {
        qDebug() << "request overtime ";
    }
}

void MainWindow::on_pushButton_start_clicked()
{
    if(ui->pushButton_start->isChecked())
    {
        //实例化接收器
        m_udpReceiver = new UdpReceiver;
        m_udpReceiver->startReceive();

        //再实例化发送器
        m_udpSender = new UdpSender();
        uint16_t dstId = ui->lineEdit_dstId->text().toUShort();
        bool ok = m_udpSender->startSend(dstId);
        if(!ok)
            return;

        ui->pushButton_start->setChecked(true);
        ui->pushButton_start->setText("stop");
    }
    else
    {
        m_udpSender->stopSend();
        m_udpReceiver->stopReceive();

        ui->pushButton_start->setChecked(false);
        ui->pushButton_start->setText("start");
    }
}


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
