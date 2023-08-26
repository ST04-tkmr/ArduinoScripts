#ifndef _CAN_TEMP_H_
#define _CAN_TEMP_H_

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
mcp2515_can CAN(SPI_CS_PIN); // Set CS pin
//#endif

union CAN_Temp_MSG
{
    unsigned char msg[8];
    struct
    {
        unsigned char avr : 8;
        unsigned char max : 8;
        unsigned char min : 8;
    };

    CAN_Temp_MSG();
};

class CAN_Temp
{
private:
    const unsigned long id;
    CAN_Temp_MSG *msg;

public:
    CAN_Temp(const unsigned long id);

    unsigned char sendTempMsg(unsigned char printFlag);
};

#endif