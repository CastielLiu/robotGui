#ifndef CAMERA_CONFIG_DIALOG_H_
#define CAMERA_CONFIG_DIALOG_H_
#include <QDialog>
#include <QImage>
#include <QSize>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLayout>
#include <QDebug>
#include <QThread>
#include <ui_cameraconfigdialog.h>
#include <globalvariable.h>
#include <QCamera>
#include <QCameraViewfinderSettings>
#include <QCameraInfo>
#include <cvimagegraber.h>
#include <QString>

namespace Ui{
class CameraConfigDialog;
}

class CameraConfigDialog: public QDialog
{
Q_OBJECT
public:
    explicit CameraConfigDialog(QWidget *parent = 0);
    ~CameraConfigDialog();

    bool applyConfig(bool ok);

private:
    void flushAvailableDivice();
    bool flushAvailableResolutionByQCamera(int index);
    bool flushAvailableResolutionByOpencv(int index);
    void displayImageThread();
    void closeCamera();

private slots:
    void on_comboBox_cameraInfo_activated(int index);
    void on_comboBox_camera_reslotion_activated(int index);

    void on_comboBox_imageScale_currentIndexChanged(const QString &arg1);

private:
    Ui::CameraConfigDialog *ui;
    QList<QCameraInfo> mCameraInfos;
    QList<QSize> mCameraResolutions;
    QSize mCameraResolution;

    CvImageGraber* mCvImageGraber;
    bool mImageDisplaying;
    float mImageScale;
};


#endif // CAMERA_CONFIG_DIALOG_H_
