#ifndef BIOLOGICALRADAR_H
#define BIOLOGICALRADAR_H
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QObject>
#include <QList>
#include <qdebug.h>
#include <QMetaType>
#include <QtEndian>

typedef struct bioRadarData
{
    uint8_t heartBeatRate;
    uint8_t breathRate;
    float temperature;
    uint8_t bloodPressureH;
    uint8_t bloodPressureL;
    uint8_t bloodOxygen;
}bioRadarData_t;


class BiologicalRadar : public QObject
{
Q_OBJECT
  public:
    BiologicalRadar();
    virtual ~BiologicalRadar();
    bool openSerial(const QString& serialPort);
    void closeSerial();
    void run();

  signals:
    void updateData(bioRadarData_t data);

  private slots:
    void onSerialReadyRead();


  private:
    QSerialPort *mSerial;
    bioRadarData_t mRadarData;
};





















#endif // BIOLOGICALRADAR_H
