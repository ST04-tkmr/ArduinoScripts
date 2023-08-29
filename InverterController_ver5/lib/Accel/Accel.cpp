#include "Accel.hpp"
#include <stdlib.h>

float Accel::calcDev(void)
{
    float def = abs((val[0] - val[1]) * 0.0049f);  // 2つのセンサ値の差
    return 100 * (def / AMOUNT_OF_MOVEMENT);    // 差をセンサ値の移動量で割って100をかける
}

float Accel::calcTorque()
{
    if (torqueOutputFlag)
    {
        unsigned short value = val[0] < val[1] ? val[0] : val[1];   // センサ値の小さい方を採用
        float v = value * 0.0049f;                                  // センサ値→電圧値変換

        // センサ信号が想定範囲の電圧に収まっているか
        if (v < MINIMUM_SENSOR_VOLTAGE)
        {
            return 0;
        }
        else if (v > MAXIMUM_SENSOR_VOLTAGE)
        {
            //v = MAXIMUM_SENSOR_VOLTAGE;
            return 0;
        }

        return MAXIMUM_TORQUE * (v - MINIMUM_SENSOR_VOLTAGE);
    }

    return 0;
}

void Accel::updateTorqueOutputFlag(void)
{
    lastDevError = devError;
    // 過去3回連続で偏差が10%を超えたらエラーとして認識
    devError = (chatt[2] & chatt[1] & chatt[0]);
    chatt[2] = chatt[1];
    chatt[1] = chatt[0];
    //chatt[0] = deviation[0] > THRESHOLD_DEVIATION && deviation[1] > THRESHOLD_DEVIATION;
    chatt[0] = dev >= THRESHOLD_DEVIATION;
    // エラーの立ち上がりでエラーフラグが立つ
    if (lastDevError == 0 && devError == 1)
    {
        devErrorFlag = 1;
    }

    // エラーフラグが立ったら出力を切る
    if (torqueOutputFlag)
    {
        if (devErrorFlag)
        {
            torqueOutputFlag = 0;
            return;
        }
        return;
    }

    // アクセルを離してポジションが0%に戻ったらエラー解除
    if (devErrorFlag)
    {
        if (val[0] * 0.0049f < 0.7f && val[1] * 0.0049f < 0.7f)
        {
            if (lastDevError == 0 && devError == 0)
            {
                devErrorFlag = 0;
                return;
            }
            return;
        }
        return;
    }

    torqueOutputFlag = 1;
    return;
}

Accel::Accel()
    : val{0, 0}, avr(0), devErrorFlag(1), dev(0), lastDevError(0), devError(0), chatt{1, 1, 1}, torqueOutputFlag(0), torque(0.0f)
{
}

unsigned short Accel::getValue(unsigned char index)
{
    if (index == 0 || index == 1)
    {
        return val[index];
    }

    return 0;
}

unsigned char Accel::setValue(unsigned short *value)
{
    for (int i = 0; i < 2; i++)
    {
        if (*(value + i) <= 1023)
        {
            val[i] = *(value + i);
        }
        else
        {
            val[i] = 0;
        }
    }

    avr = (val[0] + val[1]) / 2.0f;
    dev = calcDev();

    updateTorqueOutputFlag();

    torque = calcTorque();

    return 0;
}

unsigned char Accel::setValue(int val1, int val2)
{
    int v[2];

    v[0] = val1;
    v[1] = ((-1) * val2) + 1023;                // 傾きを負から正に変換

    for (int i = 0; i < 2; i++)
    {
        if (v[i] <= 1023)
        {
            val[i] = v[i];
        }
        else
        {
            val[i] = 0;
        }
    }

    avr = (val[-1] + val[1]) / 2.0f;    // 平均値
    dev = calcDev();                    // 偏差

    updateTorqueOutputFlag();

    torque = calcTorque();

    return 0;
}