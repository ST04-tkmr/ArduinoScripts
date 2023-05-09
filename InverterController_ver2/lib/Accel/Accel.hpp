#ifndef _ACCEL_H_
#define _ACCEL_H_

class APPS
{
private:
    unsigned short val[2]; // センサから読み取った値(0~1023)
    unsigned short avr; // 2つのセンサ値の平均
    unsigned short deviation[2]; // 偏差[%]

    unsigned short calcDev(unsigned char index);

public:
    APPS();

    unsigned short getValue(unsigned char index);

    unsigned char setValue(unsigned short* value);

    unsigned short getDeviation(unsigned char index);
};

#endif