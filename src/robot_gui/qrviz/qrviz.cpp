#include "qrviz.h"
#include "ui_qrviz.h"

QRviz::QRviz(QWidget *parent) :
  QWidget(parent),
  addtopic_form(nullptr)
{
  ui.setupUi(this);

  this->nodename="robot_gui";
  this->layout=ui.verticalLayout_displayWindow;

  //创建rviz容器
  render_panel_=new rviz::RenderPanel;
  //向layout添加widget
  layout->addWidget(render_panel_);
  //初始化rviz控制对象
  manager_=new rviz::VisualizationManager(render_panel_);
  ROS_ASSERT(manager_!=NULL);
  //获取当前rviz控制对象的 tool控制对象
  tool_manager_=manager_->getToolManager();
  ROS_ASSERT(tool_manager_!=NULL);
 //初始化camera 这行代码实现放大 缩小 平移等操作
  render_panel_->initialize(manager_->getSceneManager(),manager_);
  manager_->initialize();
  tool_manager_->initialize();
  manager_->removeAllDisplays();



  ui.treeWidget_rviz->setWindowTitle("Displays");
  ui.treeWidget_rviz->setWindowIcon(QIcon("://rviz_icon/classes/Displays.svg"));
  //header 设置
  ui.treeWidget_rviz->setHeaderHidden(true);
  ui.treeWidget_rviz->setHeaderLabels(QStringList()<<"key"<<"value");

  //Global options
  QTreeWidgetItem *Global=new QTreeWidgetItem(QStringList()<<"Global Options");
  Global->setIcon(0,QIcon("://rviz_icon/options.png"));
  ui.treeWidget_rviz->addTopLevelItem(Global);
  Global->setExpanded(true);

  //Global Options -> Fixed Frame
  QTreeWidgetItem* FixedFrame=new QTreeWidgetItem(QStringList()<<"Fixed Frame");
  Global->addChild(FixedFrame);

  //添加combox控件
  QComboBox *frame=new QComboBox();
  frame->addItem("map");
  frame->setEditable(true);
  frame->setMaximumWidth(150);
  ui.treeWidget_rviz->setItemWidget(FixedFrame,1,frame);


  QTreeWidgetItem* bcolor=new QTreeWidgetItem(QStringList()<<"Background Color");
  Global->addChild(bcolor);
  //添加lineedit控件
  QLineEdit *colorval=new QLineEdit("48;48;48");
  colorval->setMaximumWidth(150);
  ui.treeWidget_rviz->setItemWidget(bcolor,1,colorval);

  QSpinBox *framerateval=new QSpinBox();
  framerateval->setStyleSheet("border:none");
  framerateval->setMaximumWidth(150);
  framerateval->setRange(10,50);
  framerateval->setValue(30);
  QTreeWidgetItem* framerate=new QTreeWidgetItem(QStringList()<<"Frame Rate");
  Global->addChild(framerate);
  ui.treeWidget_rviz->setItemWidget(framerate,1,framerateval);

  //grid
  QTreeWidgetItem *Grid=new QTreeWidgetItem(QStringList()<<"Grid");
  Grid->setIcon(0,QIcon("://rviz_icon/classes/Grid.png"));

  ui.treeWidget_rviz->addTopLevelItem(Grid);
  Grid->setExpanded(true);
  QCheckBox* gridcheck=new QCheckBox;
  gridcheck->setChecked(true);
  ui.treeWidget_rviz->setItemWidget(Grid,1,gridcheck);

  QTreeWidgetItem *Grid_Status=new QTreeWidgetItem(QStringList()<<"State:");
  Grid_Status->setIcon(0,QIcon("://rviz_icon/ok.png"));
  Grid->addChild(Grid_Status);
  QLabel *Grid_Status_Value=new QLabel("ok");
  Grid_Status_Value->setMaximumWidth(150);
  ui.treeWidget_rviz->setItemWidget(Grid_Status,1,Grid_Status_Value);

  QTreeWidgetItem* Reference_Frame=new QTreeWidgetItem(QStringList()<<"Reference Frame");
  QComboBox* Reference_Frame_Value=new QComboBox();
  Grid->addChild(Reference_Frame);
  Reference_Frame_Value->setMaximumWidth(150);
  Reference_Frame_Value->setEditable(true);
  Reference_Frame_Value->addItem("<Fixed Frame>");
  ui.treeWidget_rviz->setItemWidget(Reference_Frame,1,Reference_Frame_Value);

  QTreeWidgetItem* Plan_Cell_Count=new QTreeWidgetItem(QStringList()<<"Plan Cell Count");
  Grid->addChild(Plan_Cell_Count);
  QSpinBox* Plan_Cell_Count_Value=new QSpinBox();

  Plan_Cell_Count_Value->setMaximumWidth(150);
  Plan_Cell_Count_Value->setRange(1,100);
  Plan_Cell_Count_Value->setValue(10);
  ui.treeWidget_rviz->setItemWidget(Plan_Cell_Count,1,Plan_Cell_Count_Value);

  QTreeWidgetItem* Grid_Color=new QTreeWidgetItem(QStringList()<<"Color");
  QLineEdit* Grid_Color_Value=new QLineEdit();
  Grid_Color_Value->setMaximumWidth(150);
  Grid->addChild(Grid_Color);

  Grid_Color_Value->setText("160;160;160");
  ui.treeWidget_rviz->setItemWidget(Grid_Color,1,Grid_Color_Value);

  //qucik treewidget
  //ui.treeWidget_quick_cmd->setHeaderLabels(QStringList()<<"key"<<"values");
  //ui.treeWidget_quick_cmd->setHeaderHidden(true);

  QComboBox *Global_op=(QComboBox *) ui.treeWidget_rviz->itemWidget(ui.treeWidget_rviz->topLevelItem(0)->child(0),1);
  QString Reference_text=Global_op->currentText();
  this->Display_Grid(true,Reference_text,10,QColor(160,160,160));
}

