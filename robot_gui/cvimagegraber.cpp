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
    int w = m_cap.get(cv::CAP_PROP_FRAME_WIDTH);
    int h = m_cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    return QSize(w,h);
}

QList<QSize> CvImageGraber::getAvailableResolutions()
{
   QList<QSize> resolutions;
   if(!m_cap.isOpened())
   {
       std::cerr << "please open the camera before getAvailableResolutions!" << std::endl;
       return resolutions;
   }
   setResolution(5000,5000);
   QSize maxResolution = getResolution();
   for(int i=5; i>0; --i)
   {
       resolutions.append(maxResolution/i);
   }
   return resolutions;
}

