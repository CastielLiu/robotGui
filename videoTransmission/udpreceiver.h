#ifndef UDPRECEIVER_H
#define UDPRECEIVER_H
#include <QThread>
#include <utils.h>
#include "audiohandler.h"
#include "vediohandler.h"



class UdpReceiver: public QThread
{
    Q_OBJECT
public:
    UdpReceiver();
    ~UdpReceiver() override;

    virtual void run(void) override;
    void startReceive();
    void stopReceive();
    void registerToServer();
    void logout();
private slots:
    void onReadyRead();

signals:
    void startSendSignal();

private:
    void handleRequestConnect(uint16_t id, bool flag);
    void handleVedioMsg(char * const buf);
    void handleAudioMsg(char * const buf);
    void registerToServerThread();

private:
    QUdpSocket *m_udpSocket;
    int m_maxBufLength;
    char * m_dataBuf;

    AudioHandler *m_audioPlayer;
    VedioHandler *m_vedioPlayer;

};

#endif // UDPRECEIVER_H
