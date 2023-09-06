#ifndef _CAN_TEMP_H_
#define _CAN_TEMP_H_

#include "Parameter.hpp"
#include "CAN_Temp_dfs.hpp"

#include <SPI.h>
/*
#define CAN_2515
#if defined(SEEED_WIO_TERMINAL) && defined(CAN_2518FD)
const int SPI_CS_PIN = BCM8;
const int CAN_INT_PIN = BCM25;
#else
*/
const int SPI_CS_PIN = 9;
const int CAN_INT_PIN = 2;
/*
#endif
#ifdef CAN_2518FD
#include "mcp2518fd_can.h"
mcp2518fd CAN(SPI_CS_PIN); // Set CS pin
#endif
#ifdef CAN_2515
*/
#include "mcp2515_can.h"
// mcp2515_can CAN(SPI_CS_PIN); // Set CS pin
// #endif

union CAN_Temp_MSG
{
    unsigned char msg[8];
    struct
    {
        unsigned char avrTemp : 8;
        unsigned char maxTemp : 8;
        unsigned char minTemp : 8;
    };

    CAN_Temp_MSG();
};

class CAN_Temp
{
private:
    const unsigned long id;
    CAN_Temp_MSG *msg;
    Parameter *tempPara;

public:
    CAN_Temp(const unsigned long id);

    void init(void);

    inline float getTemp(Type type)
    {
        switch (type)
        {
        case Type::AVR_TEMP:
            return tempPara->calcPhysical(msg->avrTemp);
            break;

        case Type::MAX_TEMP:
            return tempPara->calcPhysical(msg->maxTemp);
            break;

        case Type::MIN_TEMP:
            return tempPara->calcPhysical(msg->minTemp);
            break;

        default:
            return tempPara->getMaxPhysical();
            break;
        }
    }

    unsigned char setTemp(Type type, float physicalValue);

    unsigned char sendTempMsg(unsigned char printFlag);

    void checkBuf(unsigned char *buf);

    void checkMsg(void);
};

#endif