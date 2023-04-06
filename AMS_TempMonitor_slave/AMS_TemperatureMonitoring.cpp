#include "AMS_TemperatureMonitoring.h"

namespace {
  //  サーミスタ数
  volatile unsigned char thermistorNum;

  volatile Thermistor* pThm;

  //  最高許容温度
  volatile float allowableMaxTemp;

  //  最低許容温度
  volatile float allowableMinTemp;

  //  最高温度のサーミスタのインデックス
  volatile unsigned char indexMax;

  //  最低温度のサーミスタのインデックス
  volatile unsigned char indexMin;

  //  温度の判定まで行う場合は1, 測定のみの場合は0
  volatile unsigned char judgeMode;

  volatile State pastState, nowState, nextState;

  //  最後にキャリブレーションを実行した時間
  volatile unsigned long lastCalTime;

  //  キャリブレーションのインターバル(ms)
  static const unsigned long calInterval = 5000;

  // setReadThmFuncを実行したら立つ
  volatile unsigned char setFuncFlag;

  //  警告フラグ
  volatile unsigned char warningFlag;

  //  危険フラグ、1になったら再起動するまで0にならない
  volatile unsigned char dangerFlag;

  //  ヒステリシス
  volatile float hys;
}

void (*AmsTempMonitor::readValofThm)();

Thermistor::Thermistor() {
  val = 0;
  r = 0.0f;
  temp = 0.0f;
}

char Thermistor::setVal(int val) volatile {
  if (0 <= val && val <= 1023) {
    this->val = val;
    r = calcR(this->val);
    temp = calcTemp(this->r);
    return 1;
  } else {
    return 0;
  }
}

void AmsTempMonitor::initAMS(unsigned char thmNum, const float maxTemp, const float minTemp, unsigned char judgeFlag) {
  thermistorNum = thmNum;
  pThm = new Thermistor[thmNum];
  allowableMaxTemp = maxTemp;
  allowableMinTemp = minTemp;
  setFuncFlag = 0;
  warningFlag = 0;
  dangerFlag = 0;
  judgeMode = judgeFlag;
  pastState = State::INIT;
  nowState = State::INIT;
  nextState = State::INIT;
}

void AmsTempMonitor::initAMS(unsigned char thmNum, unsigned char judgeFlag) {
  AmsTempMonitor::initAMS(thmNum, 60.0f, 0.0f, judgeFlag);
}

unsigned char AmsTempMonitor::setValofThm(unsigned char index, int val) {
  if (index < thermistorNum) {
    return (pThm + index)->setVal(val);
  } else {
    return 0;
  }
}

void AmsTempMonitor::setReadThmFunc(void (*f)()) {
  readValofThm = f;
  setFuncFlag = 1;

}

unsigned char AmsTempMonitor::runAMS(unsigned long nowTime) {
  if (setFuncFlag) {
    (*readValofThm)();
  }

  float maxTemp = AmsParams::getMaxTemp();
  float minTemp = AmsParams::getMinTemp();
  float avrTemp = AmsParams::getAvrTemp();

  if ((nowTime - lastCalTime) > calInterval) {
    //  キャリブレーション
    hys = avrTemp / 10;

    //  最後にキャリブレーションを行った時間を更新
    lastCalTime = nowTime;
  }

  //  millisがオーバーフロー(約50日)したら更新
  if (nowTime < lastCalTime) {
    lastCalTime = nowTime;
  }

  if (judgeMode) {
    AmsTempMonitor::judgeTemp(maxTemp, minTemp, &warningFlag, &dangerFlag);
  }

  return dangerFlag;
}

void AmsTempMonitor::judgeTemp(float maxTemp, float minTemp, unsigned char *wFlag, unsigned char *dFlag) {
  //許容温度にヒステリシスをもたせる
  if (minTemp <= (allowableMinTemp + hys) || maxTemp >= (allowableMaxTemp - hys)) {
    if (nowState == State::SAFE) {
      nextState = State::WARNING;  //SAFE -> WARNING
    } else if (nowState == State::WARNING) {
      if (minTemp <= allowableMinTemp || maxTemp >= allowableMaxTemp) {
        nextState = State::DANGER; //WARNING -> DANGER
      } else if (minTemp > (allowableMinTemp + hys) && maxTemp < (allowableMaxTemp - hys)) {
        nextState = State::SAFE; //WARNING -> SAFE
      }
    } else if (nowState == State::DANGER) {
      if ((minTemp > allowableMinTemp) && (maxTemp < allowableMaxTemp)) {
        nextState = State::WARNING;  //DANGER -> WARNING
      }
    }
  }

  //  過去と現在の状態を更新
  pastState = nowState;
  nowState = nextState;

  //  警告、危険が連続したらフラグが立つ
  *wFlag = (pastState == State::WARNING) && (nowState == State::WARNING);
  if (!(*dFlag)) {
    //  フラグが立ったら更新しない
    *dFlag = (pastState == State::DANGER) && (nowState == State::DANGER);
  }
}

int AmsTempMonitor::AmsParams::getMaxVal() {
  return (pThm + indexMax)->getVal();
}

int AmsTempMonitor::AmsParams::getMinVal() {
  return (pThm + indexMin)->getVal();
}

int AmsTempMonitor::AmsParams::getAvrVal() {
  int avr = 0;
  for (int i=0; i<thermistorNum; i++) {
    avr += (pThm + i)->getVal();
  }
  avr = avr / thermistorNum;
  return avr;
}

float AmsTempMonitor::AmsParams::getThmR(unsigned char index) {
  if (index < thermistorNum) {
    return (pThm + index)->getR();
  } else {
    return 0;
  }
}

float AmsTempMonitor::AmsParams::getThmTemp(unsigned char index) {
  if (index < thermistorNum) {
    return (pThm + index)->getTemp();
  } else {
    return -273;
  }
}

float AmsTempMonitor::AmsParams::getMaxTemp() {
  float maxTemp = -273;
  indexMax = 0;
  for (int i=0; i<thermistorNum; i++) {
    float temp = (pThm + i)->getTemp();
    if (temp > maxTemp) {
      maxTemp = temp;
      indexMax = i;
    }
  }
  return maxTemp;
}

float AmsTempMonitor::AmsParams::getMinTemp() {
  float minTemp = 273;
  indexMin = 0;
  for (int i=0; i<thermistorNum; i++) {
    float temp = (pThm + i)->getTemp();
    if (temp < minTemp) {
      minTemp = temp;
      indexMin = i;
    }
  }
  return minTemp;
}

float AmsTempMonitor::AmsParams::getAvrTemp() {
  float avr = 0;
  for (int i=0; i<thermistorNum; i++) {
    avr += (pThm + i)->getTemp();
  }
  avr = avr / thermistorNum;
  return avr;
}

float calcR(int val) {
  const float r = 10000.0f; //サーミスタと直列につなぐ抵抗の値
  float vd,thmR;
  vd = val * 0.0049f; //実際の電圧値に変換
  thmR = (r * vd) / (5 - vd);  //サーミスタの抵抗計算
  return thmR;
}

float calcTemp(float thmR) {
  const float r0 = 10000.0f;  //25度の時のサーミスタの抵抗値
  const float t0 = 25.0f;
  const float b = 3380.0f;  //B定数
  float logOfr = (float) (log(thmR / r0));
  float temp = 1 / ((logOfr / b) + (1 / (t0 + 273.0f))) - 273.0f;
  return temp;
}
