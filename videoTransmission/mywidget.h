#ifndef MYWIDGET_H
#define MYWIDGET_H

#include <QObject>
#include <QWidget>
#include <QKeyEvent>
#include <QDebug>

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
