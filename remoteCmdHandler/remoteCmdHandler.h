#ifndef REMOTECMDHANDLER
#define  REMOTECMDHANDLER

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <iostream>
#include "utils.h"
#include <thread>
#include <chrono>
#include <mutex>
#include <ctime>

#pragma pack(push,1)

typedef struct ControlCmd
{
    int8_t xSpeed; //前进后退的速度
    int8_t zSpeed; //左右旋转的速度
}controlCmd_t;

typedef struct  ControlCmdPkg
{
    pkgHeader_t header;
    controlCmd_t cmd;

}controlCmdPkg_t;

#pragma pack(pop)

typedef void(*cmdCallBack_t)(controlCmd_t);

class RemoteCmdHandler
{
public:
	RemoteCmdHandler();
	~RemoteCmdHandler();
	bool start();
	void stop();
	void bindCallbackFunction(cmdCallBack_t fun){m_cmd_callback = fun;}
	void setServerAddr(const std::string& ip, uint16_t port);
	void setRobotId(uint16_t id);
		
private: 
	void receiveThread(const int fd);
	void confirmRegister(const int fd, struct sockaddr_in addr);
	int initSocket(const int port=0, const std::string ip="0.0.0.0", int time_out=0);
	bool registerToServer(const int fd, struct sockaddr_in addr);
	void heartBeatThread(const int fd, struct sockaddr_in addr); 

private:
	int m_fd;
	std::string m_serverIp;
	uint16_t m_registerPort;
	
	struct sockaddr_in m_registerAddr;
	struct sockaddr_in m_msgAddr;
	
	uint16_t m_myId;
	bool m_isRegisterOk;
	bool m_runFlag;
	
	std::mutex m_heartBeatMutex;
    std::time_t m_serverLastHeartBeatTime; //服务器上次心跳时间 
    const int m_heartBeatInterval; //心跳间隔 
    const int m_maxHeartBeatDelay; //最长心跳延迟 
    const int m_registerTimeOut; //注册到服务器超时时间 
    cmdCallBack_t m_cmd_callback;
};


#endif
