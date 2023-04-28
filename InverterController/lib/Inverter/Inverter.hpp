#ifndef INVERTER
#define INVERTER

#include <SPI.h>
#define CAN_2515
#if defined(SEEED_WIO_TERMINAL) && defined(CAN_2518FD)
const int SPI_CS_PIN = BCM8;
const int CAN_INT_PIN = BCM25;
#else
const int SPI_CS_PIN = 9;
const int CAN_INT_PIN = 2;
#endif
#ifdef CAN_2518FD
#include "mcp2518fd_can.h"
mcp2518fd CAN(SPI_CS_PIN); // Set CS pin
#endif
#ifdef CAN_2515
#include "mcp2515_can.h"
mcp2515_can CAN(SPI_CS_PIN); // Set CS pin
#endif

namespace Inverter
{
    namespace EV_ECU1
    {
        volatile unsigned char ecuEnable;   //MG-ECU実行要求
        volatile unsigned char dischargeCommand;    //平滑コンデンサ放電要求
        volatile unsigned short requestTorque;  //HV-ECU要求トルク
    }

    namespace MG_ECU1
    {
        volatile unsigned char shutdownEnable;  //MG_ECUシャットダウン許可
        volatile unsigned char PWM; //ゲート駆動状態
        volatile unsigned char workingStatus;   //制御状態
        volatile unsigned short motorSpeed; //モータ回転数
        volatile unsigned short motorPhaseCurrent;  //モータ相電流
        volatile unsigned short inputDCVoltage; //入力直流電圧
        volatile unsigned char failureStatus;   //異常状態
    }

    namespace MG_ECU2
    {
        volatile unsigned char inverterTemp;    //インバータ温度
        volatile unsigned short maxMotorTorque; //モータ上限制限トルク
        volatile unsigned short maxGenerateTorque;  //モータ下限制限トルク
        volatile unsigned char motorTemp;   //モータ温度
    }
}

#endif