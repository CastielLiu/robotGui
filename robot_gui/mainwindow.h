#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"
#include <QMainWindow>
#include "enums.h"
#include "udpreceiver.h"
#include "udpsender.h"
#include <QCloseEvent>
#include <QMessageBox>
#include <myqlabel.h>
#include <QInputDialog>
#include <QSettings>
#include <QKeyEvent>
#include <biologicalradar.h>
#include <utils.h>
#include <chrono>
#include <QDateTime>
#include "cameraconfigdialog.h"
#include "keyboard.h"
#include "navigation.h"
#include "fifo.hpp"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;
private slots:
    void on_pushButton_call_clicked();
    void onLableRegisterStatusClicked();

    void startChat(uint16_t id, bool is_called = false);
    void stopChat(bool is_auto=false);
    void onCalledBusy();
    void onCalledOffline();
    void onConnectAcceptted();
    void logout();
    void login();
    void updateRegisterStatus(int status);
    void showMsgInStatusBar(const QString& msg,int timeout=0);
    void updateRobotStatus(const QString& str);
    void addWorkLog(const QString& str, bool vip=false);

    //action slots
    void onActionUserId();
    void onActionAutoLogin();
    void onActionRobotCallId();
    void onActionRobotControlId();
    void onActionCameraConfig();
    void onActionAudioConfig();
    void onActionAbout();
    void onActionDebugConfig();
    void onActionBiologicalRadar();
    void onActionWorkLog();
    void onActionServerConfig();

    //checkbox
    void on_checkBox_vedio_stateChanged(int arg1);
    void on_checkBox_audio_stateChanged(int arg1);

    void enableImageDisplay(bool status);
    void enableMyImageLabel(bool status);
    void switchImagePosition();

    void on_pushButton_debugPageOk_clicked();
    void on_pushButton_Up_pressed();
    void on_pushButton_Up_released();
    void on_pushButton_Down_pressed();
    void on_pushButton_Down_released();
    void on_pushButton_Left_pressed();
    void on_pushButton_Left_released();
    void on_pushButton_Right_pressed();
    void on_pushButton_Right_released();
    void on_pushButton_radarOpenSerial_clicked(bool checked);
    void onBioRadarUpdateData(bioRadarData_t data);
    void on_pushButton_bioRadarExit_clicked();
    void on_pushButton_roscore_clicked();
    void on_pushButton_remoteCtrl_clicked();
    void on_pushButton_logpageReturn_clicked();
    void on_pushButton_clearWorkLog_clicked();
    void on_pushButton_audioTest_clicked(bool checked);
    void on_pushButton_playLocalAudio_clicked(bool checked);
    void on_pushButton_home_clicked();
    void on_pushButton_toChatPage_clicked();
    void on_pushButton_toTransportPage_clicked();
    void on_stackedWidget_currentChanged(int arg1);
    void on_pushButton_goToNavGoal_clicked();


    void onUpdateNavGoalsInfo(const std::vector<goalInfo_t>& goalsInfo);
    void onKeyboardUpdate(const QString& qstr);

private:
    void closeEvent(QCloseEvent *event) override;
    void loadPerformance();
    void savePerformance();
    void timerEvent(QTimerEvent *event) override;
    void displayImage();
    void updateAvailaleSerial();
    void updateStatusBarPemanentMsg();
    bool clientModeSelection();
    void updateClockDisplay();

private:
    Ui::MainWindow *ui;
    QUdpSocket *m_udpSocket;
    UdpReceiver *m_udpReceiver;
    UdpSender *m_udpSender;

    bool m_autoRegister;
    QString m_configFile;
    QString m_configFileType;

    MyQLabel *m_imageLabel;
    BiologicalRadar *m_radar;
    Navigation *mNavigation;
    std::vector<goalInfo_t> mNavgoalsInfo;

    KeyBoard *mKeyBorad;

    bool m_myImageBig ;
    int m_imageDisplayTimer =0;
    int m_clockDisplayTimer = 0;
    QString mWindowTitle;
};

#endif // MAINWINDOW_H
