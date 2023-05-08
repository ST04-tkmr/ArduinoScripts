#include "Inverter.hpp"
#include "Inverter_dfs.hpp"

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

Inverter::Inverter()
{
    evecu1 = new EV_ECU1::ECU(EV_ECU1_ID);
    mgecu1 = new MG_ECU1::ECU(MG_ECU1_ID);
    mgecu2 = new MG_ECU2::ECU(MG_ECU2_ID);
};

Inverter::~Inverter()
{
    delete (evecu1);
    delete (mgecu1);
    delete (mgecu2);
};

void Inverter::init(void)
{
    SERIAL_PORT_MONITOR.begin(115200);
    while (CAN_OK != CAN.begin(CAN_500KBPS))
    {
        SERIAL_PORT_MONITOR.println("CAN init fail, retry...");
        delay(100);
    }
    SERIAL_PORT_MONITOR.println("CAN init OK!");
};

unsigned char Inverter::setMgecuRequest(unsigned char request)
{
    unsigned char ws = mgecu1->getWorkingStatus();
    unsigned short idcv = mgecu1->getInputDCVoltage();
    float ms = fabs(mgecu1->getMotorSpeed());

    switch (ws)
    {
    case WORKING_PRECHARGE:
        if (MINIMUM_INPUT_VOLTAGE <= idcv)
        {
            if ((BATTERY_VOLTAGE - idcv) < (BATTERY_VOLTAGE / 10))
            {
                return evecu1->setEcuEnable(request);
            }
            else
            {
                evecu1->setEcuEnable(0);
                return 1;
            }
        }
        else
        {
            evecu1->setEcuEnable(0);
            return 1;
        }
        break;

    case WORKING_TORQUE_CONTROL:
        if (ms < ECU_DISABLE_MOTOR_SPEED)
        {
            return evecu1->setEcuEnable(request);
        }
        else
        {
            evecu1->setEcuEnable(1);
            return 1;
        }
        break;

    default:
        return evecu1->setEcuEnable(0);
        break;
    }
};

unsigned char Inverter::setRapidDischargeRequest(unsigned char request)
{
    unsigned char ws = mgecu1->getWorkingStatus();

    if (ws == WORKING_RAPID_DISCHARGE)
    {
        return evecu1->setDischargeCommand(request);
    }

    evecu1->setDischargeCommand(0);
    return 1;
};

unsigned char Inverter::torqueRequest(float torque)
{
    unsigned char ws = mgecu1->getWorkingStatus();

    if (ws == WORKING_TORQUE_CONTROL)
    {
        float mamt = mgecu2->getMaxAvailableMotorTorque();

        if (mamt < torque)
        {
            return evecu1->setRequestTorque(mamt);
        }

        float magt = mgecu2->getMaxAvailableGenerateTorque();

        if (torque < magt)
        {
            return evecu1->setRequestTorque(magt);
        }

        return evecu1->setRequestTorque(torque);
    }

    evecu1->setRequestTorque(0);
    return 1;
};

unsigned char Inverter::sendMsgToInverter(unsigned char printFlag)
{
    unsigned char buf[8];

    for (int i = 0; i < 8; i++)
    {
        buf[i] = evecu1->getMsgByte(i);
    }

    unsigned char result = CAN.sendMsgBuf(evecu1->getID(), 0, 8, buf);

    if (printFlag)
    {
        switch (result)
        {
        case CAN_OK:
            SERIAL_PORT_MONITOR.println("send massage to inverter");
            SERIAL_PORT_MONITOR.println("----------Massage----------");
            checkBuf(buf);
            SERIAL_PORT_MONITOR.println("---------------------------");
            SERIAL_PORT_MONITOR.println();
            break;

        case CAN_GETTXBFTIMEOUT:
            SERIAL_PORT_MONITOR.println("get TXBF time out");
            break;

        case CAN_SENDMSGTIMEOUT:
            SERIAL_PORT_MONITOR.println("send MSG time out");
            break;

        default:
            SERIAL_PORT_MONITOR.println("send MSG fail");
            break;
        }
    }

    return result;
};

