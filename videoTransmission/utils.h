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

// 消息类型
enum dataType
{
    Vedio=0,          //视频消息
    Audio=1,          //语音
    RequestConnect=2, //请求连接
    AcceptConnect=3,  //接收连接
    RefuseConnect=4,  //拒绝连接
    DisConnect=5,     //挂断连接

    Register=6,       //注册到服务器

};

//数据传输头
#pragma pack(push,1)
typedef struct
{
    uint8_t head[2];
    uint16_t length;
    uint8_t type;
    uint8_t checkNum;

    uint16_t senderId;
    uint16_t receiverId;
    uint16_t listenerPort; //消息发送者的监听端口，期望消息接收者向此端口发送数据

} transPack_t;

#pragma pack(pop)

enum systemStatus
{
    SystemIdle,     //空闲
    SystemBusy,     //忙(正在呼叫、正在被叫)
    SystemRunning,  //正在通话
    SystemRefused,  //请求被拒绝
    SystemAccepted, //请求被接受
};

extern QHostAddress g_serverIp;
extern quint16 g_serverPort;
extern systemStatus g_systemStatus;
extern uint16_t g_myId;
extern Ui::MainWindow *g_ui;

int ipConvert(const std::string& ip_str);
std::string ipConvert(const int ip_int);


#endif // UTILS_H
