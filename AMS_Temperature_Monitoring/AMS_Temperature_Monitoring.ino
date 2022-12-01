#include "AMS_TemperatureMonitoring.h"
#include <MsTimer2.h>
#include <Wire.h>

#define ADRS 8

//電圧読み取り用ピン番号
#define THM_0 0
#define THM_1 1
#define THM_2 2
#define THM_3 3
#define THM_4 6
#define THM_5 7

// バッテリー温度危険信号出力ピン番号
#define Danger_SGN 6

#define INIT 0
#define SAFE 1
#define WARNING 2
#define DANGER 3

//すべてのサーミスタの最大・最小・平均温度
volatile double temp_THM_MAX,temp_THM_MIN,temp_THM_AVR;
volatile unsigned char nowState = INIT;
volatile unsigned char nextState = INIT;

//サーミスタの構造体配列
volatile thermistor data_THM[THM_NUM];
//data_THMのポインタ
volatile thermistor *p_thm;

//マスターからデータのリクエストがあった時に実行
void sendData();
//初期化関数、センサーのキャリブレーションなど
void _init(int time);

void setup() {
  _init(1000);

  Wire.begin(ADRS);
  Wire.onRequest(sendData);
  
  Serial.begin(9600);
  
  pinMode(THM_0,INPUT);
  pinMode(THM_1,INPUT);
  pinMode(THM_2,INPUT);
  pinMode(THM_3,INPUT);
  pinMode(THM_4,INPUT);
  pinMode(THM_5,INPUT);

  pinMode(Danger_SGN,OUTPUT);

  //シリアルモニタを使わない時または実機で動かす時はコメントアウトしておく
  //MsTimer2::set(500, checkTemp);
  //MsTimer2::start();
}

void loop() {

  data_THM[0].val = analogRead(THM_0);
  data_THM[1].val = analogRead(THM_1);
  data_THM[2].val = analogRead(THM_2);
  data_THM[3].val = analogRead(THM_3);
  //data_THM[4].val = analogRead(THM_4);
  //data_THM[5].val = analogRead(THM_5);

  for (int i=0; i<THM_NUM; i++) {
    (p_thm + i)->val_r = calc_R((p_thm + i)->val);
    (p_thm + i)->val_temp = calc_temp((p_thm + i)->val_r);
  }

  temp_THM_MAX = getMaxTemp(data_THM);
  temp_THM_MIN = getMinTemp(data_THM);
  temp_THM_AVR = getTempAvr(data_THM);

  if(
    (data_THM[0].val_temp > TEMP_MIN)
    && (data_THM[1].val_temp > TEMP_MIN)
    && (data_THM[2].val_temp > TEMP_MIN)
    && (data_THM[3].val_temp > TEMP_MIN)
    //&& (data_THM[4].val_temp > TEMP_MIN)
    //&& (data_THM[5].val_temp > TEMP_MIN)
    ) {
      if(
      (TEMP_MAX > data_THM[0].val_temp)
      && (TEMP_MAX > data_THM[1].val_temp) 
      && (TEMP_MAX > data_THM[2].val_temp) 
      && (TEMP_MAX > data_THM[3].val_temp) 
      //&& (TEMP_MAX > data_THM[4].val_temp)
      //&& (TEMP_MAX > data_THM[5].val_temp)
      ){
        digitalWrite(Danger_SGN, LOW);  //常時LOW
      }
      else{
        digitalWrite(Danger_SGN, HIGH); //異常検知HIGH
      }
    }else{
      digitalWrite(Danger_SGN, HIGH); //異常検知HIGH
    }
}

void checkTemp() {
  Serial.print("THM_0 : ");
  Serial.println(data_THM[0].val_temp);
  Serial.print("THM_1 : ");
  Serial.println(data_THM[1].val_temp);
  Serial.print("THM_2 : ");
  Serial.println(data_THM[2].val_temp);
  Serial.print("THM_3 : ");
  Serial.println(data_THM[3].val_temp);
  /*Serial.print("THM_4 : ");　　//はんだがめんどくさいから実験中はつかわない
  Serial.println(data_THM[4].val_temp);*/
  Serial.print("MAX TEMP : ");
  Serial.println(temp_THM_MAX);
  Serial.print("MIN TEMP : ");
  Serial.println(temp_THM_MIN);
  Serial.print("AVR TEMP : ");
  Serial.println(temp_THM_AVR);
}

void sendData() {
  for (int i=0; i<THM_NUM; i++) {
    Wire.write((char) ((p_thm + i)->val / 4));
  }
}

void _init(int time) {
  p_thm = data_THM;
  while (millis() < time) {
    data_THM[0].val = analogRead(THM_0);
    data_THM[1].val = analogRead(THM_1);
    data_THM[2].val = analogRead(THM_2);
    data_THM[3].val = analogRead(THM_3);
    //data_THM[4].val = analogRead(THM_4);
    //data_THM[5].val = analogRead(THM_5);
    for (int i=0; i<THM_NUM; i++) {
      (p_thm + i)->val_r = calc_R((p_thm + i)->val);
      (p_thm + i)->val_temp = calc_temp((p_thm + i)->val_r);
      (p_thm + i)->chatt[2] = (p_thm + i)->chatt[1];
      (p_thm + i)->chatt[1] = (p_thm + i)->chatt[0];
      (p_thm + i)->chatt[0] = (p_thm + i)->val;
    }
    delay(1);    
  }
}