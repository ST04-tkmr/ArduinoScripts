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

//電圧読み取り用ピン番号
#define THM_0 A0
#define THM_1 A1
#define THM_2 A2
#define THM_3 A3
#define THM_4 A4

// バッテリー温度危険信号出力ピン番号
#define Danger_SGN 6

//サーミスタと直列につなぐ抵抗の値
#define R 10000.0f

//25度の時のサーミスタの抵抗値
#define R0 10000.0f
#define T0 25.0f

//B定数
#define B 3380.0f

//最高限度温度（本番は38℃）
#define TEMP_MAX 30.0f
#define TEMP_MIN 0.0f

//サーミスタ数(使うアナログピン数)
#define THM_NUM 4

//アナログピンから読み取った値を格納
int BCT[THM_NUM];

//サーミスタの抵抗値を格納
float THM_R[THM_NUM];

//サーミスタの抵抗から計算した温度を格納
double temp_THM[THM_NUM];

double temp_THM_MAX,temp_THM_AVR;

float chatt[THM_NUM][3];

char circuitOpenFlag;

float calc_R(int BCT);
double calc_temp(float r);
void checkTemp();
void checkOpen();

void setup() {
  
  Serial.begin(9600);
  
  pinMode(THM_0,INPUT);
  pinMode(THM_1,INPUT);
  pinMode(THM_2,INPUT);
  pinMode(THM_3,INPUT);
  pinMode(THM_4,INPUT);

  pinMode(Danger_SGN,OUTPUT);

  digitalWrite(Danger_SGN, HIGH);

  MsTimer2::set(500, checkTemp);
  MsTimer2::start();

  circuitOpenFlag = false;
}

void loop() {

  BCT[0] = analogRead(THM_0);
  BCT[1] = analogRead(THM_1);
  BCT[2] = analogRead(THM_2);
  BCT[3] = analogRead(THM_3);
  //BCT[4] = analogRead(THM_4);

  for (char i=0; i<THM_NUM; i++) {
    THM_R[i] = calc_R(BCT[i]);
    temp_THM[i] = calc_temp(THM_R[i]);
    chatt[i][2] = chatt[i][1];
    chatt[i][1] = chatt[i][0];
    chatt[i][0] = temp_THM[i];
  }

  //基準温度を超えたら信号をローにする
  if(temp_THM[0] > TEMP_MIN
  && temp_THM[1] > TEMP_MIN
  && temp_THM[2] > TEMP_MIN
  && temp_THM[3] > TEMP_MIN
  /*&& temp_THM[4] > TEMP_MIN*/) {
    if((TEMP_MAX > temp_THM[0])
    && (TEMP_MAX > temp_THM[1]) 
    && (TEMP_MAX > temp_THM[2]) 
    && (TEMP_MAX > temp_THM[3]) 
    /*&& (TEMP_MAX > temp_THM[4])*/){
      digitalWrite(Danger_SGN, LOW);  //常時LOW
    }
    else{
      digitalWrite(Danger_SGN, HIGH); //異常検知HIGH
    }
  }else{
    digitalWrite(Danger_SGN, HIGH); //異常検知HIGH
  }
  
}

//読み込んだ電圧からサーミスタの抵抗を計算
float calc_R(int BCT) {
  float vd,r;
  vd = BCT * 0.0049f; //実際の電圧値に変換
  r = (R * vd) / (5 - vd);  //サーミスタの抵抗計算
  return r;
}

//抵抗から温度計算
double calc_temp(float r) {
  double logOfr = log(r / R0);
  double temp = 1 / ((logOfr / B) + (1 / (T0 + 273.0f))) - 273.0f;
  return temp;
}

void checkTemp() {
  Serial.print("THM_0 : ");
  Serial.println(temp_THM[0]);
  Serial.print("THM_1 : ");
  Serial.println(temp_THM[1]);
  Serial.print("THM_2 : ");
  Serial.println(temp_THM[2]);
  Serial.print("THM_3 : ");
  Serial.println(temp_THM[3]);
  /*Serial.print("THM_4 : ");　　//はんだがめんどくさいから実験中はつかわない
  Serial.println(temp_THM[4]);*/
  temp_THM_MAX = temp_THM[getMaxTempIndex(temp_THM)];
  temp_THM_AVR = getTempAvr(temp_THM);
  Serial.print("MAX TEMP : ");
  Serial.println(temp_THM_MAX);
  Serial.print("AVR TEMP : ");
  Serial.println(temp_THM_AVR);
}

void checkOpen() {
  
}

char getMaxTempIndex(double temp[]) {
  char max_index = 0;
  double max_temp = -273;
  for (int i=0; i<THM_NUM; i++) {
    if (temp[i] > max_temp) {
      max_temp = temp[i];
      max_index = i;
    }
  }
  return max_index;
}

double getTempAvr(double temp[]) {
  double avr = 0;
  for (int i=0; i<THM_NUM; i++) {
    avr += temp[i];
  }
  avr = avr / THM_NUM;
  return avr;
}
