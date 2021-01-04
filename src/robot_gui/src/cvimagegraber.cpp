#include "cvimagegraber.h"

CvImageGraber::CvImageGraber()
{

}
CvImageGraber::~CvImageGraber()
{
    if(m_cap.isOpened())
        m_cap.release();
}

bool CvImageGraber::isOpen()
{
    return m_cap.isOpened();
}

bool CvImageGraber::openCamera(int id, int _try)
{
    if(m_cap.isOpened())
        return true;
    while(_try --)
    {
        m_cap.open(id);
        if(m_cap.isOpened())
            return true;
        m_cap.release();
    }
    return false;
}

void CvImageGraber::closeCamera()
{
    if(m_cap.isOpened())
        m_cap.release();
}

QImage CvImageGraber::capture()
{
    std::lock_guard<std::mutex> lck(m_mutex);
    if(m_cap.isOpened())
    {
        if(m_cap.read(m_cvImage))
            return cvMatToQImage(m_cvImage);
    }
    return QImage();
}
QImage CvImageGraber::cvMatToQImage(cv::Mat& mtx)
{
    auto imgType = mtx.type();
    if(CV_8UC1 == imgType)
    {
        QImage img((const unsigned char *)(mtx.data), mtx.cols, mtx.rows, mtx.cols, QImage::Format_Grayscale8);
        return img;
    }
    else if(CV_8UC3 == imgType)
    {
        QImage img((const unsigned char *)(mtx.data), mtx.cols, mtx.rows, mtx.cols * 3, QImage::Format_RGB888);
        return img.rgbSwapped();
    }
    else if(CV_8UC4 == imgType)
    {
        QImage img((const unsigned char *)(mtx.data), mtx.cols, mtx.rows, mtx.cols * 4, QImage::Format_ARGB32);
        return img;
    }
    else
        return QImage();
}

bool CvImageGraber::setResolution(int w, int h)
{
    std::lock_guard<std::mutex> lck(m_mutex);

    if(!m_cap.isOpened())
    {
        std::cerr << "please open the camera before setResolution!" << std::endl;
        return false;
    }
    bool res1 = m_cap.set(cv::CAP_PROP_FRAME_WIDTH, w);
    bool res2 = m_cap.set(cv::CAP_PROP_FRAME_HEIGHT, h);
    bool res = res1&&res2;
    if(!res)
    {
        QSize size = getResolution();
        std::cout << "set camera resolution: " << w << "x" << h << " failed!  "
                  << "use " << size.width() << "x" << size.height() << " instead!"
                  <<std::endl;
    }

    return res;
}

bool CvImageGraber::setResolution(const QSize& size)
{
    return setResolution(size.width(), size.height());
}

QSize CvImageGraber::getResolution()
{
    std::lock_guard<std::mutex> lck(m_mutex);
    int w = m_cap.get(cv::CAP_PROP_FRAME_WIDTH);
    int h = m_cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    return QSize(w,h);
}

bool operator>=(const QSize& size1, const QSize& size2)
{
    return (size1.width()*size1.height())>= (size2.width()*size2.height());
}

//获取摄像头可用的分辨率，从大到小排序
QList<QSize> CvImageGraber::getAvailableResolutions()
{
   QList<QSize> resolutions;
   if(!m_cap.isOpened())
   {
       std::cerr << "please open the camera before getAvailableResolutions!" << std::endl;
       return resolutions;
   }
   //opencv无法直接获取摄像头分辨率，通过设置超大的尺寸使内部自动设置为可用的最大分辨率
   //然后再对最大分辨率进行拆分并尝试设置，最终以获取到的实际的分辨率作为结果
   setResolution(5000,5000);
   QSize maxResolution = getResolution();
   resolutions.append(maxResolution);
   for(int k=2; ; ++k)
   {
        setResolution(maxResolution/k);
        QSize resolution = getResolution();
        if(resolution>=resolutions.last())
            break;
        resolutions.append(resolution);
   }
   return resolutions;
}

