#include "qnode.h"
#include <QThread>
#include <QDebug>

QNode::QNode(QObject *parent) :
  QObject(parent),
  is_init(false),
  wait_roscore(false)
{

}

bool QNode::init(int argc, char**argv)
{
  ros::init(argc, argv,"robot_gui");

  int wait_num = 8;
  while (!ros::master::check())
  {
    if(wait_num -- < 0)
        return false;

    if(!wait_roscore)
    {
      std::thread t(&QNode::roscoreThread, this);
      t.detach();
      wait_roscore = true;
      qDebug() << "roscore is auto starting...";
    }

    QThread::sleep(1);
  }
  is_init = true;

  return true;
}
