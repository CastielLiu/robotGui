#include "cameraconfigdialog.h"


CameraConfigDialog::CameraConfigDialog(QWidget *parent):
    QDialog(parent),
    ui(new Ui::CameraConfigDialog)
{
    ui->setupUi(this);
    setWindowTitle("Camera Configure");
    //setWindowModality(Qt::NonModal); //非模态对话框
    //setWindowModality(Qt::WindowModal); //窗口级模态对话框
    setWindowModality(Qt::ApplicationModal); //应用程序级模态对话框
    //对话框关闭时自动删除对话框对象,用于不需要读取返回值的对话框
    //setAttribute(Qt::WA_DeleteOnClose);
}
CameraConfigDialog::~CameraConfigDialog()
{

}

void CameraConfigDialog::applyConfig(bool ok)
{
    if(!ok)
        return;

    int cameraId = ui->comboBox_cameraid->currentText().toInt();
    qDebug() << "camera id: " << cameraId;
    g_cameraId = cameraId;
    //QSize mImageSize = ;
}
