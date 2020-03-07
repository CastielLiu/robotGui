#include "cameraframegrabber.h"

CameraFrameGrabber::CameraFrameGrabber(QObject *parent) :
    QAbstractVideoSurface(parent)
{
}

//支持的像素格式
QList<QVideoFrame::PixelFormat> CameraFrameGrabber::supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const
{
    Q_UNUSED(handleType);
    return QList<QVideoFrame::PixelFormat>()
        << QVideoFrame::Format_ARGB32
        << QVideoFrame::Format_ARGB32_Premultiplied
        << QVideoFrame::Format_RGB32
        << QVideoFrame::Format_RGB24
        << QVideoFrame::Format_RGB565
        << QVideoFrame::Format_RGB555
        << QVideoFrame::Format_ARGB8565_Premultiplied
        << QVideoFrame::Format_BGRA32
        << QVideoFrame::Format_BGRA32_Premultiplied
        << QVideoFrame::Format_BGR32
        << QVideoFrame::Format_BGR24
        << QVideoFrame::Format_BGR565
        << QVideoFrame::Format_BGR555
        << QVideoFrame::Format_BGRA5658_Premultiplied
        << QVideoFrame::Format_AYUV444
        << QVideoFrame::Format_AYUV444_Premultiplied
        << QVideoFrame::Format_YUV444
        << QVideoFrame::Format_YUV420P
        << QVideoFrame::Format_YV12
        << QVideoFrame::Format_UYVY
        << QVideoFrame::Format_YUYV
        << QVideoFrame::Format_NV12
        << QVideoFrame::Format_NV21
        << QVideoFrame::Format_IMC1
        << QVideoFrame::Format_IMC2
        << QVideoFrame::Format_IMC3
        << QVideoFrame::Format_IMC4
        << QVideoFrame::Format_Y8
        << QVideoFrame::Format_Y16
        << QVideoFrame::Format_Jpeg
        << QVideoFrame::Format_CameraRaw
        << QVideoFrame::Format_AdobeDng;
}

//每一帧图片都会经过这里
bool CameraFrameGrabber::present(const QVideoFrame &frame)
{
    if (!frame.isValid()) //判断有效性
        return false;
    emit imageGrabed(frame);

    /*
    QVideoFrame cloneFrame(frame); //复制视频帧，信号槽传递来的frame不可直接读取
    cloneFrame.map(QAbstractVideoBuffer::ReadOnly); //必须要map，否则无法读取
    QImage::Format imageFormat = QVideoFrame::imageFormatFromPixelFormat(cloneFrame .pixelFormat());
    //QVideoFrame 转QImage
    const QImage image(cloneFrame.bits(),
                       cloneFrame.width(),
                       cloneFrame.height(),
                       imageFormat);
    cloneFrame.unmap();
    */
    return true;
}

bool CameraFrameGrabber::isFormatSupported(const QVideoSurfaceFormat & format) const
{
    return QVideoFrame::imageFormatFromPixelFormat(format.pixelFormat()) != QImage::Format_Invalid
            && !format.frameSize().isEmpty()
            && format.handleType() == QAbstractVideoBuffer::NoHandle;
}

bool CameraFrameGrabber::start(const QVideoSurfaceFormat &format)
{
    const QImage::Format imageFormat
            = QVideoFrame::imageFormatFromPixelFormat(format.pixelFormat());
    const QSize size = format.frameSize();
    if (imageFormat != QImage::Format_Invalid && !size.isEmpty())
    {
        this->imageFormat_ = imageFormat;
        QAbstractVideoSurface::start(format);
        return true;
    }
    return false;
}

void CameraFrameGrabber::stop()
{
    QAbstractVideoSurface::stop();
}
