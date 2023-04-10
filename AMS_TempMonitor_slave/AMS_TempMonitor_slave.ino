#include <MsTimer2.h>
#include <Wire.h>
#include "AMS_TemperatureMonitoring.h"

#define ADRS (0b0000000)

//電圧読み取り用ピン番号
#define THM_0 (0)
#define THM_1 (1)
#define THM_2 (2)
#define THM_3 (3)
#define THM_4 (6)
#define THM_5 (7)

//バッテリー温度警告信号出力ピン番号
#define WARNING_SGN (7)

// バッテリー温度危険信号出力ピン番号
#define DANGER_SGN (6)

namespace AMS = AmsTempMonitor;
namespace AMSPara = AmsTempMonitor::AmsParams;

//すべてのサーミスタの最大・最小・平均温度
volatile float tempThmMax,tempThmMin,tempThmAvr;

//最大・最小温度のサーミスタのインデックス
volatile unsigned char thmDataIndexMax, thmDataIndexMin;

//サーミスタ数
volatile static const unsigned char thmNum = 4;

//シリアルモニタで温度チェック+最大・最小・平均温度更新
void checkTemp();

//マスターからデータのリクエストがあった時に実行
void sendData();

//初期化関数、センサーのキャリブレーションなど
void _init(int time);

//サーミスタの電圧を読み取る処理をまとめた関数
void readThm();

void setup() {
  AMS::initAMS(thmNum, 30.0f, 0.0f, 0);
  AMS::setReadThmFunc(readThm);

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

  pinMode(WARNING_SGN,OUTPUT);
  pinMode(DANGER_SGN,OUTPUT);

  //シリアルモニタで監視
  //割り込み処理で実行するので、シリアルモニタを使わない時または実機で動かす時はコメントアウトしておく
  //MsTimer2::set(500, checkTemp);
  //MsTimer2::start();
}

void loop() {
  unsigned char dangerFlag = AMS::runAMS(millis());

  //危険信号出力
  if (dangerFlag) {
    digitalWrite(DANGER_SGN, HIGH);
  }
}

void checkTemp() {
  Serial.print("THM_0 : ");
  //Serial.println(thmData[0].valTemp);
  Serial.print("THM_1 : ");
  //Serial.println(thmData[1].valTemp);
  Serial.print("THM_2 : ");
  //Serial.println(thmData[2].valTemp);
  Serial.print("THM_3 : ");
  //Serial.println(thmData[3].valTemp);
  /*Serial.print("THM_4 : ");
  Serial.println(thmData[4].valTemp);
  Serial.print("THM_5 : ");
  Serial.println(thmData[5].valTemp);*/
  Serial.print("MAX TEMP : ");
  Serial.println(tempThmMax);
  Serial.print("MIN TEMP : ");
  Serial.println(tempThmMin);
  Serial.print("AVR TEMP : ");
  Serial.println(tempThmAvr);
}

void sendData() {
  //アナログピンから読み取った値をマスタに送る
  //送れるデータは1バイトなのでvalを4で割る
  for (int i=0; i<thmNum; i++) {
    //Wire.write((char) ((pThm + i)->val / 4));
  }
}

void _init(int time) {
  //スレーブ側では判定を行わないのでコメントアウト
  /*
  pastState = INIT;
  nowState = INIT;
  nextState = INIT;
  warningFlag = 0;
  dangerFlag = 0;
  */

  //引数で受け取った時間分データ更新
  while (millis() < time) {
    readThm();

    delay(1);
  }
}

void readThm() {
  AmsTempMonitor::setValofThm(0, analogRead(THM_0));
  AmsTempMonitor::setValofThm(1, analogRead(THM_1));
  AmsTempMonitor::setValofThm(2, analogRead(THM_2));
  AmsTempMonitor::setValofThm(3, analogRead(THM_3));
  //AmsTempMonitor::setValofThm(4, analogRead(THM_4));
  //AmsTempMonitor::setValofThm(5, analogRead(THM_5));

  //スレーブ側では判定を行わないのでコメントアウト
  //起動時(初期化状態)のみ実行
  //最大最小平均すべてが許容範囲内である場合のみSAFE状態とする
  /*
  //ヒステリシス(平均の1割)
  hys = tempThmAvr / 10;

  if (nowState == INIT) {
    if (
      (temp_THM_MIN > (TEMP_MIN + hys)) &&
      (temp_THM_MAX < (TEMP_MAX - hys)) &&
      (temp_THM_AVR > (TEMP_MIN + hys)) && (temp_THM_AVR < (TEMP_MAX - hys))
    ) {
      nowState = SAFE;
    } else {
      nowState = WARNING;
    }
  }
  */
}
