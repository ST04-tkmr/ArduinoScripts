#include "Thermistor.hpp"

THM_DATA::THM_DATA()
    : data{0, 0, 0, 0, 0, 0, 0, 0}
{
}

Thermistor::Thermistor()
{
    thm = new THM_DATA();
    for (char i = 0; i < thmNum; i++)
    {
        val[i] = 0;
        r[i] = 0;
        temp[i] = 0;
    }
}

unsigned char Thermistor::setData(unsigned char *data, unsigned char length)
{
    if (length <= 8)
    {
        for (char i = 0; i < length; i++)
        {
            thm->data[i] = *(data + i);
        }

        setVal(thm->thm1, 0);
        setVal(thm->thm2, 1);
        setVal(thm->thm3, 2);
        setVal(thm->thm4, 3);
        setVal(thm->thm5, 4);
        setVal(thm->thm6, 5);

        for (char i = 0; i < thmNum; i++)
        {
            this->r[i] = calcR(this->val[i]);
            this->temp[i] = calcTemp(this->r[i]);
        }

        return 0;
    }
    else
    {
        return 1;
    }
}

unsigned char Thermistor::setVal(unsigned short val, unsigned char index) volatile
{
    if (index < thmNum)
    {
        if (val <= 1023)
        {
            this->val[index] = val;
            return 0;
        }
        else
        {
            return 1;
        }
    }

    return 1;
}

float Thermistor::calcR(unsigned short val) volatile
{
    float vd, thmR;              // 実際の電圧値, サーミスタの抵抗値
    vd = val * 0.0049f;          // 実際の電圧値に変換
    thmR = (rs * vd) / (5 - vd); // サーミスタの抵抗値算出
    return thmR;
}

float Thermistor::calcTemp(float thmR) volatile
{
    float logOfthmR = static_cast<float>(log(thmR / r0));              //(サーミスタの抵抗値/r0)の常用対数をとる
    float temp = 1 / ((logOfthmR / b) + (1 / (t0 + 273.0f))) - 273.0f; // 抵抗値から温度を算出
    return temp;
}