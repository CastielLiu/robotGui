#ifndef STRUCTS_H_
#define STRUCTS_H_

#include<iostream>
#include "enums.h"

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






#endif // STRUCTS_H_
