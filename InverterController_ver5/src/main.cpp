#include <Arduino.h>
#include "Inverter.hpp"
#include "Accel.hpp"
#include "Switch.hpp"
#include "IO_dfs.hpp"

#ifdef ARDUINO_UNO_R4
#include "AGTimerR4.hpp"
#else
#include <MsTimer2.h>
#endif

Inverter *inverter = new Inverter();
Accel *accel = new Accel();
int val[2] = {0, 0};
float torque = 0;
Switch *driveSW = new Switch();
Switch *shutdownDetect = new Switch();

/**
 * flags[0] = airFlag
 * flags[1] = torqueControlFlag
 * flags[2] = shutdownFlag
 * flags[3] = driverFlag
 */
unsigned char flags[4] = {0, 0, 0, 0};

unsigned char setupFlag = 0;

/*
unsigned long deltaTime = 0;
unsigned long lastTime = 0;
unsigned long nowTime = 0;
*/

unsigned short accVol = 400;

void timerCallback(void);

void setup()
{
    Serial.begin(115200);

    pinMode(ACCEL_SENSOR1, INPUT);
    pinMode(ACCEL_SENSOR2, INPUT);

    pinMode(READY_TO_DRIVE_SW, INPUT);
    pinMode(READY_TO_DRIVE_LED, OUTPUT);
    digitalWrite(READY_TO_DRIVE_LED, LOW);

    pinMode(AIR_PLUS_SIG, OUTPUT);
    digitalWrite(AIR_PLUS_SIG, LOW);
    pinMode(AIR_MINUS_SIG, OUTPUT);
    digitalWrite(AIR_MINUS_SIG, LOW);

    pinMode(SHUTDOWN_DETECT, INPUT);

#ifdef ARDUINO_UNO_R4
    AGTimer.init(500000, timerCallback);
    AGTimer.start();
#else
    MsTimer2::set(500, timerCallback);
    MsTimer2::start();
#endif

    inverter->init();
}

void loop()
{
    inverter->readMsgFromInverter(0);

    val[0] = analogRead(ACCEL_SENSOR1);
    val[1] = analogRead(ACCEL_SENSOR2);
    accel->setValue(val[0], val[1]);

    // If Torque Control Flag is 1, calculate torque from accelerator opening
    torque = flags[1] ? accel->getTorque() : 0;

    /**
     * SHUTDOWN_DETECT Port is pulldown
     * When Shutdown Circuit is OPEN, digitalRead(SHUTdOWN_DETECT) will return 0
    */
    shutdownDetect->updateState(~digitalRead(SHUTDOWN_DETECT));
    flags[2] = shutdownDetect->getSWFlag();

    /**
     * Initial value of setupFlag is 0.
     * When Low Voltage is ON, Shutdown Circuit is OPEN. In other words, shutdownFlag(flags[2]) is 1.
     * If setupFlag is 0 and shutdownFlag is 1, this programs set setupFlag to 1 and reset shutdownFlag to 0.
     */
    if (!setupFlag && flags[2])
    {
        shutdownDetect->resetFlag();
        setupFlag = 1;
    }

    /**
     * READY_TO_DRIVE_SW Port is pulldown
     * When Ready to Drive SW is pushed, digitalRead(READY_TO_DRIVE_SW) will return 1
    */
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

    /**
     * AIR_MINUS is directly connected with TSMS
    */
    digitalWrite(AIR_MINUS_SIG, HIGH);

    digitalWrite(READY_TO_DRIVE_LED, flags[3]);

    inverter->sendMsgToInverter(0);
}

void timerCallback(void)
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
    Serial.println(accel->getValue(0));
    Serial.print("Accel2 : ");
    Serial.println(accel->getValue(1));
    Serial.print("Deviation : ");
    Serial.println(accel->getDev());
    Serial.print("Torque : ");
    Serial.println(torque);
    inverter->checkMsg(MG_ECU1_ID);
    inverter->checkMsg(MG_ECU2_ID);
}