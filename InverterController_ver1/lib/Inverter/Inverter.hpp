#ifndef _INVERTER_H_
#define _INVERTER_H_

/**
 * Parameter for changing massage to physical
 * offset, resolution, physical(minPhysical~maxPhysical)
 */
class Parameter
{
private:
    unsigned short offset;
    float resolution;
    unsigned short minPhysical, maxPhysical;

public:
    Parameter(unsigned short offset, float resolution, unsigned short minPhysical, unsigned short maxPhysical);

    inline unsigned short getOffset() { return offset; };
    inline float getResolution() { return resolution; };
    inline unsigned short getMinPhysical() { return minPhysical; };
    inline unsigned short getMaxPhysical() { return maxPhysical; };
    inline float calcPhysical(unsigned short normal) { return (normal * resolution) + offset; };
    inline unsigned short calcNormal(float physicalValue) { return static_cast<unsigned short>((physicalValue - offset) / resolution); };
};

namespace EV_ECU1
{
    union MSG // CAN Massage
    {
        unsigned char msgs[8];
        struct
        {
            unsigned char ecuEnable : 1;        // MG-ECU実行要求
            unsigned char dischargeCommand : 1; // 平滑コンデンサ放電要求
            unsigned char reserve0 : 6;
            unsigned short requestTorque : 12; // HV-ECU要求トルク Range 0~4000
        };

        MSG();
    };

    class ECU
    {
    private:
        const unsigned short id;
        union MSG *msg;
        Parameter *torqueRequestPara; // HV-ECU要求トルク

    public:
        ECU(unsigned short id);
        ~ECU();

        // Massage取得, 指定したインデックスが0~7以外の時は0を返す
        unsigned char getMsgByte(unsigned char index);

        inline unsigned char getEcuEnable() { return msg->ecuEnable; };
        inline unsigned char getDischargeCommand() { return msg->dischargeCommand; };

        // 戻り値はPhysical Value
        inline float getRequestTorque() { return torqueRequestPara->calcPhysical(msg->requestTorque); };

        // 戻り値はNormal Value
        inline unsigned short getNormalRequestTorque() { return msg->requestTorque; };

        // ecuEnable = 0 or 1
        unsigned char setEcuEnable(unsigned char ecuEnable);

        // dischargeCommand = 0 or 1
        unsigned char setDischargeCommand(unsigned char dischargeCommand);

        /**
         * 要求トルクをセット
         * -1000 <= physicalValue <= 1000
         */
        unsigned char setRequestTorque(float physicalValue);
    };
}

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
            unsigned short motorSpeed;             // モータ回転数 Range 0~28000
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
        const unsigned short id;
        union MSG *msg;
        Parameter *motorSpeedPara;        // モータ回転数
        Parameter *motorPhaseCurrentPara; // モータ相電流

    public:
        ECU(unsigned short id);
        ~ECU();

        // Massage取得, 指定したインデックスが0~7以外の時は0を返す
        unsigned char getMsgByte(unsigned char index);

        inline unsigned char getShutdownEnable() { return msg->shutdownEnable; };
        inline unsigned char getPWM() { return msg->PWM; };
        inline unsigned char getWorkingStatus() { return msg->workingStatus; };

        // 戻り値はPhysical Value
        inline float getMotorSpeed() { return motorSpeedPara->calcPhysical(msg->motorSpeed); };

        // 戻り値はNormal Value
        inline unsigned short getNormalMotorSpeed() { return msg->motorSpeed; };

        // 戻り値はPhysical Value
        inline float getMotorPhaseCurrent() { return motorPhaseCurrentPara->calcPhysical(msg->motorPhaseCurrent); };

        // 戻り値はNormal Value
        inline unsigned short getNormalMotorPhaseCurrent() { return msg->motorPhaseCurrent; };

        inline unsigned short getInputDCVoltage() { return msg->inputDCVoltage; };
        inline unsigned char getFailureStatus() { return msg->failureStatus; };

        // 各パラメータの状態をシリアルモニタでチェック
        void checkMGECU1();
    };
}

namespace MG_ECU2
{
    union MSG // Can Massage
    {
        unsigned char msgs[8];
        struct
        {
            unsigned char inverterTemp;                     // インバータ温度
            unsigned short maxAvailableMotorTorque : 12;    // モータ上限制限トルク
            unsigned short maxAvailableGenerateTorque : 12; // モータ下限制限トルク
            unsigned char motorTemp;                        // モータ温度
        };

        MSG();
    };

    class ECU
    {
    private:
        const unsigned short id;
        union MSG *msg;
        Parameter *inverterTemperaturePara;              // インバータ温度
        Parameter *maximumAvailableMotoringTorquePara;   // モータ上限制限トルク
        Parameter *maximumAvailableGeneratingTorquePara; // モータ下限制限トルク
        Parameter *motorTemperaturePara;                 // モータ温度

    public:
        ECU(unsigned short id);
        ~ECU();

        // Massage取得, 指定したインデックスが0~7以外の時は0を返す
        unsigned char getMsgByte(unsigned char index);

        // 戻り値はPhysical Value
        inline float getInverterTemp() { return inverterTemperaturePara->calcPhysical(msg->inverterTemp); };

        // 戻り値はNormal Value
        inline unsigned char getNormalInverterTemp() { return msg->inverterTemp; };

        // 戻り値はPhysical Value
        inline float getMaxAvailableMotorTorque() { return maximumAvailableMotoringTorquePara->calcPhysical(msg->maxAvailableMotorTorque); };

        // 戻り値はNormal Value
        inline unsigned short getNormalMaxAvailableMotorTorque() { return msg->maxAvailableMotorTorque; };

        // 戻り値はPhysical Value
        inline float getMaxAvailableGenerateTorque() { return maximumAvailableGeneratingTorquePara->calcPhysical(msg->maxAvailableGenerateTorque); };

        // 戻り値はNormal Value
        inline unsigned short getNormalMaxAvailableGenerateTorque() { return msg->maxAvailableGenerateTorque; };

        // 戻り値はPhysical Value
        inline float getMotorTemp() { return motorTemperaturePara->calcPhysical(msg->motorTemp); };

        // 戻り値はNormal Value
        inline unsigned char getNormalMotorTemp() { return msg->motorTemp; };

        // 各パラメータの状態をシリアルモニタでチェック
        void checkMGECU2();
    };
}


void init_CAN(void);



#endif