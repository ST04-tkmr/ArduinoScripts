#include <Arduino.h>
#include <MsTimer2.h>
#include "Inverter_dfs.hpp"
#include "Inverter.hpp"
#include "UserInterface.h"

unsigned char torqueControlFlag;
unsigned char inputTorqueMode;
Inverter *inverter;

void run_command(unsigned int);

void interrupt(void);

void setup()
{
    torqueControlFlag = 0;
    inputTorqueMode = 0;

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
 * cmd = e : set MG-ECU Enable to ON and send massage
 * cmd = d : set MG-ECU Enable to OFF and send massage
 *
 * cmd = a : set Rapid Discharge Command to Active and send massage
 * cmd = i : set Rapid Discharge Command to Inactive and send massage
 *
 * cmd = r : Read massage from MG-ECU
 * cmd = c : check massage status
 * cmd = C : check massage bit
 *
 * cmd = s : start Torque Control
 * cmd = q : quit Torque Control
 * cmd = t : toggle input torque mode
 */
void run_command(unsigned int cmd)
{
    switch (cmd)
    {
    case 'e':
        inverter->setMgecuRequest(ON);
        inverter->sendMsgToInverter(1);
        break;

    case 'd':
        inverter->setMgecuRequest(OFF);
        inverter->sendMsgToInverter(1);
        break;

    case 'a':
        inverter->setRapidDischargeRequest(ON);
        inverter->sendMsgToInverter(1);
        break;

    case 'i':
        inverter->setRapidDischargeRequest(OFF);
        inverter->sendMsgToInverter(1);
        break;

    case 'r':
        inverter->readMsgFromInverter(1);
        break;

    case 'c':
        inverter->checkMsg(EV_ECU1_ID);
        inverter->checkMsg(MG_ECU1_ID);
        inverter->checkMsg(MG_ECU2_ID);
        break;

    case 's':
        torqueControlFlag = 1;
        break;

    case 'q':
        torqueControlFlag = 0;
        break;

    case 't':
        inputTorqueMode = !inputTorqueMode;
        break;

    default:
        break;
    }
};

void interrupt(void)
{
    inverter->readMsgFromInverter(0);
    inverter->sendMsgToInverter(0);
};