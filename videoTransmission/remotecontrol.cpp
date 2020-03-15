#include "remotecontrol.h"

RemoteControl::RemoteControl()
{
    xSpeed = 127;
    zSpeed = 127;
}
RemoteControl::~RemoteControl()
{

}

void RemoteControl::sendControlCmd(QUdpSocket* sockect, uint16_t receiverId)
{
    QMutexLocker lock(&m_mutex);
    m_ctrlCmd.header.receiverId = receiverId;
    sockect->writeDatagram((const char *)&m_ctrlCmd,sizeof(m_ctrlCmd), g_serverIp, g_msgPort);
}

//方向键按下后修改控制指令
void RemoteControl::onDirKeyPressed(int key)
{
    qDebug() << "onDirKeyPressed";
    QMutexLocker lock(&m_mutex);
    if(key == Qt::Key_Up)
        m_ctrlCmd.xSpeed = xSpeed;
    else if(key == Qt::Key_Down)
        m_ctrlCmd.xSpeed = -xSpeed;
    else if(key == Qt::Key_Left)
        m_ctrlCmd.zSpeed = zSpeed;
    else if(key == Qt::Key_Right)
        m_ctrlCmd.zSpeed = -zSpeed;
}

//方向键松开后控制指令置零
void RemoteControl::onDirKeyReleased(int key)
{
    qDebug() << "onDirKeyReleased";
    QMutexLocker lock(&m_mutex);
    if(key == Qt::Key_Up)
        m_ctrlCmd.xSpeed = 0;
    else if(key == Qt::Key_Down)
        m_ctrlCmd.xSpeed = 0;
    else if(key == Qt::Key_Left)
        m_ctrlCmd.zSpeed = 0;
    else if(key == Qt::Key_Right)
        m_ctrlCmd.zSpeed = 0;
}