QRviz::~QRviz()
{
}

void QRviz::init()
{
  //treewidget的值改变的槽函数
  //绑定treeiew所有控件的值改变函数
  for(int i=0;i<ui.treeWidget_rviz->topLevelItemCount();i++)
  {
      //top 元素
      QTreeWidgetItem *top=ui.treeWidget_rviz->topLevelItem(i);
//        qDebug()<<top->text(0)<<endl;
      for(int j=0;j<top->childCount();j++)
      {
           //获取该WidgetItem的子节点
           QTreeWidgetItem* tmp= top->child(j);
           QWidget* controls=ui.treeWidget_rviz->itemWidget(tmp,1);
//             qDebug()<<controls;
           //将当前控件对象和父级对象加入到map中
           widget_to_parentItem_map[controls]=top;
           //判断这些widget的类型 并分类型进行绑定槽函数
           if(QString(controls->metaObject()->className())=="QComboBox")
           {
               connect(controls,SIGNAL(currentTextChanged(QString)),this,SLOT(slot_treewidget_item_value_change(QString)));
           }
           else if(QString(controls->metaObject()->className())=="QLineEdit")
            {
               connect(controls,SIGNAL(textChanged(QString)),this,SLOT(slot_treewidget_item_value_change(QString)));
             }
           else if(QString(controls->metaObject()->className())=="QSpinBox")
           {
               connect(controls,SIGNAL(valueChanged(QString)),this,SLOT(slot_treewidget_item_value_change(QString)));
           }
      }
  }
  //绑定treeview checkbox选中事件
 // stateChanged

  for(int i=0;i<ui.treeWidget_rviz->topLevelItemCount();i++)
  {
      //top 元素
      QTreeWidgetItem *top=ui.treeWidget_rviz->topLevelItem(i);
      QWidget *check=ui.treeWidget_rviz->itemWidget(top,1);
      //记录父子关系
      widget_to_parentItem_map[check]=top;
      connect(check,SIGNAL(stateChanged(int)),this,SLOT(slot_treewidget_item_check_change(int)));
  }
  //connect(ui.treeWidget_rviz,SIGNAL(itemChanged(QTreeWidgetItem*,int)),this,SLOT(slot_treewidget_item_value_change(QTreeWidgetItem*,int)));
}


//显示robotModel
void QRviz::Display_RobotModel(bool enable)
{

    if(RobotModel_==NULL)
    {
        RobotModel_=manager_->createDisplay("rviz/RobotModel","Qrviz RobotModel",enable);
    }
    else{
        delete RobotModel_;
        RobotModel_=manager_->createDisplay("rviz/RobotModel","Qrviz RobotModel",enable);
    }
}

