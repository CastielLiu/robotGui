#ifndef CVIMAGEGRABER_H
#define CVIMAGEGRABER_H
#include <QImage>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

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
    bool openCamera(int id);
    void closeCamera();
    QImage capture();

private:
    QImage cvMatToQImage(cv::Mat &mtx);
private:
    cv::VideoCapture m_cap;
    cv::Mat m_cvImage;
};

#endif // CVIMAGEGRABER_H
