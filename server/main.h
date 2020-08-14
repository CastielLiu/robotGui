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
#define SERVER_ID     0

// 传输消息类型
enum dataType
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


#pragma pack(push,1)
typedef struct TransPack
{
    uint8_t head[2];
    uint16_t length;
    uint8_t type;
    uint8_t checkNum;

    uint16_t senderId;
    uint16_t receiverId;
    uint16_t seq; 

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
	int fd; //ÓëžÃ¿Í»§¶ËœšÁ¢Á¬œÓµÄsocketÌ×œÓ×Ö 
	        //ËäÈ»ÒÑŸ­±£ŽæÁË¿Í»§¶ËµÄµØÖ·£¬µ«ÊÇžÃµØÖ·Ö»¶Ôµ±Ê±ÓëÆäœšÁ¢Á¬œÓµÄsocketÓÐÐ§
			//µ±ÆäËûÓÃ»§ÏòŽËÓÃ»§·¢ËÍÏûÏ¢Ê±£¬·þÎñÆ÷±ØÐëÊ¹ÓÃŽËfdœøÐÐ×ª·¢£¬¶ø²»ÊÇÊ¹ÓÃœÓÊÕÆäËû¿Í»§ÏûÏ¢µÄfd 
	sockaddr_in addr; //¿Í»§¶ËµØÖ· 
	bool connect; //¿Í»§¶ËµÄÁ¬œÓ×ŽÌ¬(ÊÇ·ñÔÚÏß) 
	std::time_t lastHeartBeatTime; //ÉÏÒ»ŽÎÐÄÌøÊ±Œä
	uint16_t callingID; //ÕýÔÚÍš»°µÄID 
	ClientInfo()
	{
		lastHeartBeatTime=0;
		callingID = 0; 
	}
	 
} clientInfo_t;  

//Ê¹ÓÃmapŽæ·Å¿Í»§¶ËÐÅÏ¢ÒÔŒ°¶ÔÓŠµÄid 
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
	void receiveAndTransThread(int server_fd, uint16_t clientId);
	
	void heartBeatThread();
	void printThread(int interval); 
	void msgTransmit(const uint8_t* buf, int len);
	void cmdAndStatusTransmit(const uint8_t* buf, int len);
	void removeClient(uint16_t id); 
	
private:
	clientsMap clients_;
	
	//¿Í»§¶Ë×¢²á¶Ë¿ÚºÅ 
	const int register_port_; 
	
	uint16_t heartBeatInterval_ = 5; //ÐÄÌø°ü·¢ËÍŒäžô(s)
	uint16_t maxHeartBeatDelay_ = 3; //ÐÄÌø°üÈÝÐíÑÓ³ÙÊ±Œä(s)
	
};



#endif 
