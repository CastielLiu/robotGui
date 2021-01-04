#include "cameraconfigdialog.h"



CameraConfigDialog::CameraConfigDialog(QWidget *parent):
    QDialog(parent),
    ui(new Ui::CameraConfigDialog),
    mCvImageGraber(nullptr),
    mImageDisplaying(false),
    mImageScale(1.0)
{
    ui->setupUi(this);
    setWindowTitle("Camera Configure");
    //setWindowModality(Qt::NonModal); //非模态对话框
    //setWindowModality(Qt::WindowModal); //窗口级模态对话框
    //setWindowModality(Qt::ApplicationModal); //应用程序级模态对话框
    //对话框关闭时自动删除对话框对象,用于不需要读取返回值的对话框
    //setAttribute(Qt::WA_DeleteOnClose);
    this->setFixedSize(500,389);
    flushAvailableDivice();
    updateCurrentImageScale();
}

void CameraConfigDialog::updateCurrentImageScale()
{
    for(int i=0; i<ui->comboBox_imageScale->count(); ++i)
    {
        //qDebug() <<ui->comboBox_imageScale->itemText(i) <<"\t" << g_sendVideoScale;
        if(ui->comboBox_imageScale->itemText(i).toFloat()==g_sendVideoScale)
        {
            ui->comboBox_imageScale->setCurrentIndex(i);
            mImageScale = g_sendVideoScale;
        }
    }
}

void CameraConfigDialog::flushAvailableDivice()
{
    ui->comboBox_cameraInfo->clear();

    mCameraInfos = QCameraInfo::availableCameras();

    int camera_index = 0;
    for(int i=0; i<mCameraInfos.size();++i )
    {
        const QCameraInfo &cameraInfo = mCameraInfos[i];
        ui->comboBox_cameraInfo->addItem(cameraInfo.description());
        if(cameraInfo.description() == g_cameraDescription)
            camera_index = i;
    }

    ui->comboBox_cameraInfo->addItem("flush");
    ui->comboBox_cameraInfo->setCurrentIndex(camera_index);

    on_comboBox_cameraInfo_activated(camera_index);

}

bool CameraConfigDialog::flushAvailableResolutionByOpencv(int index)
{
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
}

bool CameraConfigDialog::flushAvailableResolutionByQCamera(int index)
{
    QCamera camera(mCameraInfos[index]);
    camera.start(); //启动摄像头才可获取相关参数
    mCameraResolutions = camera.supportedViewfinderResolutions();
    camera.stop();  //参数获取后关闭摄像头
    if(mCameraResolutions.size())
        return true;
    return false;
}

bool CameraConfigDialog::flushAvailableResolution(int index)
{
    ui->comboBox_camera_reslotion->clear();
    bool flush_ok = flushAvailableResolutionByQCamera(index); //刷新可用分辨率
    if(!flush_ok)
        flush_ok = flushAvailableResolutionByOpencv(index);

    for(int i=0; i<mCameraResolutions.size(); ++i)
    {
        auto resolution = mCameraResolutions[i];
        QString qstr = QString::number(resolution.width()) +
                       QString("x") +
                       QString::number(resolution.height());

        ui->comboBox_camera_reslotion->addItem(qstr);
        if(resolution == g_cameraResolution)
        {
            ui->comboBox_camera_reslotion->setCurrentIndex(i);
            mCameraResolution = resolution;
        }
    }
    if(mCameraResolutions.size()==0)
    {
        mCameraResolution = QSize(320,240); //default
    }

    return flush_ok;
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
    g_sendVideoScale = mImageScale;
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

    flushAvailableResolution(index); //刷新可用分辨率

    mCvImageGraber = new CvImageGraber;
    if(!mCvImageGraber->openCamera(index))
    {
        delete  mCvImageGraber;
        mCvImageGraber = nullptr;
        return;
    }

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
        QSize imageSize = image.size();

        if(mImageScale != 1.0)
            image = image.scaled(imageSize*mImageScale);

        imageSize = image.size();

        QRect windowSize = ui->label_showImage->geometry();
        float windowRatio = windowSize.width()/windowSize.height();

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

void CameraConfigDialog::on_comboBox_imageScale_activated(const QString &arg1)
{
    mImageScale = arg1.toFloat();
}
