#include <Arduino.h>
#include "Inverter.hpp"
#include "UserInterface.h"

EV_ECU1::ECU evecu1(0x301);
MG_ECU1::ECU mgecu1(0x311);
MG_ECU2::ECU mgecu2(0x321);

void setup()
{
  //init_CAN();
  Serial.begin(115200);
}

void loop()
{
  if (Serial.available())
  {
    unsigned int command = read_int();
    Serial.println(command);

  }
}

void checkBuf(void)
{
  Serial.println("----------buf----------");

  for (int i = 0; i < 8; i++)
  {
    for (int j = 7; j >= 0; j--)
    {
    }
  }

  Serial.println("-----------------------");
}