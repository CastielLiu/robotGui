#include "biologicalradar.h"

//静态变量定义
bool BiologicalRadar::mHasNewData = false;
QMutex BiologicalRadar::mDataMutex;
bioRadarData_t BiologicalRadar::mRadarData;

BiologicalRadar::BiologicalRadar():
    mSerial(nullptr)
{
    qRegisterMetaType<bioRadarData_t>("bioRadarData"); //注册自定义消息
}
BiologicalRadar::~BiologicalRadar()
{
    closeSerial();
}

bool BiologicalRadar::openSerial(const QString& serialPort)
{
    mSerial = new QSerialPort();
    mSerial->setPortName(serialPort);
    if(!mSerial->open(QIODevice::ReadWrite))
    {
        delete mSerial;
        mSerial = nullptr;
        return false;
    }

    mSerial->setBaudRate(QSerialPort::Baud115200);
    mSerial->setParity(QSerialPort::NoParity);
    mSerial->setDataBits(QSerialPort::Data8);
    mSerial->setStopBits(QSerialPort::OneStop);

    connect(mSerial, SIGNAL(readyRead()), this, SLOT(onSerialReadyRead()));
    return true;
}

void BiologicalRadar::closeSerial()
{
    if(mSerial == nullptr)
        return;
    if(mSerial->isOpen())
    {
        mSerial->close();
        delete mSerial;
        mSerial = nullptr;
    }
}

void BiologicalRadar::run()
{

}

uint8_t sumCheck(const uint8_t* buf,int len)
{
    uint8_t sum = 0;
    for(int i=0; i<len; ++i)
        sum += buf[i];
    return sum;
}

void BiologicalRadar::onSerialReadyRead()
{
    QByteArray datas = mSerial->readAll();
    if(datas.length()<3) return;

    const uint8_t *buf = (const uint8_t*)(datas.data());

    if(buf[0]!=0x66 || buf[1]!=0xCC)
    {
        qDebug() << "header error" ;
        return;
    }

    int pkgLen = buf[2];
    if(datas.length() < 3+pkgLen+1) //接收到的数据包不完整
        return;

    if(sumCheck(buf+2,pkgLen+1) != buf[3+pkgLen])
    {
        qDebug() << "datas check failed";
        return;
    }

    int pkgID = buf[3];
    if(pkgID == 0x01)
    {
        mDataMutex.lock();
        mRadarData.heartBeatRate = buf[4];
        mRadarData.breathRate = buf[5];
        mRadarData.temperature = (buf[6]*256 + buf[7])*0.1;
        mRadarData.bloodPressureH = buf[8];
        mRadarData.bloodPressureL = buf[9];
        mRadarData.bloodOxygen = buf[10];
        mHasNewData = true;
        mDataMutex.unlock();

        emit updateData(mRadarData);
    }
}

/*
 * sockect   @sockect指针
 * receiverId @消息接受者id
 */
void BiologicalRadar::sendData(QUdpSocket* sockect, uint16_t receiverId)
{
    if(!mHasNewData)
        return;
    mHasNewData = false;

    bioRadarDataPkg_t pkg;
    pkg.header.senderId = g_myId;
    pkg.header.receiverId = receiverId;
    pkg.data = mRadarData;

    sockect->writeDatagram((char *)&pkg,sizeof(bioRadarDataPkg_t),g_serverIp,g_msgPort);
}
