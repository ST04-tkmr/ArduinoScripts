#include <Arduino.h>
#include "Inverter.hpp"
#include "UserInterface.h"

EV_ECU1::ECU evecu1(0x301);
MG_ECU1::ECU mgecu1(0x311);
MG_ECU2::ECU mgecu2(0x321);

void run_command(unsigned int cmd);

void setup()
{
  init_CAN();
  //Serial.begin(115200);
}

void loop()
{
  if (Serial.available())
  {
    unsigned int command = read_int();
    run_command(command);
    Serial.println(command);
  }
  else
  {
    readMsgFromInverter(&mgecu1, &mgecu2);
  }
}

/**
 * cmd = 1 : MG-ECU実行要求ON
 * cmd = 2 : MG-ECU実行要求OFF
 * cmd = 3 : Send massage to EV-ECU
 * cmd = 4 : Read massage from MG-ECU
 * cmd = 5 : check massage of MG-ECU1
 * cmd = 6 : check massage of MG-ECU2
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
    break;
  case 3:
    sendMsgToInverter(&evecu1);
    break;
  case 4:
    readMsgFromInverter(&mgecu1, &mgecu2);
    break;
  case 5:
    mgecu1.checkMGECU1();
    break;
  case 6:
    mgecu2.checkMGECU2();
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