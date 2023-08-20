#include <Arduino.h>
#include <MsTimer2.h>
#include <Wire.h>
#include "Thermistor.hpp"
#include "CAN_Temp.hpp"

volatile uint8_t count;
uint8_t data[8];
Thermistor *thm;
volatile uint8_t countFlag;

void handler(void);

void setup()
{
    count = 0;
    for (int i = 0; i < 8; i++)
    {
        data[i] = 0;
    }
    thm = new Thermistor();
    countFlag = 0;

    Wire.begin();

    Serial.begin(115200);

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

            thm->setData(data, sizeof(data));

            Serial.print("raw data : ");
            Serial.println(count);
            for (char j = 0; j < thmNum; j++)
            {
                Serial.println(thm->getVal(j));
            }

            Serial.println("temperature");
            for (char j = 0; j < thmNum; j++)
            {
                Serial.println(thm->getTemp(j));
            }

            Serial.println();
        }
        else
        {
            // CANバスにメッセージを流す

        }

        countFlag = 1;
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
    Serial.println(count);
}