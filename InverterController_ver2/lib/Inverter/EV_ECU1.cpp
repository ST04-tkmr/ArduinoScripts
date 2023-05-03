#include "EV_ECU1.hpp"

EV_ECU1::MSG::MSG()
    : ecuEnable(0x0), dischargeCommand(0x0), reserve0(0x0), requestTorque(0x7D0){};

EV_ECU1::ECU::ECU(unsigned long id)
    : id(id)
{
    msg = new MSG();
    torqueRequestPara = new Parameter(-1000, 0.5f, -1000, 1000);
};

EV_ECU1::ECU::~ECU()
{
    delete (msg);
    delete (torqueRequestPara);
};

unsigned char EV_ECU1::ECU::getMsgByte(unsigned char index)
{
    if (0 <= index && index <= 7)
    {
        return msg->msgs[index];
    }

    return 0;
};

float EV_ECU1::ECU::getRequestTorque()
{
    return torqueRequestPara->calcPhysical(msg->requestTorque);
};

unsigned char EV_ECU1::ECU::setEcuEnable(unsigned char ecuEnable)
{
    if (ecuEnable == 0 || ecuEnable == 1)
    {
        msg->ecuEnable = ecuEnable;
        return 0;
    }

    msg->ecuEnable = 0;
    return 1;
};

unsigned char EV_ECU1::ECU::setDischargeCommand(unsigned char dischargeCommand)
{
    if (dischargeCommand == 0 || dischargeCommand == 1)
    {
        msg->dischargeCommand = dischargeCommand;
        return 0;
    }

    msg->dischargeCommand = 0;
    return 1;
};

unsigned char EV_ECU1::ECU::setRequestTorque(float physicalValue)
{
    if (torqueRequestPara->getMinPhysical() <= physicalValue && physicalValue <= torqueRequestPara->getMaxPhysical())
    {
        msg->requestTorque = torqueRequestPara->calcNormal(physicalValue);
        return 0;
    }

    msg->requestTorque = torqueRequestPara->calcNormal(0);
    return 1;
};
