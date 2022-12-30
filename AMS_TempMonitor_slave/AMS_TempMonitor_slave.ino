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

//すべてのサーミスタの最大・最小・平均温度
volatile float tempThmMax,tempThmMin,tempThmAvr;

//スレーブ側では判定を行わないのでコメントアウト
/*
//過去の状態
volatile state pastState;
//現在の状態
volatile state nowState;
//次の状態
volatile state nextState;
//警告フラグ
volatile unsigned char warningFlag;
//危険フラグ、1になったら再起動するまで0にならない
volatile unsigned char dangerFlag;
*/

//サーミスタの構造体配列
volatile thermistor thmData[THM_NUM];

//data_THMのポインタ
volatile thermistor *const pThm = thmData;

//ヒステリシス
volatile float hys;

//現在の時間
volatile unsigned long nowTime;

//最後にキャリブレーションを実行した時間
volatile unsigned long lastCalTime;

//キャリブレーションのインターバル(ms)
static const unsigned long calInterval = 5000;

//シリアルモニタで温度チェック+最大・最小・平均温度更新
void checkTemp();
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

  pinMode(WARNING_SGN,OUTPUT);
  pinMode(DANGER_SGN,OUTPUT);

  //シリアルモニタで監視
  //割り込み処理で実行するので、シリアルモニタを使わない時または実機で動かす時はコメントアウトしておく
  //MsTimer2::set(500, checkTemp);
  //MsTimer2::start();
}

void loop() {
  //電圧読み取り
  pThm->val = analogRead(THM_0);
  (pThm + 1)->val = analogRead(THM_1);
  (pThm + 2)->val = analogRead(THM_2);    
  (pThm + 3)->val = analogRead(THM_3);
  //(pThm + 4)->val = analogRead(THM_4);
  //(pThm + 5)->val = analogRead(THM_5);

  //抵抗、温度計算
  for (int i=0; i<THM_NUM; i++) {
    (pThm + i)->valR = calcR((pThm + i)->val);
    (pThm + i)->valTemp = calcTemp((pThm + i)->valR);
  }

  tempThmMax = getMaxTemp(pThm);
  tempThmMin = getMinTemp(pThm);
  tempThmAvr = getAvrTemp(pThm);

  //温度判定、次の状態を更新
  //スレーブ側ではやらないのでコメントアウト
  /*
  if (tempThmMin <= (getAllowableTempMin() + hys) || tempThmMax >= (getAllowableTempMax() - hys)) {
    if (nowState == SAFE) {
      nextState = WARNING;  //SAFE -> WARNING
    } else if (nowState == WARNING) {
      if (tempThmMin <= getAllowableTempMin() || tempThmMax >= getAllowableTempMax()) {
        nextState = DANGER; //WARNING -> DANGER
      } else if (tempThmMin > (getAllowableTempMin() + hys) && tempThmMax < (getAllowableTempMax() - hys)) {
        nextState = SAFE; //WARNING -> SAFE
      }
    } else if (nowState == DANGER) {
      if ((tempThmMin > getAllowableTempMin()) && (tempThmMax < getAllowableTempMax())) {
        nextState = WARNING;  //DANGER -> WARNING
      }
    }
  }

  pastState = nowState; //過去の状態を更新
  nowState = nextState; //現在の状態を更新

  //警告、危険が連続したらフラグが立つ
  warningFlag = (pastState == WARNING) && (nowState == WARNING);
  if (!dangerFlag) {  //フラグが立ったら更新しない
    dangerFlag = (pastState == DANGER) && (nowState == DANGER);
  }

  //警告、危険信号出力
  if (warningFlag) {

  } else if (dangerFlag) {
    digitalWrite(Danger_SGN, HIGH);
  }
  */

  nowTime = millis();
  //前回のキャリブレーションから一定時間後に再度実行
  if ((nowTime - lastCalTime) > calInterval) {
    runCalibration(); //キャリブレーション
    lastCalTime = nowTime;  //最後にキャリブレーションを行った時間を更新
  }

  //millisがオーバーフロー(約50日)したら更新
  if (nowTime < lastCalTime) {
    lastCalTime = nowTime;
  }
}

void checkTemp() {
  Serial.print("THM_0 : ");
  Serial.println(thmData[0].valTemp);
  Serial.print("THM_1 : ");
  Serial.println(thmData[1].valTemp);
  Serial.print("THM_2 : ");
  Serial.println(thmData[2].valTemp);
  Serial.print("THM_3 : ");
  Serial.println(thmData[3].valTemp);
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
  for (int i=0; i<THM_NUM; i++) {
    Wire.write((char) ((pThm + i)->val / 4));
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
    runCalibration();

    delay(1); 
  }

  //初期値をセット
  nowTime = millis();
  lastCalTime = nowTime;
}

void runCalibration() {
  pThm->val = analogRead(THM_0);
  (pThm + 1)->val = analogRead(THM_1);
  (pThm + 2)->val = analogRead(THM_2);    
  (pThm + 3)->val = analogRead(THM_3);
  //(pThm + 4)->val = analogRead(THM_4);
  //(pThm + 5)->val = analogRead(THM_5);

  //抵抗値、温度計算
  for (int i=0; i<THM_NUM; i++) {
    (pThm + i)->valR = calcR((pThm + i)->val);
    (pThm + i)->valTemp = calcTemp((pThm + i)->valR);
  }

  //各データ算出
  tempThmMax = getMaxTemp(thmData);
  tempThmMin = getMinTemp(thmData);
  tempThmAvr = getAvrTemp(thmData);

  //ヒステリシス(平均の1割)
  hys = tempThmAvr / 10;

  //スレーブ側では判定を行わないのでコメントアウト
  //起動時(初期化状態)のみ実行
  //最大最小平均すべてが許容範囲内である場合のみSAFE状態とする
  /*
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
