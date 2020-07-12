#ifndef CVIMAGEGRABER_H
#define CVIMAGEGRABER_H
#include <QImage>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <QList>

/*
cvImageGraber imageGraber;
if(!imageGraber.openCamera(0))
    return;
QImage img = imageGraber.capture();
*/

class CvImageGraber
{
public:
    CvImageGraber();
    ~CvImageGraber();
    bool isOpen();
    bool openCamera(int id);
    void closeCamera();
    QImage capture();
    bool setResolution(int w, int h);
    bool setResolution(const QSize& size);
    QSize getResolution();
    QList<QSize> getAvailableResolutions();

private:
    QImage cvMatToQImage(cv::Mat &mtx);
private:
    cv::VideoCapture m_cap;
    cv::Mat m_cvImage;
};

#endif // CVIMAGEGRABER_H
