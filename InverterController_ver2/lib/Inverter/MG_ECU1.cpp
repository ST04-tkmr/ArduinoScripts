#include "MG_ECU1.hpp"

MG_ECU1::MSG::MSG()
    : shutdownEnable(0x0), PWM(0x1), workingStatus(0x0), reserve0(0x0), motorSpeed(0x36B0), motorPhaseCurrent(0x0), inputDCVoltage(0x3FF), reserve1(0x0), failureStatus(0x0){}

MG_ECU1::ECU::ECU(unsigned long id)
    : id(id)
{
    msg = new MSG();
    motorSpeedPara = new Parameter(-14000, 1, -14000, 14000);
    motorPhaseCurrentPara = new Parameter(0, 0.5f, 0, 500);
}

MG_ECU1::ECU::~ECU()
{
    delete (msg);
    delete (motorSpeedPara);
    delete (motorPhaseCurrentPara);
}

unsigned char MG_ECU1::ECU::getMsgByte(unsigned char index)
{
    if (0 <= index && index <= 7)
    {
        return msg->msgs[index];
    }

    return 0;
}

float MG_ECU1::ECU::getMotorSpeed()
{
    return motorSpeedPara->calcPhysical(msg->motorSpeed);
}

float MG_ECU1::ECU::getMotorPhaseCurrent()
{
    return motorPhaseCurrentPara->calcPhysical(msg->motorPhaseCurrent);
}

unsigned char MG_ECU1::ECU::setMsg(unsigned char *buf)
{
    for (int i = 0; i < 8; i++)
    {
        msg->msgs[i] = *(buf + i);
    }

    return 0;
}
