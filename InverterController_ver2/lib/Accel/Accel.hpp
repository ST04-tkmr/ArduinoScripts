#ifndef _ACCEL_H_
#define _ACCEL_H_

class Accel
{
private:
    unsigned short val[2];          // センサから読み取った値(0~1023)
    unsigned short avr;             // 2つのセンサ値の平均
    unsigned char devErrorFlag;     // 偏差異常フラグ
    unsigned short deviation[2];    // 偏差[%]
    unsigned char torqueOutputFlag; // トルク出力フラグ
    float torque;                   // アクセル開度から計算したトルク

    unsigned short calcDev(unsigned char index);

    float calcTorque();

    void updateTorqueOutputFlag(void);

public:
    Accel();

    unsigned short getValue(unsigned char index);

    /**
     * アクセルセンサから読み取った値をセットする
     * analogReadで読み取った値をそのままセット
     * 要素数2の配列に格納してそのポインタを渡す
    */
    unsigned char setValue(unsigned short *value);

    inline unsigned char getDevErrorFlag() { return devErrorFlag; }

    unsigned short getDeviation(unsigned char index);

    inline float getTorque() { return torque; }

    inline unsigned char getTorqueOutputFlag() { return torqueOutputFlag; }
};

#endif