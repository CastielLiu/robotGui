#include "vediohandler.h"

//类静态成员函数需要在类的外部分配内存空间
QMutex VedioHandler::m_tempImageMutex;
std::shared_ptr<QImage> VedioHandler::m_tempImage = nullptr;

#define QCAMERA_IMAGE_CAPTURE 1
#define CAMERA_FRAME_GRABER 2
#define CV_IMAGE_GRABER 3

#define WHAT_CAMERE_TOOL CV_IMAGE_GRABER

VedioHandler::VedioHandler():
    m_camera(nullptr),
    m_cameraViewFinder(nullptr),
    m_imageCapture(nullptr),
    m_imageGrabber(nullptr),
    m_cvImageGrabber(nullptr),
    m_myQlabel(nullptr)

{

}

VedioHandler::~VedioHandler()
{
    this->stopCapture();
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
        m_imageBuffer.setSize(5);

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
        {
            qDebug() << "open camera failed!" ;
            return false;
        }
#endif
        return true;
    }
    else if(mode == "play")
    {
        m_imageBuffer.setSize(5);
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

    std::shared_ptr<QImage> imgPtr = std::shared_ptr<QImage>(new QImage(m_cvImageGrabber->capture()));
    if(imgPtr->isNull()) return;

    m_tempImageMutex.lock();
    m_tempImage = imgPtr;
    m_tempImageMutex.unlock();

    imgPtr->save(&Buffer,"JPG");//用于直接将一张图片保存在QByteArray中
#endif

    /* //QImage转QByteArray 方式2
    QBuffer buffer;
    buffer.open(QIODevice::ReadWrite);
    image.save(&buffer,"JPG");
    QByteArray imageByteArray;
    imageByteArray.append(buffer.data()); //此处多了一遍数据复制，应该没有方式以效率高，未深入研究
    */

    pkgHeader_t header(PkgType_Video) ;
    header.length = imageByteArray.size();
    header.senderId = g_myId;
    header.receiverId = receiverId;

    QByteArray sendArray = QByteArray((char*)&header, sizeof(pkgHeader_t)) + imageByteArray;

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

//将本地捕获的图片保存至缓冲区，等待发送以及显示
void VedioHandler::writeImageToBuffer(QImage& image)
{
    QSize size = image.size();
    image = image.scaled(size.width()/2,size.height()/2);
    std::shared_ptr<QImage> imgPtr = std::shared_ptr<QImage>(new QImage(image));

    m_imageMutex.lock();
    m_imageBuffer.write(imgPtr);
    m_imageMutex.unlock();

    m_tempImageMutex.lock();
    m_tempImage = imgPtr;
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
//将接收到的图片保存至缓冲区
void VedioHandler::appendData(char* const buf, int len)
{
    //qDebug() << "void VedioHandler::appendData(char* const buf, int len):" << len;
    //先将图片字节数据转换为图片QImage，然后添加到缓冲区
    QByteArray imageByteArray(buf,len);
    QBuffer buffer(&imageByteArray);
    buffer.open(QIODevice::ReadOnly);
    QImageReader reader(&buffer,"JPG");

    //qDebug() << "receive " <<imageByteArray ;

    //读取图片，通过拷贝构造函数生成智能指针
    std::shared_ptr<QImage> imgPtr = std::shared_ptr<QImage>(new QImage(reader.read()));
    // qDebug() << reader.errorString() << " " << reader.error();


    QMutexLocker locker(&m_imageMutex);

    bool ok = m_imageBuffer.write(imgPtr);
    //if(ok)
    //    qDebug() << "received one image and write to imageBuffer ok. size:" << image.size()
    //             << "image len: " << len;
}

void VedioHandler::playVedio()
{
    //从缓冲区读取一张图片并显示
    std::shared_ptr<QImage> imgPtr;

    QMutexLocker locker(&m_imageMutex);
   // qDebug() << "data cnt in buffer: " << m_imageBuffer.getDataCnt();
    bool ok = m_imageBuffer.read(imgPtr);
    locker.unlock();

    //image=image.convertToFormat(QImage::Format_RGB888);
    if(m_myImageBig) //我方大图
    {
        m_tempImageMutex.lock();
        if(m_tempImage!=nullptr && !m_tempImage->isNull())
            g_ui->label_showImageMain->setPixmap(
                        QPixmap::fromImage(m_tempImage->scaled(g_ui->label_showImageMain->size())));
        m_tempImageMutex.unlock();
        if(ok)
            m_myQlabel->setPixmap(QPixmap::fromImage(imgPtr->scaled(m_myQlabel->size())));
    }
    else //我方小图
    {
        m_tempImageMutex.lock();
        if(m_tempImage!=nullptr && !m_tempImage->isNull())
            m_myQlabel->setPixmap(QPixmap::fromImage(m_tempImage->scaled(m_myQlabel->size())));
        m_tempImageMutex.unlock();

        if(ok)
            g_ui->label_showImageMain->setPixmap(
                        QPixmap::fromImage(imgPtr->scaled(g_ui->label_showImageMain->size())));
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
    if(m_myImageBig)
        m_myQlabel->clear();
    else
        g_ui->label_showImageMain->clear();


    //qDebug() << m_myImageBig ;
}
