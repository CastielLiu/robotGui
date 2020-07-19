#include "keyboard.h"
#include "ui_keyboard.h"
#include <QDebug>

KeyBoard::KeyBoard(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::KeyBoard)
{
    qDebug() << " keyboard init.";
    ui->setupUi(this);
    ui->lineEdit_show->setAlignment(Qt::AlignRight); //居右
    ui->gridLayout->setContentsMargins(0,0,0,0);
}
KeyBoard::~KeyBoard()
{
    delete ui;
}
void KeyBoard::update()
{
    ui->lineEdit_show->setText(qstr);
    if(!qstr.isEmpty())
        emit update_signal(qstr);
}
void KeyBoard::on_pushButton_0_clicked()
{
    qstr += '0';
    update();
}
void KeyBoard::on_pushButton_1_clicked()
{
    qstr += '1';
    update();
}
void KeyBoard::on_pushButton_2_clicked()
{
    qstr += '2';
    update();
}
void KeyBoard::on_pushButton_3_clicked()
{
    qstr += '3';
    update();
}
void KeyBoard::on_pushButton_4_clicked()
{
    qstr += '4';
    update();
}
void KeyBoard::on_pushButton_5_clicked()
{
    qstr += '5';
    update();
}
void KeyBoard::on_pushButton_6_clicked()
{
    qstr += '6';
    update();
}
void KeyBoard::on_pushButton_7_clicked()
{
    qstr += '7';
    update();
}
void KeyBoard::on_pushButton_8_clicked()
{
    qstr += '8';
    update();
}
void KeyBoard::on_pushButton_9_clicked()
{
    qstr += '9';
    update();
}
void KeyBoard::on_pushButton_back_clicked()
{
    qstr.remove(qstr.length()-1,1);
    update();
}
void KeyBoard::on_pushButton_clear_clicked()
{
    qstr.clear();
    update();
}
void KeyBoard::on_pushButton_ok_clicked()
{
    update();
}
