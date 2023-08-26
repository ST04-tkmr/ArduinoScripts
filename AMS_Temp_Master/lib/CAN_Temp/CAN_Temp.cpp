#include "CAN_Temp.hpp"

CAN_Temp_MSG::CAN_Temp_MSG()
    : msg{0, 0, 0, 0, 0, 0, 0, 0}
{
}

CAN_Temp::CAN_Temp(const unsigned long id)
    : id(id)
{
    msg = new CAN_Temp_MSG();
}

unsigned char CAN_Temp::sendTempMsg(unsigned char printFlag)
{
    return 1;
}