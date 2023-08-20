#include <Arduino.h>
#include <MsTimer2.h>
#include <Wire.h>

#define ADRS (0b0000001)
// #define ADRS (0b0000010)
// #define ADRS (0b0000100)
// #define ADRS (0b0001000)

union THM_DATA
{
    uint8_t data[8];
    struct
    {
        uint16_t thm1 : 10;
        uint16_t thm2 : 10;
        uint16_t thm3 : 10;
        uint16_t thm4 : 10;
        uint16_t thm5 : 10;
        uint16_t thm6 : 10;
        uint8_t received : 4;
    };

    THM_DATA()
        : data{ 0, 0, 0, 0, 0, 0, 0, 0 }
    {
    }
};

const uint8_t thmNum = 6; // サーミスタ本数
static const uint8_t INPUT_PINS[6] = { A0, A1, A2, A3, A6, A7 };
THM_DATA *thm = new THM_DATA();

// データ送信
void sendData(void);

// シリアルモニタで温度確認
// シリアルモニタを使わないときはi2cの邪魔をするので実行しない
void checkData(void);

// サーミスタにかかる電圧を読んで値をセット
void readThmVoltage(void);

void setup()
{
    for (int8_t i = 0; i < thmNum; i++)
    {
        pinMode(INPUT_PINS[i], INPUT);
    }

    // シリアル通信開始
    Wire.begin(ADRS);
    Wire.onRequest(sendData);

    Serial.begin(115200);

    //MsTimer2::set(500, checkData);
    //MsTimer2::start();
}

void loop()
{
    readThmVoltage();
}

void sendData(void)
{
    Wire.write(thm->data, sizeof thm->data);
}

void checkData(void)
{
    Serial.println(thm->thm1);
    Serial.println(thm->thm2);
    Serial.println(thm->thm3);
    Serial.println(thm->thm4);
    Serial.println(thm->thm5);
    Serial.println(thm->thm6);
    Serial.println();
}

void readThmVoltage(void)
{
    thm->thm1 = analogRead(INPUT_PINS[0]);
    thm->thm2 = analogRead(INPUT_PINS[1]);
    thm->thm3 = analogRead(INPUT_PINS[2]);
    thm->thm4 = analogRead(INPUT_PINS[3]);
    thm->thm5 = analogRead(INPUT_PINS[4]);
    thm->thm6 = analogRead(INPUT_PINS[5]);
}