/*
 *  電圧の測定はサーミスタの電圧降下を測定
 *
 *      .+5V
 *      |
 *      _
 *     | | R=10kΩ
 *     | |
 *      -
 *      |
 *      .-----.Vd(アナログピンで読み取り、10bitADCで変換されるので実際の電圧にするには4.9mVを掛け算する)
 *      |
 *      _
 *     | | サーミスタ r
 *     | |
 *      -
 *      |
 *      .GND
 *
 *    Vd = { 5/(10k+r) } x r よりrを算出
 *
 *  温度T0=25度のときのサーミスタの抵抗値R0=10kΩ
 *  B定数 = 3423(25℃~80℃)
 *  データシート(https://akizukidenshi.com/download/ds/murata/NXFT15-series.pdf)
 *
 *  抵抗から温度を算出する式
 *    T = 1/{ln(r/R0)/B + 1/(T0+273)} -273
 */

#ifndef THERMISTOR
#define THERMISTOR

#include <math.h>
#include "Thermistor_dfs.hpp"

//-------------------------------------------------------
//  型定義
//-------------------------------------------------------
// ADCの結果を保存するデータ構造
union THM_DATA
{
    unsigned char data[8];
    struct
    {
        unsigned short thm1 : 10;
        unsigned short thm2 : 10;
        unsigned short thm3 : 10;
        unsigned short thm4 : 10;
        unsigned short thm5 : 10;
        unsigned short thm6 : 10;
        unsigned char received : 4;
    };

    THM_DATA();
};

// サーミスタのパラメータ
// アナログピンで読み取った値と算出したサーミスタの情報を格納する
class Thermistor
{
private:
    volatile THM_DATA *thm[ecuNum];
    volatile unsigned short val[ecuNum][thmNum]; // アナログピンで読み取った値
    volatile float r[ecuNum][thmNum];            // 抵抗値
    volatile float temp[ecuNum][thmNum];         // 抵抗値から計算した温度

public:
    Thermistor();

    /**
     * @fn  setData
     *
     * @brief   i2cで取得したデータをセット. 温度計算まで行う.
     *
     * @param   ecuIndex   ECU番号（0~ecuNum）
     * @param   data    データ配列のポインタ
     * @param   length  データ配列の要素数（<= 8）
     *
     * @return
     */
    unsigned char setData(unsigned char ecuIndex, unsigned char *data, unsigned char length);

    /**
     * @fn  setVal
     *
     * @brief   analogReadした値をセットする.
     *
     * @param   val analogReadした値（0~1023）
     * @param   ecuIndex    ECU番号（0~ecuNum）
     * @param   thmIndex    サーミスタ番号（0~thmNum）
     *
     * @return  Success(0), Fail(1)
     */
    unsigned char setVal(unsigned short val, unsigned char ecuIndex, unsigned char thmIndex) volatile;

    inline int getVal(unsigned char ecuIndex, unsigned char thmIndex) { return val[ecuIndex][thmIndex]; }
    inline float getR(unsigned char ecuIndex, unsigned char thmIndex) { return r[ecuIndex][thmIndex]; }
    inline float getTemp(unsigned char ecuIndex, unsigned char thmIndex) { return temp[ecuIndex][thmIndex]; }

    /**
     * @fn  getMaxTemp
     * 
     * @brief   各セグメントの最大温度を取得
     * 
     * @param   ecuIndex    ECU番号（0~ecuNum）
     * 
     * @return  セグメントの最大温度
    */
    inline float getMaxTemp(unsigned char ecuIndex)
    {
        float maxTemp = -273.0f;
        for (int i = 0; i < thmNum; i++)
        {
            maxTemp = maxTemp > temp[ecuIndex][i] ? maxTemp : temp[ecuIndex][i];
        }
        return maxTemp;
    }

    /**
     * @fn  getMinTemp
     * 
     * @brief   各セグメントの最低温度を取得
     * 
     * @param   ecuIndex    ECU番号（0~ecuNum）
     * 
     * @return  セグメントの最低温度
    */
    inline float getMinTemp(unsigned char ecuIndex)
    {
        float minTemp = 125.0f;
        for (int i = 0; i < thmNum; i++)
        {
            minTemp = minTemp < temp[ecuIndex][i] ? minTemp : temp[ecuIndex][i];
        }
        return minTemp;
    }

    //-------------------------------------------------------
    //  読み込んだ電圧からサーミスタの抵抗を計算
    //  引数：アナログピンで読み取った値(analogReadの戻り値0~1023)
    //  戻り値：サーミスタの抵抗値
    //-------------------------------------------------------
    float calcR(unsigned short val) volatile;

    //-------------------------------------------------------
    //  抵抗から温度計算
    //  引数：サーミスタの抵抗値
    //  戻り値：サーミスタの温度
    //-------------------------------------------------------
    float calcTemp(float r) volatile;
};

#endif