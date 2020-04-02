#include "globalvariable.h"


/**********条件变量***********/
const bool g_canCalled = true; //是否启用被叫
const bool g_openRemoteControl = true; //是否启动远程控制


uint16_t g_myId = 123;   // 注：分配客户端id时保留0
uint16_t g_heartBeatInterval = 5; //心跳包发送间隔(s)，需与服务器保持一致
uint16_t g_maxHeartBeatDelay = 3; //心跳包容许延迟时间(s)
const bool g_ignoreCalledOffline = false; //是否忽略被叫离线

systemStatus g_systemStatus = SystemIdle;
QHostAddress g_serverIp;
quint16 g_registerPort; //服务器接收注册信息的端口号

quint16 g_msgPort;                   //服务器接收数据的端口号
int g_registerStatus = RegisterStatus_None;
bool g_isCaller = false;  //当前是否为主叫方

uint16_t g_robotCallId = 0; //机器人视频通话ID
uint16_t g_robotControlId = 0;//机器人远程控制ID

Ui::MainWindow *g_ui;


/***** 跨线程传递变量   ****/

//QImage

