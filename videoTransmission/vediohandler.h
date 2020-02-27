#ifndef VEDIOHANDLER_H
#define VEDIOHANDLER_H

#include <QWidget>
#include <QCamera>
#include <QCameraInfo>
#include <QCameraViewfinder>
#include <QCameraImageCapture>
#include <QMutexLocker>
#include <QPainter>
#include <QByteArray>
#include <QBuffer>
#include <QImageReader>
#include <vector>
#include "utils.h"

#include "circlebuffer.h"

class VedioHandler : public QObject
{
    Q_OBJECT
public:
    VedioHandler();
    ~VedioHandler();
    bool init(const std::string &mode);
    void stopCapture();

    void sendImage(QUdpSocket *sockect, uint16_t receiverId);

    void appendData(char* const buf, int len);
    void playVedio();

private:
    bool captureImage();

private slots:
    void onImageCaptured(int id, QImage image);
private:
    QCamera *m_camera;//摄像头
    QCameraViewfinder *m_cameraViewFinder;//摄像头取景器部件
    QCameraImageCapture * m_imageCapture;//截图部件

    QMutex m_imageMutex;
    CircleBuffer<QImage> *m_imageBuffer;
};

#endif // VEDIOHANDLER_H
