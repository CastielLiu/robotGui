#ifndef UDPSENDER_H
#define UDPSENDER_H
#include <enums.h>
#include "audiohandler.h"
#include "vediohandler.h"
#include "remotecontrol.h"
#include <QObject>

class UdpSender: public QThread
{
    Q_OBJECT
public:
    UdpSender();
    ~UdpSender() override;
    bool startSend(uint16_t dstId);
    void stopSend();
    virtual void run() override;

    void openVedio();
    void closeVedio();

    void openAudio();
    void closeAudio();

private:
    QUdpSocket *m_udpSocket;
    AudioHandler *m_audioRecorder;
    VedioHandler *m_vedioCaptor;
    RemoteControl *m_remoteControler;

    //通话目标id
    uint16_t m_dstId;
};

#endif // UDPSENDER_H
