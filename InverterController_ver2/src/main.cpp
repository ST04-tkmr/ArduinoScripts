#include <Arduino.h>
#include <MsTimer2.h>
#include "Inverter_dfs.hpp"
#include "Inverter.hpp"
#include "UserInterface.h"
#include "Accel.hpp"

unsigned char torqueControlFlag;
Inverter *inverter;
unsigned short val;
float torque;
unsigned short interval;

void run_command(unsigned int);

void interrupt(void);

void setup()
{
    torqueControlFlag = 0;

    inverter = new Inverter();
    inverter->init();
    inverter->setBatVol(270);

    val = 0;
    torque = 0;
    interval = 0;

    pinMode(A0, INPUT);

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

    if (torqueControlFlag)
    {
        val = analogRead(A0);
        if (0.5f <= val * 0.0049f && val * 0.0049f <= 4.5f)
        {
            torque = val / (1023 / 20.0f) - 2;
        }
        else
        {
            torque = 0;
        }
        inverter->torqueRequest(torque);
    }
}

/**
 * cmd = e : set MG-ECU Enable to ON
 * cmd = d : set MG-ECU Enable to OFF
 *
 * cmd = a : set Rapid Discharge Command to Active
 * cmd = i : set Rapid Discharge Command to Inactive
 *
 * cmd = r : Read massage from MG-ECU
 * cmd = c : check massage status
 * cmd = C : check massage bit
 *
 * cmd = s : start Torque Control
 * cmd = q : quit Torque Control
 *
 * cmd = m : print menu
 */
void run_command(unsigned int cmd)
{
    switch (cmd)
    {
    case 'e':
        if (inverter->setMgecuRequestON(270))
        {
            Serial.println("MG-ECU Enable request fail");
        }
        else
        {
            Serial.println("MG-ECU Enable request success");
        }
        break;

    case 'd':
        if (inverter->setMgecuRequestOFF())
        {
            Serial.println("MG-ECU Disable request fail");
        }
        else
        {
            Serial.println("MG-ECU Disable request success");
        }
        break;

    case 'a':
        if (inverter->setRapidDischargeRequestON())
        {
            Serial.println("warking status is not rapid discharge");
        }
        else
        {
            Serial.println("rapid discharge ON request success");
        }
        break;

    case 'i':
        inverter->setRapidDischargeRequestOFF();
        Serial.println("rapid discharge OFF request done");
        break;

    case 'r':
        inverter->readMsgFromInverter(1);
        break;

    case 'c':
        inverter->checkMsg(EV_ECU1_ID);
        inverter->checkMsg(MG_ECU1_ID);
        inverter->checkMsg(MG_ECU2_ID);
        break;

    case 'C':
        inverter->checkMsgBit(EV_ECU1_ID);
        inverter->checkMsgBit(MG_ECU1_ID);
        inverter->checkMsgBit(MG_ECU2_ID);
        break;

    case 's':
        if (inverter->torqueRequest(0))
        {
            Serial.println("working status is not torque control");
        }
        else
        {
            torqueControlFlag = 1;
            Serial.println("torque control start");
        }
        break;

    case 'q':
        if (torqueControlFlag)
        {
            torqueControlFlag = 0;
            inverter->torqueRequest(0);
            Serial.println("torque control stop");
        }
        break;

    case 'm':
        Serial.println("e : set MG-ECU Enable");
        Serial.println("d : set MG-ECU Disable");
        Serial.println("a : rapid discharge request ON");
        Serial.println("i : rapid discharge request OFF");
        Serial.println("r : read massage from inverter");
        Serial.println("c : check massage");
        Serial.println("C : check massage bit");
        Serial.println("s : start torque control");
        Serial.println("q : quit torque control");
        Serial.println("m : print menu");
        break;

    default:
        break;
    }
}

void interrupt(void)
{
    inverter->readMsgFromInverter(0);
    inverter->sendMsgToInverter(0);
    interval++;
    if (interval >= 50)
    {
        interval = 0;
    }
}