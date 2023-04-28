#include <Arduino.h>
#include "Inverter.hpp"

namespace MGECU1 = Inverter::MG_ECU1;
namespace MGECU2 = Inverter::MG_ECU2;

void setup()
{
}

void loop()
{
}

void checkBuf(void)
{
  Serial.println("----------buf----------");

  for (int i=0; i<8; i++)
  {
    for (int j=7; j>=0; j--)
    {

      Serial.print("");
    }
  }
}

void checkMGECU1(void)
{
  Serial.println("-------MGECU1-------");

  Serial.print("shutdownEnable = ");
  Serial.println(MGECU1::msg.shutdownEnable, BIN);
  Serial.print("PWM = ");
  Serial.println(MGECU1::msg.PWM, BIN);
  Serial.print("WorkingStatus = ");
  Serial.println(MGECU1::msg.workingStatus, BIN);
  Serial.print("motorSpeed = ");
  Serial.println(MGECU1::msg.motorSpeed, BIN);
  Serial.print("motorPhaseCurrent = ");
  Serial.println(MGECU1::msg.motorPhaseCurrent, BIN);
  Serial.print("inputDCVoltage = ");
  Serial.println(MGECU1::msg.inputDCVoltage, BIN);
  Serial.print("failureStatus = ");
  Serial.println(MGECU1::msg.failureStatus, BIN);

  if (MGECU1::msg.shutdownEnable)
  {
    Serial.println("シャットダウン許可有効");
  }
  else
  {
    Serial.println("シャットダウン許可無効");
  }

  Serial.print("ゲート駆動状態 ");
  switch (MGECU1::msg.PWM)
  {
  case 0b00:
    Serial.println("短絡");
    break;
  case 0b01:
    Serial.println("正常 トルクなし");
    break;
  case 0b10:
    Serial.println("正常 制御中");
    break;
  default:
    Serial.println("error");
    break;
  }

  Serial.print("制御状態 ");
  switch (MGECU1::msg.workingStatus)
  {
  case 0b000:
    Serial.println("初期状態");
    break;
  case 0b001:
    Serial.println("プリチャージ中");
    break;
  case 0b010:
    Serial.println("スタンバイ");
    break;
  case 0b011:
    Serial.println("トルク制御中");
    break;
  case 0b111:
    Serial.println("急速放電中");
    break;
  default:
    Serial.println("error");
    break;
  }

  Serial.print("モータ回転数 ");
  if (MGECU1::msg.motorSpeed == 0x00)
  {
    Serial.println("調整中 0位置");
  }
  else
  {
    Serial.println(MGECU1::msg.motorSpeed - 14000);
  }

  Serial.print("モータ相電流 ");
  if (MGECU1::msg.motorPhaseCurrent == 0x00)
  {
    Serial.println("調整中 0位置");
  }
  else
  {
    Serial.println(MGECU1::msg.motorPhaseCurrent);
  }

  Serial.print("入力直流電圧 ");
  if (MGECU1::msg.inputDCVoltage == 0x00)
  {
    Serial.println("調整中 0位置");
  }
  else
  {
    Serial.println(MGECU1::msg.inputDCVoltage);
  }

  Serial.print("異常状態 ");
  switch (MGECU1::msg.failureStatus)
  {
  case 0b000:
    Serial.println("エラーなし");
    break;
  case 0b001:
    Serial.println("負荷軽減");
    break;
  case 0b010:
    Serial.println("警告");
    break;
  case 0b101:
    Serial.println("クリティカルエラー");
    break;
  default:
    Serial.println("error");
    break;
  }

  Serial.println("--------------------");
  Serial.println();
}

void checkMGECU2(void)
{
  Serial.println("-------MGECU2-------");

  Serial.print("inverterTemp = ");
  Serial.println(MGECU2::msg.inverterTemp, BIN);
  Serial.print("maxMotorTorque = ");
  Serial.println(MGECU2::msg.maxAvailableMotorTorque, BIN);
  Serial.print("maxGenerateTorque = ");
  Serial.println(MGECU2::msg.maxAvailableGenerateTorque, BIN);
  Serial.print("motorTemp = ");
  Serial.println(MGECU2::msg.motorTemp, BIN);

  Serial.print("インバータ温度 ");
  if (MGECU1::msg.motorSpeed == 0x00)
  {
    Serial.println("調整中 0位置");
  }
  else
  {
    Serial.println(MGECU2::msg.inverterTemp - 40);
  }

  Serial.print("モーター上限制限トルク ");
  if (MGECU1::msg.motorSpeed == 0x00)
  {
    Serial.println("調整中 0位置");
  }
  else
  {
    Serial.println(MGECU2::msg.maxAvailableMotorTorque);
  }

  Serial.print("モーター下限制限トルク ");
  if (MGECU1::msg.motorSpeed == 0x00)
  {
    Serial.print("調整中 0位置");
  }
  else
  {
    Serial.println(MGECU2::msg.maxAvailableGenerateTorque - 1000);
  }

  Serial.print("モーター温度 ");
  if (MGECU1::msg.motorSpeed == 0x00)
  {
    Serial.println("調整中 0位置");
  }
  else
  {
    Serial.println(MGECU2::msg.motorTemp - 40);
  }

  Serial.println("--------------------");
  Serial.println();
}