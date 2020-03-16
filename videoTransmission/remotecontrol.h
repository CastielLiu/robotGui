#ifndef REMOTECONTROL_H
#define REMOTECONTROL_H
#include <QUdpSocket>
#include <utils.h>
#include <QByteArray>
#include <QObject>
#include <QMutex>
#include <QMutexLocker>


#pragma pack(push,1)

typedef struct ControlCmd
{
    int8_t xSpeed; //前进后退的速度
    int8_t zSpeed; //左右旋转的速度
}controlCmd_t;

typedef struct  ControlCmdPkg
{
    pkgHeader_t header;
    controlCmd_t cmd;

    ControlCmdPkg(bool isSender=false)
    {
        if(isSender)
        {
            header.type =  PkgType_ControlCmd;
            header.length = sizeof(ControlCmdPkg) - sizeof(pkgHeader_t);
            header.senderId = g_myId;
        }
    }

}controlCmdPkg_t;
#pragma pack(pop)


class RemoteControl :public QObject
{
    Q_OBJECT
public:
    RemoteControl();
    ~RemoteControl();
    void sendControlCmd(QUdpSocket* sockect, uint16_t receiverId);

public slots:
    void onDirKeyPressed(int key);
    void onDirKeyReleased(int key);

private:
     controlCmdPkg_t m_ctrlCmdPkg; //控制指令

    int8_t xSpeed;
    int8_t zSpeed;

    QMutex m_mutex;
};

#endif // REMOTECONTROL_H


