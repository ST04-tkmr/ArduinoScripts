#ifndef _ACCEL_H_
#define _ACCEL_H_

#include "Accel_dfs.hpp"

class Accel
{
private:
    int val[2];                     // センサから読み取った値(0~1023)
    unsigned short avr;             // 2つのセンサ値の平均
    unsigned char devErrorFlag;     // 偏差異常フラグ
    float dev;                        // センサ値の差異[%]
    unsigned char lastDevError;     // ノイズ対策
    unsigned char devError;         // ノイズ対策
    unsigned char chatt[3];         // ノイズ対策
    unsigned char torqueOutputFlag; // トルク出力フラグ
    float torque;                   // アクセル開度から計算したトルク

    float calcDev(void);

    /**
     * @fn      calcTorque
     *
     * @brief   センサ値にもとづいて指令トルクを決定する
     *
     * @return  指令トルク（0~60）
    */
    float calcTorque();

    /**
     * @fn  updateTorqueOutputFlag
     *
     * @brief   2つのセンサ値の偏差から異常診断
     *          偏差10%を超えると出力を止める必要がある
     *
     * @return  none
    */
    void updateTorqueOutputFlag(void);

public:
    Accel();

    unsigned short getValue(unsigned char index);

    /**
     * アクセルセンサから読み取った値をセットする
     * analogReadで読み取った値をそのままセット
     * 要素数2の配列に格納してそのポインタを渡す
     * 2つのセンサの特性が同じ
    */
    unsigned char setValue(unsigned short *value);

    /**
     * @fn      setValue
     *
     * @brief   アクセルセンサの値をセット（0~1023）
     *          2つのセンサの特性が真逆（絶対値が同じで傾きが正負反対）
     *
     * @param   val1 - 特性が正の傾き
     * @param   val2 - 特性が負の傾き
     *
     * @return
    */
    unsigned char setValue(int val1, int val2);

    inline unsigned char getDevErrorFlag() { return devErrorFlag; }

    inline float getTorque() { return torque; }

    inline unsigned char getTorqueOutputFlag() { return torqueOutputFlag; }

    inline float getDev() { return dev; }
};

#endif