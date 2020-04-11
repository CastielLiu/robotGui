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
#include <QKeyEvent>
#include <biologicalradar.h>


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
    void onActionDebugConfig();
    void onActionBiologicalRadar();

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
    void on_comboBox_availaleSerial_activated(const QString &arg1);
    void on_pushButton_radarOpenSerial_clicked(bool checked);

    void onBioRadarUpdateData(bioRadarData_t data);

    void on_pushButton_bioRadarExit_clicked();

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
    BiologicalRadar *m_radar;

    bool m_autoRegister;
    QString m_configFile;

    MyQLabel *m_imageLabel;
    bool m_myImageBig ;
    int m_imageDisplayTimer =0;

};

#endif // MAINWINDOW_H
