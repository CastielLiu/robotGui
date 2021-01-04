#ifndef CVIMAGEGRABER_H
#define CVIMAGEGRABER_H
#include <QImage>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <mutex>
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
    bool openCamera(int id, int _try=1);
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
    std::mutex m_mutex;
};

#endif // CVIMAGEGRABER_H
