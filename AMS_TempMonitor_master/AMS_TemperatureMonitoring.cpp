#include "AMS_TemperatureMonitoring.h"

//  サーミスタと直列につなぐ抵抗の値
static const float r = 10000.0f;

//  25度の時のサーミスタの抵抗値
static const float r0 = 10000.0f;
static const float t0 = 25.0f;

//  B定数
static const float b = 3380.0f;

//  最高許容温度
static const float allowableTempMax = 30.0f;

//  最低許容温度
static const float allowableTempMin = 0.0f;

float getAllowableTempMax(void) {
  return allowableTempMax;
}

float getAllowableTempMin(void) {
  return allowableTempMin;
}

float calcR(int val) {
  float vd,thmR;
  vd = val * 0.0049f; //実際の電圧値に変換
  thmR = (r * vd) / (5 - vd);  //サーミスタの抵抗計算
  return thmR;
}

float calcTemp(float thmR) {
  float logOfr = (float) (log(thmR / r0));
  float temp = 1 / ((logOfr / b) + (1 / (t0 + 273.0f))) - 273.0f;
  return temp;
}

float getMaxTemp(volatile thermistor *thm) {
  float maxTemp = -273;
  for (int i=0; i<THM_NUM; i++) {
    if ((thm + i)->valTemp > maxTemp) {
      maxTemp = (thm + i)->valTemp;
    }
  }
  return maxTemp;
}

float getMinTemp(volatile thermistor *thm) {
  float minTemp = 273;
  for (int i=0; i<THM_NUM; i++) {
    if ((thm + i)->valTemp < minTemp) {
      minTemp = (thm + i)->valTemp;
    }
  }
  return minTemp;
}

float getAvrTemp(volatile thermistor *thm) {
  float avr = 0;
  for (int i=0; i<THM_NUM; i++) {
    avr += (thm + i)->valTemp;
  }
  avr = avr / THM_NUM;
  return avr;
}
