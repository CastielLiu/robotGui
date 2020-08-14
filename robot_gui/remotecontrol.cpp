#include "remotecontrol.h"

RemoteControl::RemoteControl()
{
    //机器人可用速度范围 -127~127
    //设置机器人速度绝对值，方向由按键决定
    m_xSpeed = 50;
    m_zSpeed = 50;

    //初始化控制指令为0
    m_ctrlCmdPkg.cmd.xSpeed = 0;
    m_ctrlCmdPkg.cmd.zSpeed = 0;
}
RemoteControl::~RemoteControl()
{

}

void RemoteControl::sendControlCmd(QUdpSocket* sockect, uint16_t receiverId)
{
    QMutexLocker lock(&m_mutex);
    m_ctrlCmdPkg.header.receiverId = receiverId;
    sockect->writeDatagram((const char *)&m_ctrlCmdPkg,sizeof(m_ctrlCmdPkg), g_serverIp, g_msgPort);
}

//方向键按下后修改控制指令
void RemoteControl::onDirKeyPressed(int key)
{
    static int last_key = -1;
    QMutexLocker lock(&m_mutex);
    if(key == Qt::Key_Up)
        m_ctrlCmdPkg.cmd.xSpeed = m_xSpeed;
    else if(key == Qt::Key_Down)
        m_ctrlCmdPkg.cmd.xSpeed = -m_xSpeed;
    else if(key == Qt::Key_Left)
        m_ctrlCmdPkg.cmd.zSpeed = m_zSpeed;
    else if(key == Qt::Key_Right)
        m_ctrlCmdPkg.cmd.zSpeed = -m_zSpeed;
    if(last_key != key)
    {
        qDebug() << "key " << key << " pressed";
        last_key = key;
    }
}

//方向键松开后控制指令置零
void RemoteControl::onDirKeyReleased(int key)
{
    QMutexLocker lock(&m_mutex);
    if(key == Qt::Key_Up)
        m_ctrlCmdPkg.cmd.xSpeed = 0;
    else if(key == Qt::Key_Down)
        m_ctrlCmdPkg.cmd.xSpeed = 0;
    else if(key == Qt::Key_Left)
        m_ctrlCmdPkg.cmd.zSpeed = 0;
    else if(key == Qt::Key_Right)
        m_ctrlCmdPkg.cmd.zSpeed = 0;
}
