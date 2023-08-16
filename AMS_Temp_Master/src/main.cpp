#include <Arduino.h>
#include <MsTimer2.h>
#include <Wire.h>
#include "Thermistor.hpp"
#include "CAN_Temp.hpp"

uint32_t count;
uint8_t data[8];
Thermistor *thm;

void interrupt(void);

void setup()
{
    count = 0;
    for (int i = 0; i < 8; i++)
    {
        data[i] = 0;
    }
    thm = new Thermistor();
    Wire.begin();

    Serial.begin(115200);

    MsTimer2::set(100, interrupt);
    // MsTimer2::start();
}

void loop()
{
    if (count >= 4)
    {
        // CANバスにメッセージを流す

        count = 0;
    }
    else
    {
        // データリクエスト
        if (count < ecuNum)
        {
            Serial.print("request from ");
            Serial.println(adrs[count]);
            Wire.requestFrom(adrs[count], sizeof(data));

            uint8_t i = 0;
            while (Wire.available())
            {
                data[i] = Wire.read();
                i++;
            }

            thm->setData(data, sizeof(data));

            for (char j = 0; j < thmNum; j++)
            {
                Serial.println(thm->getVal(j));
            }
            Serial.println();
        }

        count++;
    }

    delay(100);
}

void interrupt(void)
{
}