#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "utils.h"
#include "udpreceiver.h"
#include "udpsender.h"
#include <QCloseEvent>
#include <QMessageBox>
#include <myqlabel.h>


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    void on_pushButton_start_clicked();
    void on_pushButton_setMyId_clicked();

private:
    void closeEvent(QCloseEvent *event);
    void requestConnect(uint16_t id);
    bool registerToServer();
private:
    Ui::MainWindow *ui;

    QUdpSocket *m_udpSocket;
    UdpReceiver *m_udpReceiver;
    UdpSender *m_udpSender;

};

#endif // MAINWINDOW_H
