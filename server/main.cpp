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
	register_fd_ = -1;
}

Server::~Server()
{
	if(register_fd_ != -1)
		close(register_fd_);
}

//��ʼ��socket���ؾ��
//ipΪ����ip���˿�Ĭ��Ϊ0����ϵͳ�Զ����� 
int Server::initSocket(const int port, const std::string ip)
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
	
	if(port == 0) //���portΪĬ��ֵ�����󶨶˿ڣ����������� 
		return fd;
	
	int ret = bind(fd, (struct sockaddr*)&local_addr,sizeof(local_addr));
	if(ret < 0)
	{
		std::cout << "udp bind ip: "<< ip << "\t,port: "<< port << " failed!!" << std:: endl;
		return -1;
	}
	return fd;
 } 


//���տͻ���ע����Ϣ���߳� 
void Server::receiveRegisterThread()
{
	const int BufLen =20;
	uint8_t *recvbuf = new uint8_t [BufLen];
	const transPack_t *pkg = (const transPack_t *)recvbuf;
	struct sockaddr_in client_addr;
	socklen_t clientLen = sizeof(client_addr);
	
	while(run_flag)
	{
		int len = recvfrom(register_fd_, recvbuf, BufLen,0,(struct sockaddr*)&client_addr, &clientLen);
		if(recvbuf[0] != 0x66 || recvbuf[1] != 0xcc)
			continue;
		if(pkg->type != Register)
			continue;
		
		cout << "receiveRegisterMsg, client id:  " << pkg->senderId << "\t msg len:" << len << endl;
		
		uint16_t clientId = pkg->senderId;
		auto it = clients_.find(clientId);
		if (it != clients_.end()) //���ҵ�Ŀ��ͻ��� ,�����Ѿ�ע��
			continue; 
		clientInfo_t client;
		client.addr = client_addr;
		client.connect = true;
		clients_[clientId] = client; //��ע��Ŀͻ�������map 
		
		//�������պ�ת���߳� 
		std::thread t(&Server::receiveAndTransThread,this,clientId);
		t.detach();
	}
}

//���տͻ�����Ϣ������ת�����߳� 
void Server::receiveAndTransThread(uint16_t clientId)
{
	//���Ƚ����µ�socket�����ͽ���ע��ָ��
	//�ͻ����յ�����ע��ָ����¼���������¶˿ںţ����ô˶˿ںŽ������ݷ���
	int fd = initSocket(8618);
	if(fd < 0)
	{
		clients_[clientId].connect = false;
		return;
	}
	
	//����ע��ɹ�
	transPack_t temp_pkg;
	temp_pkg.type = RegisterOK;
	
	//���Ͷ��ע��ɹ��ź� 
	for(int i=0; i<3; ++i)
	{
		cout << "clientId: "<< clientId << " register ok.  addr " << clients_[clientId].addr.sin_port << endl; 
		sendto(fd,(char*)&temp_pkg, sizeof(temp_pkg), 0, (struct sockaddr*)&clients_[clientId].addr, sizeof(sockaddr_in));
		usleep(300000);
	}
	
	
	const int BufLen =100000;
	uint8_t *recvbuf = new uint8_t [BufLen];
	const transPack_t *pkg = (const transPack_t *)recvbuf;
	struct sockaddr_in client_addr;
	socklen_t clientLen = sizeof(client_addr);
	
	while(clients_[clientId].connect)
	{
		int len = recvfrom(fd, recvbuf, BufLen,0,(struct sockaddr*)&client_addr, &clientLen);
		
		if(pkg->head[0] != 0x66 || pkg->head[1] != 0xcc)
			continue;
		//uint8_t type = pkg.type();
	
		msgTransmit(fd, recvbuf, len);
	}
	delete [] recvbuf;
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

void Server::run()
{
	std::thread t1 = std::thread(&Server::printThread,this,5);
	t1.detach();
	
	register_fd_ = initSocket(register_port_); //��ʼ��ע��socket 
	if(register_fd_ < 0)
		return; 
	
	//�½����տͻ���ע����Ϣ���߳� 
	std::thread t = std::thread(&Server::receiveRegisterThread,this);
	t.join();
}

void Server::msgTransmit(int fd, const uint8_t* buf, int len)
{
	uint16_t dstClientId = buf[9]*256 + buf[8];
	auto it = clients_.find(dstClientId);
	if (it == clients_.end()) //δ���ҵ�Ŀ��ͻ��� 
	{
		cout << "No client : " << dstClientId << endl;
		return;
	}
	int send_len = sendto(fd, buf, len, 0, (struct sockaddr*)&clients_[dstClientId].addr, sizeof(sockaddr_in));
	cout << "transmitting : " << send_len << " bytes to id: " << dstClientId << "\tport��" << clients_[dstClientId].addr.sin_port << endl;
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
