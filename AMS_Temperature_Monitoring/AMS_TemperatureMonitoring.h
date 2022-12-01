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

#ifndef AMS_TEMP_MONITOR
#define AMS_TEMP_MONITOR

#include <math.h>

//サーミスタと直列につなぐ抵抗の値
#define R 10000.0f

//25度の時のサーミスタの抵抗値
#define R0 10000.0f
#define T0 25.0f

//B定数
#define B 3380.0f

//最高・最低限度温度
#define TEMP_MAX 30.0f
#define TEMP_MIN 0.0f

//サーミスタ数(使うアナログピン数) <= 6
#define THM_NUM 4

#define INIT 0
#define SAFE 1
#define WARNING 2
#define DANGER 3

typedef struct {
  //アナログピンから読み取った値を格納
  volatile int val;
  //サーミスタの抵抗値を格納
  volatile float val_r;
  //サーミスタの抵抗から計算した温度を格納
  volatile double val_temp;
  //過去と現在の読み込んだ値を入れておく
  volatile int chatt[3];
} thermistor;

//読み込んだ電圧からサーミスタの抵抗を計算
float calc_R(int val);

//抵抗から温度計算
double calc_temp(float r);

//シリアルモニタで温度チェック+最大・最小・平均温度更新
void checkTemp();

//最大温度取得
//thm サーミスタのパラメータ配列の先頭のポインタ
double getMaxTemp(thermistor *thm);

//最小温度取得
//thm サーミスタのパラメータ配列の先頭のポインタ
double getMinTemp(thermistor *thm);

//平均温度取得
//thm サーミスタのパラメータ配列の先頭のポインタ
double getTempAvr(thermistor *thm);

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

double getMaxTemp(thermistor *thm) {
  double max_temp = -273;
  for (int i=0; i<THM_NUM; i++) {
    if ((thm + i)->val_temp > max_temp) {
      max_temp = (thm + i)->val_temp;
    }
  }
  return max_temp;
}

double getMinTemp(thermistor *thm) {
  double min_temp = 273;
  for (int i=0; i<THM_NUM; i++) {
    if ((thm + i)->val_temp < min_temp) {
      min_temp = (thm + i)->val_temp;
    }
  }
  return min_temp;
}

double getTempAvr(thermistor *thm) {
  double avr = 0;
  for (int i=0; i<THM_NUM; i++) {
    avr += (thm + i)->val_temp;
  }
  avr = avr / THM_NUM;
  return avr;
}

#endif