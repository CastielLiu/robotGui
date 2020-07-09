#include "vediohandler.h"

#define QCAMERA_IMAGE_CAPTURE 1
#define CAMERA_FRAME_GRABER 2
#define CV_IMAGE_GRABER 3

#if defined(Q_OS_LINUX)
    #define WHAT_CAMERE_TOOL CV_IMAGE_GRABER
#else
    #define WHAT_CAMERE_TOOL CV_IMAGE_GRABER
#endif

VedioHandler::VedioHandler():
    m_camera(nullptr),
    m_cameraViewFinder(nullptr),
    m_imageCapture(nullptr),
    m_imageGrabber(nullptr),
    m_cvImageGrabber(nullptr),
    m_isVedioOpen(false)
{
    m_imageBuffer.reserve(10);
    m_imgCutK = 0.3;
    m_imgScale = 0.5;
    m_imgSize = QSize(256,192);
}

VedioHandler::~VedioHandler()
{
    this->stop(m_vedioMode);
}

void VedioHandler::stopCapture()
{
    QThread::msleep(50);

    if(m_camera != nullptr)
    {
        m_camera->stop();
        delete  m_camera;
        m_camera = nullptr;
    }
    if(m_cameraViewFinder != nullptr)
    {
        delete  m_cameraViewFinder;
        m_cameraViewFinder = nullptr;
    }
    if(m_imageCapture != nullptr)
    {
        delete m_imageCapture;
        m_imageCapture = nullptr;
    }
    if(m_imageGrabber != nullptr)
    {
        delete  m_imageGrabber;
        m_imageGrabber = nullptr;
    }
    if(m_cvImageGrabber != nullptr)
    {
        m_cvImageGrabber->closeCamera();
        delete m_cvImageGrabber;
    }
}

bool VedioHandler::init(VedioMode mode)
{
    if(m_isVedioOpen) return false;
    m_vedioMode = mode;
    if(mode == VedioMode_Capture)
    {
    #if(WHAT_CAMERE_TOOL == QCAMERA_IMAGE_CAPTURE)
        m_camera = new QCamera;//摄像头
        m_camera->setCaptureMode(QCamera::CaptureVideo);
        m_cameraViewFinder = new QCameraViewfinder;
        m_camera->setViewfinder(m_cameraViewFinder);
        m_imageCapture = new QCameraImageCapture(m_camera,this);//截图部件
        m_imageCapture->setCaptureDestination(QCameraImageCapture::CaptureToFile);
        connect(m_imageCapture, SIGNAL(imageCaptured(int,QImage)),
                this, SLOT(onImageCaptured(int,QImage)));
        m_camera->start(); //启动摄像头
    #elif(WHAT_CAMERE_TOOL == CAMERA_FRAME_GRABER)
        m_camera = new QCamera;//摄像头
        m_camera->setCaptureMode(QCamera::CaptureVideo);
        m_imageGrabber = new CameraFrameGrabber();
        m_camera->setViewfinder(m_imageGrabber);
        connect(m_imageGrabber, SIGNAL(imageGrabed(const QVideoFrame &)),
                this, SLOT(onImageGrabed(const QVideoFrame &)));
        m_camera->start(); //启动摄像头
    #elif(WHAT_CAMERE_TOOL == CV_IMAGE_GRABER)
        m_cvImageGrabber = new CvImageGraber();
        if(!m_cvImageGrabber->openCamera(g_cameraId))
        {
            qDebug() << "open camera failed!" ;
            return false;
        }
#endif
    }
    else if(mode == VedioMode_Play)
    {
        //不需要其他初始化工作，直接返回
    }
    else
    {
        qDebug() << "VedioHandler init mode: play or capture!";
        return false;
    }
    m_isVedioOpen =true;
    return true;
}

bool VedioHandler::stop(VedioMode mode)
{
    if(!m_isVedioOpen) return false;

    if(mode == VedioMode_Capture)
    {
        this->stopCapture();
        m_isVedioOpen = false;
    }
    else if(mode == VedioMode_Play)
    {

    }
    return true;
}

/***************** 视频获取和发送相关代码 *********************/

