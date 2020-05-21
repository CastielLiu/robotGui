#ifndef UDPRECEIVER_H
#define UDPRECEIVER_H
#include <QThread>
#include <enums.h>
#include "audiohandler.h"
#include "vediohandler.h"
#include <ctime>
#include <QTimer>

class UdpReceiver: public QThread
{
    Q_OBJECT
public:
    UdpReceiver();
    ~UdpReceiver() override;

    virtual void run(void) override;
    void startPlayMv();
    void stopPlayMv();
    void registerToServer();
    void confirmRegister(quint16 port);
    void logout();
    void sendCmd(PkgType );
private slots:
    void onReadyRead();

signals:
    void startChatSignal(uint16_t id);
    void stopChatSignal();
    void logoutSignal();
    void calledBusy();
    void updateRegisterStatus(int status);
    void showMsgInStatusBar(const QString& msg,int timeout=0);
    void enableMyImageLabel(bool );
    void enableImageDisplay(bool );
    void addWorkLog(const QString& log, bool vip=false);

private:
    void handleVedioMsg(char * const buf);
    void handleAudioMsg(char * const buf);
    void registerToServerThread();
    void registerToServerInMainThread();
    void heartBeatThread();
    void timerEvent(QTimerEvent *event) override;

private:
    QUdpSocket *m_udpSocket;
    int m_maxBufLength;
    char * m_dataBuf;

    AudioHandler *m_audioPlayer;
    VedioHandler *m_vedioPlayer;

    QMutex m_heartBeatMutex;
    std::time_t m_serverLastHeartBeatTime = 0;
    int m_registerTimerId;
    bool m_heartBeatThreadRunning = false;
};

#endif // UDPRECEIVER_H
