#include <SPI.h>
#define CAN_2515
#if defined(SEEED_WIO_TERMINAL) && defined(CAN_2518FD)
const int SPI_CS_PIN  = BCM8;
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

unsigned char stmp[8] = {0, 0, 0, 0, 0, 0, 0, 0};
unsigned char len = 0;
unsigned char buf[8];
volatile unsigned long canId;
//MG-ECU_1
volatile unsigned char shutdownEnable, PWM, workingStatus, failureStatus;
volatile unsigned short motorSpeed, motorPhaseCurrent, inputDCVoltage;
//MG-ECU_2
volatile unsigned char inverterTemp, motorTemp;
volatile unsigned short maxMotorTorque, maxGenerateTorque;

void setup() {
  SERIAL_PORT_MONITOR.begin(115200);
  while (CAN_OK != CAN.begin(CAN_500KBPS)) {
    SERIAL_PORT_MONITOR.println("CAN init fail, retry...");
    delay(100);
  }
  SERIAL_PORT_MONITOR.println("CAN init ok!");
}

void loop() {
  CAN.sendMsgBuf(0x301, 0, 8, stmp);

  //delay(500);
  
  if (CAN_MSGAVAIL == CAN.checkReceive()) {
    CAN.readMsgBuf(&len, buf);
    canId = CAN.getCanId();

    //MG-ECU_1
    if (canId == 0x311) {
      //0bit
      shutdownEnable = buf[0] & 0x01;
      //1~2bit
      PWM = (buf[0] >> 1) & 0x03;
      //3~5bit
      workingStatus = (buf[0] >> 3) & 0x07;
      //8~23bit
      motorSpeed = buf[2] & 0xFF;
      motorSpeed = motorSpeed << 8;
      motorSpeed = motorSpeed | (buf[1] & 0xFF);
      //24~33bit
      motorPhaseCurrent = buf[4] & 0x03;
      motorPhaseCurrent = motorPhaseCurrent << 8;
      motorPhaseCurrent = motorPhaseCurrent | (buf[3] & 0xFF);
      //34~43bit
      inputDCVoltage = buf[5] & 0x0F;
      inputDCVoltage = inputDCVoltage << 6;
      inputDCVoltage = inputDCVoltage | ((buf[4] >> 2) & 0x3F);
      //61~63bit
      failureStatus = (buf[7] >> 5) & 0x07;

      if (shutdownEnable) {
        
      } else {
        
      }

      switch (PWM) {
        case 0b00:
          break;
        case 0b01:
          break;
        case 0b10:
          break;
        default:
          break;
      }

      switch (workingStatus) {
        case 0b000:
          break;
        case 0b001:
          break;
        case 0b010:
          break;
        case 0b011:
          break;
        case 0b111:
          break;
        default:
          break;
      }

      if (motorSpeed == 0x00) {
        
      } else {
        
      }

      if (motorPhaseCurrent == 0x00) {
        
      } else {
        
      }

      if (inputDCVoltage == 0x00) {
        
      } else {
        
      }

      switch (failureStatus) {
        case 0b000:
          break;
        case 0b001:
          break;
        case 0b010:
          break;
        case 0b101:
          break;
        default:
          break;
      }
    }

    //MG-ECU_2
    if (canId == 0x321) {
      //0~7bit
      inverterTemp = buf[0] & 0xFF;
      //8~19bit
      maxMotorTorque = buf[2] & 0x0F;
      maxMotorTorque = maxMotorTorque << 8;
      maxMotorTorque = maxMotorTorque | (buf[1] & 0xFF);
      //20~31bit
      maxGenerateTorque = buf[3] & 0xFF;
      maxGenerateTorque = maxGenerateTorque << 4;
      maxGenerateTorque = maxGenerateTorque | ((buf[2] >> 4) & 0x0F);
      //32~39bit
      motorTemp = buf[4] & 0xFF;

      if (motorSpeed == 0x00) {
        
      } else {
        
      }
    }
  }
}
