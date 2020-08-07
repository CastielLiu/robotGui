#include "cameraconfigdialog.h"



CameraConfigDialog::CameraConfigDialog(QWidget *parent):
    QDialog(parent),
    ui(new Ui::CameraConfigDialog),
    mCvImageGraber(nullptr),
    mImageDisplaying(false)
{
    ui->setupUi(this);
    setWindowTitle("Camera Configure");
    //setWindowModality(Qt::NonModal); //非模态对话框
    //setWindowModality(Qt::WindowModal); //窗口级模态对话框
    setWindowModality(Qt::ApplicationModal); //应用程序级模态对话框
    //对话框关闭时自动删除对话框对象,用于不需要读取返回值的对话框
    //setAttribute(Qt::WA_DeleteOnClose);
    this->setFixedSize(500,389);
    flushAvailableDivice();
}

void CameraConfigDialog::flushAvailableDivice()
{
    ui->comboBox_cameraInfo->clear();

    mCameraInfos = QCameraInfo::availableCameras();

    foreach (const QCameraInfo &cameraInfo, mCameraInfos)
    {
        ui->comboBox_cameraInfo->addItem(cameraInfo.description());
    }
    ui->comboBox_cameraInfo->addItem("flush");
    ui->comboBox_cameraInfo->setCurrentIndex(0);

    if(mCameraInfos.size())
        on_comboBox_cameraInfo_activated(0); //默认启动0号相机
}

bool CameraConfigDialog::flushAvailableResolutionByOpencv(int index)
{
    ui->comboBox_camera_reslotion->clear();
    bool need_delete = false;

    if(mCvImageGraber == nullptr)
    {
        mCvImageGraber = new CvImageGraber();
        need_delete = true;
    }
    if(!mCvImageGraber->openCamera(index))
    {
        delete mCvImageGraber;
        mCvImageGraber = nullptr;
        return false;
    }
    mCameraResolutions = mCvImageGraber->getAvailableResolutions();

    if(need_delete)
    {
        delete mCvImageGraber;
        mCvImageGraber = nullptr;
    }

    for(const QSize& resolution:mCameraResolutions)
    {
        QString qstr = QString::number(resolution.width()) +
                       QString("x") +
                       QString::number(resolution.height());

        ui->comboBox_camera_reslotion->addItem(qstr);
    }
    if(mCameraResolutions.size())
    {
        mCameraResolution = mCameraResolutions[0];
        return true;
    }
    else
    {
        mCameraResolution = QSize(320,240);
        qDebug() << "no availbal camera resolutions! use defalt: 320*240";
        return false;
    }
}

bool CameraConfigDialog::flushAvailableResolutionByQCamera(int index)
{
    ui->comboBox_camera_reslotion->clear();

    QCamera camera(mCameraInfos[index]);
    camera.start(); //启动摄像头才可获取相关参数
    mCameraResolutions = camera.supportedViewfinderResolutions();
    camera.stop();  //参数获取后关闭摄像头

    for(const QSize& resolution:mCameraResolutions)
    {
        QString qstr = QString::number(resolution.width()) +
                       QString("x") +
                       QString::number(resolution.height());

        ui->comboBox_camera_reslotion->addItem(qstr);
    }

    if(mCameraResolutions.size())
    {
        mCameraResolution = mCameraResolutions[0];
        return true;
    }
    else
    {
        mCameraResolution = QSize(320,240);
        qDebug() << "no availbal camera resolutions! use defalt: 320*240";
        return false;
    }

}

CameraConfigDialog::~CameraConfigDialog()
{
    this->closeCamera();
}

void CameraConfigDialog::closeCamera()
{
    if(mCvImageGraber!=nullptr)
    {
        mCvImageGraber->closeCamera();
        while(mImageDisplaying)
            QThread::msleep(10); //等待播放线程结束
        delete  mCvImageGraber;
        mCvImageGraber = nullptr;
    }
}

bool CameraConfigDialog::applyConfig(bool ok)
{
    this->closeCamera();

    if(!ok)
        return false;

    int cameraIndex = ui->comboBox_cameraInfo->currentIndex();
    int cameraResolutionIndex = ui->comboBox_camera_reslotion->currentIndex();
    if(cameraIndex >= mCameraInfos.size() ||
       cameraResolutionIndex >= mCameraResolutions.size())
        return false;
    g_cameraId = cameraIndex;
    g_cameraResolution = mCameraResolutions[cameraResolutionIndex];
    return true;
}

void CameraConfigDialog::on_comboBox_cameraInfo_activated(int index)
{
    this->closeCamera();

    //索引超出可用摄像头，此时刷新列表
    if(index >= mCameraInfos.size())
    {
        flushAvailableDivice();
        return;
    }
    bool flush_ok = flushAvailableResolutionByQCamera(index); //刷新可用分辨率
    mCvImageGraber = new CvImageGraber;
    if(!mCvImageGraber->openCamera(index))
    {
        delete  mCvImageGraber;
        mCvImageGraber = nullptr;
        return;
    }
    if(!flush_ok)
        flushAvailableResolutionByOpencv(index);
    //配置分辨率
    mCvImageGraber->setResolution(mCameraResolution.width(),mCameraResolution.height());
    std::thread t(&CameraConfigDialog::displayImageThread,this);
    t.detach();
}

void CameraConfigDialog::displayImageThread()
{
    mImageDisplaying = true;
    while(mCvImageGraber->isOpen())
    {
        QImage image = mCvImageGraber->capture();
        QRect windowSize = ui->label_showImage->geometry();
        float windowRatio = windowSize.width()/windowSize.height();
        QSize imageSize = image.size();
        float imageRatio = imageSize.width()/imageSize.height();

        //qDebug() << windowSize.width() << "\t" << windowSize.height() ;
        //qDebug() << imageSize.width() << "\t" << imageSize.height() ;

        if(image.isNull())
        {
            QThread::msleep(50);
            continue;
        }

        if(windowRatio < imageRatio) //以window的高对图像进行缩放
        {
            int w = 1.0*imageSize.width()/imageSize.height()*windowSize.height();
            int h = windowSize.height();
            image = image.scaled(w, h);
        }
        else //以window的宽对图像进行缩放
        {
            int w = windowSize.width();
            int h = 1.0*imageSize.height()/imageSize.width()*windowSize.width();
            image = image.scaled(w, h);
        }

        ui->label_showImage->setPixmap(QPixmap::fromImage(image));
    }
    mImageDisplaying = false;
}

void CameraConfigDialog::on_comboBox_camera_reslotion_activated(int index)
{
    mCameraResolution = mCameraResolutions[index];
    if(!mCvImageGraber || !mCvImageGraber->isOpen())
        return;
    mCvImageGraber->setResolution(mCameraResolution.width(),mCameraResolution.height());
}
