#include "main.h"

using std::string;
using std::cout;
using std::endl;

//���߳�����ת��
/*
����1. �������ת���̣߳������߳�Ψһ. ÿ�������̶߳�����Ӧһ�����ݽ��ջ����������������������������ս����յ����ݲ��洢������x��ʱ�����߳�x����ת��
��ǰ���ݽ��ջ�����Ӧ���к���ѡ������ʹ�õĻ�����Ӧ���ñ�־λ���Ա�־λ���ж�дʱӦ����
������Գ�ʼ��N���̣߳����̲߳���ʹ��ʱ�ٶ�̬����
 **����1 ��һ���߳̽������ݶ���߳�ת�����ݣ�һ���̶��Ͽ������Ч��**
����2. ����̣߳�ÿ���߳̾����շ������ڶ����ͬһ��ַ�˿ڶԵ�socketֻ�е�һ�󶨵Ŀ��Խ������ݣ����Ӧ���ò�ͬ�̲߳�ͬ�˿ںŵķ�ʽ
	�������ṩ������ն˿ںţ�����һ������ע��˿ںţ����ͻ������ӷ�����ʱ���ʵĶ˿ںţ�
	�������յ�ע����Ϣ������һ���µ��̲߳������µ�socket��ͻ��˷���ע��ɹ���Ϣ������ʱϵͳ��������һ���µĶ˿ںţ�
	�ͻ���ע��ɹ�����ӽ��ͻ��б���ʽ���ע�ᵼ�����������̡߳� 
	֮����û����÷���������Ķ˿ںŽ�����������
 */

bool Server::run_flag = true; 

Server::Server(int port):
	register_port_(port)
{

}

Server::~Server()
{

}

//��ʼ��socket���ؾ��
//ipΪ����ip���˿�Ĭ��Ϊ0����ϵͳ�Զ����� 
int Server::initSocket(const int port, const std::string ip, int time_out)
{
	struct sockaddr_in local_addr;
	bzero(&local_addr,sizeof(local_addr));//init 0
	
	local_addr.sin_port = htons(port);
	local_addr.sin_family = AF_INET;
	int convert_ret = inet_pton(AF_INET, ip.c_str(), &local_addr.sin_addr);
	if(convert_ret !=1)
	{
		perror("convert socket ip failed, please check the format!");
		return -1;
	}

	int fd = socket(PF_INET,SOCK_DGRAM , 0);
	if(fd < 0)
	{
		perror("build socket error");
		return -1;
	}
	int udp_opt = 1;
	// ���õ�ַ�ɸ��� 
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &udp_opt, sizeof(udp_opt));
	if(time_out)
	{
		struct timeval timeout;
	    timeout.tv_sec = time_out;//��
	    //timeout.tv_usec = 0;//΢��
	    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
	}
	
	int ret = bind(fd, (struct sockaddr*)&local_addr,sizeof(local_addr));
	if(ret < 0)
	{
		std::cout << "udp bind ip: "<< ip << "\t,port: "<< port << " failed!!" << std:: endl;
		return -1;
	}
	return fd;
} 

//��ӡsocket��ַ�Լ��˿ں���Ϣ 
void showSocketMsg(const std::string& prefix, int fd)
{
	struct sockaddr_in serverAddr;
	socklen_t server_len; // = sizeof(sockaddr_in);
	//��ȡsocket��Ϣ, ip,port.. 
	getsockname(fd,  (struct sockaddr *)&serverAddr, &server_len);
	char ip[16];
	inet_ntop(AF_INET,&serverAddr.sin_addr,ip,server_len);
	cout << prefix << "\t ip: " << ip << "\t port: " << serverAddr.sin_port << endl;
}

int Server::initSocketAutoAssignPort(uint16_t& port)
{
	struct sockaddr_in local_addr;
	bzero(&local_addr,sizeof(local_addr));//init 0
	local_addr.sin_family = AF_INET;
	int convert_ret = inet_pton(AF_INET, "0.0.0.0", &local_addr.sin_addr);
	
	for(port=50000;port<60000; ++port)
	{
		local_addr.sin_port = htons(port);
		int fd = socket(PF_INET,SOCK_DGRAM , 0);
		int ret = bind(fd, (struct sockaddr*)&local_addr,sizeof(local_addr));
		if(ret == 0)
		{
			cout << "AutoAssignPort: " << port << endl;
			return fd;
		}
	}
}

