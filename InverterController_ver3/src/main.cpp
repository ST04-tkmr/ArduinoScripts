#include <Arduino.h>
#include <MsTimer2.h>
#include "Inverter.hpp"
#include "Inverter_dfs.hpp"
#include "Accel.hpp"
#include "Switch.hpp"
#include "UserInterface.h"

#define ACCEL_SENSOR1 (A0)
#define ACCEL_SENSOR2 (A1)
#define READY_TO_DRIVE_SW (3)
#define AIR_SIG (4)
#define SHUTDOWN_SW (5)

Inverter *inverter;
Accel *accel;
unsigned short val[2];
float torque;

/**
 * flags[0] = airFlag
 * flags[1] = torqueControlFlag
 * flags[2] = shutdownFlag
 * flags[3] = driveFlag
 */
unsigned char flags[4];

Switch *driveSW;
Switch *shutdownSW;

unsigned long deltaTime, lastTime, nowTime;
unsigned long id;

void interrupt();

void setup()
{
    inverter = new Inverter();
    inverter->init();

    flags[0] = 0;
    flags[1] = 0;
    flags[2] = 0;
    flags[3] = 0;

    accel = new Accel();
    val[0] = 0;
    val[1] = 0;
    pinMode(ACCEL_SENSOR1, INPUT);
    pinMode(ACCEL_SENSOR2, INPUT);

    driveSW = new Switch();
    pinMode(READY_TO_DRIVE_SW, INPUT);

    torque = 0;

    pinMode(AIR_SIG, OUTPUT);
    digitalWrite(AIR_SIG, LOW);

    shutdownSW = new Switch();
    pinMode(SHUTDOWN_SW, INPUT);

    MsTimer2::set(500, interrupt);
    MsTimer2::start();

    deltaTime = 0;
    lastTime = 0;
    nowTime = 0;
    id = 0;
}

void loop()
{
    id = inverter->readMsgFromInverter(0);
    if (id == MG_ECU1_ID)
    {
        nowTime = millis();
        deltaTime = nowTime - lastTime;
        lastTime = nowTime;
    }

    val[0] = analogRead(ACCEL_SENSOR1);
    val[1] = analogRead(ACCEL_SENSOR2);
    accel->setValue(val);
    torque = flags[1] ? accel->getTorque() : 0;

    shutdownSW->updateState(digitalRead(SHUTDOWN_SW));
    flags[2] = shutdownSW->getSWFlag();

    driveSW->updateState(digitalRead(READY_TO_DRIVE_SW));
    flags[3] = driveSW->getSWFlag();

    // if driveFlag, airFlag and !torqueControlFlag
    if (flags[3] && flags[0] && !flags[1])
    {
        // if accel pedal is stepped on
        if (accel->getValue(0) * 0.0049f >= 0.7f && accel->getValue(1) * 0.0049f >= 0.7f)
        {
           flags[3] = 0;
           driveSW->resetFlag();
        }
    }
    else if (!(flags[3] && flags[0] && flags[1]))
    {
        flags[3] = 0;
        driveSW->resetFlag();
    }

    inverter->runInverter(flags, 270, torque);

    digitalWrite(AIR_SIG, flags[0]);

    inverter->sendMsgToInverter(0);
}

void interrupt()
{

    Serial.print("airFlag : ");
    Serial.println(flags[0]);
    Serial.print("torqueControlFlag : ");
    Serial.println(flags[1]);
    Serial.print("shutdownFlag : ");
    Serial.println(flags[2]);
    Serial.print("driveFlag : ");
    Serial.println(flags[3]);
    inverter->checkMsg(MG_ECU1_ID);
    Serial.println(deltaTime);
}