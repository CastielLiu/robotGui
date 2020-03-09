#include "vediohandler.h"

//类静态成员函数需要在类的外部分配内存空间
QMutex VedioHandler::m_tempImageMutex;
QImage VedioHandler::m_tempImage;

#define QCAMERA_IMAGE_CAPTURE 1
#define CAMERA_FRAME_GRABER 2
#define CV_IMAGE_GRABER 3

#define WHAT_CAMERE_TOOL CV_IMAGE_GRABER

VedioHandler::VedioHandler():
    m_camera(nullptr),
    m_cameraViewFinder(nullptr),
    m_imageCapture(nullptr),
    m_imageGrabber(nullptr),
    m_myQlabel(nullptr)

{
    m_imageBuffer = new CircleBuffer<QImage>();
}

VedioHandler::~VedioHandler()
{
    this->stopCapture();
    delete m_imageBuffer;
}

void VedioHandler::stopCapture()
{
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
        delete  m_imageCapture;
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
    if(m_myQlabel != nullptr)
    {
        //qDebug() << "m_myQlabel->hide()";
        m_myQlabel->clear();
        m_myQlabel->hide();
        delete  m_myQlabel;
        m_myQlabel = nullptr;
    }
}

bool VedioHandler::init(const std::string& mode)
{
    if(mode == "record")
    {
        m_imageBuffer->setSize(5);

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
        if(!m_cvImageGrabber->openCamera(0))
            return false;

#endif
        return true;
    }
    else if(mode == "play")
    {
        m_imageBuffer->setSize(5);
        m_myQlabel = new MyQLabel(g_ui->label_showImageMain);
        connect(m_myQlabel,SIGNAL(clicked()),this,SLOT(modifyShowMode()));
        m_myQlabel->setGeometry(0,0,128,96);
        m_myQlabel->setOpenMoveEvent(true);
        m_myQlabel->setOpenClickEvent(true);
        m_myQlabel->show();
        return true;
    }
    else
    {
        qDebug() << "VedioHandler init mode: play or record!";
        return false;
    }
}

/***************** 视频获取和发送相关代码 *********************/

void VedioHandler::sendImage(QUdpSocket *sockect, uint16_t receiverId)
{
    QImage image;
#if(WHAT_CAMERE_TOOL == QCAMERA_IMAGE_CAPTURE)
    m_imageCapture->capture("a.jpg"); //获取图片，槽函数onImageCaptured将被调用
    m_imageMutex.lock();
    bool ok = m_imageBuffer->read(image);
    m_imageMutex.unlock();
    if(!ok) return;
#elif(WHAT_CAMERE_TOOL == CAMERA_FRAME_GRABER)
    m_imageMutex.lock();
    bool ok = m_imageBuffer->read(image);
    m_imageMutex.unlock();
    if(!ok) return;
#elif(WHAT_CAMERE_TOOL == CV_IMAGE_GRABER)
    image = m_cvImageGrabber->capture();
    if(image.isNull()) return;
#endif

    //QImage转QByteArray 方式1
    QByteArray imageByteArray;// QByteArray类提供了一个字节数组（字节流）。对使用自定义数据来操作内存区域是很有用的
    QBuffer Buffer(&imageByteArray);// QBuffer(QByteArray * byteArray, QObject * parent = 0)
    Buffer.open(QIODevice::ReadWrite);
    image.save(&Buffer,"JPG");//用于直接将一张图片保存在QByteArray中
    //qDebug() << "send " << imageByteArray ;

    /* //QImage转QByteArray 方式2
    QBuffer buffer;
    buffer.open(QIODevice::ReadWrite);
    image.save(&buffer,"JPG");
    QByteArray imageByteArray;
    imageByteArray.append(buffer.data()); //此处多了一遍数据复制，应该没有方式以效率高，未深入研究
    */

    transPack_t header(Video) ;
    header.length = imageByteArray.size();
    header.senderId = g_myId;
    header.receiverId = receiverId;

    QByteArray sendArray = QByteArray((char*)&header, sizeof(transPack_t)) + imageByteArray;

    sockect->writeDatagram(sendArray,sendArray.size(), g_serverIp, g_msgPort);
    //qDebug() << "send one frame image, size: " << imageByteArray.size();
}

//本地摄像头图片捕获槽函数
//图片捕获后放入发送缓存区，等待发送
//显示本地图片
void VedioHandler::onImageCaptured(int id,QImage image)
{
    Q_UNUSED(id);
    this->writeImageToBuffer(image);
}

void VedioHandler::writeImageToBuffer(QImage& image)
{
    QSize size = image.size();
    image = image.scaled(size.width()/2,size.height()/2);
    m_imageMutex.lock();
    m_imageBuffer->write(image);
    m_imageMutex.unlock();

    m_tempImageMutex.lock();
    m_tempImage = image.copy();
    m_tempImageMutex.unlock();
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

void VedioHandler::appendData(char* const buf, int len)
{
    //qDebug() << "void VedioHandler::appendData(char* const buf, int len):" << len;
    //先将图片字节数据转换为图片QImage，然后添加到缓冲区
    QByteArray imageByteArray(buf,len);
    QBuffer buffer(&imageByteArray);
    buffer.open(QIODevice::ReadOnly);
    QImageReader reader(&buffer,"JPG");

    //qDebug() << "receive " <<imageByteArray ;

    QMutexLocker locker(&m_imageMutex);
    QImage image= reader.read();
    // qDebug() << reader.errorString() << " " << reader.error();
    bool ok = m_imageBuffer->write(image);
    //if(ok)
    //    qDebug() << "received one image and write to imageBuffer ok. size:" << image.size()
    //             << "image len: " << len;
}

void VedioHandler::playVedio()
{
    //从缓冲区读取一张图片并显示
    QImage image;
    QMutexLocker locker(&m_imageMutex);
    bool ok = m_imageBuffer->read(image);
    locker.unlock();
    //image=image.convertToFormat(QImage::Format_RGB888);
    if(m_myImageBig) //我方大图
    {
        m_tempImageMutex.lock();
        g_ui->label_showImageMain->setPixmap(QPixmap::fromImage(m_tempImage));
        m_tempImageMutex.unlock();
        if(ok)
            m_myQlabel->setPixmap(QPixmap::fromImage(image.scaled(m_myQlabel->size())));
    }
    else //我方小图
    {
        m_tempImageMutex.lock();
        if(!m_tempImage.isNull())
            m_myQlabel->setPixmap(QPixmap::fromImage(m_tempImage.scaled(m_myQlabel->size())));
        m_tempImageMutex.unlock();

        if(ok)
            g_ui->label_showImageMain->setPixmap(QPixmap::fromImage(image));
    }

    /* //图片显示方法2
    QImage::Format format =  image.format();
    qDebug() << (int)format << "\t" << image.size();
    // 使用QPainter进行绘制
    QPainter painter;
    painter.begin(g_ui->openGLWidget);
    painter.drawImage(QPoint(0,0),m_image);
    painter.end();
    */
}

void VedioHandler::modifyShowMode()
{
    m_myImageBig = !m_myImageBig;
    //qDebug() << m_myImageBig ;
}
