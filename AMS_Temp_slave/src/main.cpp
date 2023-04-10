#include <Arduino.h>
#include <MsTimer2.h>
#include <Wire.h>
#include "Thermistor.hpp"

#define ADRS (0b0000001)

#define THM0 (0)
#define THM1 (1)
#define THM2 (2)
#define THM3 (3)
#define THM4 (6)
#define THM5 (7)

//データ送信
void sendData(void);

//シリアルモニタで温度確認
//シリアルモニタを使わないときはi2cの邪魔をするので実行しない
void checkTemp(void);

void setup() {
  //ピンモード設定
  pinMode(THM0, INPUT);
  pinMode(THM1, INPUT);
  pinMode(THM2, INPUT);
  pinMode(THM3, INPUT);
  pinMode(THM4, INPUT);
  pinMode(THM5, INPUT);

  //シリアル通信開始
  Wire.begin(ADRS);
}

void loop() {

}

void sendData(void) {

}

void checkTemp(void) {

}