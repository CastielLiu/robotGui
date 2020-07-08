#ifndef UDPRECEIVER_H
#define UDPRECEIVER_H
#include <QThread>
#include <enums.h>
#include "audiohandler.h"
#include "vediohandler.h"
#include "biologicalradar.h"
#include <ctime>
#include <QTimer>

class UdpReceiver: public QThread
{
    Q_OBJECT
public:
    UdpReceiver();
    ~UdpReceiver() override;

    virtual void run(void) override;
    void startAllmsgHandler();
    void stopAllmsgHandler();
    void registerToServer();
    void confirmRegister(quint16 port);
    void logout();
    void sendInstructions(PkgType type, uint16_t receiverId=0);
    void requestConnect(uint16_t dst_id);

private slots:
    void onReadyRead();

signals:
    void startChatSignal(uint16_t id, bool is_called = true);
    void stopChatSignal(bool is_auto = true);//对方请求断开连接,我方自动断开
    void logoutSignal();
    void calledBusy();
    void connectAcceptted();
    void calledOffline();
    void updateRegisterStatus(int status);
    void showMsgInStatusBar(const QString& msg,int timeout=0);
    void enableMyImageLabel(bool );
    void enableImageDisplay(bool );
    void addWorkLog(const QString& log, bool vip=false);

private:
    void handleVedioMsg(char * const buf);
    void handleAudioMsg(char * const buf);
    void handleBioRadarMsg(char* const buf);
    void registerToServerThread();
    void registerToServerInMainThread();
    void heartBeatThread();
    void timerEvent(QTimerEvent *event) override;
public:
    BiologicalRadar *const bioRadar(){return m_bioRadar;}
private:
    QUdpSocket *m_udpSocket;
    int m_maxBufLength;
    char * m_dataBuf;

    AudioHandler *m_audioPlayer;
    VedioHandler *m_vedioPlayer;
    BiologicalRadar *m_bioRadar;

    QMutex m_heartBeatMutex;
    std::time_t m_serverLastHeartBeatTime = 0;
    int m_registerTimerId;
    bool m_heartBeatThreadRunning = false;
};

#endif // UDPRECEIVER_H
