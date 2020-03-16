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

    ControlCmdPkg()
    {
        header.type =  PkgType_ControlCmd;
        header.length = sizeof(controlCmd_t);
        header.senderId = g_myId;
    }

}controlCmdPkg_t;

#pragma pack(pop)

/*机器人运动控制，用x轴的平移和z轴的旋转表示
 * -127 < xSpeed < 127 接收方根据最大速度进行线性转换
 * -127 < zSpeed < 127
 *
 *
 *
 */

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

    //x/z轴运动速度绝对值，方向由按键决定
    int8_t m_xSpeed;
    int8_t m_zSpeed;
    QMutex m_mutex;
};

#endif // REMOTECONTROL_H


