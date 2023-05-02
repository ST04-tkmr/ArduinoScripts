#ifndef _INVERTER_H_
#define _INVERTER_H_

/**
 * Parameter for changing massage to physical
 * offset, resolution, physical(minPhysical~maxPhysical)
 */
class Parameter
{
private:
    const short offset;
    const float resolution;
    const short minPhysical, maxPhysical;

public:
    Parameter(short offset, float resolution, short minPhysical, short maxPhysical);

    inline short getOffset() { return offset; };
    inline float getResolution() { return resolution; };
    inline short getMinPhysical() { return minPhysical; };
    inline short getMaxPhysical() { return maxPhysical; };

    //Normal ValueからPhysical Valueを計算
    float calcPhysical(unsigned short normal);

    //Physical ValueからNormal Valueを計算
    unsigned short calcNormal(float physicalValue);
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
        const unsigned long id;
        union MSG *msg;
        Parameter *torqueRequestPara; // HV-ECU要求トルク

    public:
        ECU(unsigned long id);
        ~ECU();

        unsigned long getID() { return id; };

        // Massage取得, 指定したインデックスが0~7以外の時は0を返す
        unsigned char getMsgByte(unsigned char index);

        inline unsigned char getEcuEnable() { return msg->ecuEnable; };
        inline unsigned char getDischargeCommand() { return msg->dischargeCommand; };

        // 戻り値はPhysical Value
        float getRequestTorque();

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

        // 各パラメータの状態をシリアルモニタでチェック
        void checkEVECU1(void);

        void checkTorqueRequestPara(void);
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

        inline unsigned short getInputDCVoltage() { return msg->inputDCVoltage; };
        inline unsigned char getFailureStatus() { return msg->failureStatus; };

        unsigned char setMsg(unsigned char *buf);

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
            unsigned char inverterTemp : 8;                 // インバータ温度
            unsigned short maxAvailableMotorTorque : 12;    // モータ上限制限トルク
            unsigned short maxAvailableGenerateTorque : 12; // モータ下限制限トルク
            unsigned char motorTemp : 8;                    // モータ温度
        };

        MSG();
    };

    class ECU
    {
    private:
        const unsigned long id;
        union MSG *msg;
        Parameter *inverterTemperaturePara;              // インバータ温度
        Parameter *maximumAvailableMotoringTorquePara;   // モータ上限制限トルク
        Parameter *maximumAvailableGeneratingTorquePara; // モータ下限制限トルク
        Parameter *motorTemperaturePara;                 // モータ温度

    public:
        ECU(unsigned long id);
        ~ECU();

        unsigned long getID() { return id; };

        // Massage取得, 指定したインデックスが0~7以外の時は0を返す
        unsigned char getMsgByte(unsigned char index);

        // 戻り値はPhysical Value
        float getInverterTemp();

        // 戻り値はNormal Value
        inline unsigned char getNormalInverterTemp() { return msg->inverterTemp; };

        // 戻り値はPhysical Value
        float getMaxAvailableMotorTorque();

        // 戻り値はNormal Value
        inline unsigned short getNormalMaxAvailableMotorTorque() { return msg->maxAvailableMotorTorque; };

        // 戻り値はPhysical Value
        float getMaxAvailableGenerateTorque();

        // 戻り値はNormal Value
        inline unsigned short getNormalMaxAvailableGenerateTorque() { return msg->maxAvailableGenerateTorque; };

        // 戻り値はPhysical Value
        float getMotorTemp();

        // 戻り値はNormal Value
        inline unsigned char getNormalMotorTemp() { return msg->motorTemp; };

        unsigned char setMsg(unsigned char *buf);

        // 各パラメータの状態をシリアルモニタでチェック
        void checkMGECU2();
    };
}

// CAN通信初期化処理
void init_CAN(void);

/**
 * EV-ECUへMassage送信
 * 戻り値 : 1(成功) or 0(失敗)
 */
unsigned char sendMsgToInverter(EV_ECU1::ECU *ecu);

/**
 * 受信したMassageを読み取る
 * printFlag = 1 のときID, Buf をシリアルモニタに表示
 */
unsigned long readMsgFromInverter(MG_ECU1::ECU *ecu1, MG_ECU2::ECU *ecu2, unsigned char printFlag);

/**
 * シリアルモニタにbufのビットを全て表示する
 */
void checkBuf(unsigned char *buf);

#endif