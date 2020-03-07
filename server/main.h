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

enum dataType
{
    Video=0,          //��Ƶ��Ϣ
    Audio=1,          //����
    RequestConnect=2, //��������
    AcceptConnect=3,  //��������
    RefuseConnect=4,  //�ܾ�����
    DisConnect=5,     //�Ҷ�����
    NoneType =6,
    RegisterOK=7,     //ע��ɹ�(�����������ͻ���)
	RegisterFail=8,   //ע��ʧ��(�����������ͻ���)
	RequestRegister=9,//����ע�ᵽ������
	ResponseRegister=10,//��Ӧ�ͻ�������(��������˷���˿ں�) 
    HeartBeat = 11, //������
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

    TransPack(dataType t = NoneType)
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
	int fd; //��ÿͻ��˽������ӵ�socket�׽��� 
	        //��Ȼ�Ѿ������˿ͻ��˵ĵ�ַ�����Ǹõ�ַֻ�Ե�ʱ���佨�����ӵ�socket��Ч
			//�������û�����û�������Ϣʱ������������ʹ�ô�fd����ת����������ʹ�ý��������ͻ���Ϣ��fd 
	sockaddr_in addr; //�ͻ��˵�ַ 
	bool connect; //�ͻ��˵�����״̬ 
	std::time_t lastHeartBeatTime; //��һ������ʱ��
	
	ClientInfo()
	{
		lastHeartBeatTime=0;
	}
	 
} clientInfo_t;  

//ʹ��map��ſͻ�����Ϣ�Լ���Ӧ��id 
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
	
private:
	clientsMap clients_;
	
	//�ͻ���ע��˿ں� 
	const int register_port_; 
	
	

};



#endif 
