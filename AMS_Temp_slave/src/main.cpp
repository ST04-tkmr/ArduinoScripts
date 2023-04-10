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

static const unsigned char thmNum = 6;  //サーミスタ本数
volatile float maxTemp, minTemp, avrTemp; //最高, 最低, 平均温度
const float allowableMaxTemp = 60.0f;
const float allowableMinTemp = 0.0f;

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
  Wire.onRequest(sendData);
}

void loop() {

}

void sendData(void) {

}

void checkTemp(void) {

}