#include <Arduino.h>
#include "Inverter.hpp"
#include "UserInterface.h"

EV_ECU1::ECU evecu1(0x301);
MG_ECU1::ECU mgecu1(0x311);
MG_ECU2::ECU mgecu2(0x321);

void setup()
{
  // init_CAN();
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

/**
 * cmd = 1 : MG-ECU実行要求ON
 * cmd = 2 : MG-ECU実行要求OFF
 * cmd = 3 : Send massage to EV-ECU
 */
void run_command(unsigned int cmd)
{
  switch (cmd)
  {
  case 1:
    evecu1.setEcuEnable(1);
    break;
  case 2:
    evecu1.setEcuEnable(0);
  case 3:
  case 4:
  default:
    break;
  }
};

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