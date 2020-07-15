#ifndef GLOBAL_VARIABLE_H
#define GLOBAL_VARIABLE_H

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
#include <QSize>
#include "enums.h"
#include "structs.h"

#define HIGH_LINUX_VERSION 1

extern bool g_canCalled;
extern bool g_ignoreCalledOffline;
extern bool g_isRemoteTerminal;
extern bool g_autoConnect;

extern QHostAddress g_serverIp;
extern quint16 g_registerPort;
extern int g_registerStatus;
extern quint16 g_msgPort;
extern transferStatus g_transferStatus;

extern uint16_t g_myId;
extern uint16_t g_calledId;
extern uint16_t g_robotControlId ;
extern int g_cameraId;
extern QSize g_cameraResolution;

extern uint16_t g_heartBeatInterval ;
extern uint16_t g_maxHeartBeatDelay ;

extern std::shared_ptr<QImage> g_myImage;   //我方图像
extern QMutex g_myImageMutex;
extern std::shared_ptr<QImage> g_otherImage;//对方图像
extern QMutex g_otherImageMutex;

extern bool g_isOpenVedio;
extern bool g_isOpenAudio;

extern QString g_appDir;

#endif // GLOBAL_VARIABLE_H
