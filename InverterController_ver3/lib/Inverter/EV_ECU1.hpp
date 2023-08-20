#ifndef _EV_ECU1_H_
#define _EV_ECU1_H_

#include "Parameter.hpp"

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
        Parameter *torqueRequestPara;    // HV-ECU要求トルク

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

        /**
         * ecuEnable = 0 or 1
         * 不正な値が引数に渡されたときは ecuEnable = 0 にセット
         */
        unsigned char setEcuEnable(unsigned char ecuEnable);

        /**
         * dischargeCommand = 0 or 1
         * 不正な値が引数に渡されたときは dischargeCommand = 0 にセット
         */
        unsigned char setDischargeCommand(unsigned char dischargeCommand);

        /**
         * 要求トルクをセット
         * -1000 <= physicalValue <= 1000
         * 範囲外の値が引数に渡されたときは要求トルクを 0 Nm にする
         */
        unsigned char setRequestTorque(float physicalValue);
    };
}

#endif