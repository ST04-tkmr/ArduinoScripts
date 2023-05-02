#include <Arduino.h>
#include "MsTimer2.h"
#include "Inverter.hpp"
#include "UserInterface.h"

EV_ECU1::ECU evecu1(0x301);
MG_ECU1::ECU mgecu1(0x311);
MG_ECU2::ECU mgecu2(0x321);

float requestTorque = 0;
unsigned char torqueControlFlag = 0;

void run_command(unsigned int cmd);

void exchangeMassage(void);

void setup()
{
  init_CAN();
  MsTimer2::set(10, exchangeMassage);
  MsTimer2::start();
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

  if (torqueControlFlag && mgecu1.getWorkingStatus() == 0b011)
  {
    if (mgecu1.getMotorSpeed() < 100)
    {
      requestTorque = 0.5f;
    }
    else
    {
      requestTorque = 0;
    }

    evecu1.setRequestTorque(requestTorque);
  }
  else if (evecu1.getRequestTorque() != 0)
  {
    requestTorque = 0;
    evecu1.setRequestTorque(requestTorque);
  }
}

/**
 * cmd = 1 : MG-ECU実行要求ONにセット
 * cmd = 2 : MG-ECU実行要求OFFにセット
 * cmd = 3 : 平滑コンデンサ放電要求ONにセット
 * cmd = 4 : 平滑コンデンサ放電要求OFFにセット
 * cmd = 5 : トルクコントロールスタート
 * cmd = 6 : トルクコントロールストップ
 * cmd = 7 : Send massage to EV-ECU1
 * cmd = 8 : Read massage from MG-ECU
 * cmd = 9 : check massage of EV-ECU1
 * cmd = 10 : check massage of MG-ECU1
 * cmd = 11 : check massage of MG-ECU2
 * cmd = 12 : check massage bit of EV-ECU1
 * cmd = 13 : check massage bit of MG-ECU1
 * cmd = 14 : check massage bit of MG-ECU2
 * cmd = 15 : check parameter(offset, resolution, minPhysical, maxPhysical)
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
    evecu1.setDischargeCommand(1);
    break;
  case 4:
    evecu1.setDischargeCommand(0);
    break;
  case 5:
    torqueControlFlag = 1;
    break;
  case 6:
    torqueControlFlag = 0;
    break;
  case 7:
    sendMsgToInverter(&evecu1);
    break;
  case 8:
    readMsgFromInverter(&mgecu1, &mgecu2, 1);
    break;
  case 9:
    evecu1.checkEVECU1();
    break;
  case 10:
    mgecu1.checkMGECU1();
    break;
  case 11:
    mgecu2.checkMGECU2();
    break;
  case 12:
    for (int i = 0; i < 8; i++)
    {
      buf[i] = evecu1.getMsgByte(i);
    }
    checkBuf(buf);
    break;
  case 13:
    for (int i = 0; i < 8; i++)
    {
      buf[i] = mgecu1.getMsgByte(i);
    }
    checkBuf(buf);
    break;
  case 14:
    for (int i = 0; i < 8; i++)
    {
      buf[i] = mgecu2.getMsgByte(i);
    }
    checkBuf(buf);
    break;
  case 15:
    evecu1.checkTorqueRequestPara();
    break;
  default:
    break;
  }
};

void exchangeMassage(void)
{
  readMsgFromInverter(&mgecu1, &mgecu2, 0);

  if (torqueControlFlag || mgecu1.getWorkingStatus() == 0b011)
  {
    sendMsgToInverter(&evecu1);
  }

  if (torqueControlFlag && mgecu1.getWorkingStatus() == 0b011)
  {
    evecu1.checkEVECU1();
    mgecu1.checkMGECU1();
    mgecu2.checkMGECU2();
  }
};