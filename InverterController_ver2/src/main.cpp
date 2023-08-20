#include <Arduino.h>
#include <MsTimer2.h>
#include "Inverter_dfs.hpp"
#include "Inverter.hpp"
#include "UserInterface.h"
#include "Accel.hpp"

#define ACCEL_SENSOR1 (A0)
#define ACCEL_SENSOR2 (A1)

unsigned char torqueControlFlag;
Inverter *inverter;
unsigned short val[2];
Accel *accel;
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

    accel = new Accel();

    val[0] = 0;
    val[1] = 0;
    torque = 0;
    interval = 0;

    pinMode(ACCEL_SENSOR1, INPUT);
    pinMode(ACCEL_SENSOR2, INPUT);

    MsTimer2::set(500, interrupt);
    MsTimer2::start();
}

void loop()
{
    inverter->readMsgFromInverter(0);

    if (Serial.available())
    {
        unsigned int command = read_int();
        Serial.println(command);
        run_command(command);
    }

    if (torqueControlFlag)
    {
        val[0] = analogRead(ACCEL_SENSOR1);
        val[1] = analogRead(ACCEL_SENSOR2);
        accel->setValue(val);
        inverter->torqueRequest(accel->getTorque());
        inverter->sendMsgToInverter(0);
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

            inverter->sendMsgToInverter(1);
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

            inverter->sendMsgToInverter(1);
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

            inverter->sendMsgToInverter(1);
        }
        break;

    case 'i':
        inverter->setRapidDischargeRequestOFF();
        Serial.println("rapid discharge OFF request done");

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

            inverter->sendMsgToInverter(1);
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
    if (torqueControlFlag)
    {
        Serial.println(accel->getTorqueOutputFlag());
        Serial.println(accel->getValue(0));
        Serial.println(accel->getValue(1));
        Serial.println(accel->getTorque());
    }
    interval++;
    if (interval >= 50)
    {
        interval = 0;
    }
}