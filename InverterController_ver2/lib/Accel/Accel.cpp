#include "Accel.hpp"
#include <stdlib.h>

unsigned short Accel::calcDev(unsigned char index)
{
    if (index == 0 || index == 1)
    {
        if (avr < 1023 / 2)
        {
            return abs((val[index] - avr) / static_cast<float>((1023 - avr))) * 100;
        }

        return abs((val[index] - avr) / static_cast<float>(avr)) * 100;
    }

    return 100;
}

void Accel::updateTorqueOutputFlag(void)
{
    if (torqueOutputFlag)
    {
        if (deviation[0] < 10 && deviation[1] < 10)
        {
            return;
        }

        torqueOutputFlag = 0;
        return;
    }

    if (deviation[0] < 10 && deviation[1] < 10)
    {
        if (
            val[0] * 0.0049f < 0.7f &&
            val[1] * 0.0049f < 0.7f
        )
        {
            torqueOutputFlag = 1;
            return;
        }
    }
}

Accel::Accel()
    : val({0, 0}), avr(0), deviation({0, 0}), torqueOutputFlag(0)
{
}

unsigned short Accel::getDeviation(unsigned char index)
{
    if (index == 0 || index == 1)
    {
        return deviation[index];
    }

    return 1;
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
        if (0 <= *(value + i) && *(value + i) <= 1023)
        {
            val[i] = *(value + i);
        }
    }

    avr = (val[0] + val[1]) / 2;
    deviation[0] = calcDev(0);
    deviation[1] = calcDev(1);

    updateTorqueOutputFlag();

    return 0;
}