#include "cvimagegraber.h"

CvImageGraber::CvImageGraber()
{

}
CvImageGraber::~CvImageGraber()
{
    if(m_cap.isOpened())
        m_cap.release();
}

bool CvImageGraber::openCamera(int id)
{
    m_cap.open(id);
    if(m_cap.isOpened())
        return true;
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

