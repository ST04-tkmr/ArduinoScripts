#include <Arduino.h>
#include <MsTimer2.h>
#include "Inverter.hpp"
#include "Accel.hpp"
#include "Switch.hpp"
//#include "UserInterface.h"

/**
 * チェック項目
 * - runInverterの引数
 * - Accel_dfs.hpp
 *  - MAXIMUM_TORQUE
*/

#define ACCEL_SENSOR1 (A0)
#define ACCEL_SENSOR2 (A1)
#define READY_TO_DRIVE_SW (11)
#define READY_TO_DRIVE_SIG (13)
#define AIR_PLUS_SIG (6)
#define AIR_MINUS_SIG (3)
#define SHUTDOWN_DETECT (8)

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
unsigned char setupFlag;

Switch *driveSW;
Switch *shutdownDetect;

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
    setupFlag = 0;

    accel = new Accel();
    val[0] = 0;
    val[1] = 0;
    pinMode(ACCEL_SENSOR1, INPUT);
    pinMode(ACCEL_SENSOR2, INPUT);

    driveSW = new Switch();
    pinMode(READY_TO_DRIVE_SW, INPUT);
    pinMode(READY_TO_DRIVE_SIG, OUTPUT);
    digitalWrite(READY_TO_DRIVE_SIG, LOW);

    torque = 0;

    pinMode(AIR_PLUS_SIG, OUTPUT);
    digitalWrite(AIR_PLUS_SIG, LOW);
    pinMode(AIR_MINUS_SIG, OUTPUT);
    digitalWrite(AIR_MINUS_SIG, LOW);

    shutdownDetect = new Switch();
    pinMode(SHUTDOWN_DETECT, INPUT);

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
    //accel->setValue(val);
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

    // if driveFlag, airFlag and !torqueControlFlag
    if (flags[3] && flags[0] && !flags[1])
    {
        // if accel pedal is stepped on
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

    inverter->runInverter(flags, 370, -torque);

    digitalWrite(AIR_PLUS_SIG, flags[0]);
    digitalWrite(AIR_MINUS_SIG, HIGH);
    digitalWrite(READY_TO_DRIVE_SIG, flags[3]);

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
    Serial.print("Accel1 : ");
    Serial.println(accel->getValue(0) * 0.0049f);
    Serial.print("Accel2 : ");
    Serial.println(accel->getValue(1) * 0.0049f);
    Serial.print("Torque : ");
    Serial.println(torque);
    inverter->checkMsg(MG_ECU1_ID);
    inverter->checkMsg(MG_ECU2_ID);
    Serial.println(deltaTime);

}