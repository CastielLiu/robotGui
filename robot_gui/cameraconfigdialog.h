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


namespace Ui{
class CameraConfigDialog;
}

class CameraConfigDialog: public QDialog
{
Q_OBJECT
public:
    explicit CameraConfigDialog(QWidget *parent = 0);
    ~CameraConfigDialog();

    void applyConfig(bool ok);

private slots:


private:
    Ui::CameraConfigDialog *ui;
};


#endif // CAMERA_CONFIG_DIALOG_H_