unsigned long Inverter::readMsgFromInverter(unsigned char printFlag)
{
    if (CAN_MSGAVAIL == CAN.checkReceive())
    {
        unsigned char len = 0;
        unsigned char buf[8];
        unsigned char result = CAN.readMsgBuf(&len, buf);
        unsigned long id = CAN.getCanId();

        if (result == CAN_OK)
        {
            if (id == mgecu1->getID())
            {
                if (printFlag)
                {
                    SERIAL_PORT_MONITOR.println("set massage from MG-ECU1");
                    SERIAL_PORT_MONITOR.print("ID = ");
                    SERIAL_PORT_MONITOR.println(id, HEX);
                    SERIAL_PORT_MONITOR.println("----------buf----------");
                    checkBuf(buf);
                    SERIAL_PORT_MONITOR.println("-----------------------");
                    SERIAL_PORT_MONITOR.println();
                }

                mgecu1->setMsg(buf);
                return id;
            }
            else if (id == mgecu2->getID())
            {
                if (printFlag)
                {
                    SERIAL_PORT_MONITOR.println("set massage from MG-ECU2");
                    SERIAL_PORT_MONITOR.print("ID = ");
                    SERIAL_PORT_MONITOR.println(id, HEX);
                    SERIAL_PORT_MONITOR.println("----------buf----------");
                    checkBuf(buf);
                    SERIAL_PORT_MONITOR.println("-----------------------");
                    SERIAL_PORT_MONITOR.println();
                }

                mgecu2->setMsg(buf);
                return id;
            }

            if (printFlag)
            {
                SERIAL_PORT_MONITOR.println("received ID is unknown");
            }
            return 0;
        }
        else if (result == CAN_NOMSG)
        {
            if (printFlag)
            {
                SERIAL_PORT_MONITOR.println("no massage");
            }
            return 0;
        }
    }

    if (printFlag)
    {
        SERIAL_PORT_MONITOR.println("read massage fail");
    }
    return 0;
};

void Inverter::checkBuf(unsigned char *buf)
{
    for (int i = 0; i < 8; i++)
    {
        SERIAL_PORT_MONITOR.print("buf");
        SERIAL_PORT_MONITOR.print(i);
        SERIAL_PORT_MONITOR.print(" = ");

        for (int j = 7; j >= 0; j--)
        {
            unsigned char bit = (*(buf + i) & (0x01 << j)) >> j;
            SERIAL_PORT_MONITOR.print(bit);
        }

        SERIAL_PORT_MONITOR.println();
    }
};

void Inverter::checkMsgBit(unsigned long id)
{
    unsigned char buf[8];

    switch (id)
    {
    case EV_ECU1_ID:
        for (int i = 0; i < 8; i++)
        {
            buf[i] = evecu1->getMsgByte(i);
        }
        SERIAL_PORT_MONITOR.println("----------EVECU1Massage----------");
        break;

    case MG_ECU1_ID:
        for (int i = 0; i < 8; i++)
        {
            buf[i] = mgecu1->getMsgByte(i);
        }
        SERIAL_PORT_MONITOR.println("----------MGECU1Massage----------");
        break;

    case MG_ECU2_ID:
        for (int i = 0; i < 8; i++)
        {
            buf[i] = mgecu2->getMsgByte(i);
        }
        SERIAL_PORT_MONITOR.println("----------MGECU2Massage----------");
        break;

    default:
        SERIAL_PORT_MONITOR.print(id);
        SERIAL_PORT_MONITOR.println(" is invalid ID");
        return;
        break;
    }

    checkBuf(buf);

    SERIAL_PORT_MONITOR.println("---------------------------------");
    SERIAL_PORT_MONITOR.println();
};

