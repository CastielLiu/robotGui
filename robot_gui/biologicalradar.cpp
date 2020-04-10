#include "biologicalradar.h"

BiologicalRadar::BiologicalRadar():
    mSerial(nullptr)
{

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

void BiologicalRadar::onSerialReadyRead()
{
    QByteArray datas = mSerial->readAll();
    qDebug() << datas;
}
