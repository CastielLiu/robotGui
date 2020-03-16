#ifndef UTILS_H_
#define UTILS_H_
// ��Ϣ����
enum dataType
{
    Video=0,          //��Ƶ��Ϣ
    Audio=1,          //����
    RequestConnect=2, //��������
    AcceptConnect=3,  //��������
    RefuseConnect=4,  //�ܾ�����
    DisConnect=5,     //�Ҷ�
    NoneType =6,
    RegisterOK=7,     //ע��ɹ�(�����������ͻ���)
    RegisterFail=8,   //ע��ʧ��(�����������ͻ���)
    RequestRegister=9,//����ע�ᵽ������
    ResponseRegister=10,//��Ӧ�ͻ�������(��������˷���˿ں�)
    HeartBeat = 11, //������
    LogOut = 12,    //�˳���½
    CalledOffline=13,// ���в�����
    CalledBusy = 14,//����æ

    //remote control
    ControlCmd = 20,
    RobotState = 21,
};

//���ݴ���ͷ
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
