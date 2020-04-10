#ifndef BIOLOGICALRADAR_H
#define BIOLOGICALRADAR_H
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QObject>
#include <QList>
#include <qdebug.h>
class BiologicalRadar : public QObject
{
Q_OBJECT
  public:
    BiologicalRadar();
    virtual ~BiologicalRadar();
    bool openSerial(const QString& serialPort);
    void closeSerial();
    void run();

  private slots:
    void onSerialReadyRead();


  private:
    QSerialPort *mSerial;
};



















#endif // BIOLOGICALRADAR_H
