#include <Arduino.h>
#include "MsTimer2.h"
#include "Inverter_dfs.hpp"
#include "Inverter.hpp"
#include "UserInterface.h"

EV_ECU1::ECU evecu1(EV_ECU1_ID);
MG_ECU1::ECU mgecu1(MG_ECU1_ID);
MG_ECU2::ECU mgecu2(MG_ECU2_ID);

float requestTorque = 0;
unsigned char torqueControlFlag = 0;

void run_command(unsigned int cmd);

void exchangeMassage(void);

void setup()
{
    init(&evecu1, &mgecu1, &mgecu2);
    MsTimer2::set(10, exchangeMassage);
    MsTimer2::start();
}

void loop()
{
    if (Serial.available())
    {
        int command = read_int();
        Serial.println(command);
        run_command(command);
    }

    if (torqueControlFlag && mgecu1.getWorkingStatus() == 0b011)
    {
        if (mgecu1.getMotorSpeed() < 100)
        {
            requestTorque = 0.5f;
        }
        else
        {
            requestTorque = 0;
        }

        evecu1.setRequestTorque(requestTorque);
    }
    else if (evecu1.getRequestTorque() != 0)
    {
        requestTorque = 0;
        evecu1.setRequestTorque(requestTorque);
    }
}

/**
 * cmd = 1 : set MG-ECU Enable to ON and send massage
 * cmd = 2 : set MG-ECU Enable to OFF and send massage
 * cmd = 3 : set Rapid Discharge Command to Active and send massage
 * cmd = 4 : set Rapid Discharge Command to Inactive and send massage
 * cmd = 5 : Start Torque Control
 * cmd = 6 : Stop Torque Control
 * cmd = 7 : Read massage from MG-ECU
 * cmd = 8 : check massage status
 * cmd = 9 : check massage bit
 */
void run_command(int cmd)
{
    switch (cmd)
    {
    case 1:
        evecu1.setEcuEnable(1);
        sendMsgToInverter(1);
        break;

    case 2:
        evecu1.setEcuEnable(0);
        sendMsgToInverter(1);
        break;

    case 3:
        evecu1.setDischargeCommand(1);
        sendMsgToInverter(1);
        break;

    case 4:
        evecu1.setDischargeCommand(0);
        sendMsgToInverter(1);
        break;

    case 5:
        torqueControlFlag = 1;
        break;

    case 6:
        torqueControlFlag = 0;
        break;

    case 7:
        readMsgFromInverter(1);
        break;

    case 8:
        checkMsg(evecu1.getID());
        checkMsg(evecu1.getID());
        checkMsg(evecu1.getID());
        break;

    case 9:
        checkMsgBit(evecu1.getID());
        checkMsgBit(mgecu1.getID());
        checkMsgBit(mgecu2.getID());
        break;

    default:
        break;
    }
};

void exchangeMassage(void)
{
    readMsgFromInverter(0);

    if (torqueControlFlag || mgecu1.getWorkingStatus() == 0b011)
    {
        sendMsgToInverter(0);
    }

    if (torqueControlFlag && mgecu1.getWorkingStatus() == 0b011)
    {
        checkMsg(evecu1.getID());
        checkMsg(evecu1.getID());
        checkMsg(evecu1.getID());
    }
};