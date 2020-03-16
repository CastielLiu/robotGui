#include "remotecontrol.h"

RemoteControl::RemoteControl()
{
    m_xSpeed = 127;
    m_zSpeed = 127;
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
    QMutexLocker lock(&m_mutex);
    if(key == Qt::Key_Up)
        m_ctrlCmdPkg.cmd.xSpeed = m_xSpeed;
    else if(key == Qt::Key_Down)
        m_ctrlCmdPkg.cmd.xSpeed = -m_xSpeed;
    else if(key == Qt::Key_Left)
        m_ctrlCmdPkg.cmd.zSpeed = m_zSpeed;
    else if(key == Qt::Key_Right)
        m_ctrlCmdPkg.cmd.zSpeed = -m_zSpeed;
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