//显示grid
void QRviz::Display_Grid(bool enable,QString Reference_frame,int Plan_Cell_count,QColor color)
{
    if(grid_==NULL)
    {
        grid_ = manager_->createDisplay( "rviz/Grid", "adjustable grid", true );
        ROS_ASSERT( grid_ != NULL );
        // Configure the GridDisplay the way we like it.
        grid_->subProp( "Line Style" )->setValue("Billboards");
        grid_->subProp( "Color" )->setValue(color);
        grid_->subProp( "Reference Frame" )->setValue(Reference_frame);
        grid_->subProp("Plane Cell Count")->setValue(Plan_Cell_count);

    }
    else{
        delete grid_;
        grid_ = manager_->createDisplay( "rviz/Grid", "adjustable grid", true );
        ROS_ASSERT( grid_ != NULL );
        // Configure the GridDisplay the way we like it.
        grid_->subProp( "Line Style" )->setValue("Billboards");
        grid_->subProp( "Color" )->setValue(color);
        grid_->subProp( "Reference Frame" )->setValue(Reference_frame);
        grid_->subProp("Plane Cell Count")->setValue(Plan_Cell_count);
    }
    grid_->setEnabled(enable);
    manager_->startUpdate();
}
//显示map
void QRviz::Display_Map(bool enable,QString topic,double Alpha,QString Color_Scheme)
{
    if(!enable && map_)
    {
        map_->setEnabled(false);
        return ;
    }
    if(map_==NULL)
    {
        map_=manager_->createDisplay("rviz/Map","QMap",true);
        ROS_ASSERT(map_);
        map_->subProp("Topic")->setValue(topic);
        map_->subProp("Alpha")->setValue(Alpha);
        map_->subProp("Color Scheme")->setValue(Color_Scheme);

    }
    else{
         ROS_ASSERT(map_);
         qDebug()<<"asdasdasd:"<<topic<<Alpha;

        delete map_;
        map_=manager_->createDisplay("rviz/Map","QMap",true);
        ROS_ASSERT(map_);
        map_->subProp("Topic")->setValue(topic);
        map_->subProp("Alpha")->setValue(Alpha);
        map_->subProp("Color Scheme")->setValue(Color_Scheme);
    }

    map_->setEnabled(enable);
    manager_->startUpdate();
}
//显示激光雷达
void QRviz::Display_LaserScan(bool enable,QString topic)
{
    if(laser_==NULL)
    {
        laser_=manager_->createDisplay("rviz/LaserScan","QLaser",enable);
        ROS_ASSERT(laser_);
        laser_->subProp("Topic")->setValue(topic);
    }
    else{
        delete laser_;
        laser_=manager_->createDisplay("rviz/LaserScan","QLaser",enable);
        ROS_ASSERT(laser_);
        laser_->subProp("Topic")->setValue(topic);
    }
    qDebug()<<"topic:"<<topic;
    laser_->setEnabled(enable);
    manager_->startUpdate();
}
//设置全局显示
 void QRviz::SetGlobalOptions(QString frame_name,QColor backColor,int frame_rate)
 {
     manager_->setFixedFrame(frame_name);
     manager_->setProperty("Background Color",backColor);
     manager_->setProperty("Frame Rate",frame_rate);
     manager_->startUpdate();
 }

// "rviz/MoveCamera";
// "rviz/Interact";
// "rviz/Select";
// "rviz/SetInitialPose";
// "rviz/SetGoal";
 //显示tf坐标变换
 void QRviz::Display_TF(bool enable)
 {
     if(TF_){delete TF_;TF_=NULL;}
     TF_=manager_->createDisplay("rviz/TF","QTF",enable);
 }
 //显示导航相关
 void QRviz::Display_Navigate(bool enable,QString Global_topic,QString Global_planner,QString Local_topic,QString Local_planner)
 {
    if(Navigate_localmap) {delete Navigate_localmap; Navigate_localmap=NULL;}
    if(Navigate_localplanner) {delete Navigate_localplanner; Navigate_localplanner=NULL;}
    if(Navigate_globalmap) {delete Navigate_globalmap; Navigate_globalmap=NULL;}
    if(Navigate_globalplanner) {delete Navigate_globalplanner; Navigate_globalplanner=NULL;}
    //local map
    Navigate_localmap=manager_->createDisplay("rviz/Map","Qlocalmap",enable);
    Navigate_localmap->subProp("Topic")->setValue(Local_topic);
    Navigate_localmap->subProp("Color Scheme")->setValue("costmap");
    Navigate_localplanner=manager_->createDisplay("rviz/Path","QlocalPath",enable);
    Navigate_localplanner->subProp("Topic")->setValue(Local_planner);
    Navigate_localplanner->subProp("Color")->setValue(QColor(0,12,255));
    //global map
    Navigate_globalmap=manager_->createDisplay("rviz/Map","QGlobalmap",enable);
    Navigate_globalmap->subProp("Topic")->setValue(Global_topic);
    Navigate_globalmap->subProp("Color Scheme")->setValue("costmap");
    Navigate_globalplanner=manager_->createDisplay("rviz/Path","QGlobalpath",enable);
    Navigate_globalplanner->subProp("Topic")->setValue(Global_planner);
    Navigate_globalplanner->subProp("Color")->setValue(QColor(255,0,0));
    //更新画面显示
    manager_->startUpdate();

 }
