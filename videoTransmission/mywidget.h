#ifndef MYWIDGET_H
#define MYWIDGET_H

#include <QObject>
#include <QWidget>
#include <QKeyEvent>
#include <QDebug>

/*自定义MyWidget继承自QWidget
 * 重写键盘事件函数，捕获方向键并发出信号
 *
 *
 */

class MyWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MyWidget(QWidget *parent = nullptr);
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);

signals:
    void dirKeyPressed(int key);
    void dirKeyReleased(int key);

public slots:
};

#endif // MYWIDGET_H
