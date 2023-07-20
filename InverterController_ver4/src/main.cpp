#include <Arduino.h>
#include <MsTimer2.h>
#include "Inverter.hpp"
#include "Inverter_dfs.hpp"
#include "Accel.hpp"
#include "Switch.hpp"
#include "IO_dfs.hpp"

Inverter *inverter;
Accel *accel;
unsigned short val[2];
float torque;

void setup() {
}

void loop() {
}