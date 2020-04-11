#ifndef ENUMS_H_
#define ENUMS_H_

#include<iostream>


//客户端注册状态 0未注册，1注册中，2已注册
enum RegisterStatus
{
    RegisterStatus_None = 0, //未注册
    RegisterStatus_Ing = 1, //注册中
    RegisterStatus_Ok = 2, //已注册
};

// 传输消息类型
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

//系统状态
enum systemStatus
{
    SystemIdle,       //空闲
    SystemOnThePhone, //正在通话
    SystemDebug,     //正在调试
    SystemRunning,  //正在通话
    SystemRefused,  //请求被拒绝
    SystemAccepted, //请求被接受
};

//stackWidget index

enum stackWidgetIndex
{
    stackWidget_MainPage = 0, //主页面
    stackWidget_DebugPage =1, //调试页面
    stackWidget_BioRadarPage = 2, //生物雷达界面
};

#endif // ENUMS_H_