//���տͻ���ע����Ϣ���߳� 
void Server::receiveRegisterThread()
{
	int register_fd = initSocket(register_port_); //��ʼ��ע��socket 
	showSocketMsg("register sockect ",register_fd);
	
	const int BufLen =20;
	uint8_t *recvbuf = new uint8_t [BufLen];
	const transPack_t *pkg = (const transPack_t *)recvbuf;
	struct sockaddr_in client_addr;
	socklen_t clientLen = sizeof(client_addr);
	
	while(run_flag)
	{
		int len = recvfrom(register_fd, recvbuf, BufLen,0,(struct sockaddr*)&client_addr, &clientLen);
		if(len <=0 )
			continue;
		if(recvbuf[0] != 0x66 || recvbuf[1] != 0xcc)
			continue;
		if(pkg->type != RequestRegister)
			continue;
		
		// �յ��ͻ�������ע�����Ϣ 
		uint16_t clientId = pkg->senderId;
		auto it = clients_.find(clientId);
		if (it != clients_.end()) //���ҵ�Ŀ��ͻ��� ,�����Ѿ�ע��
			continue; 
		
		cout << "receiveRegisterMsg, client id:  " << pkg->senderId << "\t msg len:" << len << endl;
		
		clientInfo_t client;
		client.addr = client_addr;
		client.connect = false; //��ʱ�ͻ��������˻�δ�������������� 
		clients_[clientId] = client; //��ע��Ŀͻ�������map 
		//Ϊ��ע��Ŀͻ����´���һ�������׽���
		uint16_t new_port;
		int server_fd = initSocketAutoAssignPort(new_port);
		
		//��ͻ��˻�ӦΪ�������¶˿ں���Ϣ 
		transPack_t pkg;
		int headerLen = sizeof(transPack_t); 
		pkg.length = 2;
		pkg.type = ResponseRegister;
		char *buf = new char[headerLen+pkg.length];
		memcpy(buf, &pkg, headerLen);

		buf[headerLen] = new_port%256;
		buf[headerLen+1] = new_port/256;
		sendto(register_fd, buf, headerLen+pkg.length, 0, 
				(struct sockaddr*)&client_addr, sizeof(sockaddr_in));
		
		//�������պ�ת���߳� 
		std::thread t(&Server::receiveAndTransThread,this,server_fd);
		t.detach();
	}
	
	delete [] recvbuf;
}