void QRviz::addTool( rviz::Tool* )
{

}
void QRviz::createDisplay(QString display_name,QString topic_name)
{


}
void QRviz::run()
{

}

void QRviz::on_pushBotton_move_camera_clicked()
{
  //获取设置Pos的工具
  //添加工具

  current_tool= tool_manager_->addTool("rviz/MoveCamera");
  //设置当前使用的工具为SetInitialPose（实现在地图上标点）
  tool_manager_->setCurrentTool( current_tool );
  manager_->startUpdate();
}

void QRviz::on_pushBotton_select_clicked()
{
  //获取设置Pos的工具
  //添加工具

  current_tool= tool_manager_->addTool("rviz/Select");
  //设置当前使用的工具为SetInitialPose（实现在地图上标点）
  tool_manager_->setCurrentTool( current_tool );
  manager_->startUpdate();
}

void QRviz::on_pushBotton_2DPose_clicked()
{
  //获取设置Pos的工具
  //添加工具

  current_tool= tool_manager_->addTool("rviz/SetInitialPose");
  //设置当前使用的工具为SetInitialPose（实现在地图上标点）
  tool_manager_->setCurrentTool( current_tool );
  manager_->startUpdate();
}


void QRviz::on_pushBotton_2DGoal_clicked()
{

}

void QRviz::on_pushBotton_setNavGoal_clicked()
{
  //添加工具
  current_tool= tool_manager_->addTool("rviz/SetGoal");
  //设置goal的话题
  rviz::Property* pro= current_tool->getPropertyContainer();
  pro->subProp("Topic")->setValue("/move_base_simple/goal");
  //设置当前frame
  manager_->setFixedFrame("map");
  //设置当前使用的工具为SetGoal（实现在地图上标点）
  tool_manager_->setCurrentTool( current_tool );

  manager_->startUpdate();
}

void QRviz::on_pushBotton_setReturn_clicked()
{

}

