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
    PkgType_NoneType        = 0,

    //客户端注册相关消息类型
    PkgType_RegisterOK      = 1, //注册成功(服务器发往客户端)
    PkgType_RegisterFail    = 2, //注册失败(服务器发往客户端)
    PkgType_RequestRegister = 3, //请求注册到服务器
    PkgType_ResponseRegister= 4, //回应客户端请求(包含服务端服务端口号)
    PkgType_repeatLogin     = 5, //重复登录
    PkgType_LogOut          = 6, //退出登陆

    //客户端连接请求相关消息类型
    PkgType_RequestConnect  = 20, //请求连接
    PkgType_AcceptConnect   = 21, //接收连接
    PkgType_RefuseConnect   = 22, //拒绝连接
    PkgType_DisConnect      = 23, //挂断连接
    PkgType_CalledOffline   = 24, // 被叫不在线
    PkgType_CalledBusy      = 25, //被叫忙

    //心跳包消息类型
    PkgType_HeartBeat       = 50, //心跳包

    //实时传输相关消息类型
    PkgType_Video           = 81, //视频消息
    PkgType_Audio           = 82, //语音
    PkgType_BoilogicalRadar = 83, //生物雷达

    //远程控制相关消息类型
    PkgType_ControlCmd      = 101,
    PkgType_RobotState      = 102,

};

//远程传输状态
enum transferStatus
{
    transferStatus_Ing,     //正在传输
    transferStatus_Idle,    //传输空闲
    transferStatus_Stoping, //正在停止传输
    transferStatus_Starting,//正确启动传输
};

//stackWidget index

enum stackWidgetIndex
{
    stackWidget_ChatPage = 0, //通话页面
    stackWidget_DebugPage =1, //调试页面
    stackWidget_BioRadarPage = 2, //生物雷达界面
    stackWidget_WorkLogPage = 3, //工作日志
    stackWidget_HomePage = 4,   //Home
    stackWidget_TransportPage = 5, //送物导航界面
    stackWidget_MapsPage      = 6, //maps
};

#endif // ENUMS_H_