//���տͻ�����Ϣ������ת�����߳� 
//�ͻ��˶Ͽ����Ӻ�ر��߳� 
void Server::receiveAndTransThread(int server_fd)
{
	const int BufLen1 =2*sizeof(transPack_t);
	uint8_t *recvbuf = new uint8_t [BufLen1];
	const transPack_t *pkg = (const transPack_t *)recvbuf;
	struct sockaddr_in client_addr;
	socklen_t clientLen = sizeof(client_addr);
	uint16_t clientId;
	while(run_flag)
	{
		//cout << "new thread start to receive msgs..." << endl;
		int len = recvfrom(server_fd, recvbuf, BufLen1,0,(struct sockaddr*)&client_addr, &clientLen);
		cout << "new thread received msgs. length: " << len << endl;
		if(recvbuf[0] != 0x66 || recvbuf[1] != 0xcc || pkg->type != RequestRegister)
			continue;
		clientId = pkg->senderId;
		clients_[clientId].connect = true; //���ӳɹ�
		clients_[clientId].addr = client_addr; //д��ͻ��˵�ַ
		clients_[clientId].fd =  server_fd; //������û��������ӵ��׽��ֱ��� 
		//����ע��ɹ�
		transPack_t temp_pkg;
		temp_pkg.type = RegisterOK;
		//���Ͷ��ע��ɹ��ź� 
		for(int i=0; i<3; ++i)
		{
			cout << "clientId: "<< clientId << " register ok.  addr " << clients_[clientId].addr.sin_port << endl; 
			sendto(server_fd,(char*)&temp_pkg, sizeof(temp_pkg), 0, (struct sockaddr*)&client_addr, sizeof(sockaddr_in));
			usleep(50000);
		}
		break;
	}
	 
	delete [] recvbuf;
	const int BufLen2 = 100000;
	recvbuf = new uint8_t [BufLen2];
	const transPack_t *_pkg = (const transPack_t *)recvbuf;
	
	//����Ϊ�������������ó�ʱʱ�� 
	struct timeval timeout;
    timeout.tv_sec = 1;//��
    //timeout.tv_usec = 0;//΢��
    setsockopt(server_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
	    
	while(run_flag && clients_[clientId].connect)
	{
		int len = recvfrom(server_fd, recvbuf, BufLen2,0,(struct sockaddr*)&client_addr, &clientLen);
		//cout << "received msgs len:" << len  << "  type: " << int(_pkg->type) << endl;
		if(len <= 0) continue;
		if(_pkg->head[0] != 0x66 || _pkg->head[1] != 0xcc)
			continue;
		if(_pkg->type == HeartBeat)
		{
			sendto(server_fd,recvbuf, len, 0, (struct sockaddr*)&client_addr, clientLen);
			clients_[clientId].lastHeartBeatTime = time(0); //��¼�ͻ�����ʱ�� 
    
		}
		else if(_pkg->type == Video || _pkg->type == Audio)
			msgTransmit(recvbuf, len);
			
	}
	cout << "delete client : " << clientId;
	clients_.erase(clientId);
	delete [] recvbuf;
	close(server_fd);
}

void Server::run()
{
	std::thread t1 = std::thread(&Server::printThread,this,5);
	t1.detach();
	
	std::thread t2 = std::thread(&Server::heartBeatThread,this);
	t2.detach(); 
	
	//�½����տͻ���ע����Ϣ���߳� 
	std::thread t = std::thread(&Server::receiveRegisterThread,this);
	t.join();
}

void Server::msgTransmit(const uint8_t* buf, int len)
{
	uint16_t dstClientId = ((const transPack_t *)buf)->receiverId;

	auto it = clients_.find(dstClientId);
	if (it == clients_.end()) //δ���ҵ�Ŀ��ͻ��� 
	{
		cout << "No client : " << dstClientId << endl;
		return;
	}
	int send_len = sendto(clients_[dstClientId].fd, buf, len, 0, (struct sockaddr*)&clients_[dstClientId].addr, sizeof(sockaddr_in));
	cout << "transmitting : " << send_len << " bytes to id: " << dstClientId << "\tport��" << clients_[dstClientId].addr.sin_port << endl;
}

//�����������������Զ��Ͽ������� 
//��ʱ���ն˴�ӡ���ݵ��̣߳����ַ�����һֱ���ڻ���״̬
void Server::printThread(int interval)
{
	int i=0;
	while(run_flag)
	{
		sleep(interval);
		printf("this server has been running for %d seconds.\n",interval*(++i));
	}
}

void Server::heartBeatThread()
{
	int heartBeatInterval = 5; //�������5s 
	while(run_flag)
	{
		cout << "clients size: " << clients_.size() << "\t sending heart beat pkg...\n";
		for(auto client =clients_.begin(); client!= clients_.end(); ++client)
		{
			if(client->second.lastHeartBeatTime ==0)
			    continue;
			if(time(0) - client->second.lastHeartBeatTime >heartBeatInterval+1)
			{
				client->second.connect = false;
				cout << "client " << client->first  << "  disconnect." << endl;
			}
		}
		std::this_thread::sleep_for(std::chrono::seconds(heartBeatInterval)); 
	}

}

//ϵͳ�ж��źŲ���
void sigint_handler(int signal_num)
{
	//std::cout << "signal_num: " << signal_num << std::endl;
	Server::run_flag = false;
	usleep(100000); //Ԥ��ʱ�������߳� 
	exit(0);
}

int main(int argc,char** argv)
{
	signal(SIGINT, sigint_handler);
	int port = 8617;
	if(argc > 1)
		port = atoi(argv[1]);
	Server server(port);
	
	server.run();
 
	return 0;
}

// scp * root@aliyun:/root/seu/wendao/remote_driverless_server
