#include "Accel.hpp"
#include <stdlib.h>

unsigned short APPS::calcDev(unsigned char index)
{
    if (index == 0 || index == 1)
    {
        if (avr < 1023 / 2)
        {
            return abs((val[index] - avr) / (1023 - avr)) * 100;
        }

        return abs((val[index] - avr) / avr) * 100;
    }

    return 1;
};

APPS::APPS()
{
    val[0] = 0;
    val[1] = 0;
    avr = 0;
    deviation[0] = 0;
    deviation[1] = 0;
};

unsigned short APPS::getDeviation(unsigned char index)
{
    if (index == 0 || index == 1)
    {
        return deviation[index];
    }

    return 1;
}
unsigned short APPS::getValue(unsigned char index)
{
    if (index == 0 || index == 1)
    {
        return val[index];
    }

    return 0;
};

unsigned char APPS::setValue(unsigned short *value)
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

    return 0;
};