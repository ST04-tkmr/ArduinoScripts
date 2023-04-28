#ifndef INVERTER
#define INVERTER

#include <SPI.h>
#define CAN_2515
#if defined(SEEED_WIO_TERMINAL) && defined(CAN_2518FD)
const int SPI_CS_PIN = BCM8;
const int CAN_INT_PIN = BCM25;
#else
const int SPI_CS_PIN = 9;
const int CAN_INT_PIN = 2;
#endif
#ifdef CAN_2518FD
#include "mcp2518fd_can.h"
mcp2518fd CAN(SPI_CS_PIN); // Set CS pin
#endif
#ifdef CAN_2515
#include "mcp2515_can.h"
mcp2515_can CAN(SPI_CS_PIN); // Set CS pin
#endif

namespace Inverter
{
    uint8_t stmp[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t buf[8] = {0, 0, 0, 0, 0, 0, 0, 0};

    namespace EV_ECU1
    {
        const uint16_t id = 0x301;

        union MSG
        {
        private:
            uint8_t msgs[8];
            struct
            {
                uint8_t ecuEnable : 1;        // MG-ECU実行要求
                uint8_t dischargeCommand : 1; // 平滑コンデンサ放電要求
                uint8_t reserve0 : 6;
                uint16_t requestTorque : 12; // HV-ECU要求トルク Range 0~4000
            };

        public:
            MSG()
            {
                ecuEnable = 0x0;
                dischargeCommand = 0x0;
                reserve0 = 0x0;
                requestTorque = 0x7D0;
            }

            //Massage取得, 指定したインデックスが0~7以外の時は0を返す
            inline uint8_t getMsgVal(uint8_t index)
            {
                if (0 <= index && index < 8)
                {
                    return msgs[index];
                }
                else
                {
                    return 0;
                }
            };

            inline uint8_t getEcuEnable() { return ecuEnable; };
            inline uint8_t getDischargeCommand() { return dischargeCommand; };
            inline uint8_t getRequestTorque() { return requestTorque; };
        };

        MSG msg = MSG();

        namespace TorqueRequest // HV-ECU要求トルク
        {
            const int16_t offset = -1000;
            const float resolution = 0.5f; // 0.5Nm
            float physical = 0;            // Physical Range -1000~1000
        }
    }

    namespace MG_ECU1
    {
        const uint16_t id = 0x311;

        union MSG
        {
        private:
            uint8_t msgs[8];
            struct
            {
                uint8_t shutdownEnable : 1; // MG_ECUシャットダウン許可
                uint8_t PWM : 2;            // ゲート駆動状態
                uint8_t workingStatus : 3;  // 制御状態
                uint8_t reserve0 : 2;
                uint16_t motorSpeed;             // モータ回転数 Range 0~28000
                uint16_t motorPhaseCurrent : 10; // モータ相電流 Range 0~1000
                uint16_t inputDCVoltage : 10;    // 入力直流電圧 Range 0~500
                uint32_t reserve1 : 19;
                uint8_t failureStatus : 3; // 異常状態
            };

        public:
            MSG()
            {
                shutdownEnable = 0x0;
                PWM = 0x1;
                workingStatus = 0x0;
                reserve0 = 0x0;
                motorSpeed = 0x36B0;
                motorPhaseCurrent = 0x0;
                inputDCVoltage = 0x3FF;
                reserve1 = 0x3FF;
                failureStatus = 0x0;
            }
        };

        MSG msg = MSG();

        namespace MotorSpeed // モータ回転数
        {
            const int16_t offset = -14000;
            const int16_t resolution = 1; // 1rpm
            int16_t physical = 0;         // Physical Range -14000~14000
        }

        namespace MotorPhaseCurrent // モータ相電流
        {
            const int16_t offset = 0;
            const float resolution = 0.5f; // 0.5Arms
            float physical = 0;            // Physical Range 0~500
        }
    }

    namespace MG_ECU2
    {
        const uint16_t id = 0x321;

        union MSG
        {
        private:
            uint8_t msgs[8];
            struct
            {
                uint8_t inverterTemp;                     // インバータ温度
                uint16_t maxAvailableMotorTorque : 12;    // モータ上限制限トルク
                uint16_t maxAvailableGenerateTorque : 12; // モータ下限制限トルク
                uint8_t motorTemp;                        // モータ温度
            };

        public:
            MSG()
            {
                inverterTemp = 0x3C;
                maxAvailableMotorTorque = 0x0;
                maxAvailableGenerateTorque = 0x7D0;
                motorTemp = 0x3C;
            }
        };

        MSG msg = MSG();

        namespace InverterTemperature
        {
            const int8_t offset = -40;
            const int8_t resolution = 1; // 1℃
            int8_t physical = 20;        // Physical Range -40~210
        }

        namespace MaximumAvailableMotoringTorque
        {
            const uint16_t offset = 0;
            const float resolution = 0.5f; // 0.5Nm
            uint16_t Physical = 0;         // Physical Range 0~1000
        }

        namespace MaximumAvailableGeneratingTorque
        {
            const int16_t offset = -1000;
            const float resolution = 0.5f; // 0.5Nm
            int16_t Physical = 0;          // Physical Range -1000~0
        }

        namespace MotorTemperature
        {
            const int8_t offset = -40;
            const int8_t resolution = 1;
            int8_t Physical = 20;
        }
    }
}

#endif