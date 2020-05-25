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
#include "globalvariable.h"
#include <myqlabel.h>
#include <memory>
#include "cameraframegrabber.h"
#include "cvimagegraber.h"
#include "circlebuffer.h"
#include <chrono>

class VedioHandler : public QObject
{
    Q_OBJECT
public:
    enum VedioMode
    {
        VedioMode_Capture,
        VedioMode_Play,
    };
    VedioHandler();
    ~VedioHandler();
    bool init(VedioMode mode);
    bool stop(VedioMode mode);

    void sendImage(QUdpSocket *sockect, uint16_t receiverId);
    void appendData(char* const buf, int len);

private:
    void writeImageToBuffer(QImage& image);
    void stopCapture();
private slots:
    void onImageCaptured(int id, QImage image);
    void onImageGrabed(const QVideoFrame &frame);
private:
    QCamera *m_camera;//摄像头
    QCameraViewfinder *m_cameraViewFinder;//摄像头取景器部件
    QCameraImageCapture * m_imageCapture;//截图部件
    CameraFrameGrabber *m_imageGrabber;
    CvImageGraber *m_cvImageGrabber;

    bool m_isVedioOpen;
    VedioMode m_vedioMode;
    float m_imgScale; //图像缩放比例
    float m_imgCutK;  //图像裁剪比例
    QSize m_imgSize;

    //m_imageBuffer
    //作为发送器存放要发送的图片
    //作为接收器存放接收到的图片
    QMutex m_imageMutex;
    CircleBuffer<std::shared_ptr<QImage> > m_imageBuffer;
    std::shared_ptr<QImage> m_receivedImage = nullptr;

};

#endif // VEDIOHANDLER_H
