#ifndef UTILS_H_
#define UTILS_H_


// 传输消息类型
enum PkgType
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


//ÊýŸÝŽ«ÊäÍ·
#pragma pack(push,1)
typedef struct PkgHeader
{
    uint8_t head[2];
    uint16_t length;
    uint8_t type;
    uint8_t checkNum;
    uint16_t senderId;
    uint16_t receiverId;
    uint16_t seq;

    PkgHeader(PkgType t = PkgType_NoneType)
    {
        head[0] = 0x66;
        head[1] = 0xcc;
        type = t;
        receiverId = 0; //默认receiver_id为0, 即接受者为服务器,无需转发
        length = checkNum = 0;
    }
} pkgHeader_t;

#pragma pack(pop)
 


#endif
