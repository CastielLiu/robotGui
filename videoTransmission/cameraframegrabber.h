#ifndef CAMERAFRAMEGRABBER_H
#define CAMERAFRAMEGRABBER_H

#include <QAbstractVideoSurface>
#include <QVideoSurfaceFormat>
#include <QList>
#include <QCamera>

class CameraFrameGrabber : public QAbstractVideoSurface
{
    Q_OBJECT
public:
    explicit CameraFrameGrabber(QObject *parent = 0);
    QList<QVideoFrame::PixelFormat>
        supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const override;
    bool present(const QVideoFrame &frame) override;

    //以下三函数为非必要函数
    bool isFormatSupported(const QVideoSurfaceFormat &format) const override;
    bool start(const QVideoSurfaceFormat &format) override;
    void stop() override;

signals:
    void imageGrabed(const QVideoFrame &frame);

private:
    QImage::Format imageFormat_;
    QSize imageSize_;
};

#endif
