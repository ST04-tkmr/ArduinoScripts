#include "Accel.hpp"
#include "Accel_dfs.hpp"
#include <stdlib.h>

unsigned short Accel::calcDev(unsigned char index)
{
    if (index == 0 || index == 1)
    {
        if (avr < 1023 / 2)
        {
            // return abs((val[index] - avr) / static_cast<float>((1023 - avr))) * 100;
            return abs(val[index] - avr);
        }

        // return abs((val[index] - avr) / static_cast<float>(avr)) * 100;
        return abs(val[index] - avr);
    }

    return 100;
}

float Accel::calcTorque()
{
    if (torqueOutputFlag)
    {
        unsigned short value = val[0] < val[1] ? val[0] : val[1];
        float v = value * 0.0049f;

        if (v < MINIMUM_SENSOR_VOLTAGE)
        {
            return 0;
        }
        else if (v > MAXIMUM_SENSOR_VOLTAGE)
        {
            return 0;
        }

        return MAXIMUM_TORQUE * ((v - MINIMUM_SENSOR_VOLTAGE) / (MAXIMUM_SENSOR_VOLTAGE - MINIMUM_SENSOR_VOLTAGE));
    }

    return 0;
}

void Accel::updateTorqueOutputFlag(void)
{
    lastDevError = devError;
    devError = (chatt[2] & chatt[1] & chatt[0]);
    chatt[2] = chatt[1];
    chatt[1] = chatt[0];
    chatt[0] = deviation[0] > THRESHOLD_DEVIATION && deviation[1] > THRESHOLD_DEVIATION;
    if (lastDevError == 0 && devError == 1)
    {
        devErrorFlag = 1;
    }

    if (torqueOutputFlag)
    {
        if (devErrorFlag)
        {
            torqueOutputFlag = 0;
            return;
        }
        return;
    }

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

    /*
    if (torqueOutputFlag)
    {
        if (deviation[0] > THRESHOLD_DEVIATION && deviation[1] > THRESHOLD_DEVIATION)
        {
            devErrorFlag = 1;
            torqueOutputFlag = 0;
            return;
        }
        return;
    }

    if (devErrorFlag)
    {
        if (val[0] * 0.0049f < 0.7f && val[1] * 0.0049f < 0.7f)
        {
            if (deviation[0] <= THRESHOLD_DEVIATION && deviation[1] <= THRESHOLD_DEVIATION)
            {
                devErrorFlag = 0;
                return;
            }
            return;
        }
        return;
    }
    */

    torqueOutputFlag = 1;
    return;
}

Accel::Accel()
    : val{0, 0}, avr(0), devErrorFlag(1), deviation{0, 0}, lastDevError(0), devError(0), chatt{1, 1, 1}, torqueOutputFlag(0), torque(0.0f)
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
        if (0 <= *(value + i) && *(value + i) <= 1023)
        {
            val[i] = *(value + i);
        }
        else
        {
            val[i] = 0;
        }
    }

    avr = (val[0] + val[1]) / 2;
    deviation[0] = calcDev(0);
    deviation[1] = calcDev(1);

    updateTorqueOutputFlag();

    torque = calcTorque();

    return 0;
}

unsigned char Accel::setValue(unsigned short val1, unsigned short val2)
{
    unsigned short v[2];
    v[0] = val1;
    v[1] = ((-1) * val2) + 1023;

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

    avr = (val[0] + val[1]) / 2;
    deviation[0] = calcDev(0);
    deviation[1] = calcDev(1);

    updateTorqueOutputFlag();

    torque = calcTorque();

    return 0;
}

unsigned short Accel::getDeviation(unsigned char index)
{
    if (index == 0 || index == 1)
    {
        return deviation[index];
    }

    return 1;
}