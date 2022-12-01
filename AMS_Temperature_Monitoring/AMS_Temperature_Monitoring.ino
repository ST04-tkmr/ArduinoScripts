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

//すべてのサーミスタの最大・最小・平均温度
volatile double temp_THM_MAX,temp_THM_MIN,temp_THM_AVR;
volatile unsigned char nowState = INIT;
volatile unsigned char nextState = INIT;

//サーミスタの構造体配列
volatile thermistor data_THM[THM_NUM];
//data_THMのポインタ
volatile thermistor *p_thm;

//アナログピンで読み取る値の平均
volatile int val_avr;
//val_avrの1割、過去と現在のvalの差がこの値以内に治っている状態で判定を行う(ノイズ対策)
volatile int allowable_val_err;
//現在の時間
volatile unsigned long nowTime;
//最後にキャリブレーションを実行した時間
volatile unsigned long lastCalTime;
//キャリブレーションのインターバル(ms)
const unsigned long CAL_INTERVAL = 5000;

//マスターからデータのリクエストがあった時に実行
void sendData();
//初期化関数、センサーのキャリブレーションなど
void _init(int time);
//キャリブレーション実行
void runCalibration();

void setup() {
  //安定させるために起動後1秒間のキャリブレーション
  _init(1000);

  //シリアル通信開始  
  Wire.begin(ADRS);
  //データ要求された時に実行する関数を登録
  Wire.onRequest(sendData);
  
  Serial.begin(9600);
  
  //ピンモード設定
  pinMode(THM_0,INPUT);
  pinMode(THM_1,INPUT);
  pinMode(THM_2,INPUT);
  pinMode(THM_3,INPUT);
  pinMode(THM_4,INPUT);
  pinMode(THM_5,INPUT);

  pinMode(Danger_SGN,OUTPUT);

  //シリアルモニタで監視
  //割り込み処理で実行するので、シリアルモニタを使わない時または実機で動かす時はコメントアウトしておく
  //MsTimer2::set(500, checkTemp);
  //MsTimer2::start();
}

void loop() {
  //電圧読み取り
  data_THM[0].val = analogRead(THM_0);
  data_THM[1].val = analogRead(THM_1);
  data_THM[2].val = analogRead(THM_2);
  data_THM[3].val = analogRead(THM_3);
  //data_THM[4].val = analogRead(THM_4);
  //data_THM[5].val = analogRead(THM_5);

  //抵抗、温度計算
  for (int i=0; i<THM_NUM; i++) {
    (p_thm + i)->val_r = calc_R((p_thm + i)->val);
    (p_thm + i)->val_temp = calc_temp((p_thm + i)->val_r);
  }

  temp_THM_MAX = getMaxTemp(data_THM);
  temp_THM_MIN = getMinTemp(data_THM);
  temp_THM_AVR = getTempAvr(data_THM);

  if (
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
    } else {
      digitalWrite(Danger_SGN, HIGH); //異常検知HIGH
    }
  } else {
    digitalWrite(Danger_SGN, HIGH); //異常検知HIGH
  }

  nowTime = millis();
  //前回のキャリブレーションから一定時間後に再度実行
  if ((nowTime - lastCalTime) > CAL_INTERVAL) {
    runCalibration();
  }

  //millisがオーバーフローしたら更新
  if (nowTime <= lastCalTime) {
    lastCalTime = nowTime;
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
  //アナログピンから読み取った値をマスタに送る
  //送れるデータは1バイトなのでvalを4で割る
  for (int i=0; i<THM_NUM; i++) {
    Wire.write((char) ((p_thm + i)->val / 4));
  }
}

void _init(int time) {
  //アナログピンから読み取った値を合計
  int val_sum = 0;
  p_thm = data_THM;

  //引数で受け取った時間分データ更新
  while (millis() < time) {
    data_THM[0].val = analogRead(THM_0);
    data_THM[1].val = analogRead(THM_1);
    data_THM[2].val = analogRead(THM_2);
    data_THM[3].val = analogRead(THM_3);
    //data_THM[4].val = analogRead(THM_4);
    //data_THM[5].val = analogRead(THM_5);

    //過去と現在の値更新
    for (int i=0; i<THM_NUM; i++) {
      (p_thm + i)->chatt[2] = (p_thm + i)->chatt[1];
      (p_thm + i)->chatt[1] = (p_thm + i)->chatt[0];
      (p_thm + i)->chatt[0] = (p_thm + i)->val;
    }

    delay(1); 
  }

  //抵抗値、温度、valの合計
  for (int i=0; i<THM_NUM; i++) {
    (p_thm + i)->val_r = calc_R((p_thm + i)->val);
    (p_thm + i)->val_temp = calc_temp((p_thm + i)->val_r);
    val_sum += (p_thm + i)->val;
  }
  //平均値
  val_avr = val_sum / THM_NUM;
  //平均値の1割
  allowable_val_err = val_avr / 10;

  //各データ算出
  temp_THM_MAX = getMaxTemp(data_THM);
  temp_THM_MIN = getMinTemp(data_THM);
  temp_THM_AVR = getTempAvr(data_THM);

  //初期値をセット
  lastCalTime = millis();
  nowTime = millis();
}

void runCalibration() {
  //アナログピンから読み取った値を合計
  int val_sum = 0;
  data_THM[0].val = analogRead(THM_0);
  data_THM[1].val = analogRead(THM_1);
  data_THM[2].val = analogRead(THM_2);    
  data_THM[3].val = analogRead(THM_3);
  //data_THM[4].val = analogRead(THM_4);
  //data_THM[5].val = analogRead(THM_5);

  //過去と現在の値更新
  for (int i=0; i<THM_NUM; i++) {
    (p_thm + i)->chatt[2] = (p_thm + i)->chatt[1];
    (p_thm + i)->chatt[1] = (p_thm + i)->chatt[0];
    (p_thm + i)->chatt[0] = (p_thm + i)->val;
  }

  //抵抗値、温度、valの合計
  for (int i=0; i<THM_NUM; i++) {
    (p_thm + i)->val_r = calc_R((p_thm + i)->val);
    (p_thm + i)->val_temp = calc_temp((p_thm + i)->val_r);
    val_sum += (p_thm + i)->val;
  }
  //平均値
  val_avr = val_sum / THM_NUM;
  //平均値の1割
  allowable_val_err = val_avr / 10;

  lastCalTime = nowTime;
}