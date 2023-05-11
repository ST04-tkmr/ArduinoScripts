#ifndef _ACCEL_H_
#define _ACCEL_H_

class Accel
{
private:
    unsigned short val[2];          // センサから読み取った値(0~1023)
    unsigned short avr;             // 2つのセンサ値の平均
    unsigned short deviation[2];    // 偏差[%]
    unsigned char torqueOutputFlag; // トルク出力フラグ

    unsigned short calcDev(unsigned char index);

    void updateTorqueOutputFlag(void);

public:
    Accel();

    unsigned short getValue(unsigned char index);

    unsigned char setValue(unsigned short *value);

    unsigned short getDeviation(unsigned char index);

    inline unsigned char getTorqueOutputFlag() { return torqueOutputFlag; }
};

#endif