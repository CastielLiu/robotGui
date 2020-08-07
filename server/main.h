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

enum dataType
{
	PkgType_NoneType        = 0,
	
	//�ͻ���ע�������Ϣ���� 
	PkgType_RegisterOK      = 1, //ע��ɹ�(�����������ͻ���)
	PkgType_RegisterFail    = 2, //ע��ʧ��(�����������ͻ���)
	PkgType_RequestRegister = 3, //����ע�ᵽ������
	PkgType_ResponseRegister= 4, //��Ӧ�ͻ�������(��������˷���˿ں�) 
	PkgType_repeatLogin     = 5, //�ظ���¼
	PkgType_LogOut          = 6, //�˳���½
	
	//�ͻ����������������Ϣ���� 
	PkgType_RequestConnect  = 20, //��������
    PkgType_AcceptConnect   = 21, //��������
    PkgType_RefuseConnect   = 22, //�ܾ�����
    PkgType_DisConnect      = 23, //�Ҷ�����
    PkgType_CalledOffline   = 24, // ���в�����
	PkgType_CalledBusy      = 25, //����æ 
    
    //��������Ϣ���� 
    PkgType_HeartBeat       = 50, //������
	
	//ʵʱ���������Ϣ���� 
    PkgType_Video           = 81, //��Ƶ��Ϣ
    PkgType_Audio           = 82, //����
    PkgType_BoilogicalRadar = 83, //�����״� 

	//Զ�̿��������Ϣ���� 
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
	int fd; //��ÿͻ��˽������ӵ�socket�׽��� 
	        //��Ȼ�Ѿ������˿ͻ��˵ĵ�ַ�����Ǹõ�ַֻ�Ե�ʱ���佨�����ӵ�socket��Ч
			//�������û�����û�������Ϣʱ������������ʹ�ô�fd����ת����������ʹ�ý��������ͻ���Ϣ��fd 
	sockaddr_in addr; //�ͻ��˵�ַ 
	bool connect; //�ͻ��˵�����״̬(�Ƿ�����) 
	std::time_t lastHeartBeatTime; //��һ������ʱ��
	uint16_t callingID; //����ͨ����ID 
	ClientInfo()
	{
		lastHeartBeatTime=0;
		callingID = 0; 
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
	void receiveAndTransThread(int server_fd, uint16_t clientId);
	
	void heartBeatThread();
	void printThread(int interval); 
	void msgTransmit(const uint8_t* buf, int len);
	void cmdAndStatusTransmit(const uint8_t* buf, int len);
	void removeClient(uint16_t id); 
	
private:
	clientsMap clients_;
	
	//�ͻ���ע��˿ں� 
	const int register_port_; 
	
	uint16_t heartBeatInterval_ = 5; //���������ͼ��(s)
	uint16_t maxHeartBeatDelay_ = 3; //�����������ӳ�ʱ��(s)
	
};



#endif 
