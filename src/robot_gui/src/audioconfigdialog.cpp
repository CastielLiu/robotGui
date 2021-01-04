#include "audioconfigdialog.h"
#include "ui_audioconfigdialog.h"

audioconfigdialog::audioconfigdialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::audioconfigdialog)
{
    ui->setupUi(this);
}

audioconfigdialog::~audioconfigdialog()
{
    delete ui;
}
