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
#include "enums.h"
#include "structs.h"

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

extern Ui::MainWindow *g_ui;

#endif // GLOBAL_VARIABLE_H
