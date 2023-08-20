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
void setup() {
  SERIAL_PORT_MONITOR.begin(115200);
  while (CAN_OK != CAN.begin(CAN_500KBPS)) {             // init can bus : baudrate = 500k
        Serial.println("CAN init fail, retry...");
        delay(100);
  }
  SERIAL_PORT_MONITOR.println("CAN init ok!");
}
unsigned char stmp[8] = {0, 0, 0, 0, 0, 0, 0, 0};
unsigned char len = 0;
unsigned char buf[8];

void loop() {
  CAN.sendMsgBuf(0x301, 0, 8, stmp);

  delay(500);
 
  String a1bit0,a1bit1,a1bit2,a1bit3,a1bit4,a1bit5,a1bit6,a1bit7,a1bit8,a1bit9,a1bit10,a1bit11,a1bit12,a1bit13,a1bit14,a1bit15,a1bit16,a1bit17,a1bit18,a1bit19,a1bit20,a1bit21,a1bit22,a1bit23,a1bit24,a1bit25,a1bit26,a1bit27,a1bit28,a1bit29,a1bit30,a1bit31,a1bit32,a1bit33,a1bit34,a1bit35,a1bit36,a1bit37,a1bit38,a1bit39,a1bit40,a1bit41,a1bit42,a1bit43,a1bit44,a1bit45,a1bit46,a1bit47,a1bit48,a1bit49,a1bit50,a1bit51,a1bit52,a1bit53,a1bit54,a1bit55,a1bit56,a1bit57,a1bit58,a1bit59,a1bit60,a1bit61,a1bit62,a1bit63,Shutdown,PWM,Working,Speed,Current, Voltage,Failure;
  String a2bit0,a2bit1,a2bit2,a2bit3,a2bit4,a2bit5,a2bit6,a2bit7,a2bit8,a2bit9,a2bit10,a2bit11,a2bit12,a2bit13,a2bit14,a2bit15,a2bit16,a2bit17,a2bit18,a2bit19,a2bit20,a2bit21,a2bit22,a2bit23,a2bit24,a2bit25,a2bit26,a2bit27,a2bit28,a2bit29,a2bit30,a2bit31,a2bit32,a2bit33,a2bit34,a2bit35,a2bit36,a2bit37,a2bit38,a2bit39,a2bit40,a2bit41,a2bit42,a2bit43,a2bit44,a2bit45,a2bit46,a2bit47,a2bit48,a2bit49,a2bit50,a2bit51,a2bit52,a2bit53,a2bit54,a2bit55,a2bit56,a2bit57,a2bit58,a2bit59,a2bit60,a2bit61,a2bit62,a2bit63,Inverter,MAX,MIN,Motor;
  
 
  if (CAN_MSGAVAIL == CAN.checkReceive()) {         // check if data coming
        CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf   
        unsigned long canId = CAN.getCanId();  
        
        if(canId==785){   
        Serial.println(buf[0], BIN);
        Serial.println(buf[0], HEX);
        
        a1bit0 =bitRead(buf[0], 0);
        a1bit1 =bitRead(buf[0], 1);
        a1bit2 =bitRead(buf[0], 2);
        a1bit3 =bitRead(buf[0], 3);
        a1bit4 =bitRead(buf[0], 4);
        a1bit5 =bitRead(buf[0], 5);
        a1bit6 =bitRead(buf[0], 6);
        a1bit7 =bitRead(buf[0], 7);
        a1bit8 =bitRead(buf[1], 0);
        a1bit9 =bitRead(buf[1], 1);
        a1bit10 =bitRead(buf[1], 2);
        a1bit11 =bitRead(buf[1], 3);
        a1bit12 =bitRead(buf[1], 4);
        a1bit13 =bitRead(buf[1], 5);
        a1bit14 =bitRead(buf[1], 6);
        a1bit15 =bitRead(buf[1], 7);
        a1bit16 =bitRead(buf[2], 0);
        a1bit17 =bitRead(buf[2], 1);
        a1bit18 =bitRead(buf[2], 2);
        a1bit19 =bitRead(buf[2], 3);
        a1bit20 =bitRead(buf[2], 4);
        a1bit21 =bitRead(buf[2], 5);
        a1bit22 =bitRead(buf[2], 6);
        a1bit23 =bitRead(buf[2], 7);
        a1bit24 =bitRead(buf[3], 0);
        a1bit25 =bitRead(buf[3], 1);
        a1bit26 =bitRead(buf[3], 2);
        a1bit27 =bitRead(buf[3], 3);
        a1bit28 =bitRead(buf[3], 4);
        a1bit29 =bitRead(buf[3], 5);
        a1bit30 =bitRead(buf[3], 6);
        a1bit31 =bitRead(buf[3], 7);
        a1bit32 =bitRead(buf[4], 0);
        a1bit33 =bitRead(buf[4], 1);
        a1bit34 =bitRead(buf[4], 2);
        a1bit35 =bitRead(buf[4], 3);
        a1bit36 =bitRead(buf[4], 4);
        a1bit37 =bitRead(buf[4], 5);
        a1bit38 =bitRead(buf[4], 6);
        a1bit39 =bitRead(buf[4], 7);
        a1bit40 =bitRead(buf[5], 0);
        a1bit41 =bitRead(buf[5], 1);
        a1bit42 =bitRead(buf[5], 2);
        a1bit43 =bitRead(buf[5], 3);
        a1bit61 =bitRead(buf[7], 5);
        a1bit62 =bitRead(buf[7], 6);
        a1bit63 =bitRead(buf[7], 7);


        Shutdown=a1bit0;
        if(Shutdown==String(1)){
          Serial.println("シャットダウン許可有効");
        }else{
          
          Serial.println("シャットダウン許可無効");
        }
        
        PWM=a1bit1+a1bit2;
        Serial.print("ゲート駆動状態 ");
        if(PWM==String(00)){
          Serial.println("短絡");
        }else if(PWM==String(01)){
          Serial.println("正常　トルクなし");
        }else if(PWM==String(10)){
          Serial.println("正常　制御中");
        }else {
          Serial.println("error");
        }

        Working=a1bit3+a1bit4+a1bit5;
        Serial.print("制御状態 ");
        if(Working==String(000)){
          Serial.println("初期状態");
        }else if(Working==String(001)){
          Serial.println("プリチャージ中");
        }else if(Working==String(010)){
          Serial.println("スタンバイ");
          }else if(Working==String(011)){
          Serial.println("トルク制御中");
          }else if(Working==String(111)){
          Serial.println("急速放電中");
        }else {
          Serial.println("error");
        }

        Speed=a1bit8+a1bit9+a1bit10+a1bit11+a1bit12+a1bit13+a1bit14+a1bit15+a1bit16+a1bit17+a1bit18+a1bit19+a1bit20+a1bit21+a1bit22+a1bit23;
         Serial.print("モータ回転数 ");
        if(Speed==String(0000000000000000)){
           Serial.println("調整中　0位置");
         }else{
          
           Serial.println(Speed);
         }
        Current=a1bit24+a1bit25+a1bit26+a1bit27+a1bit28+a1bit29+a1bit30+a1bit31+a1bit32+a1bit33;
         Serial.print("モータ相電流 ");
        if(Current==String(0000000000)){
           Serial.println("調整中　0位置");
         }else{
          
           Serial.println(Current);
          }

        Voltage=a1bit34+a1bit35+a1bit36+a1bit37+a1bit38+a1bit39+a1bit40+a1bit41+a1bit42+a1bit43;
         Serial.print("入力直流電圧 ");
        if(Voltage==String(0000000000)){
           Serial.println("調整中　0位置");
         }else{
          
           Serial.println(Voltage);
        }
        Failure=a1bit61+a1bit62+a1bit63;
         Serial.print("異常状態 ");
        if(Failure==String(000)){
          Serial.println("エラーなし");
        }else if(Failure==String(001)){
          Serial.println("負荷軽減");
        }else if(Failure==String(010)){
          Serial.println("警告");
          }else if(Failure==String(101)){
          Serial.println("クリティカルエラー");
        }else {
          Serial.println("error");
        }

        if(canId==801){   
        Serial.println(buf[0], BIN);
        Serial.println(buf[0], HEX);
        
        a2bit0 =bitRead(buf[0], 0);
        a2bit1 =bitRead(buf[0], 1);
        a2bit2 =bitRead(buf[0], 2);
        a2bit3 =bitRead(buf[0], 3);
        a2bit4 =bitRead(buf[0], 4);
        a2bit5 =bitRead(buf[0], 5);
        a2bit6 =bitRead(buf[0], 6);
        a2bit7 =bitRead(buf[0], 7);
        a2bit8 =bitRead(buf[1], 0);
        a2bit9 =bitRead(buf[1], 1);
        a2bit10 =bitRead(buf[1], 2);
        a2bit11 =bitRead(buf[1], 3);
        a2bit12 =bitRead(buf[1], 4);
        a2bit13 =bitRead(buf[1], 5);
        a2bit14 =bitRead(buf[1], 6);
        a2bit15 =bitRead(buf[1], 7);
        a2bit16 =bitRead(buf[2], 0);
        a2bit17 =bitRead(buf[2], 1);
        a2bit18 =bitRead(buf[2], 2);
        a2bit19 =bitRead(buf[2], 3);
        a2bit20 =bitRead(buf[2], 4);
        a2bit21 =bitRead(buf[2], 5);
        a2bit22 =bitRead(buf[2], 6);
        a2bit23 =bitRead(buf[2], 7);
        a2bit24 =bitRead(buf[3], 0);
        a2bit25 =bitRead(buf[3], 1);
        a2bit26 =bitRead(buf[3], 2);
        a2bit27 =bitRead(buf[3], 3);
        a2bit28 =bitRead(buf[3], 4);
        a2bit29 =bitRead(buf[3], 5);
        a2bit30 =bitRead(buf[3], 6);
        a2bit31 =bitRead(buf[3], 7);
        a2bit32 =bitRead(buf[4], 0);
        a2bit33 =bitRead(buf[4], 1);
        a2bit34 =bitRead(buf[4], 2);
        a2bit35 =bitRead(buf[4], 3);
        a2bit36 =bitRead(buf[4], 4);
        a2bit37 =bitRead(buf[4], 5);
        a2bit38 =bitRead(buf[4], 6);
        a2bit39 =bitRead(buf[4], 7);
        
        Inverter=a2bit0+a2bit1+a2bit2+a2bit3+a2bit4+a2bit5+a2bit6+a2bit7;
         Serial.print("インバータ温度　");
        if(Speed==String(00000000000000)){
           Serial.println("調整中　0位置");
         }else{
          
           Serial.println(Inverter);
         }
        }

        MAX=a2bit8+a2bit9+a2bit10+a2bit11+a2bit12+a2bit13+a2bit14+a2bit15+a2bit16+a2bit17+a2bit18+a2bit19;
          Serial.print("モーター上限制限トルク　");
        if(Speed==String(00000000000000)){
           Serial.println("調整中　0位置");
         }else{
          
           Serial.println(MAX);
         }
        }

        MIN=a2bit20+a2bit21+a2bit22+a2bit23+a2bit24+a2bit25+a2bit26+a2bit27+a2bit28+a2bit29+a2bit30+a2bit31;
          Serial.print("モーター下限制限トルク　");
        if(Speed==String(00000000000000)){
           Serial.println("調整中　0位置");
         }else{
          
           Serial.println(MIN);
         }

        Motor=a2bit32+a2bit33+a2bit34+a2bit35+a2bit36+a2bit37+a2bit38+a2bit39;
          Serial.print("モーター温度　");
        if(Speed==String(00000000000000)){
           Serial.println("調整中　0位置");
         }else{
          
           Serial.println(Motor);
         }
        }



        // BIT=bit3+bit4+bit5;
        // SERIAL_PORT_MONITOR.println(BIT);

        // if(BIT==String(110)){
        //   SERIAL_PORT_MONITOR.println("rapid discharge");
        // }else{
          
        //   SERIAL_PORT_MONITOR.println("error");
        // }
        // }
      }
    
