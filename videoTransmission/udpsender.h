#ifndef UDPSENDER_H
#define UDPSENDER_H
#include <utils.h>
#include "audiohandler.h"
#include "vediohandler.h"

class UdpSender: public QThread
{
    Q_OBJECT
public:
    UdpSender();
    ~UdpSender() override;
    bool startSend(uint16_t dstId);
    void stopSend();
    virtual void run() override;
    void setDstId(uint16_t id) {m_dstId = id;}



private:
    QUdpSocket *m_udpSocket;
    AudioHandler *m_audioRecorder;
    VedioHandler *m_vedioCaptor;

    //通信目标id
    uint16_t m_dstId;
    bool m_sendFlag;



};

#endif // UDPSENDER_H
