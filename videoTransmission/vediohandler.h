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
    VedioHandler();
    ~VedioHandler();
    bool init(const std::string &mode);

    void sendImage(QUdpSocket *sockect, uint16_t receiverId);
    bool startVedioTransmission();
    bool stopVedioTransmission();

    void appendData(char* const buf, int len);
    void playVedio();

private:
    void writeImageToBuffer(QImage& image);
    void stopCapture();
private slots:
    void onImageCaptured(int id, QImage image);
    void modifyShowMode();
    void onImageGrabed(const QVideoFrame &frame);
private:
    QCamera *m_camera;//摄像头
    QCameraViewfinder *m_cameraViewFinder;//摄像头取景器部件
    QCameraImageCapture * m_imageCapture;//截图部件
    CameraFrameGrabber *m_imageGrabber;
    CvImageGraber *m_cvImageGrabber;

    bool m_isVedioOpen;
    float m_imgScale;

    //m_imageBuffer
    //作为发送器存放要发送的图片
    //作为接收器存放接收到的图片
    QMutex m_imageMutex;
    CircleBuffer<std::shared_ptr<QImage> > m_imageBuffer;
    std::shared_ptr<QImage> m_receivedImage = nullptr;

    //类静态成员函数需要在类的外部分配内存空间
    //m_tempImage 存放本地捕获的图片，用于显示
    //由于图片是在record模式下捕获，在play模式下播放，所以一定要使用静态成员。
    static QMutex m_tempImageMutex;
    static std::shared_ptr<QImage> m_tempImage;

    //QMutex m_myImageMutex, m_heImageMutex;
    //CircleBuffer<std::shared_ptr<QImage>> m_myImageBuffer;//我方图片缓冲区
    //CircleBuffer<std::shared_ptr<QImage>> m_heImageBuffer;//对方图片缓冲区

    MyQLabel *m_myQlabel; //可捕获鼠标事件的label用于显示图片，可拖动，点击
    bool m_myImageBig = false; //我方图片为大图
};

#endif // VEDIOHANDLER_H
