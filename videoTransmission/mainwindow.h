#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "enums.h"
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
    void onLableRegisterStatusClicked();

    void startChat(uint16_t id);
    void stopChat();
    void onCalledBusy();
    void logout();
    void login();
    void updateRegisterStatus(int status);
    void showMsgInStatusBar(const QString& msg,int timeout=0);
    void updateRobotStatus(const QString& str);

    //action slots
    void onActionUserId();
    void onActionAutoLogin();
    void onActionRobotCallId();
    void onActionRobotControlId();
    void onActionAbout();

    //checkbox
    void on_checkBox_vedio_stateChanged(int arg1);
    void on_checkBox_audio_stateChanged(int arg1);

    void enableImageDisplay(bool status);
    void enableMyImageLabel(bool status);
    void switchImagePosition();

private:
    void closeEvent(QCloseEvent *event) override;
    void loadPerformance();
    void savePerformance();
    void timerEvent(QTimerEvent *event) override;
    void displayImage();

private:
    Ui::MainWindow *ui;
    QUdpSocket *m_udpSocket;
    UdpReceiver *m_udpReceiver;
    UdpSender *m_udpSender;

    bool m_autoRegister;
    QString m_configFile;

    MyQLabel *m_imageLabel;
    bool m_myImageBig ;
    int m_imageDisplayTimer =0;

};

#endif // MAINWINDOW_H
