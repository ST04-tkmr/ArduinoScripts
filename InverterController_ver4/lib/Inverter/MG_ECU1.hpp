#ifndef _MG_ECU1_H_
#define _MG_ECU1_H_

#include "Parameter.hpp"

namespace MG_ECU1
{
    union MSG // CAN Massage
    {
        unsigned char msgs[8];
        struct
        {
            unsigned char shutdownEnable : 1; // MG_ECUシャットダウン許可
            unsigned char PWM : 2;            // ゲート駆動状態
            unsigned char workingStatus : 3;  // 制御状態
            unsigned char reserve0 : 2;
            unsigned short motorSpeed : 16;        // モータ回転数 Range 0~28000
            unsigned short motorPhaseCurrent : 10; // モータ相電流 Range 0~1000
            unsigned short inputDCVoltage : 10;    // 入力直流電圧 Range 0~500
            unsigned long reserve1 : 19;
            unsigned char failureStatus : 3; // 異常状態
        };

        MSG();
    };

    class ECU
    {
    private:
        const unsigned long id;
        union MSG *msg;
        Parameter *motorSpeedPara;        // モータ回転数
        Parameter *motorPhaseCurrentPara; // モータ相電流

    public:
        ECU(unsigned long id);
        ~ECU();

        unsigned long getID() { return id; };

        // Massage取得, 指定したインデックスが0~7以外の時は0を返す
        unsigned char getMsgByte(unsigned char index);

        inline unsigned char getShutdownEnable() { return msg->shutdownEnable; };
        inline unsigned char getPWM() { return msg->PWM; };
        inline unsigned char getWorkingStatus() { return msg->workingStatus; };

        // 戻り値はPhysical Value
        float getMotorSpeed();

        // 戻り値はNormal Value
        inline unsigned short getNormalMotorSpeed() { return msg->motorSpeed; };

        // 戻り値はPhysical Value
        float getMotorPhaseCurrent();

        // 戻り値はNormal Value
        inline unsigned short getNormalMotorPhaseCurrent() { return msg->motorPhaseCurrent; };

        // 戻り値はPhysical Value
        inline unsigned short getInputDCVoltage() { return msg->inputDCVoltage; };

        // 戻り値はPhysical Value
        inline unsigned char getFailureStatus() { return msg->failureStatus; };

        // 受信した Message を配列 buf に渡して変数 msg に保存
        unsigned char setMsg(unsigned char *buf);
    };
}

#endif