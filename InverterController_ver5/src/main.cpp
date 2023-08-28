#include <Arduino.h>
#include "Inverter.hpp"
#include "Accel.hpp"
#include "Switch.hpp"
#include "IO_dfs.hpp"

Inverter *inverter;
Accel *accel;
unsigned short val[2];
float torque;
Switch *driveSW;
Switch *shutdownDetect;

/**
 * flags[0] = airFlag
 * flags[1] = torqueControlFlag
 * flags[2] = shutdownFlag
 * flags[3] = driverFlag
*/
unsigned char flags[4];
unsigned char setupFlag;

unsigned long deltaTime, lastTime, nowTime;

unsigned short accVol = 400;

void setup() {
    inverter = new Inverter();
    inverter->init();

    flags[0] = 0;
    flags[1] = 0;
    flags[2] = 0;
    flags[3] = 0;
    setupFlag = 0;

    accel = new Accel();
    val[0] = 0;
    val[1] = 0;
    pinMode(ACCEL_SENSOR1, INPUT);
    pinMode(ACCEL_SENSOR2, INPUT);

    driveSW = new Switch();
    pinMode(READY_TO_DRIVE_SW, INPUT);
    pinMode(READY_TO_DRIVE_LED, OUTPUT);
    digitalWrite(READY_TO_DRIVE_LED, LOW);

    torque = 0;

    pinMode(AIR_PLUS_SIG, OUTPUT);
    digitalWrite(AIR_PLUS_SIG, LOW);
    pinMode(AIR_MINUS_SIG, OUTPUT);
    digitalWrite(AIR_MINUS_SIG, LOW);

    shutdownDetect = new Switch();
    pinMode(SHUTDOWN_DETECT, INPUT);

    deltaTime = 0;
    lastTime = 0;
    nowTime = 0;
}

void loop() {
    inverter->readMsgFromInverter(0);

    val[0] = analogRead(ACCEL_SENSOR1);
    val[1] = analogRead(ACCEL_SENSOR2);
    accel->setValue(val[0], val[1]);
    torque = flags[1] ? accel->getTorque() : 0;

    shutdownDetect->updateState(~digitalRead(SHUTDOWN_DETECT));
    flags[2] = shutdownDetect->getSWFlag();
    if (!setupFlag && flags[2])
    {
        shutdownDetect->resetFlag();
        setupFlag = 1;
    }

    driveSW->updateState(digitalRead(READY_TO_DRIVE_SW));
    flags[3] = driveSW->getSWFlag();

    if (flags[3] && flags[0] && !flags[1])
    {
        if (accel->getValue(0) * 0.0049f >= MINIMUM_SENSOR_VOLTAGE && accel->getValue(1) * 0.0049f >= MINIMUM_SENSOR_VOLTAGE)
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

    inverter->runInverter(flags, accVol, torque);

    digitalWrite(AIR_PLUS_SIG, flags[0]);
    digitalWrite(AIR_MINUS_SIG, HIGH);
    digitalWrite(READY_TO_DRIVE_LED, flags[3]);

    inverter->sendMsgToInverter(0);
}