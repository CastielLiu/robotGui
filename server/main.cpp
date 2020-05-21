#include "main.h"

using std::string;
using std::cout;
using std::endl;

//���߳�����ת��
/*
	����̣߳�ÿ���߳̾����շ���
	�������ṩ������ն˿ںţ�����һ������ע��˿ںţ����ͻ������ӷ�����ʱ���ʵĶ˿ںţ�
	�������յ�ע����Ϣ������һ���µ��̲߳������µ�socket����ʱϵͳ��������һ���µĶ˿ںţ�
	���¶˿ں�ͨ��ע��socket���͸��ͻ��ˣ��ͻ����յ��¶˿ںź����¶˿ںŷ���ע��ָ���������Ӧ��ע��ɹ��� 
	�ͻ���ע��ɹ�����ӽ��ͻ��б���ֹ���ע�ᵼ�����������̡߳� 
	֮��ÿͻ�������������¶˿ںŽ������ݽ���
 */

//��̬���������б�־�������ж϶��߳� 
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
//time_out Ϊ�������յȴ�ʱ����Ĭ��Ϊ0 
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

//��ʼ��socket���Զ�����˿ںţ�����ֵΪ�׽���fd
//����Ϊ���ã�����д�����Ķ˿ں� 
//�˺�����ʼ��socketʱ�������ö˿ںŸ��ù��ܣ�ͬһ�˿ںű���ΰ󶨵���֮ǰ�󶨵��޷�ʹ�� 
int Server::initSocketAutoAssignPort(uint16_t& port)
{
	struct sockaddr_in local_addr;
	bzero(&local_addr,sizeof(local_addr));//init 0
	local_addr.sin_family = AF_INET;
	int convert_ret = inet_pton(AF_INET, "0.0.0.0", &local_addr.sin_addr);
	
	//���������˿ںţ�ֱ���󶨳ɹ� 
	for(port=50000;port<60000; ++port)
	{
		local_addr.sin_port = htons(port);
		int fd = socket(PF_INET,SOCK_DGRAM , 0);
		int ret = bind(fd, (struct sockaddr*)&local_addr,sizeof(local_addr));
		if(ret == 0)
		{
			cout << "auto assign port: " << port << endl;
			return fd;
		}
	}
}

//���տͻ���ע����Ϣ���߳� 
void Server::receiveRegisterThread()
{
	int register_fd = initSocket(register_port_); //��ʼ��ע��socket 
	//showSocketMsg("register sockect ",register_fd);
	
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
		if(pkg->type != PkgType_RequestRegister)
			continue;
		
		// �յ��ͻ�������ע�����Ϣ 
		uint16_t clientId = pkg->senderId;
		auto it = clients_.find(clientId);
		if (it != clients_.end()) //���ҵ�Ŀ��ͻ��� ,�����Ѿ�ע��
		{
			//cout << "client id: " << clientId << "has in map\n";
			transPack_t pkg(PkgType_repeatLogin);
			sendto(register_fd,(char*)&pkg, sizeof(pkg), 0, 
				  (struct sockaddr*)&client_addr, sizeof(sockaddr_in));
			continue; 
		}
			
		cout << "client id:  " << pkg->senderId  << " request register... " << endl;
		
		clientInfo_t client;
		client.addr = client_addr;
		client.connect = false; //��ʱ�ͻ��������˻�δ�������������� 
		clients_[clientId] = client; //��ע��Ŀͻ�������map 
		
		//cout << "write clientId: " << clientId << "to map\n";
		 	
		uint16_t new_port;
		
		//Ϊ��ע��Ŀͻ����´���һ�������׽���
		int server_fd = initSocketAutoAssignPort(new_port);
		//�������պ�ת���߳� 
		std::thread t(&Server::receiveAndTransThread,this,server_fd, clientId);
		t.detach();
		
		//��ͻ��˻�ӦΪ�������¶˿ں���Ϣ 
		transPack_t pkg(PkgType_ResponseRegister);
		int headerLen = sizeof(transPack_t); 
		pkg.length = 2;
		char *buf = new char[headerLen+pkg.length];
		memcpy(buf, &pkg, headerLen);

		buf[headerLen] = new_port%256;
		buf[headerLen+1] = new_port/256;
		sendto(register_fd, buf, headerLen+pkg.length, 0, 
				(struct sockaddr*)&client_addr, sizeof(sockaddr_in));
	}
	
	delete [] recvbuf;
}

