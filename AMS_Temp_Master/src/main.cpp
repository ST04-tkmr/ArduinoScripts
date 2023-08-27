#include <Arduino.h>
#include <MsTimer2.h>
#include <Wire.h>
#include "Thermistor.hpp"
#include "CAN_Temp.hpp"

#define DANGER_OUTPUT (3)

const unsigned long CAL_INTERVAL = 5000;

volatile uint8_t count;
uint8_t data[8];
Thermistor *thm;
volatile uint8_t countFlag;
volatile unsigned long nowTime, lastCalTime;
volatile unsigned char pastErrFlag[3];
volatile unsigned char dangerFlag;
CAN_Temp *ACC_Temp;

void handler(void);

void _init_(unsigned long time);

void runCalibration(void);

void setup()
{
    _init_(1000);

    pinMode(DANGER_OUTPUT, OUTPUT);

    Wire.begin();

    Serial.begin(115200);

    ACC_Temp = new CAN_Temp(ACC_ID);
    ACC_Temp->init();

    MsTimer2::set(100, handler);
    MsTimer2::start();
}

void loop()
{
    if (!countFlag)
    {
        if (count < ecuNum)
        {
            // データリクエスト
            Serial.print("count : ");
            Serial.println(count);

            Wire.requestFrom(adrs[count], sizeof(data));

            uint8_t i = 0;
            while (Wire.available())
            {
                data[i] = Wire.read();
                i++;
            }

            thm->setData(count, data, sizeof(data));

            Serial.print("raw data : ");
            Serial.println(count);
            for (char j = 0; j < thmNum; j++)
            {
                Serial.println(thm->getVal(count, j));
            }

            Serial.println("temperature");
            for (char j = 0; j < thmNum; j++)
            {
                Serial.println(thm->getTemp(count, j));
            }

            Serial.println();
        }
        else
        {
            // CANバスにメッセージを流す
        }

        countFlag = 1;
    }

    pastErrFlag[2] = pastErrFlag[1];
    pastErrFlag[1] = pastErrFlag[0];

    for (int i = 0; i < ecuNum; i++)
    {
        if (thm->getMaxTemp(i) > maxAllowableTemp)
        {
            pastErrFlag[0] = 1;
            break;
        }
        else if (thm->getMinTemp(i) < minAllowableTemp)
        {
            pastErrFlag[0] = 1;
            break;
        }
        else
        {
            pastErrFlag[0] = 0;
        }
    }

    //if (!dangerFlag)
    //{
        dangerFlag = pastErrFlag[2] & pastErrFlag[1] & pastErrFlag[0];
    //}

    digitalWrite(DANGER_OUTPUT, !dangerFlag);

    nowTime = millis();

    if ((nowTime - lastCalTime) > CAL_INTERVAL)
    {
        runCalibration();
        lastCalTime = nowTime;
    }

    if (nowTime < lastCalTime)
    {
        lastCalTime = nowTime;
    }
}

void handler(void)
{
    if (countFlag)
    {
        if (count < ecuNum)
        {
            count++;
        }
        else
        {
            count = 0;
        }
        countFlag = 0;
    }
    // Serial.println(count);
}

void _init_(unsigned long time)
{
    count = 0;
    for (int i = 0; i < 8; i++)
    {
        data[i] = 0;
    }
    thm = new Thermistor();
    countFlag = 0;
    for (int i = 0; i < 3; i++)
    {
        pastErrFlag[i] = 0;
    }
    dangerFlag = 0;
}

void runCalibration(void)
{
}