//从本地图片缓存区中读取一张图片然后发送  QCameraImageCapture，CameraFrameGrabber
//或者直接从摄像头读取图片然后发送       CvImageGraber
void VedioHandler::sendImage(QUdpSocket *sockect, uint16_t receiverId)
{
    if(!m_isVedioOpen) return;
    QByteArray imageByteArray;// QByteArray类提供了一个字节数组（字节流）。对使用自定义数据来操作内存区域是很有用的
    QBuffer Buffer(&imageByteArray);// QBuffer(QByteArray * byteArray, QObject * parent = 0)
    Buffer.open(QIODevice::ReadWrite);

#if(WHAT_CAMERE_TOOL == QCAMERA_IMAGE_CAPTURE)
    std::shared_ptr<QImage> imgPtr;
    m_imageCapture->capture("a.jpg"); //获取图片，槽函数onImageCaptured将被调用
    m_imageMutex.lock();
    bool ok = m_imageBuffer.read(imgPtr);
    m_imageMutex.unlock();
    if(!ok) return;
    imgPtr->save(&Buffer,"JPG");
#elif(WHAT_CAMERE_TOOL == CAMERA_FRAME_GRABER)
    std::shared_ptr<QImage> imgPtr;
    m_imageMutex.lock();
    bool ok = m_imageBuffer.read(imgPtr);
    m_imageMutex.unlock();
    if(!ok) return;
    imgPtr->save(&Buffer,"JPG");
#elif(WHAT_CAMERE_TOOL == CV_IMAGE_GRABER)

    //quint64 start = QDateTime::currentMSecsSinceEpoch();
    std::shared_ptr<QImage> imgPtr = std::shared_ptr<QImage>(new QImage(m_cvImageGrabber->capture()));
    if(imgPtr->isNull()) return;

    QSize size = imgPtr->size();
    //std::cout << "rawImage Size: " << size.width() << "x" << size.height() << std::endl;
    //*imgPtr = imgPtr->scaled(int(size.width()*m_imgScale),int(size.height()*m_imgScale));
    static size_t  cut_x = m_imgCutK*size.width()/2,
                   cut_y = m_imgCutK*size.height(),
                   w = size.width()-2*cut_x,
                   h = size.height()-cut_y;

    *imgPtr = imgPtr->copy(cut_x,cut_y,w,h);
    *imgPtr = imgPtr->scaled(m_imgSize);

    g_myImageMutex.lock();
    g_myImage = imgPtr;
    g_myImageMutex.unlock();


    //size = imgPtr->size();
    //std::cout << "rawImage Size: " << size.width() << "x" << size.height() << std::endl;
    if(!imgPtr->save(&Buffer,"JPG"))//将图片保存在QByteArray中
        return;

#endif

    pkgHeader_t header(PkgType_Video) ;
    header.length = imageByteArray.size();
    header.senderId = g_myId;
    header.receiverId = receiverId;

    QByteArray sendArray = QByteArray((char*)&header, sizeof(pkgHeader_t)) + imageByteArray;

    sockect->writeDatagram(sendArray,sendArray.size(), g_serverIp, g_msgPort);

    //static quint64 sum_time = 0;
    //static int cnt = 0;
    //sum_time += QDateTime::currentMSecsSinceEpoch()-start;
    //cnt ++;
    //qDebug() << "fetch image and send it average cost time: " << sum_time/cnt ;
}

//QCameraImageCapture  摄像头图片捕获槽函数
//图片捕获后放入发送缓存区，等待发送
void VedioHandler::onImageCaptured(int id,QImage image)
{
    Q_UNUSED(id);
    this->writeImageToBuffer(image);
}

//将本地捕获的图片保存至全局图像指针
//此函数用于 发送图片捕获信号 与 图片捕获 不在同一位置时的情况
void VedioHandler::writeImageToBuffer(QImage& image)
{
    QSize size = image.size();
    size.setWidth(int(size.width() * m_imgScale));
    size.setHeight(int(size.height() * m_imgScale));

    std::shared_ptr<QImage> imgPtr =
            std::shared_ptr<QImage>(new QImage(image.scaled(size)));

    m_imageMutex.lock();
    m_imageBuffer.write(imgPtr);
    m_imageMutex.unlock();

    g_myImageMutex.lock();
    g_myImage = imgPtr;
    g_myImageMutex.unlock();
}

//cameraFrameGraber每次抓到图片将触发此槽函数
void VedioHandler::onImageGrabed(const QVideoFrame &frame)
{
    qDebug() << "VedioHandler::onImageGrabed size:" <<  frame.size();

    QVideoFrame cloneFrame(frame); //复制视频帧，信号槽传递来的frame不可直接读取
    cloneFrame.map(QAbstractVideoBuffer::ReadOnly); //必须要map，否则无法读取
    QImage::Format imageFormat = QVideoFrame::imageFormatFromPixelFormat(cloneFrame .pixelFormat());
    //QVideoFrame 转QImage
    QImage image(cloneFrame.bits(),
                       cloneFrame.width(),
                       cloneFrame.height(),
                       imageFormat);
    cloneFrame.unmap();

    this->writeImageToBuffer(image);
}

/****************** 视频播放相关代码 ********************/
//将接收到的图片赋值给全局图片指针
void VedioHandler::appendData(char* const buf, int len)
{
    //先将图片字节数据转换为图片QImage，然后添加到缓冲区
    QByteArray imageByteArray = QByteArray::fromRawData(buf,len); //不拷贝
    //QByteArray imageByteArray(buf,len); //拷贝
    QBuffer buffer(&imageByteArray);
    buffer.open(QIODevice::ReadOnly);
    QImageReader reader(&buffer,"JPG");

    //读取图片，通过拷贝构造函数生成智能指针
    std::shared_ptr<QImage> imgPtr = std::shared_ptr<QImage>(new QImage(reader.read()));
    // qDebug() << reader.errorString() << " " << reader.error();

    g_otherImageMutex.lock();
    g_otherImage = imgPtr;
    g_otherImageMutex.unlock();
    //此处可用信号槽提醒主程序播放视频
}
