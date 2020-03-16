#ifndef UTILS_H_
#define UTILS_H_


// 消息类型
enum PkgType
{
    PkgType_Video=0,          //视频消息
    PkgType_Audio=1,          //语音
    PkgType_RequestConnect=2, //请求连接
    PkgType_AcceptConnect=3,  //接收连接
    PkgType_RefuseConnect=4,  //拒绝连接
    PkgType_DisConnect=5,     //挂断
    PkgType_NoneType =6,
    PkgType_RegisterOK=7,     //注册成功(服务器发往客户端)
    PkgType_RegisterFail=8,   //注册失败(服务器发往客户端)
    PkgType_RequestRegister=9,//请求注册到服务器
    PkgType_ResponseRegister=10,//回应客户端请求(包含服务端服务端口号)
    PkgType_HeartBeat = 11, //心跳包
    PkgType_LogOut = 12,    //退出登陆
    PkgType_CalledOffline=13,// 被叫不在线
    PkgType_CalledBusy = 14,//被叫忙

    //remote control
    PkgType_ControlCmd = 20,
    PkgType_RobotState = 21,
};

//数据传输头
#pragma pack(push,1)
typedef struct PkgHeader
{
    uint8_t head[2];
    uint16_t length;
    uint8_t type;
    uint8_t checkNum;

    uint16_t senderId;
    uint16_t receiverId;

    PkgHeader(PkgType t = PkgType_NoneType)
    {
        head[0] = 0x66;
        head[1] = 0xcc;
        type = t;
        length = checkNum = 0;
    }
} pkgHeader_t;

#pragma pack(pop)
 


#endif
