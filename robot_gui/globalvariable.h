#ifndef GLOBAL_VARIABLE_H
#define GLOBAL_VARIABLE_H
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
#include <QImage>
#include <QMutex>
#include "enums.h"
#include "structs.h"

extern bool g_canCalled;
extern bool g_ignoreCalledOffline;
extern bool g_isRemoteTerminal;

extern QHostAddress g_serverIp;
extern quint16 g_registerPort;
extern int g_registerStatus;
extern quint16 g_msgPort;
extern systemStatus g_systemStatus;

extern uint16_t g_myId;
extern uint16_t g_robotCallId;
extern uint16_t g_robotControlId ;

extern uint16_t g_heartBeatInterval ;
extern uint16_t g_maxHeartBeatDelay ;

extern Ui::MainWindow *g_ui;

extern std::shared_ptr<QImage> g_myImage;   //我方图像
extern QMutex g_myImageMutex;
extern std::shared_ptr<QImage> g_otherImage;//对方图像
extern QMutex g_otherImageMutex;

extern bool g_isOpenVedio;
extern bool g_isOpenAudio;

#endif // GLOBAL_VARIABLE_H
