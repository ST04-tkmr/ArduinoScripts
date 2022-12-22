#include "AMS_TemperatureMonitoring.h"
#include <MsTimer2.h>
#include <Wire.h>

#define PROMINI_ADRS0 0b0000000
#define PROMINI_ADRS1 0b0000001
#define PROMINI_ADRS2 0b0000010
#define PROMINI_ADRS3 0b0000100
#define PROMINI_ADRS4 0b0001000
#define PROMINI_ADRS5 0b0010000
#define PROMINI_ADRS6 0b0100000
#define PROMINI_ADRS7 0b1000000

void setup() {
  Wire.begin();
  Serial.begin(9600);
}

void loop() {

}
