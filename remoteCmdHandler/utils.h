#ifndef UTILS_H_
#define UTILS_H_


// ��Ϣ����
enum PkgType
{
    PkgType_Video=0,          //��Ƶ��Ϣ
    PkgType_Audio=1,          //����
    PkgType_RequestConnect=2, //��������
    PkgType_AcceptConnect=3,  //��������
    PkgType_RefuseConnect=4,  //�ܾ�����
    PkgType_DisConnect=5,     //�Ҷ�
    PkgType_NoneType =6,
    PkgType_RegisterOK=7,     //ע��ɹ�(�����������ͻ���)
    PkgType_RegisterFail=8,   //ע��ʧ��(�����������ͻ���)
    PkgType_RequestRegister=9,//����ע�ᵽ������
    PkgType_ResponseRegister=10,//��Ӧ�ͻ�������(��������˷���˿ں�)
    PkgType_HeartBeat = 11, //������
    PkgType_LogOut = 12,    //�˳���½
    PkgType_CalledOffline=13,// ���в�����
    PkgType_CalledBusy = 14,//����æ

    //remote control
    PkgType_ControlCmd = 20,
    PkgType_RobotState = 21,
};

//���ݴ���ͷ
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
