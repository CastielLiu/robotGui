#ifndef MAIN_H_
#define MAIN_H_
#include <string>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <thread>
#include <mutex>
#include <iostream>
#include <map>

#include <signal.h>

#define ROBOT_TEST_ID 5050

enum dataType
{
    PkgType_Video=0,          //视频消息
    PkgType_Audio=1,          //语音
    PkgType_RequestConnect=2, //请求连接
    PkgType_AcceptConnect=3,  //接收连接
    PkgType_RefuseConnect=4,  //拒绝连接
    PkgType_DisConnect=5,     //挂断连接
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
	PkgType_ControlCmd = 20,
	PkgType_RobotState = 21,
	
	
};

#pragma pack(push,1)
typedef struct TransPack
{
    uint8_t head[2];
    uint16_t length;
    uint8_t type;
    uint8_t checkNum;

    uint16_t senderId;
    uint16_t receiverId;

    TransPack(dataType t = PkgType_NoneType)
    {
        head[0] = 0x66;
        head[1] = 0xcc;
        type = t;
        length = checkNum = 0;
    }
} transPack_t;

#pragma pack(pop)

typedef struct ClientInfo 
{
	int fd; //与该客户端建立连接的socket套接字 
	        //虽然已经保存了客户端的地址，但是该地址只对当时与其建立连接的socket有效
			//当其他用户向此用户发送消息时，服务器必须使用此fd进行转发，而不是使用接收其他客户消息的fd 
	sockaddr_in addr; //客户端地址 
	bool connect; //客户端的连接状态(是否在线) 
	std::time_t lastHeartBeatTime; //上一次心跳时间
	uint16_t callingID; //正在通话的ID 
	ClientInfo()
	{
		lastHeartBeatTime=0;
		callingID = 0; 
	}
	 
} clientInfo_t;  

//使用map存放客户端信息以及对应的id 
typedef std::map<uint16_t,clientInfo_t> clientsMap;

class Server
{
public:
	Server(int port);
	~Server();
	void run();
	
	static bool run_flag;
	
private:
	int initSocket(const int port=0, const std::string ip = "0.0.0.0", int time_out=0); 
	int initSocketAutoAssignPort(uint16_t& port);
	void receiveRegisterThread();
	void receiveAndTransThread(int server_fd); 
	
	void heartBeatThread();
	void printThread(int interval); 
	void msgTransmit(const uint8_t* buf, int len);
	void removeClient(uint16_t id); 
	
private:
	clientsMap clients_;
	
	//客户端注册端口号 
	const int register_port_; 
	
	uint16_t heartBeatInterval_ = 5; //心跳包发送间隔(s)
	uint16_t maxHeartBeatDelay_ = 2; //心跳包容许延迟时间(s)
	
};



#endif 
