#include"utils.h"

systemStatus g_systemStatus = SystemIdle;
QHostAddress g_serverIp;
quint16 g_registerPort; //服务器接收注册信息的端口号

quint16 g_msgPort;                   //服务器接收数据的端口号
int g_registerStatus = 0;  //客户端注册状态 0未注册，1注册中，2已注册
bool g_isCaller = false;  //当前是否为主叫方
uint16_t g_myId = 123;   // 注：分配客户端id时保留0
uint16_t g_robotCallId = 0; //机器人视频通话ID
uint16_t g_robotControlId = 0;//机器人远程控制ID
const bool g_canCalled = false; //禁用被叫
const bool g_openRemoteControl = true; //启动远程控制

uint16_t g_heartBeatInterval = 5; //心跳包发送间隔(s)，需与服务器保持一致
uint16_t g_maxHeartBeatDelay = 2; //心跳包容许延迟时间(s)

Ui::MainWindow *g_ui;






// ip地址转换 字符串转int 便于传输
int ipConvert(const std::string& ip_str)
{
    uint8_t buf[4];
    sscanf(ip_str.c_str(),"%d.%d.%d.%d",&buf[0],&buf[1],&buf[2],&buf[3]);
    std::stringstream ss(ip_str);

    return *(int *)buf;
}

// ip地址转换 int转字符串
std::string ipConvert(const int ip_int)
{
    uint8_t *buf = (uint8_t *)&ip_int;
    std::stringstream ss;
    for(int i=0; i<3; ++i)
    {
        ss << int(buf[i]) << ".";
    }
    ss << int(buf[3]);
    return ss.str();
}