//rviz添加topic的槽函数
void QRviz::on_pushButton_add_topic_clicked()
{
    if(!addtopic_form)
    {
        addtopic_form=new AddTopics();
        //阻塞其他窗体
        addtopic_form->setWindowModality(Qt::ApplicationModal);
        //绑定添加rviz话题信号
        connect(addtopic_form,SIGNAL(Topic_choose(QTreeWidgetItem *)),this,SLOT(slot_choose_topic(QTreeWidgetItem *)));
        addtopic_form->show();
    }
    else
    {
      /*
        QPoint p=addtopic_form->pos();
        delete addtopic_form;
        addtopic_form=new AddTopics();
        //阻塞其他窗体
        addtopic_form->setWindowModality(Qt::ApplicationModal);
        connect(addtopic_form,SIGNAL(Topic_choose(QTreeWidgetItem *)),this,SLOT(slot_choose_topic(QTreeWidgetItem *)));
        addtopic_form->show();
        addtopic_form->move(p.x(),p.y());
        */
      addtopic_form->show();
    }
}
//选中要添加的话题的槽函数
void QRviz::slot_choose_topic(QTreeWidgetItem *choose)
{
  //将选中的话题添加到列表
  ui.treeWidget_rviz->addTopLevelItem(choose);
  //添加是否启用的checkbox
  QCheckBox *check=new QCheckBox();
  ui.treeWidget_rviz->setItemWidget(choose,1,check);
  //记录父子关系
  widget_to_parentItem_map[check]=choose;
  //绑定checkbox的槽函数
  connect(check,SIGNAL(stateChanged(int)),this,SLOT(slot_treewidget_item_check_change(int)));
  //添加状态的对应关系到map
  tree_rviz_stues[choose->text(0)]=choose->child(0);

  if(choose->text(0)=="Map")
  {
    QComboBox *Map_Topic=new QComboBox();
    Map_Topic->addItem("map");
    Map_Topic->setEditable(true);
    Map_Topic->setMaximumWidth(150);
    widget_to_parentItem_map[Map_Topic]=choose;
    ui.treeWidget_rviz->setItemWidget(choose->child(1),1,Map_Topic);
    //绑定值改变了的事件
    connect(Map_Topic,SIGNAL(currentTextChanged(QString)),this,SLOT(slot_treewidget_item_value_change(QString)));
    QLineEdit *map_alpha=new QLineEdit;
    map_alpha->setMaximumWidth(150);
    map_alpha->setText("0.7");
    widget_to_parentItem_map[map_alpha]=choose;
    //绑定值改变了的事件
    connect(map_alpha,SIGNAL(textChanged(QString)),this,SLOT(slot_treewidget_item_value_change(QString)));
    ui.treeWidget_rviz->setItemWidget(choose->child(2),1,map_alpha);

    QComboBox *Map_Scheme=new QComboBox;
    Map_Scheme->setMaximumWidth(150);
    Map_Scheme->addItems(QStringList()<<"map"<<"costmap"<<"raw");
    widget_to_parentItem_map[Map_Scheme]=choose;
    //绑定值改变了的事件
    connect(Map_Scheme,SIGNAL(currentTextChanged(QString)),this,SLOT(slot_treewidget_item_value_change(QString)));
    ui.treeWidget_rviz->setItemWidget(choose->child(3),1,Map_Scheme);
  }
  else if(choose->text(0)=="LaserScan")
  {

    QComboBox *Laser_Topic=new QComboBox;
    Laser_Topic->setMaximumWidth(150);
    Laser_Topic->addItem("scan");
    Laser_Topic->setEditable(true);
    widget_to_parentItem_map[Laser_Topic]=choose;
    ui.treeWidget_rviz->setItemWidget(choose->child(1),1,Laser_Topic);
    //绑定值改变了的事件
    connect(Laser_Topic,SIGNAL(currentTextChanged(QString)),this,SLOT(slot_treewidget_item_value_change(QString)));

  }
  else if(choose->text(0)=="Navigate")
  {
    //Global Map
    QTreeWidgetItem *Global_map=new QTreeWidgetItem(QStringList()<<"Global Map");
    Global_map->setIcon(0,QIcon("://rviz_icon/classes/Group.png"));
    choose->addChild(Global_map);
    QTreeWidgetItem *Costmap=new QTreeWidgetItem(QStringList()<<"Costmap");
    Costmap->setIcon(0,QIcon("://rviz_icon/classes/Map.png"));
    QTreeWidgetItem *Costmap_topic=new QTreeWidgetItem(QStringList()<<"Topic");
    Costmap->addChild(Costmap_topic);
    QComboBox *Costmap_Topic_Vel=new QComboBox();
    Costmap_Topic_Vel->setEditable(true);
    Costmap_Topic_Vel->setMaximumWidth(150);
    Costmap_Topic_Vel->addItem("/move_base/global_costmap/costmap");
    ui.treeWidget_rviz->setItemWidget(Costmap_topic,1,Costmap_Topic_Vel);
    Global_map->addChild(Costmap);
    //绑定子父关系
    widget_to_parentItem_map[Costmap_Topic_Vel]=choose;
    //绑定值改变了的事件
    connect(Costmap_Topic_Vel,SIGNAL(currentTextChanged(QString)),this,SLOT(slot_treewidget_item_value_change(QString)));

    QTreeWidgetItem* CostMap_Planner=new QTreeWidgetItem(QStringList()<<"Planner");
    CostMap_Planner->setIcon(0,QIcon("://rviz_icon/classes/Path.png"));
   Global_map->addChild(CostMap_Planner);

   QTreeWidgetItem* CostMap_Planner_topic=new QTreeWidgetItem(QStringList()<<"Topic");
    QComboBox* Costmap_Planner_Topic_Vel=new QComboBox();
    Costmap_Planner_Topic_Vel->setMaximumWidth(150);
    Costmap_Planner_Topic_Vel->addItem("/move_base/DWAPlannerROS/global_plan");
    Costmap_Planner_Topic_Vel->setEditable(true);
    CostMap_Planner->addChild(CostMap_Planner_topic);
    ui.treeWidget_rviz->setItemWidget(CostMap_Planner_topic,1,Costmap_Planner_Topic_Vel);
    //绑定子父关系
    widget_to_parentItem_map[Costmap_Planner_Topic_Vel]=choose;
    //绑定值改变了的事件
    connect(Costmap_Planner_Topic_Vel,SIGNAL(currentTextChanged(QString)),this,SLOT(slot_treewidget_item_value_change(QString)));



    //Local Map
    QTreeWidgetItem *Local_map=new QTreeWidgetItem(QStringList()<<"Local Map");
    Local_map->setIcon(0,QIcon("://rviz_icon/classes/Group.png"));
    choose->addChild(Local_map);

    QTreeWidgetItem *Local_Costmap=new QTreeWidgetItem(QStringList()<<"Costmap");
    Local_Costmap->setIcon(0,QIcon("://rviz_icon/classes/Map.png"));
    Local_map->addChild(Local_Costmap);

    QTreeWidgetItem *local_costmap_topic=new QTreeWidgetItem(QStringList()<<"Topic");
    Local_Costmap->addChild(local_costmap_topic);
    QComboBox *local_costmap_topic_vel=new QComboBox();
    local_costmap_topic_vel->setEditable(true);
    local_costmap_topic_vel->setMaximumWidth(150);
    local_costmap_topic_vel->addItem("/move_base/local_costmap/costmap");
    ui.treeWidget_rviz->setItemWidget(local_costmap_topic,1,local_costmap_topic_vel);

    //绑定子父关系
    widget_to_parentItem_map[local_costmap_topic_vel]=choose;
    //绑定值改变了的事件
    connect(local_costmap_topic_vel,SIGNAL(currentTextChanged(QString)),this,SLOT(slot_treewidget_item_value_change(QString)));

    QTreeWidgetItem* LocalMap_Planner=new QTreeWidgetItem(QStringList()<<"Planner");
    LocalMap_Planner->setIcon(0,QIcon("://rviz_icon/classes/Path.png"));
   Local_map->addChild(LocalMap_Planner);

   QTreeWidgetItem* Local_Planner_topic=new QTreeWidgetItem(QStringList()<<"Topic");

    QComboBox* Local_Planner_Topic_Vel=new QComboBox();
    Local_Planner_Topic_Vel->setMaximumWidth(150);
    Local_Planner_Topic_Vel->addItem("/move_base/DWAPlannerROS/local_plan");
    Local_Planner_Topic_Vel->setEditable(true);
    LocalMap_Planner->addChild(Local_Planner_topic);
    ui.treeWidget_rviz->setItemWidget(Local_Planner_topic,1, Local_Planner_Topic_Vel);
    //绑定子父关系
    widget_to_parentItem_map[Local_Planner_Topic_Vel]=choose;
    //绑定值改变了的事件
    connect(Local_Planner_Topic_Vel,SIGNAL(currentTextChanged(QString)),this,SLOT(slot_treewidget_item_value_change(QString)));
    //CostCloud
    QTreeWidgetItem* CostCloud=new QTreeWidgetItem(QStringList()<<"Cost Cloud");
    CostCloud->setIcon(0,QIcon("://rviz_icon/classes/PointCloud2.png"));
    Local_map->addChild(CostCloud);
    QTreeWidgetItem *CostCloud_Topic=new QTreeWidgetItem(QStringList()<<"Topic");
    QComboBox* CostCloud_Topic_Vel=new QComboBox();
    CostCloud_Topic_Vel->setMaximumWidth(150);
    CostCloud_Topic_Vel->setEditable(true);
    CostCloud_Topic_Vel->addItem("/move_base/DWAPlannerROS/cost_cloud");
    CostCloud->addChild(CostCloud_Topic);
    ui.treeWidget_rviz->setItemWidget(CostCloud_Topic,1,CostCloud_Topic_Vel);
    //绑定子父关系
    widget_to_parentItem_map[CostCloud_Topic_Vel]=CostCloud;
    //绑定值改变了的事件
    connect(CostCloud_Topic_Vel,SIGNAL(currentTextChanged(QString)),this,SLOT(slot_treewidget_item_value_change(QString)));
    //Trajectory Cloud
    QTreeWidgetItem* TrajectoryCloud=new QTreeWidgetItem(QStringList()<<"Trajectory Cloud");
    TrajectoryCloud->setIcon(0,QIcon("://rviz_icon/classes/PointCloud2.png"));
    Local_map->addChild(TrajectoryCloud);
    QTreeWidgetItem *TrajectoryCloud_Topic=new QTreeWidgetItem(QStringList()<<"Topic");
    QComboBox* TrajectoryCloud_Topic_Vel=new QComboBox();
    TrajectoryCloud_Topic_Vel->setMaximumWidth(150);
    TrajectoryCloud_Topic_Vel->setEditable(true);
    TrajectoryCloud_Topic_Vel->addItem("/move_base/DWAPlannerROS/trajectory_cloud");
    TrajectoryCloud->addChild(TrajectoryCloud_Topic);
    ui.treeWidget_rviz->setItemWidget(TrajectoryCloud_Topic,1,TrajectoryCloud_Topic_Vel);
    //绑定子父关系
    widget_to_parentItem_map[TrajectoryCloud_Topic_Vel]=TrajectoryCloud;
    //绑定值改变了的事件
    connect(TrajectoryCloud_Topic_Vel,SIGNAL(currentTextChanged(QString)),this,SLOT(slot_treewidget_item_value_change(QString)));
    ui.treeWidget_rviz->addTopLevelItem(choose);
    choose->setExpanded(true);
  }
  else if(choose->text(0)=="TF")
  {
    ui.treeWidget_rviz->addTopLevelItem(choose);
  }

  //默认选中
  check->setChecked(true);
}

