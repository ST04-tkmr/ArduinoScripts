#include <Arduino.h>
#include "Inverter.hpp"

void checkMGECU1(void);

void checkMGECU2(void);

unsigned char stmp[8] = {0, 0, 0, 0, 0, 0, 0, 1};
unsigned char len = 0;
unsigned char buf[8];
volatile unsigned long canId;
volatile unsigned short requestTorque = 1;  //-1000~1000

// MG-ECU_1
volatile unsigned char shutdownEnable, PWM, workingStatus, failureStatus;
volatile unsigned short motorSpeed, motorPhaseCurrent, inputDCVoltage;

// MG-ECU_2
volatile unsigned char inverterTemp, motorTemp;
volatile unsigned short maxMotorTorque, maxGenerateTorque;

void setup()
{
  SERIAL_PORT_MONITOR.begin(115200);
  while (CAN_OK != CAN.begin(CAN_500KBPS))
  {
    SERIAL_PORT_MONITOR.println("CAN init fail, retry...");
    delay(100);
  }
  SERIAL_PORT_MONITOR.println("CAN init ok!");
}

void loop()
{
  if (motorSpeed >= 100) {
    requestTorque = 0;
  }

  stmp[1] = requestTorque & 0xFF;
  stmp[2] = (requestTorque >> 8) & 0x0F;
  CAN.sendMsgBuf(0x301, 0, 8, stmp);

  if (CAN_MSGAVAIL == CAN.checkReceive())
  {
    CAN.readMsgBuf(&len, buf);
    canId = CAN.getCanId();

    Serial.println("--------------------");

    for (int i = 0; i < 8; i++)
    {
      Serial.print("buf = ");
      for (int j = 7; j >= 0; j--)
      {
        Serial.print((buf[i] & (0x01 << j)) >> j);
      }
      Serial.println();
    }

    Serial.println();

    // MG-ECU_1
    if (canId == 0x311)
    {
      // 0bit
      shutdownEnable = buf[0] & 0x01;
      // 1~2bit
      PWM = (buf[0] >> 1) & 0x03;
      // 3~5bit
      workingStatus = (buf[0] >> 3) & 0x07;
      // 8~23bit
      motorSpeed = buf[2] & 0xFF;
      motorSpeed = motorSpeed << 8;
      motorSpeed = motorSpeed | (buf[1] & 0xFF);
      // 24~33bit
      motorPhaseCurrent = buf[4] & 0x03;
      motorPhaseCurrent = motorPhaseCurrent << 8;
      motorPhaseCurrent = motorPhaseCurrent | (buf[3] & 0xFF);
      // 34~43bit
      inputDCVoltage = buf[5] & 0x0F;
      inputDCVoltage = inputDCVoltage << 6;
      inputDCVoltage = inputDCVoltage | ((buf[4] >> 2) & 0x3F);
      // 61~63bit
      failureStatus = (buf[7] >> 5) & 0x07;

      checkMGECU1();
    }

    // MG-ECU_2
    if (canId == 0x321)
    {
      // 0~7bit
      inverterTemp = buf[0] & 0xFF;
      // 8~19bit
      maxMotorTorque = buf[2] & 0x0F;
      maxMotorTorque = maxMotorTorque << 8;
      maxMotorTorque = maxMotorTorque | (buf[1] & 0xFF);
      // 20~31bit
      maxGenerateTorque = buf[3] & 0xFF;
      maxGenerateTorque = maxGenerateTorque << 4;
      maxGenerateTorque = maxGenerateTorque | ((buf[2] >> 4) & 0x0F);
      // 32~39bit
      motorTemp = buf[4] & 0xFF;

      checkMGECU2();
    }
  }

  delay(1000);
}

void checkMGECU1(void)
{
  Serial.println("-------MGECU1-------");

  Serial.print("shutdownEnable = ");
  Serial.println(shutdownEnable, BIN);
  Serial.print("PWM = ");
  Serial.println(PWM, BIN);
  Serial.print("WorkingStatus = ");
  Serial.println(workingStatus, BIN);
  Serial.print("motorSpeed = ");
  Serial.println(motorSpeed, BIN);
  Serial.print("motorPhaseCurrent = ");
  Serial.println(motorPhaseCurrent, BIN);
  Serial.print("inputDCVoltage = ");
  Serial.println(inputDCVoltage, BIN);
  Serial.print("failureStatus = ");
  Serial.println(failureStatus, BIN);

  if (shutdownEnable)
  {
    Serial.println("シャットダウン許可有効");
  }
  else
  {
    Serial.println("シャットダウン許可無効");
  }

  Serial.print("ゲート駆動状態 ");
  switch (PWM)
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
  switch (workingStatus)
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
  if (motorSpeed == 0x00)
  {
    Serial.println("調整中 0位置");
  }
  else
  {
    Serial.println(motorSpeed - 14000);
  }

  Serial.print("モータ相電流 ");
  if (motorPhaseCurrent == 0x00)
  {
    Serial.println("調整中 0位置");
  }
  else
  {
    Serial.println(motorPhaseCurrent);
  }

  Serial.print("入力直流電圧 ");
  if (inputDCVoltage == 0x00)
  {
    Serial.println("調整中 0位置");
  }
  else
  {
    Serial.println(inputDCVoltage);
  }

  Serial.print("異常状態 ");
  switch (failureStatus)
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
  Serial.println(inverterTemp, BIN);
  Serial.print("maxMotorTorque = ");
  Serial.println(maxMotorTorque, BIN);
  Serial.print("maxGenerateTorque = ");
  Serial.println(maxGenerateTorque, BIN);
  Serial.print("motorTemp = ");
  Serial.println(motorTemp, BIN);

  Serial.print("インバータ温度 ");
  if (motorSpeed == 0x00)
  {
    Serial.println("調整中 0位置");
  }
  else
  {
    Serial.println(inverterTemp - 40);
  }

  Serial.print("モーター上限制限トルク ");
  if (motorSpeed == 0x00)
  {
    Serial.println("調整中 0位置");
  }
  else
  {
    Serial.println(maxMotorTorque);
  }

  Serial.print("モーター下限制限トルク ");
  if (motorSpeed == 0x00)
  {
    Serial.print("調整中 0位置");
  }
  else
  {
    Serial.println(maxGenerateTorque - 1000);
  }

  Serial.print("モーター温度 ");
  if (motorSpeed == 0x00)
  {
    Serial.println("調整中 0位置");
  }
  else
  {
    Serial.println(motorTemp - 40);
  }

  Serial.println("--------------------");
  Serial.println();
}