#ifndef _ACCEL_H_
#define _ACCEL_H_

#include "Accel_dfs.hpp"

class Accel
{
private:
    unsigned short val[2];          // センサから読み取った値(0~1023)
    unsigned short avr;             // 2つのセンサ値の平均
    unsigned char devErrorFlag;     // 偏差異常フラグ
    unsigned short deviation[2];    // 偏差[%]
    unsigned char lastDevError;     // ノイズ対策
    unsigned char devError;         // ノイズ対策
    unsigned char chatt[3];         // ノイズ対策
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

    /**
     * @fn      setValue
     *
     * @brief   センサから読み取った値（0~1023）をセットする.
     *          2つの特性において、傾きの絶対値が同じで符号が異なる.
     *
     * @param   val1    正の傾きの特性を持つセンサの値
     * @param   val2    負の傾きの特性を持つセンサの値
     *
     * @return
    */
    unsigned char setValue(unsigned short val1, unsigned short val2);

    inline unsigned char getDevErrorFlag() { return devErrorFlag; }

    unsigned short getDeviation(unsigned char index);

    inline float getTorque() { return torque; }

    inline unsigned char getTorqueOutputFlag() { return torqueOutputFlag; }
};

#endif