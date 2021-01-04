#ifndef QRVIZ_H
#define QRVIZ_H

#include <QWidget>
#include <QVBoxLayout>
#include <rviz/visualization_manager.h>
#include <rviz/render_panel.h>
#include <rviz/display.h>
#include <rviz/tool_manager.h>
#include <rviz/visualization_manager.h>
#include <rviz/render_panel.h>
#include <rviz/display.h>
#include <rviz/tool.h>
#include "rviz/image/ros_image_texture.h"
#include <rviz/tool_manager.h>
#include <QThread>
#include <QDebug>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QException>
#include <QSettings>
#include <QStandardItemModel>
#include <QTreeWidgetItem>
#include "ui_qrviz.h"
#include "addtopics.h"

namespace Ui {
class QRviz;
}

class QRviz : public QWidget
{
  Q_OBJECT

public:
  explicit QRviz(QWidget *parent = 0);
  ~QRviz();
  void connections();
  void loadUserDisplays();
  void createDisplay(QString display_name,QString topic_name);
  //显示Grid
  void Display_Grid(bool enable,QString Reference_frame,int Plan_Cell_count,QColor color=QColor(125,125,125));
  //显示map
  void Display_Map(bool enable,QString topic,double Alpha,QString Color_Scheme);
  //设置全局显示属性
  void SetGlobalOptions(QString frame_name,QColor backColor,int frame_rate);
  //显示激光雷达点云
  void Display_LaserScan(bool enable,QString topic);
  //显示导航相关控件
  void Display_Navigate(bool enable,QString Global_topic,QString Global_planner,QString Local_topic,QString Local_planner);
  //显示tf坐标变换
  void Display_TF(bool enable);

  //发布goal话题的坐标
  void Send_Goal_topic();
  //显示robotmodel
  void Display_RobotModel(bool enable);

  //使能参数存储
  void enablePerformanceStorage(const QString& name, const QString& type);

private:
  void loadPerformance();
  void savePerformance();

private:
  Ui::QRviz ui;
  //rviz显示容器
  rviz::RenderPanel *render_panel_;
  rviz::VisualizationManager *manager_;
  rviz::Display* grid_=NULL ;

  //显示tf坐标变换
  rviz::Display* TF_=NULL ;
  rviz::Display* map_=NULL ;
  rviz::Display* laser_=NULL ;
  rviz::Display* Navigate_localmap=NULL;
  rviz::Display* Navigate_localplanner=NULL;
  rviz::Display* Navigate_globalmap=NULL;
  rviz::Display* Navigate_globalplanner=NULL;
  rviz::Display* Navigate_amcl=NULL;
  rviz::Display* RobotModel_=NULL;

  //rviz工具
  rviz::Tool *current_tool;
  //rviz工具控制器
  rviz::ToolManager *tool_manager_;
  QVBoxLayout *layout;
  QString nodename;

  QStandardItemModel* treeView_rviz_model=NULL;

  //存放rviz treewidget当前显示的控件及控件的父亲的地址
  QMap <QWidget*,QTreeWidgetItem *> widget_to_parentItem_map;
  //存放状态栏的对应关系 display名 状态item
  QMap <QString,QTreeWidgetItem *> tree_rviz_stues;
  //存放display的当前值 item名，参数名称和值
  QMap <QTreeWidgetItem*,QMap<QString,QString>> tree_rviz_values;
  AddTopics *addtopic_form = NULL;

  QColor grid_color;
  QColor bg_color;

  //ui界面参数存储
  bool is_storeParams;
  QString m_configFileName;
  QString m_configFileType;


 private slots:
  void addTool( rviz::Tool* );
  void slot_choose_topic(QTreeWidgetItem *choose);
  void slot_treewidget_item_value_change(QString);
  void slot_treewidget_item_check_change(int);
  void on_pushBotton_move_camera_clicked();
  void on_pushBotton_select_clicked();
  void on_pushBotton_2DPose_clicked();
  void on_pushBotton_2DGoal_clicked();
  void on_pushBotton_setNavGoal_clicked();
  void on_pushBotton_setReturn_clicked();
  void on_pushButton_add_topic_clicked();

};

#endif // QRVIZ_H