//treewidget中checkbox状态发生变化
void QRviz::slot_treewidget_item_check_change(int is_check)
{
    QCheckBox* sen = (QCheckBox*)sender();
    qDebug()<<"check:"<<is_check<<"parent:"<<widget_to_parentItem_map[sen]->text(0)<<"地址："<<widget_to_parentItem_map[sen];
    QTreeWidgetItem *parentItem=widget_to_parentItem_map[sen];
    QString dis_name=widget_to_parentItem_map[sen]->text(0);
    bool enable=is_check>1?true:false;
    if(dis_name=="Grid")
    {

        QLineEdit *Color_text=(QLineEdit *) ui.treeWidget_rviz->itemWidget(parentItem->child(3),1);
        QString co=Color_text->text();
        QStringList colorList=co.split(";");
        QColor cell_color=QColor(colorList[0].toInt(),colorList[1].toInt(),colorList[2].toInt());

        QComboBox *Reference_box=(QComboBox *) ui.treeWidget_rviz->itemWidget(parentItem->child(1),1);
        QString Reference_text=Reference_box->currentText();
        if(Reference_box->currentText()=="<Fixed Frame>")
        {
            QComboBox *Global_op=(QComboBox *) ui.treeWidget_rviz->itemWidget(ui.treeWidget_rviz->topLevelItem(0)->child(0),1);
            Reference_text=Global_op->currentText();
        }
        QSpinBox *plan_cell_count=(QSpinBox *) ui.treeWidget_rviz->itemWidget(parentItem->child(2),1);
        Display_Grid(enable,Reference_text,plan_cell_count->text().toInt(),cell_color);

    }
    else if(dis_name=="Map")
    {
        QComboBox *topic_box=(QComboBox *) ui.treeWidget_rviz->itemWidget(parentItem->child(1),1);
        QLineEdit *alpha=(QLineEdit *) ui.treeWidget_rviz->itemWidget(parentItem->child(2),1);
        QComboBox *scheme=(QComboBox *) ui.treeWidget_rviz->itemWidget(parentItem->child(3),1);

        Display_Map(enable,topic_box->currentText(),alpha->text().toDouble(),scheme->currentText());
        qDebug()<<topic_box->currentText()<<alpha->text()<<scheme->currentText();
    }
    else if(dis_name=="LaserScan")
    {
        QComboBox *topic_box=(QComboBox *) ui.treeWidget_rviz->itemWidget(parentItem->child(1),1);
        Display_LaserScan(enable,topic_box->currentText());
    }
    else if(dis_name=="Navigate")
    {
        QComboBox* Global_map=(QComboBox *) ui.treeWidget_rviz->itemWidget(parentItem->child(0)->child(0)->child(0),1);
        QComboBox* Global_plan=(QComboBox *) ui.treeWidget_rviz->itemWidget(parentItem->child(0)->child(1)->child(0),1);
        QComboBox* Local_map=(QComboBox *) ui.treeWidget_rviz->itemWidget(parentItem->child(1)->child(0)->child(0),1);
        QComboBox* Local_plan=(QComboBox *) ui.treeWidget_rviz->itemWidget(parentItem->child(1)->child(1)->child(0),1);

        Display_Navigate(enable,Global_map->currentText(),Global_plan->currentText(),Local_map->currentText(),Local_plan->currentText());
    }
    else if(dis_name=="RobotModel")
    {
        Display_RobotModel(enable);
    }
}

