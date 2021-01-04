#ifndef QNODE_H
#define QNODE_H

#include <QObject>
#include <ros/ros.h>
#include <thread>

class QNode : public QObject
{
  Q_OBJECT
public:
  explicit QNode(QObject *parent = nullptr);
  bool init(int argc, char **argv);
  void roscoreThread() {system("roscore&");}

signals:

public slots:

private:
  bool  is_init;
  bool  wait_roscore;
};

#endif // QNODE_H
