#ifndef BIOLOGICALRADAR_H
#define BIOLOGICALRADAR_H
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QObject>
#include <QList>
#include <qdebug.h>
#include <QMetaType>
#include <QUdpSocket>
#include <QtEndian>
#include <enums.h>
#include <structs.h>
#include <globalvariable.h>
#include <QMutex>

#pragma pack(push,1)
typedef struct bioRadarData
{
    uint8_t heartBeatRate;
    uint8_t breathRate;
    float temperature;
    uint8_t bloodPressureH;
    uint8_t bloodPressureL;
    uint8_t bloodOxygen;
}bioRadarData_t;

typedef struct bioRadarDataPkg
{
    bioRadarDataPkg()
    {
        header.type = PkgType_BoilogicalRadar;
        header.length = sizeof (bioRadarData_t);
    }
    pkgHeader_t header;
    bioRadarData_t data;
}bioRadarDataPkg_t;
#pragma pack(pop)

class BiologicalRadar : public QObject
{
Q_OBJECT
  public:
    BiologicalRadar();
    virtual ~BiologicalRadar();
    bool openSerial(const QString& serialPort);
    void closeSerial();
    void run();
    void sendData(QUdpSocket* sockect, uint16_t receiverId);

  private:

  signals:
    void updateData(bioRadarData_t data);

  private slots:
    void onSerialReadyRead();

  private:
    QSerialPort *mSerial;

    //生物雷达在MainWindow中被实例化并接收串口数据，
    //然而数据远程发送需要在udpSender中完成，
    //故定义静态成员变量以在不同实例中传递数据
    static QMutex mDataMutex;
    static bioRadarData_t mRadarData;
    static bool mHasNewData; //是否有新数据读入
};





















#endif // BIOLOGICALRADAR_H
