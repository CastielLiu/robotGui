#include "vediohandler.h"

//类静态成员函数需要在类的外部分配内存空间
QMutex VedioHandler::m_tempImageMutex;
QImage VedioHandler::m_tempImage;

VedioHandler::VedioHandler():
    m_camera(nullptr),
    m_cameraViewFinder(nullptr),
    m_imageCapture(nullptr),
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
    if(m_myQlabel != nullptr)
    {
        //qDebug() << "m_myQlabel->hide()";
        m_myQlabel->hide();
        delete  m_myQlabel;
        m_myQlabel = nullptr;
    }
}

bool VedioHandler::init(const std::string& mode)
{
    if(mode == "record")
    {
        m_camera = new QCamera;//摄像头
        m_cameraViewFinder = new QCameraViewfinder;
        m_camera->setViewfinder(m_cameraViewFinder);
        m_camera->setCaptureMode(QCamera::CaptureVideo);
        m_camera->start(); //启动摄像头

        m_imageCapture = new QCameraImageCapture(m_camera,this);//截图部件
        m_imageCapture->setCaptureDestination(QCameraImageCapture::CaptureToFile);

        m_imageBuffer->setSize(5);

        connect(m_imageCapture, SIGNAL(imageCaptured(int,QImage)),
                this, SLOT(onImageCaptured(int,QImage)));
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
        //m_myQlabel->setText("testsfdd");
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
bool VedioHandler::captureImage()
{
    m_imageCapture->capture();
    return true;
}

void VedioHandler::sendImage(QUdpSocket *sockect, uint16_t receiverId)
{
    m_imageCapture->capture(); //获取图片，槽函数onImageCaptured将被调用

    QImage image;
    m_imageMutex.lock();
    bool ok = m_imageBuffer->read(image);
    m_imageMutex.unlock();

    if(!ok)
        return;
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

    transPack_t header ;
    header.head[0] = 0x66;
    header.head[1] = 0xcc;
    header.type = Vedio;
    header.length = imageByteArray.size();
    header.checkNum = 0;
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
    QSize size = image.size();
    image = image.scaled(size.width()/2,size.height()/2);
    m_imageMutex.lock();
    m_imageBuffer->write(image);
    m_imageMutex.unlock();

    m_tempImageMutex.lock();
    m_tempImage = image.copy();
    m_tempImageMutex.unlock();

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
