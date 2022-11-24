/*
 * 電圧の測定はサーミスタの電圧降下を測定
 * 
 *      |
 *      _
 *     | | R=10kΩ
 *     | |
 *      -
 *      |
 *      ・-----・ Vd(アナログピンで読み取り、10bitADCで変換されるので実際の電圧にするには4.9mVを掛け算する)
 *      |
 *      _
 *     | | サーミスタ r
 *     | |
 *      -
 *      |
 *      
 * Vd = { 5/(10k+r) } x r よりrを算出
 *      
 * 温度T0=25度のときのサーミスタの抵抗値R0=10kΩ
 * データシートよりB定数(25度/50度)は3380K
 * 
 * 抵抗から温度を算出する式
 * T = 1/{ln(r/R0)/B + 1/(T0+273)} -273
 */

#include <math.h>
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

//サーミスタと直列につなぐ抵抗の値
#define R 10000.0f

//25度の時のサーミスタの抵抗値
#define R0 10000.0f
#define T0 25.0f

//B定数
#define B 3380.0f

//最高・最低限度温度（本番は38℃）
#define TEMP_MAX 30.0f
#define TEMP_MIN 0.0f

//サーミスタ数(使うアナログピン数) <= 6
#define THM_NUM 4

//すべてのサーミスタの最大・最小・平均温度
volatile double temp_THM_MAX,temp_THM_MIN,temp_THM_AVR;
volatile unsigned char nowState = INIT;
volatile unsigned char nextState = INIT;

typedef struct {
  //アナログピンから読み取った値を格納
  volatile int val;
  //サーミスタの抵抗値を格納
  volatile float val_r;
  //サーミスタの抵抗から計算した温度を格納
  volatile double val_temp;
  //過去の読み込んだ値を入れておく
  volatile int chatt[3];
} thermistor;

//サーミスタの構造体配列
volatile thermistor data_THM[THM_NUM];
//data_THMのポインタ
volatile thermistor *p_thm;

//読み込んだ電圧からサーミスタの抵抗を計算
float calc_R(int val);
//抵抗から温度計算
double calc_temp(float r);
//シリアルモニタで温度チェック+最大・最小・平均温度更新
void checkTemp();
//最大温度取得
double getMaxTemp(thermistor *thm);
//最小温度取得
double getMinTemp(thermistor *thm);
//平均温度取得
double getTempAvr(thermistor *thm);
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
  digitalWrite(Danger_SGN, HIGH);

  MsTimer2::set(500, checkTemp);
  MsTimer2::start();
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

float calc_R(int val) {
  float vd,r;
  vd = val * 0.0049f; //実際の電圧値に変換
  r = (R * vd) / (5 - vd);  //サーミスタの抵抗計算
  return r;
}

double calc_temp(float r) {
  double logOfr = log(r / R0);
  double temp = 1 / ((logOfr / B) + (1 / (T0 + 273.0f))) - 273.0f;
  return temp;
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

double getMaxTemp(thermistor *thm) {
  double max_temp = -273;
  for (int i=0; i<THM_NUM; i++) {
    if ((p_thm + i)->val_temp > max_temp) {
      max_temp = (p_thm + i)->val_temp;
    }
  }
  return max_temp;
}

double getMinTemp(thermistor *thm) {
  double min_temp = 273;
  for (int i=0; i<THM_NUM; i++) {
    if ((p_thm + i)->val_temp < min_temp) {
      min_temp = (p_thm + i)->val_temp;
    }
  }
  return min_temp;
}

double getTempAvr(thermistor *thm) {
  double avr = 0;
  for (int i=0; i<THM_NUM; i++) {
    avr += (p_thm + i)->val_temp;
  }
  avr = avr / THM_NUM;
  return avr;
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