//treewidget 的值改变槽函数
void QRviz::slot_treewidget_item_value_change(QString value)
{

    QWidget* sen = (QWidget*)sender();
    qDebug()<<sen->metaObject()->className()<<"parent:"<<widget_to_parentItem_map[sen]->text(0);
    qDebug()<<value;
    QTreeWidgetItem *parentItem=widget_to_parentItem_map[sen];
    QString Dis_Name=widget_to_parentItem_map[sen]->text(0);

//    qDebug()<<"sdad"<<enable;
    //判断每种显示的类型
    if(Dis_Name=="Grid")
    {
        //是否启用该图层
        QCheckBox *che_box=(QCheckBox *) ui.treeWidget_rviz->itemWidget(parentItem,1);
        bool enable=che_box->isChecked();
        QLineEdit *Color_text=(QLineEdit *) ui.treeWidget_rviz->itemWidget(parentItem->child(3),1);
        QString co=Color_text->text();
        QStringList colorList=co.split(";");
        QColor cell_color=QColor(colorList[0].toInt(),colorList[1].toInt(),colorList[2].toInt());

        QComboBox *Reference_box=(QComboBox *) ui.treeWidget_rviz->itemWidget(parentItem->child(1),1);
        QString Reference_text=Reference_box->currentText();
        if(Reference_box->currentText()=="<Fixed Frame>")
        {
            QComboBox *Global_op=(QComboBox *) ui.treeWidget_rviz->itemWidget(ui.treeWidget_rviz->topLevelItem(0)->child(0),1);
            Reference_text=Global_op->currentText();
        }
        QSpinBox *plan_cell_count=(QSpinBox *) ui.treeWidget_rviz->itemWidget(parentItem->child(2),1);
        Display_Grid(enable,Reference_text,plan_cell_count->text().toInt(),cell_color);

    }
    else if(Dis_Name=="Global Options")
    {
        QComboBox *Global_op=(QComboBox *) ui.treeWidget_rviz->itemWidget(ui.treeWidget_rviz->topLevelItem(0)->child(0),1);
        QString Reference_text=Global_op->currentText();
        QLineEdit *back_color=(QLineEdit *) ui.treeWidget_rviz->itemWidget(ui.treeWidget_rviz->topLevelItem(0)->child(1),1);
        QStringList coList=back_color->text().split(";");
        QColor colorBack=QColor(coList[0].toInt(),coList[1].toInt(),coList[2].toInt());
        QSpinBox *FrameRaBox=(QSpinBox *) ui.treeWidget_rviz->itemWidget(ui.treeWidget_rviz->topLevelItem(0)->child(2),1);
        SetGlobalOptions(Reference_text,colorBack,FrameRaBox->value());
    }
    else if(Dis_Name=="Map")
    {
        //是否启用该图层
        QCheckBox *che_box=(QCheckBox *) ui.treeWidget_rviz->itemWidget(parentItem,1);
        bool enable=che_box->isChecked();
        QComboBox *topic_box=(QComboBox *) ui.treeWidget_rviz->itemWidget(parentItem->child(1),1);
        QLineEdit *alpha=(QLineEdit *) ui.treeWidget_rviz->itemWidget(parentItem->child(2),1);
        QComboBox *scheme=(QComboBox *) ui.treeWidget_rviz->itemWidget(parentItem->child(3),1);
        qDebug()<<topic_box->currentText()<<alpha->text()<<scheme->currentText();
        Display_Map(enable,topic_box->currentText(),alpha->text().toDouble(),scheme->currentText());
    }
    else if(Dis_Name=="LaserScan")
    {
        //是否启用该图层
        QCheckBox *che_box=(QCheckBox *) ui.treeWidget_rviz->itemWidget(parentItem,1);
        bool enable=che_box->isChecked();
        QComboBox *topic_box=(QComboBox *) ui.treeWidget_rviz->itemWidget(parentItem->child(1),1);
        Display_LaserScan(enable,topic_box->currentText());
    }


}


