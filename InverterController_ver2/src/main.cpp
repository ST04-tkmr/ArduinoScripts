#include <Arduino.h>
#include <MsTimer2.h>
#include "Inverter_dfs.hpp"
#include "Inverter.hpp"
#include "UserInterface.h"

unsigned char torqueControlFlag;
Inverter *inverter;

void run_command(unsigned int);

void interrupt(void);

void setup()
{
    torqueControlFlag = 0;

    inverter = new Inverter();
    inverter->init();

    MsTimer2::set(10, interrupt);
    MsTimer2::start();
}

void loop()
{
    if (Serial.available())
    {
        unsigned int command = read_int();
        Serial.println(command);
        run_command(command);
    }
}

/**
 * cmd = 1 : set MG-ECU Enable to ON and send massage
 * cmd = 2 : set MG-ECU Enable to OFF and send massage
 * cmd = 3 : set Rapid Discharge Command to Active and send massage
 * cmd = 4 : set Rapid Discharge Command to Inactive and send massage
 * cmd = s : start Torque Control
 * cmd = q : quit Torque Control
 * cmd = 5 : Read massage from MG-ECU
 * cmd = 6 : check massage status
 * cmd = 7 : check massage bit
 */
void run_command(unsigned int cmd)
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
        inverter->readMsgFromInverter(1);
        break;

    case 6:
        inverter->checkMsg(EV_ECU1_ID);
        inverter->checkMsg(MG_ECU1_ID);
        inverter->checkMsg(MG_ECU2_ID);
        break;

    case 7:
        inverter->checkMsgBit(EV_ECU1_ID);
        inverter->checkMsgBit(MG_ECU1_ID);
        inverter->checkMsgBit(MG_ECU2_ID);
        break;

    default:
        if (cmd == 's')
        {
            torqueControlFlag = 1;
        }
        else if (cmd == 'q')
        {
            torqueControlFlag = 0;
        }
        break;
    }
};

void interrupt(void)
{
    inverter->readMsgFromInverter(0);
    inverter->sendMsgToInverter(0);
};