#include"utils.h"

systemStatus g_systemStatus = SystemIdle;
QHostAddress g_serverIp("111.229.192.158");
quint16 g_serverPort = 8617;
uint16_t g_myId = 1;

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