void Inverter::checkMsg(unsigned long id)
{
    switch (id)
    {
    case EV_ECU1_ID:
        SERIAL_PORT_MONITOR.println("----------EVECU1----------");

        SERIAL_PORT_MONITOR.print("ecuEnable = ");
        SERIAL_PORT_MONITOR.println(evecu1->getEcuEnable(), BIN);
        SERIAL_PORT_MONITOR.print("dischargeCommand = ");
        SERIAL_PORT_MONITOR.println(evecu1->getDischargeCommand(), BIN);
        SERIAL_PORT_MONITOR.print("requestTorque = ");
        SERIAL_PORT_MONITOR.println(evecu1->getNormalRequestTorque(), BIN);

        if (evecu1->getEcuEnable())
        {
            SERIAL_PORT_MONITOR.println("MG-ECU実行要求ON");
        }
        else
        {
            SERIAL_PORT_MONITOR.println("MG-ECU実行要求OFF");
        }

        if (evecu1->getDischargeCommand())
        {
            SERIAL_PORT_MONITOR.println("平滑コンデンサ放電要求ON");
        }
        else
        {
            SERIAL_PORT_MONITOR.println("平滑コンデンサ放電要求OFF");
        }

        SERIAL_PORT_MONITOR.print("HV-ECU要求トルク ");
        SERIAL_PORT_MONITOR.println(evecu1->getRequestTorque());

        SERIAL_PORT_MONITOR.println("--------------------------");
        SERIAL_PORT_MONITOR.println();
        break;

    case MG_ECU1_ID:
        SERIAL_PORT_MONITOR.println("----------MGECU1----------");

        SERIAL_PORT_MONITOR.print("shutdownEnable = ");
        SERIAL_PORT_MONITOR.println(mgecu1->getShutdownEnable(), BIN);
        SERIAL_PORT_MONITOR.print("PWM = ");
        SERIAL_PORT_MONITOR.println(mgecu1->getPWM(), BIN);
        SERIAL_PORT_MONITOR.print("WorkingStatus = ");
        SERIAL_PORT_MONITOR.println(mgecu1->getWorkingStatus(), BIN);
        SERIAL_PORT_MONITOR.print("motorSpeed = ");
        SERIAL_PORT_MONITOR.println(mgecu1->getNormalMotorSpeed(), BIN);
        SERIAL_PORT_MONITOR.print("motorPhaseCurrent = ");
        SERIAL_PORT_MONITOR.println(mgecu1->getNormalMotorPhaseCurrent(), BIN);
        SERIAL_PORT_MONITOR.print("inputDCVoltage = ");
        SERIAL_PORT_MONITOR.println(mgecu1->getInputDCVoltage(), BIN);
        SERIAL_PORT_MONITOR.print("failureStatus = ");
        SERIAL_PORT_MONITOR.println(mgecu1->getFailureStatus(), BIN);

        if (mgecu1->getShutdownEnable())
        {
            SERIAL_PORT_MONITOR.println("シャットダウン許可有効");
        }
        else
        {
            SERIAL_PORT_MONITOR.println("シャットダウン許可無効");
        }

        SERIAL_PORT_MONITOR.print("ゲート駆動状態 ");
        switch (mgecu1->getPWM())
        {
        case 0b00:
            SERIAL_PORT_MONITOR.println("短絡");
            break;

        case 0b01:
            SERIAL_PORT_MONITOR.println("正常 トルクなし");
            break;

        case 0b10:
            SERIAL_PORT_MONITOR.println("正常 制御中");
            break;

        default:
            SERIAL_PORT_MONITOR.println("error");
            break;
        }

        SERIAL_PORT_MONITOR.print("制御状態 ");
        switch (mgecu1->getWorkingStatus())
        {
        case 0b000:
            SERIAL_PORT_MONITOR.println("初期状態");
            break;

        case 0b001:
            SERIAL_PORT_MONITOR.println("プリチャージ中");
            break;

        case 0b010:
            SERIAL_PORT_MONITOR.println("スタンバイ");
            break;

        case 0b011:
            SERIAL_PORT_MONITOR.println("トルク制御中");
            break;

        case 0b111:
            SERIAL_PORT_MONITOR.println("急速放電中");
            break;

        default:
            SERIAL_PORT_MONITOR.println("error");
            break;
        }

        SERIAL_PORT_MONITOR.print("モータ回転数 ");
        SERIAL_PORT_MONITOR.println(mgecu1->getMotorSpeed());

        SERIAL_PORT_MONITOR.print("モータ相電流 ");
        SERIAL_PORT_MONITOR.println(mgecu1->getMotorPhaseCurrent());

        SERIAL_PORT_MONITOR.print("入力直流電圧 ");
        SERIAL_PORT_MONITOR.println(mgecu1->getInputDCVoltage());

        SERIAL_PORT_MONITOR.print("異常状態 ");
        switch (mgecu1->getFailureStatus())
        {
        case 0b000:
            SERIAL_PORT_MONITOR.println("エラーなし");
            break;

        case 0b001:
            SERIAL_PORT_MONITOR.println("モータ出力制限中");
            break;

        case 0b010:
            SERIAL_PORT_MONITOR.println("警告");
            break;

        case 0b101:
            SERIAL_PORT_MONITOR.println("クリティカルエラー");
            break;

        default:
            SERIAL_PORT_MONITOR.println("error");
            break;
        }

        SERIAL_PORT_MONITOR.println("--------------------------");
        SERIAL_PORT_MONITOR.println();
        break;

    case MG_ECU2_ID:
        SERIAL_PORT_MONITOR.println("----------MGECU2----------");

        SERIAL_PORT_MONITOR.print("inverterTemp = ");
        SERIAL_PORT_MONITOR.println(mgecu2->getNormalInverterTemp(), BIN);
        SERIAL_PORT_MONITOR.print("maxMotorTorque = ");
        SERIAL_PORT_MONITOR.println(mgecu2->getNormalMaxAvailableMotorTorque(), BIN);
        SERIAL_PORT_MONITOR.print("maxGenerateTorque = ");
        SERIAL_PORT_MONITOR.println(mgecu2->getNormalMaxAvailableGenerateTorque(), BIN);
        SERIAL_PORT_MONITOR.print("motorTemp = ");
        SERIAL_PORT_MONITOR.println(mgecu2->getNormalMotorTemp(), BIN);

        SERIAL_PORT_MONITOR.print("インバータ温度 ");
        SERIAL_PORT_MONITOR.println(mgecu2->getInverterTemp());

        SERIAL_PORT_MONITOR.print("モーター上限制限トルク ");
        SERIAL_PORT_MONITOR.println(mgecu2->getMaxAvailableMotorTorque());

        SERIAL_PORT_MONITOR.print("モーター下限制限トルク ");
        SERIAL_PORT_MONITOR.println(mgecu2->getMaxAvailableGenerateTorque());

        SERIAL_PORT_MONITOR.print("モーター温度 ");
        SERIAL_PORT_MONITOR.println(mgecu2->getMotorTemp());

        SERIAL_PORT_MONITOR.println("--------------------------");
        SERIAL_PORT_MONITOR.println();
        break;

    default:
        break;
    }
};