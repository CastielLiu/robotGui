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
    Vedio=0,          //��Ƶ��Ϣ
    Audio=1,          //����
    RequestConnect=2, //��������
    AcceptConnect=3,  //��������
    RefuseConnect=4,  //�ܾ�����
    DisConnect=5,     //�Ҷ�����
    Register=6,       //ע�ᵽ������
    RegisterOK=7,     //ע��ɹ�(�����������ͻ���)
	RegisterFail=8,   //ע��ʧ��(�����������ͻ���) 

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
    
    TransPack()
    {
        head[0] = 0x66;
        head[1] = 0xcc;
        length = type = checkNum = 0;
    }
    
} transPack_t;

#pragma pack(pop)

typedef struct
{
	sockaddr_in addr;
	bool connect; //�ͻ��˵�����״̬ 
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
	int initSocket(const int port=0, const std::string ip = "0.0.0.0");
	void receiveRegisterThread();
	void receiveAndTransThread(uint16_t clientId);
	
	void printThread(int interval); 
	void msgTransmit(int fd, const uint8_t* buf, int len);
	
private:
	clientsMap clients_;
	
	//�ͻ���ע��˿ں� 
	const int register_port_; 
	int register_fd_; // ע��socket�׽��� 
	

};



#endif 
