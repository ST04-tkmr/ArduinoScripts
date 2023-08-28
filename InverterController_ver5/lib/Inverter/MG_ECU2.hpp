#ifndef _MG_ECU2_H_
#define _MG_ECU2_H_

#include "Parameter.hpp"

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

        // 受信した Message を配列 buf に渡して変数 msg に保存
        unsigned char setMsg(unsigned char *buf);
    };
}

#endif
