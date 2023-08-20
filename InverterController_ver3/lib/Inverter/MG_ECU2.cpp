#include "MG_ECU2.hpp"

MG_ECU2::MSG::MSG()
    : inverterTemp(0x3C), maxAvailableMotorTorque(0x0), maxAvailableGenerateTorque(0x7D0), motorTemp(0x3C){}

MG_ECU2::ECU::ECU(unsigned long id)
    : id(id)
{
    msg = new MSG();
    inverterTemperaturePara = new Parameter(-40, 1, -40, 210);
    maximumAvailableMotoringTorquePara = new Parameter(0, 0.5f, 0, 1000);
    maximumAvailableGeneratingTorquePara = new Parameter(-1000, 0.5f, -1000, 0);
    motorTemperaturePara = new Parameter(-40, 1, -40, 210);
}

MG_ECU2::ECU::~ECU()
{
    delete (msg);
    delete (inverterTemperaturePara);
    delete (maximumAvailableMotoringTorquePara);
    delete (maximumAvailableGeneratingTorquePara);
    delete (motorTemperaturePara);
}

unsigned char MG_ECU2::ECU::getMsgByte(unsigned char index)
{
    if (0 <= index && index <= 7)
    {
        return msg->msgs[index];
    }

    return 0;
}

float MG_ECU2::ECU::getInverterTemp()
{
    return inverterTemperaturePara->calcPhysical(msg->inverterTemp);
}

float MG_ECU2::ECU::getMaxAvailableMotorTorque()
{
    return maximumAvailableMotoringTorquePara->calcPhysical(msg->maxAvailableMotorTorque);
}

float MG_ECU2::ECU::getMaxAvailableGenerateTorque()
{
    return maximumAvailableGeneratingTorquePara->calcPhysical(msg->maxAvailableGenerateTorque);
}

float MG_ECU2::ECU::getMotorTemp()
{
    return motorTemperaturePara->calcPhysical(msg->motorTemp);
}

unsigned char MG_ECU2::ECU::setMsg(unsigned char *buf)
{
    for (int i = 0; i < 8; i++)
    {
        msg->msgs[i] = *(buf + i);
    }

    return 0;
}
