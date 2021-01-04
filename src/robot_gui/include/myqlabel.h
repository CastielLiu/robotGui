#ifndef MYQLABEL_H
#define MYQLABEL_H
#include <QLabel>
#include <QObject>
#include <QMouseEvent>
#include <QDebug>

// 继承QLabel并重写鼠标点击事件函数、鼠标移动事件函数
// 可实现控件拖动以及响应点击事件
//点击事件和拖动事件可手动设置（默认为不响应点击和不可拖动）
//connect(myLable,SIGNAL(clicked()),this,SLOT(onClicked())); 需将点击信号进行连接
// 在ui界面将基类控件提升为自定义的控件，即可实现
class MyQLabel : public QLabel
{
    Q_OBJECT
public:
    explicit MyQLabel(QWidget *parent);
    ~MyQLabel();
    void setOpenClickEvent(bool key){m_isOpenClickEvent = key;}
    void setOpenMoveEvent(bool key){m_isOpenMoveEvent = key;}

protected:
    virtual void mousePressEvent(QMouseEvent *e);  //鼠标点击事件
    virtual void mouseReleaseEvent(QMouseEvent *e); //鼠标释放事件
    virtual void mouseMoveEvent(QMouseEvent *e);    //鼠标移动事件

signals:
    void clicked();  //自定义点击信号，在mousePressEvent事件发生时触发

//private slots:
//    void onClicked();

private:
    bool m_shouldMove=false;//是否应该改变窗口的位置
    bool m_moveOk=false; //是否已经完成移动

    QPoint m_formerWidgetPos;//移动前控件的位置
    QPoint m_formerMousePos;//第一次按下左键时鼠标的位置

    bool m_isOpenClickEvent;
    bool m_isOpenMoveEvent;
};

#endif // MYQLABEL_H
