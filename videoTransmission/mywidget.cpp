#include "mywidget.h"

MyWidget::MyWidget(QWidget *parent) : QWidget(parent)
{
    setFocusPolicy(Qt::ClickFocus); //设置聚焦方式
    //setFocusPolicy(Qt::StrongFocus); //设置聚焦方式
    //setFocusPolicy(Qt::TabFocus);
}
/*
Qt::Key_Left 0x01000012
Qt::Key_Up 0x01000013
Qt::Key_Right 0x01000014
Qt::Key_Down 0x01000015
*/

void MyWidget::keyPressEvent(QKeyEvent *event)
{
    //如果为自动重复的时间，不予处理
    //当用户长按按键时，系统会自动重复发出按下、释放等信号
    if(event->isAutoRepeat()) return;

    if(event->key() >= Qt::Key_Left && event->key() <= Qt::Key_Down)
        emit dirKeyPressed(event->key());

}

void MyWidget::keyReleaseEvent(QKeyEvent *event)
{
    if(event->isAutoRepeat()) return;

    if(event->key() >= Qt::Key_Left && event->key() <= Qt::Key_Down)
        emit dirKeyReleased(event->key());
}

