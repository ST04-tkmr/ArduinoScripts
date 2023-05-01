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
  // Serial.begin(115200);
}

void loop()
{
  if (Serial.available())
  {
    unsigned int command = read_int();
    Serial.println(command);
    run_command(command);
  }
  else
  {
    readMsgFromInverter(&mgecu1, &mgecu2, 0);
  }
}

/**
 * cmd = 1 : MG-ECU実行要求ON
 * cmd = 2 : MG-ECU実行要求OFF
 * cmd = 3 : Send massage to EV-ECU1
 * cmd = 4 : Read massage from MG-ECU
 * cmd = 5 : check massage of MG-ECU1
 * cmd = 6 : check massage of MG-ECU2
 * cmd = 7 : check massage bit of EV-ECU1
 * cmd = 8 : check massage bit of MG-ECU1
 * cmd = 9 : check massage bit of MG-ECU2
 */
void run_command(unsigned int cmd)
{
  unsigned char buf[8];
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
    readMsgFromInverter(&mgecu1, &mgecu2, 1);
    break;
  case 5:
    mgecu1.checkMGECU1();
    break;
  case 6:
    mgecu2.checkMGECU2();
    break;
  case 7:
    for (int i = 0; i < 8; i++)
    {
      buf[i] = evecu1.getMsgByte(i);
    }
    checkBuf(buf);
    break;
  case 8:
    for (int i = 0; i < 8; i++)
    {
      buf[i] = mgecu1.getMsgByte(i);
    }
    checkBuf(buf);
    break;
  case 9:
    for (int i = 0; i < 8; i++)
    {
      buf[i] = mgecu2.getMsgByte(i);
    }
    checkBuf(buf);
    break;
  default:
    break;
  }
};