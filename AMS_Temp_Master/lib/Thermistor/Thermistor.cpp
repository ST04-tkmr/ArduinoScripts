#include "Thermistor.hpp"

THM_DATA::THM_DATA()
    : data{0, 0, 0, 0, 0, 0, 0, 0}
{
}

Thermistor::Thermistor()
{
    for (int i = 0; i < ecuNum; i++)
    {
        thm[i] = new THM_DATA();
        for (int j = 0; j < thmNum; j++)
        {
            val[i][j] = 0;
            r[i][j] = 0;
            temp[i][j] = 0;
        }
    }
}

unsigned char Thermistor::setData(unsigned char ecuIndex, unsigned char *data, unsigned char length)
{
    if (length <= 8)
    {
        for (int i = 0; i < length; i++)
        {
            thm[ecuIndex]->data[i] = *(data + i);
        }

        setVal(thm[ecuIndex]->thm1, ecuIndex, 0);
        setVal(thm[ecuIndex]->thm2, ecuIndex, 1);
        setVal(thm[ecuIndex]->thm3, ecuIndex, 2);
        setVal(thm[ecuIndex]->thm4, ecuIndex, 3);
        setVal(thm[ecuIndex]->thm5, ecuIndex, 4);
        setVal(thm[ecuIndex]->thm6, ecuIndex, 5);

        for (int i = 0; i < thmNum; i++)
        {
            this->r[ecuIndex][i] = calcR(this->val[ecuIndex][i]);
            this->temp[ecuIndex][i] = calcTemp(this->r[ecuIndex][i]);
        }

        return 0;
    }
    else
    {
        return 1;
    }
}

unsigned char Thermistor::setVal(unsigned short val, unsigned char ecuIndex, unsigned char thmIndex) volatile
{
    if (ecuIndex < ecuNum)
    {
        if (thmIndex < thmNum)
        {
            if (val <= 1023)
            {
                this->val[ecuIndex][thmIndex] = val;
                return 0;
            }
            else
            {
                return 1;
            }
        }
    }

    return 1;
}

float Thermistor::calcR(unsigned short val) volatile
{
    float vd, thmR; // 実際の電圧値, サーミスタの抵抗値
    if (val > 1000)
    {
        vd = 0;
    }
    else
    {
        vd = val * 0.0049f; // 実際の電圧値に変換
    }
    thmR = (rs * vd) / (5 - vd); // サーミスタの抵抗値算出
    return thmR;
}

float Thermistor::calcTemp(float thmR) volatile
{
    float logOfthmR = static_cast<float>(log(thmR / r0));              //(サーミスタの抵抗値/r0)の常用対数をとる
    float temp = 1 / ((logOfthmR / b) + (1 / (t0 + 273.0f))) - 273.0f; // 抵抗値から温度を算出
    return temp;
}