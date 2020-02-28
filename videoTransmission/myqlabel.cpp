#include "myqlabel.h"

MyQLabel::MyQLabel(QWidget *parent):
    QLabel(parent)
{
    setMouseTracking(true);
    //connect(this,SIGNAL(clicked()),this,SLOT(onClicked()));
}

MyQLabel::~MyQLabel()
{

}

void MyQLabel::mousePressEvent(QMouseEvent *event)
{
    //qDebug() << "pressed!!" ;
    if(event->button() == Qt::LeftButton)
    {
        m_shouldMove = true;
        m_formerMousePos = event->globalPos();//获取鼠标全局位置
        m_formerWidgetPos = this->pos(); //获取当前控件位置
    }

    //将该事件传给父类处理，这句话很重要，如果没有，父类无法处理本来的点击事件
    QLabel::mousePressEvent(event);
}

void MyQLabel::mouseReleaseEvent(QMouseEvent *event)
{
    //qDebug() << "released!!" ;
    if(event->button()!=Qt::LeftButton) return;

    m_shouldMove = false;
    if(!m_moveOk) //没有移动，表明为点击事件
    {
        emit clicked();  //触发clicked信号
    }
    else //松开鼠标，移动完成
    {
        m_moveOk = false;
    }

    QLabel::mouseReleaseEvent(event);
}

void MyQLabel::mouseMoveEvent(QMouseEvent *event)
{
    if(!m_shouldMove) return;

    QPoint nowMousePos = event->globalPos();
    QPoint dis = nowMousePos - m_formerMousePos;
    //qDebug() << "move: " << dis;
    if(abs(dis.x())>5 || abs(dis.y())>5)
    {
        this->move(m_formerWidgetPos+dis);
        m_moveOk = true;
    }

    QLabel::mouseMoveEvent(event);
}
