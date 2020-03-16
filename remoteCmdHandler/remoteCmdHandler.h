#ifndef REMOTECMDHANDLER
#define  REMOTECMDHANDLER

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "utils.h"
#include <thread>
#include <chrono>
#include <mutex>
#include <ctime>

#pragma pack(push,1)

typedef struct ControlCmdPkg
{
    transPack_t header;
    int8_t xSpeed; //前进后退的速度
    int8_t zSpeed; //左右旋转的速度

    ControlCmdPkg(bool isSender=false)
    {
        if(isSender)
        {
            header.type = ControlCmd;
            header.length = sizeof(ControlCmdPkg) - sizeof(transPack_t);
            header.senderId = g_myId;
        }
    }
} controlCmdPkg_t;
#pragma pack(pop)


class RemoteCmdHandler
{
public:
	RemoteCmdHandler();
	~RemoteCmdHandler();
	bool init();
	
private：
	int initSocket(const int port, const std::string ip, int time_out); 
	bool registerToServer();
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
    int m_heartBeatInterval; //心跳间隔 
    int m_maxHeartBeatDelay; //最长心跳延迟 
};


#endif