//���տͻ�����Ϣ������ת�����߳� 
//�ͻ��˶Ͽ����Ӻ�ر��߳� 
void Server::receiveAndTransThread(int server_fd, uint16_t clientId)
{
	//���Ƚ��տͻ��˵�ȷ��ע����Ϣ��
	//ȷ��ע����ע����Ϣ����һ�£�
	//ע����Ϣ�ɿͻ��˷��͵��������Ĺ���ע��˿ں� 
	//ȷ��ע����Ϣ�ɿͻ��˷��͵�������Ϊ�ÿͻ��˷�����¶˿ں� 
	const int BufLen1 =2*sizeof(transPack_t);
	uint8_t *recvbuf = new uint8_t [BufLen1];
	const transPack_t *pkg = (const transPack_t *)recvbuf;
	struct sockaddr_in client_addr;
	socklen_t clientLen = sizeof(client_addr);
	
	//����Ϊ�������������ó�ʱʱ�� 
	struct timeval timeout;
    timeout.tv_sec = 10;//��
    //timeout.tv_usec = 0;//΢��
    setsockopt(server_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    
	cout << "new thread start to receive msgs..." << endl;
	int len = recvfrom(server_fd, recvbuf, BufLen1,0,(struct sockaddr*)&client_addr, &clientLen);
	if(len <=0 || //���ճ�ʱ 
	   recvbuf[0] != 0x66 || recvbuf[1] != 0xcc || //��ͷ���� 
	   pkg->type != PkgType_RequestRegister ||  //ָ����� ӦΪȷ��ע����Ϣ�� 
	   clientId != pkg->senderId ) //id��ƥ�� 
	{
		removeClient(clientId); //ɾ���û�
		return; 
	} 
	
	std::cout << "received confirm register msg." << std:: endl; 
	
	clients_[clientId].connect = true; //���ӳɹ�
	clients_[clientId].addr = client_addr; //д��ͻ��˵�ַ
	clients_[clientId].fd =  server_fd; //������û��������ӵ��׽��ֱ��� 
	//����ע��ɹ�
	transPack_t temp_pkg(PkgType_RegisterOK);

	//���Ͷ��ע��ɹ��ź� 
	for(int i=0; i<3; ++i)
	{
		sendto(server_fd,(char*)&temp_pkg, sizeof(temp_pkg), 0, (struct sockaddr*)&client_addr, sizeof(sockaddr_in));
		usleep(50000);
	}
	cout << "clientId: "<< clientId << " register ok ^-^ ^-^ ^-^ ^-^ ^-^ ^-^" << endl; 
	 
	delete [] recvbuf;
	const int BufLen2 = 100000;
	recvbuf = new uint8_t [BufLen2];
	const transPack_t *_pkg = (const transPack_t *)recvbuf;
	
	timeout.tv_sec = 1;//��
    setsockopt(server_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    
	while(run_flag && clients_[clientId].connect)
	{
		//�˴�����ӦΪ����������ʱ���� run_flag �Լ��û�������״̬
		//����1.�����޷������˳���2.�û��˳����ڵȴ��������� 
		int len = recvfrom(server_fd, recvbuf, BufLen2,0,(struct sockaddr*)&client_addr, &clientLen);
		
		if(len <= 0) continue;
		if(_pkg->head[0] != 0x66 || _pkg->head[1] != 0xcc)
			continue;
		
		//std::cout << "received msg, sender id:" << _pkg->senderId << " type: " << int(_pkg->type) << " len:" << len<< std::endl;
		
		if(_pkg->type == PkgType_HeartBeat) //������ 
		{
			cout << "received client heartbeat :" << clientId << endl;
			sendto(server_fd,recvbuf, len, 0, (struct sockaddr*)&client_addr, clientLen); //�ط����ͻ��� 
			clients_[clientId].lastHeartBeatTime = time(0); //��¼�ͻ�����ʱ�� 
		}
		else if(_pkg->type == PkgType_LogOut)
		{
			cout << "client logout...."<< endl;
			clients_[clientId].connect = false;
		}
		else if(_pkg->type == PkgType_AcceptConnect) //�����û��������� 
		{
			uint16_t srcClientId = ((const transPack_t *)recvbuf)->senderId;
			uint16_t dstClientId = ((const transPack_t *)recvbuf)->receiverId;
			clients_[srcClientId].callingID = dstClientId;
			clients_[dstClientId].callingID = srcClientId;
			cout << "connected " << srcClientId << "\t" << dstClientId << endl;
		}
		else if(_pkg->type == PkgType_RefuseConnect)
		{
			uint16_t dstClientId = ((const transPack_t *)recvbuf)->receiverId;
			sendto(clients_[dstClientId].fd, (char*)&pkg, sizeof(transPack_t), 0, (struct sockaddr*)&clients_[dstClientId].addr, sizeof(sockaddr_in));
		}
		else if(_pkg->type == PkgType_DisConnect)
		{
			uint16_t clientA =  clientId;
			//A������Ͽ������A�д�ŵ�B��Id������B���ͶϿ����� 
			uint16_t clientB = clients_[clientId].callingID;
			if(clientB == 0) //clientAû������ͨ���Ŀͻ� 
				continue; 
			clients_[clientA].callingID = 0; //���A��B��ID 
			transPack_t pkg(PkgType_DisConnect);
    		sendto(clients_[clientB].fd, (char*)&pkg, sizeof(transPack_t), 0, (struct sockaddr*)&clients_[clientB].addr, sizeof(sockaddr_in));
		}
		else if((pkg->type == PkgType_Video) || (pkg->type == PkgType_Audio))
			msgTransmit(recvbuf, len);
		else if(pkg->type == PkgType_ControlCmd || pkg->type == PkgType_RobotState)
			cmdAndStatusTransmit(recvbuf, len);
		else
		{
			
		}
	}
	cout << "delete client : " << clientId;
	removeClient(clientId);
	delete [] recvbuf;
	close(server_fd);
}

void Server::removeClient(uint16_t id)
{
	uint16_t otherId = clients_[id].callingID;
	if(otherId != 0) //�����жϣ�����ֵΪ0��������ʽ����²���IDΪ0�Ŀͻ�����map���ʲ����ڵļ����Զ����룩 
		clients_[otherId].callingID = 0; //ɾ���û�ǰ����֮ͨ���Ŀͻ�����Ϣ��λ	
	clients_.erase(id);
}

void Server::run()
{
	std::thread t1 = std::thread(&Server::printThread,this,15);
	t1.detach();
	
	std::thread t2 = std::thread(&Server::heartBeatThread,this);
	t2.detach(); 
	
	//�½����տͻ���ע����Ϣ���߳� 
	std::thread t = std::thread(&Server::receiveRegisterThread,this);
	t.join();
}

void Server::cmdAndStatusTransmit(const uint8_t* buf, int len)
{
	uint16_t srcClientId = ((const transPack_t *)buf)->senderId;
	uint16_t dstClientId = ((const transPack_t *)buf)->receiverId;
	
	if(dstClientId == ROBOT_TEST_ID) //�����˶˲���ID
		return; 
	auto it = clients_.find(dstClientId);
	if (it == clients_.end()) //δ���ҵ�Ŀ��ͻ��� 
	{
		//�������û��ظ� ,�ظ�ʱ��ԭ��ͷ�޸�ָ�����ͺ󷵻�
		transPack_t pkg = *(transPack_t*)buf;
		pkg.type =  PkgType_CalledOffline;
		pkg.length = 0;
		
		sendto(clients_[srcClientId].fd, (char*)&pkg, sizeof(transPack_t), 0, (struct sockaddr*)&clients_[srcClientId].addr, sizeof(sockaddr_in));
		//cout << "No client : " << dstClientId << endl;
		return;
	}
	sendto(clients_[dstClientId].fd, buf, len, 0, (struct sockaddr*)&clients_[dstClientId].addr, sizeof(sockaddr_in));
}


void Server::msgTransmit(const uint8_t* buf, int len)
{
	uint16_t srcClientId = ((const transPack_t *)buf)->senderId;
	uint16_t dstClientId = ((const transPack_t *)buf)->receiverId;
	int type = ((const transPack_t *)buf)->type;
	
	if(dstClientId == 0) //����ID 
		return; 
	
	auto it = clients_.find(dstClientId);
	if (it == clients_.end()) //δ���ҵ�Ŀ��ͻ��� 
	{
		//�������û��ظ� ,�ظ�ʱ��ԭ��ͷ�޸�ָ�����ͺ󷵻�
		transPack_t pkg;
		memcpy(&pkg, buf, sizeof(transPack_t));
		pkg.type =  PkgType_CalledOffline;
		pkg.length = 0;
		
		sendto(clients_[srcClientId].fd, (char*)&pkg, sizeof(transPack_t), 0, (struct sockaddr*)&clients_[srcClientId].addr, sizeof(sockaddr_in));
		//cout << "No client : " << dstClientId << endl;
		return;
	}
	
	//�����û���Ϣ�еı���ID�ѱ�д�룬������ͨ�ɹ� 
	if(clients_[srcClientId].callingID == dstClientId) 
	{
		int send_len = sendto(clients_[dstClientId].fd, buf, len, 0, (struct sockaddr*)&clients_[dstClientId].addr, sizeof(sockaddr_in));
		//cout << "transmitting " << send_len << " bytes\t from:" <<srcClientId <<" to:" <<  dstClientId << " type: " << type <<  endl;
	}
	//callingID Ϊ�գ������������ 
	else if(clients_[srcClientId].callingID == 0) 
	{
		//�򱻽з�����������ָ�� ,��ԭ��ͷ�޸�ָ�����ͺ��ͣ�ּ�ڰ������кͱ��е���Ϣ�� 
		transPack_t pkg;
		memcpy(&pkg, buf, sizeof(transPack_t));
		pkg.type =  PkgType_RequestConnect;
		pkg.length = 0;
		
		sendto(clients_[dstClientId].fd, (char*)&pkg, sizeof(transPack_t), 0, (struct sockaddr*)&clients_[dstClientId].addr, sizeof(sockaddr_in));
	}
	else //���������������û�ͨ��
	{
		transPack_t pkg;
		memcpy(&pkg, buf, sizeof(transPack_t));
		pkg.type =  PkgType_CalledBusy;
		sendto(clients_[srcClientId].fd, (char*)&pkg, sizeof(transPack_t), 0, (struct sockaddr*)&clients_[srcClientId].addr, sizeof(sockaddr_in));	
	} 
	
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
	while(run_flag)
	{
		//cout << "clients size: " << clients_.size() << "\t sending heart beat pkg...\n";
		for(auto client =clients_.begin(); client!= clients_.end(); ++client)
		{
			//cout << "id: " << client->first <<endl;
			//��ʼ����¿ͻ��ϴ�����ʱ��Ϊ0 
			if(client->second.lastHeartBeatTime ==0)
			{
				//�˴��޸��ϴ�����ʱ�䣬��ֹ��Щ�ͻ�ע����δ������ 
				client->second.lastHeartBeatTime = 1; 
				continue;
			} 
			std::time_t diff =  time(0) - client->second.lastHeartBeatTime;
			
			//�ͻ������ӱ�־ connect��λ�󣬽��˳�������̣߳�Ȼ���Զ�ɾ���û�
			//��ĳЩ�û�δ���Զ�ɾ�����ֶ�ɾ�� 
			 
			if(diff > heartBeatInterval_ + maxHeartBeatDelay_*10) 
			{
				//ɾ��δ������ɾ�����û�
				removeClient(client->first);
			}
			else if( diff >heartBeatInterval_ + maxHeartBeatDelay_) 
			{
				//������false���ȴ��߳��˳����Զ�ɾ���û� 
				client->second.connect = false;
				cout << "client " << client->first  << "  disconnect." << endl;
			}
			 
		}
		std::this_thread::sleep_for(std::chrono::seconds(heartBeatInterval_)); 
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
