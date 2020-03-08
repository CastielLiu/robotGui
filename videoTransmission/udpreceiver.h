#ifndef UDPRECEIVER_H
#define UDPRECEIVER_H
#include <QThread>
#include <utils.h>
#include "audiohandler.h"
#include "vediohandler.h"
#include <ctime>

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
    void sendCmd(dataType);
private slots:
    void onReadyRead();

signals:
    void startChatSignal(uint16_t id);
    void stopChatSignal();
    void logoutSignal();
    void calledBusy();
private:
    void handleVedioMsg(char * const buf);
    void handleAudioMsg(char * const buf);
    void registerToServerThread();
    void heartBeatThread();

private:
    QUdpSocket *m_udpSocket;
    int m_maxBufLength;
    char * m_dataBuf;

    AudioHandler *m_audioPlayer;
    VedioHandler *m_vedioPlayer;

    QMutex m_heartBeatMutex;
    std::time_t m_serverLastHeartBeatTime = 0;
};

#endif // UDPRECEIVER_H
