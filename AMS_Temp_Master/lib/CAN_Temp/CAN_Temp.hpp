#ifndef _CAN_TEMP_H_
#define _CAN_TEMP_H_

#include "CAN_Temp_dfs.hpp"

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