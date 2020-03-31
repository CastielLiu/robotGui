#ifndef UTILS_H
#define UTILS_H

#include "ui_mainwindow.h"

#include<iostream>
#include<sstream>
#include<stdint.h>
#include<cstring>
#include <QObject>
#include <QtNetwork/QUdpSocket>
#include <QHostAddress>
#include <QThread>
#include <thread>

//客户端注册状态 0未注册，1注册中，2已注册
enum RegisterStatus
{
    RegisterStatus_None = 0, //未注册
    RegisterStatus_Ing = 1, //注册中
    RegisterStatus_Ok = 2, //已注册
};

// 消息类型
enum PkgType
{
    PkgType_Video=0,          //视频消息
    PkgType_Audio=1,          //语音
    PkgType_RequestConnect=2, //请求连接
    PkgType_AcceptConnect=3,  //接收连接
    PkgType_RefuseConnect=4,  //拒绝连接
    PkgType_DisConnect=5,     //挂断
    PkgType_NoneType =6,
    PkgType_RegisterOK=7,     //注册成功(服务器发往客户端)
    PkgType_RegisterFail=8,   //注册失败(服务器发往客户端)
    PkgType_RequestRegister=9,//请求注册到服务器
    PkgType_ResponseRegister=10,//回应客户端请求(包含服务端服务端口号)
    PkgType_HeartBeat = 11, //心跳包
    PkgType_LogOut = 12,    //退出登陆
    PkgType_CalledOffline=13,// 被叫不在线
    PkgType_CalledBusy = 14,//被叫忙

    //remote control
    PkgType_ControlCmd = 20, //控制指令
    PkgType_RobotState = 21, //机器人状态数据
};

//数据传输头
#pragma pack(push,1)
typedef struct PkgHeader
{
    uint8_t head[2];
    uint16_t length;
    uint8_t type;
    uint8_t checkNum;

    uint16_t senderId;
    uint16_t receiverId;

    PkgHeader(PkgType t = PkgType_NoneType)
    {
        head[0] = 0x66;
        head[1] = 0xcc;
        type = t;
        length = checkNum = 0;
    }
} pkgHeader_t;

#pragma pack(pop)

enum systemStatus
{
    SystemIdle,       //空闲
    SystemOnThePhone, //正在通话
    SystemDebug,     //正在调试
    SystemRunning,  //正在通话
    SystemRefused,  //请求被拒绝
    SystemAccepted, //请求被接受
};


extern QHostAddress g_serverIp;
extern quint16 g_registerPort;
extern int g_registerStatus;
extern quint16 g_msgPort;
extern systemStatus g_systemStatus;
extern bool g_isCaller;
extern const bool g_canCalled;
extern const bool g_openRemoteControl;
extern const bool g_ignoreCalledOffline;

extern uint16_t g_myId;
extern uint16_t g_robotCallId;
extern uint16_t g_robotControlId ;

extern uint16_t g_heartBeatInterval ;
extern uint16_t g_maxHeartBeatDelay ;

//extern bool
extern Ui::MainWindow *g_ui;

int ipConvert(const std::string& ip_str);
std::string ipConvert(const int ip_int);


#endif // UTILS_H
