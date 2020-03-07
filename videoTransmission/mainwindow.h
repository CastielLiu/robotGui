#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "utils.h"
#include "udpreceiver.h"
#include "udpsender.h"
#include <QCloseEvent>
#include <QMessageBox>
#include <myqlabel.h>
#include <QInputDialog>
#include <QSettings>


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
    void on_pushButton_call_clicked();
    void setUserId();
    void setAutoLogin();
    void onLableRegisterStatusClicked();

    void startSendSlot();

private:
    void closeEvent(QCloseEvent *event);
    void loadPerformance();
    void savePerformance();

private:
    Ui::MainWindow *ui;
    QUdpSocket *m_udpSocket;
    UdpReceiver *m_udpReceiver;
    UdpSender *m_udpSender;

    bool m_autoRegister;
    QString m_configFile;

};

#endif // MAINWINDOW_H
