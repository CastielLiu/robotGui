#ifndef UTILS_H_
#define UTILS_H_
// 消息类型
enum dataType
{
    Video=0,          //视频消息
    Audio=1,          //语音
    RequestConnect=2, //请求连接
    AcceptConnect=3,  //接收连接
    RefuseConnect=4,  //拒绝连接
    DisConnect=5,     //挂断
    NoneType =6,
    RegisterOK=7,     //注册成功(服务器发往客户端)
    RegisterFail=8,   //注册失败(服务器发往客户端)
    RequestRegister=9,//请求注册到服务器
    ResponseRegister=10,//回应客户端请求(包含服务端服务端口号)
    HeartBeat = 11, //心跳包
    LogOut = 12,    //退出登陆
    CalledOffline=13,// 被叫不在线
    CalledBusy = 14,//被叫忙

    //remote control
    ControlCmd = 20,
    RobotState = 21,
};

//数据传输头
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
 


#endif
