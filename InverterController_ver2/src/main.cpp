#include <Arduino.h>
#include <MsTimer2.h>
#include "Inverter_dfs.hpp"
#include "Inverter.hpp"
#include "UserInterface.h"

Inverter* inverter;

void interrupt(void);

void setup()
{
    inverter = new Inverter();

    MsTimer2::set(10, interrupt);
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
        inverter->setMgecuRequest(ON);
        inverter->sendMsgToInverter(1);
        break;

    case 2:
        inverter->setMgecuRequest(OFF);
        inverter->sendMsgToInverter(1);
        break;

    case 3:
        inverter->setRapidDischargeRequest(ON);
        inverter->sendMsgToInverter(1);
        break;

    case 4:
        inverter->setRapidDischargeRequest(OFF);
        inverter->sendMsgToInverter(1);
        break;

    case 5:
        torqueControlFlag = 1;
        break;

    case 6:
        torqueControlFlag = 0;
        break;

    case 7:
        Inverter::readMsgFromInverter(1);
        break;

    case 8:
        Inverter::checkMsg(EV_ECU1_ID);
        Inverter::checkMsg(MG_ECU1_ID);
        Inverter::checkMsg(MG_ECU2_ID);
        break;

    case 9:
        Inverter::checkMsgBit(EV_ECU1_ID);
        Inverter::checkMsgBit(MG_ECU1_ID);
        Inverter::checkMsgBit(MG_ECU2_ID);
        break;

    default:
        break;
    }
};

void interrupt(void)
{
    Inverter::readMsgFromInverter(0);
    Inverter::sendMsgToInverter(0);
};