#include <Arduino.h>
#include <MsTimer2.h>
#include "Accel.hpp"

#define ACCEL_SENSOR1 (A0)
#define ACCEL_SENSOR2 (A1)

Accel *accel;
unsigned short val1, val2;
float torque;

void interrupt(void);

void setup() {
    accel = new Accel();
    val1 = 0;
    val2 = 0;
    pinMode(ACCEL_SENSOR1, INPUT);
    pinMode(ACCEL_SENSOR2, INPUT);

    torque = 0;

    MsTimer2::set(100, interrupt);
    MsTimer2::start();

    Serial.begin(115200);
}

void loop() {
    val1 = analogRead(ACCEL_SENSOR1);
    val2 = analogRead(ACCEL_SENSOR2);

    accel->setValue(val1, val2);
    torque = accel->getTorque();
}

void interrupt(void)
{
    Serial.print("SENSOR1 : ");
    Serial.print(val1);
    Serial.print(", ");
    Serial.print(accel->getValue(0));
    Serial.print(", ");
    Serial.println(accel->getValue(0) * 0.0049f);
    Serial.print("SENSOR2 : ");
    Serial.print(val2);
    Serial.print(", ");
    Serial.print(accel->getValue(1));
    Serial.print(", ");
    Serial.println(accel->getValue(1) * 0.0049f);
    Serial.print("TORQUE : ");
    Serial.println(torque);
    Serial.println();
}