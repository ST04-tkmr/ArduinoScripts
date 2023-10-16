#ifndef _INVERTER_H_
#define _INVERTER_H_

#include "EV_ECU1.hpp"
#include "MG_ECU1.hpp"
#include "MG_ECU2.hpp"
#include "Inverter_dfs.hpp"

#ifdef ARDUINO_UNO_R4
#include <Arduino_CAN.h>
#else
#include <SPI.h>
#include "mcp2515_can.h"
#endif

class Inverter
{
private:
    EV_ECU1::ECU *evecu1;
    MG_ECU1::ECU *mgecu1;
    MG_ECU2::ECU *mgecu2;
    unsigned short batteryVoltage;

public:
    Inverter();
    ~Inverter();

    // CAN通信初期化処理
    void init(void);

    /**
     * flags[0] = airFlag
     * flags[1] = torqueControlFlag
     * flags[2] = shutdownFlag
     * flags[3] = driveFlag
    */
    void runInverter(unsigned char* flags, unsigned short batVol, float torque);

    /**
     * バッテリー電圧をセット
     * 280 <= batVol <= 400
    */
    unsigned char setBatVol(unsigned short batVol);

    /**
     * MG-ECU実行要求をONにセット
     * batVol : バッテリー電圧
     * 終了シーケンスにしたがってONにしないとCritical Errorが発生する
     */
    unsigned char setMgecuRequestON(unsigned short batVol);

    /**
     * MG-ECU実行要求をOFFにセット
     * 終了シーケンスにしたがってOFFにしないとCritical Errorが発生する
    */
    unsigned char setMgecuRequestOFF();

    /**
     * 平滑コンデンサ放電要求をONにセット
     * 放電要求をONにする前にAIRをOFFする
     */
    unsigned char setRapidDischargeRequestON();

    /**
     * 平滑コンデンサ放電要求をOFFにセット
    */
    unsigned char setRapidDischargeRequestOFF();

    /**
     * トルク指令
     * -1000 <= torque <= 1000 (resolution 0.5 Nm)
     * トルク制限(モータによる)
     * -60 <= torque <= 60
    */
    unsigned char torqueRequest(float torque);

    /**
     * EV-ECUへMassage送信
     * printFlag = 1 のとき, Buf をシリアルモニタに表示
     * 戻り値 : 0(成功) or 1(失敗)
     */
    int sendMsgToInverter(unsigned char printFlag);

    /**
     * 受信したMassageを読み取る
     * printFlag = 1 のときID, Buf をシリアルモニタに表示
     * 戻り値は受信したMassageのID(失敗時は0を返す)
     */
    unsigned long readMsgFromInverter(unsigned char printFlag);

    /**
     * シリアルモニタにbufのビットを全て表示する
     */
    void checkBuf(unsigned char *buf);

    /**
     * 指定したIDのECUのMassageのビットをシリアルモニタに表示
     */
    void checkMsgBit(unsigned long id);

    /**
     * 指定したIDのECUの各パラメータをシリアルモニタに表示
     */
    void checkMsg(unsigned long id);
};

/*
inline uint32_t CanStandardId(uint32_t const id)
{
    static uint32_t constexpr CAN_SFF_MASK = 0x000007FFU;
    return (id & CAN_SFF_MASK);
}
*/

#endif