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

EV_ECU1::ECU *evecu1_p;
MG_ECU1::ECU *mgecu1_p;
MG_ECU2::ECU *mgecu2_p;

Parameter::Parameter(short offset, float resolution, short minPhysical, short maxPhysical)
    : offset(offset), resolution(resolution), minPhysical(minPhysical), maxPhysical(maxPhysical){};

float Parameter::calcPhysical(unsigned short normal)
{
    return (normal * resolution) + offset;
};

unsigned short Parameter::calcNormal(float physicalValue)
{
    return static_cast<unsigned short>((physicalValue - offset) / resolution);
};

EV_ECU1::MSG::MSG()
    : ecuEnable(0x0), dischargeCommand(0x0), reserve0(0x0), requestTorque(0x7D0){};

EV_ECU1::ECU::ECU(unsigned long id)
    : id(id)
{
    msg = new MSG();
    torqueRequestPara = new Parameter(-1000, 0.5f, -1000, 1000);
};

EV_ECU1::ECU::~ECU()
{
    delete (msg);
    delete (torqueRequestPara);
};

unsigned char EV_ECU1::ECU::getMsgByte(unsigned char index)
{
    if (0 <= index && index <= 7)
    {
        return msg->msgs[index];
    }
    else
    {
        return 0;
    }
};

float EV_ECU1::ECU::getRequestTorque()
{
    return torqueRequestPara->calcPhysical(msg->requestTorque);
};

unsigned char EV_ECU1::ECU::setEcuEnable(unsigned char ecuEnable)
{
    if (ecuEnable == 0 || ecuEnable == 1)
    {
        msg->ecuEnable = ecuEnable;
        return 1;
    }
    else
    {
        return 0;
    }
};

unsigned char EV_ECU1::ECU::setDischargeCommand(unsigned char dischargeCommand)
{
    if (dischargeCommand == 0 || dischargeCommand == 1)
    {
        msg->dischargeCommand = dischargeCommand;
        return 1;
    }
    else
    {
        return 0;
    }
};

unsigned char EV_ECU1::ECU::setRequestTorque(float physicalValue)
{
    if (torqueRequestPara->getMinPhysical() <= physicalValue && physicalValue <= torqueRequestPara->getMaxPhysical())
    {
        msg->requestTorque = torqueRequestPara->calcNormal(physicalValue);
        return 1;
    }
    else
    {
        return 0;
    }
};

MG_ECU1::MSG::MSG()
    : shutdownEnable(0x0), PWM(0x1), workingStatus(0x0), reserve0(0x0), motorSpeed(0x36B0), motorPhaseCurrent(0x0), inputDCVoltage(0x3FF), reserve1(0x0), failureStatus(0x0){};

MG_ECU1::ECU::ECU(unsigned long id)
    : id(id)
{
    msg = new MSG();
    motorSpeedPara = new Parameter(-14000, 1, -14000, 14000);
    motorPhaseCurrentPara = new Parameter(0, 0.5f, 0, 500);
};

MG_ECU1::ECU::~ECU()
{
    delete (msg);
    delete (motorSpeedPara);
    delete (motorPhaseCurrentPara);
};

unsigned char MG_ECU1::ECU::getMsgByte(unsigned char index)
{
    if (0 <= index && index <= 7)
    {
        return msg->msgs[index];
    }
    else
    {
        return 0;
    }
};

float MG_ECU1::ECU::getMotorSpeed()
{
    return motorSpeedPara->calcPhysical(msg->motorSpeed);
};

float MG_ECU1::ECU::getMotorPhaseCurrent()
{
    return motorPhaseCurrentPara->calcPhysical(msg->motorPhaseCurrent);
};

unsigned char MG_ECU1::ECU::setMsg(unsigned char *buf)
{
    for (int i = 0; i < 8; i++)
    {
        msg->msgs[i] = *(buf + i);
    }

    return 1;
};

MG_ECU2::MSG::MSG()
    : inverterTemp(0x3C), maxAvailableMotorTorque(0x0), maxAvailableGenerateTorque(0x7D0), motorTemp(0x3C){};

MG_ECU2::ECU::ECU(unsigned long id)
    : id(id)
{
    msg = new MSG();
    inverterTemperaturePara = new Parameter(-40, 1, -40, 210);
    maximumAvailableMotoringTorquePara = new Parameter(0, 0.5f, 0, 1000);
    maximumAvailableGeneratingTorquePara = new Parameter(-1000, 0.5f, -1000, 0);
    motorTemperaturePara = new Parameter(-40, 1, -40, 210);
};

MG_ECU2::ECU::~ECU()
{
    delete (msg);
    delete (inverterTemperaturePara);
    delete (maximumAvailableMotoringTorquePara);
    delete (maximumAvailableGeneratingTorquePara);
    delete (motorTemperaturePara);
};

unsigned char MG_ECU2::ECU::getMsgByte(unsigned char index)
{
    if (0 <= index && index <= 7)
    {
        return msg->msgs[index];
    }
    else
    {
        return 0;
    }
};

float MG_ECU2::ECU::getInverterTemp()
{
    return inverterTemperaturePara->calcPhysical(msg->inverterTemp);
};

float MG_ECU2::ECU::getMaxAvailableMotorTorque()
{
    return maximumAvailableMotoringTorquePara->calcPhysical(msg->maxAvailableMotorTorque);
};

float MG_ECU2::ECU::getMaxAvailableGenerateTorque()
{
    return maximumAvailableGeneratingTorquePara->calcPhysical(msg->maxAvailableGenerateTorque);
};

float MG_ECU2::ECU::getMotorTemp()
{
    return motorTemperaturePara->calcPhysical(msg->motorTemp);
};

unsigned char MG_ECU2::ECU::setMsg(unsigned char *buf)
{
    for (int i = 0; i < 8; i++)
    {
        msg->msgs[i] = *(buf + i);
    }

    return 1;
};

void init(EV_ECU1::ECU *ee1, MG_ECU1::ECU *me1, MG_ECU2::ECU *me2)
{
    evecu1_p = ee1;
    mgecu1_p = me1;
    mgecu2_p = me2;

    SERIAL_PORT_MONITOR.begin(115200);
    while (CAN_OK != CAN.begin(CAN_500KBPS))
    {
        SERIAL_PORT_MONITOR.println("CAN init fail, retry...");
        delay(100);
    }
    SERIAL_PORT_MONITOR.println("CAN init OK!");
};

unsigned char sendMsgToInverter(unsigned char printFlag)
{
    unsigned char buf[8];

    for (int i = 0; i < 8; i++)
    {
        buf[i] = evecu1_p->getMsgByte(i);
    }

    CAN.sendMsgBuf(evecu1_p->getID(), 0, 8, buf);

    if (printFlag)
    {
        SERIAL_PORT_MONITOR.println("send massage to inverter");
        checkBuf(buf);
    }

    return 1;
};

unsigned long readMsgFromInverter(unsigned char printFlag)
{
    if (CAN_MSGAVAIL == CAN.checkReceive())
    {
        unsigned char len = 0;
        unsigned char buf[8];
        CAN.readMsgBuf(&len, buf);
        unsigned long id = CAN.getCanId();

        if (id == mgecu1_p->getID())
        {
            if (printFlag)
            {
                SERIAL_PORT_MONITOR.println("set massage from MG-ECU1");
                SERIAL_PORT_MONITOR.print("ID = ");
                SERIAL_PORT_MONITOR.println(id, HEX);
                checkBuf(buf);
            }

            mgecu1_p->setMsg(buf);
            return id;
        }
        else if (id == mgecu2_p->getID())
        {
            if (printFlag)
            {
                SERIAL_PORT_MONITOR.println("set massage from MG-ECU2");
                SERIAL_PORT_MONITOR.print("ID = ");
                SERIAL_PORT_MONITOR.println(id, HEX);
                checkBuf(buf);
            }

            mgecu2_p->setMsg(buf);
            return id;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }
};

void checkBuf(unsigned char *buf)
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

void checkMsgBit(unsigned long id)
{
    unsigned char buf[8];

    switch (id)
    {
    case EV_ECU1_ID:
        for (int i = 0; i < 8; i++)
        {
            buf[i] = evecu1_p->getMsgByte(i);
        }
        SERIAL_PORT_MONITOR.println("----------EVECU1Massage----------");
        break;

    case MG_ECU1_ID:
        for (int i = 0; i < 8; i++)
        {
            buf[i] = mgecu1_p->getMsgByte(i);
        }
        SERIAL_PORT_MONITOR.println("----------MGECU1Massage----------");
        break;

    case MG_ECU2_ID:
        for (int i = 0; i < 8; i++)
        {
            buf[i] = mgecu2_p->getMsgByte(i);
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

void checkMsg(unsigned long id)
{
    switch (id)
    {
    case EV_ECU1_ID:
        SERIAL_PORT_MONITOR.println("----------EVECU1----------");

        SERIAL_PORT_MONITOR.print("ecuEnable = ");
        SERIAL_PORT_MONITOR.println(evecu1_p->getEcuEnable(), BIN);
        SERIAL_PORT_MONITOR.print("dischargeCommand = ");
        SERIAL_PORT_MONITOR.println(evecu1_p->getDischargeCommand(), BIN);
        SERIAL_PORT_MONITOR.print("requestTorque = ");
        SERIAL_PORT_MONITOR.println(evecu1_p->getNormalRequestTorque(), BIN);

        if (evecu1_p->getEcuEnable())
        {
            SERIAL_PORT_MONITOR.println("MG-ECU実行要求ON");
        }
        else
        {
            SERIAL_PORT_MONITOR.println("MG-ECU実行要求OFF");
        }

        if (evecu1_p->getDischargeCommand())
        {
            SERIAL_PORT_MONITOR.println("平滑コンデンサ放電要求ON");
        }
        else
        {
            SERIAL_PORT_MONITOR.println("平滑コンデンサ放電要求OFF");
        }

        SERIAL_PORT_MONITOR.print("HV-ECU要求トルク ");
        SERIAL_PORT_MONITOR.println(evecu1_p->getRequestTorque());

        SERIAL_PORT_MONITOR.println("--------------------------");
        SERIAL_PORT_MONITOR.println();
        break;

    case MG_ECU1_ID:
        SERIAL_PORT_MONITOR.println("----------MGECU1----------");

        SERIAL_PORT_MONITOR.print("shutdownEnable = ");
        SERIAL_PORT_MONITOR.println(mgecu1_p->getShutdownEnable(), BIN);
        SERIAL_PORT_MONITOR.print("PWM = ");
        SERIAL_PORT_MONITOR.println(mgecu1_p->getPWM(), BIN);
        SERIAL_PORT_MONITOR.print("WorkingStatus = ");
        SERIAL_PORT_MONITOR.println(mgecu1_p->getWorkingStatus(), BIN);
        SERIAL_PORT_MONITOR.print("motorSpeed = ");
        SERIAL_PORT_MONITOR.println(mgecu1_p->getNormalMotorSpeed(), BIN);
        SERIAL_PORT_MONITOR.print("motorPhaseCurrent = ");
        SERIAL_PORT_MONITOR.println(mgecu1_p->getNormalMotorPhaseCurrent(), BIN);
        SERIAL_PORT_MONITOR.print("inputDCVoltage = ");
        SERIAL_PORT_MONITOR.println(mgecu1_p->getInputDCVoltage(), BIN);
        SERIAL_PORT_MONITOR.print("failureStatus = ");
        SERIAL_PORT_MONITOR.println(mgecu1_p->getFailureStatus(), BIN);

        if (mgecu1_p->getShutdownEnable())
        {
            SERIAL_PORT_MONITOR.println("シャットダウン許可有効");
        }
        else
        {
            SERIAL_PORT_MONITOR.println("シャットダウン許可無効");
        }

        SERIAL_PORT_MONITOR.print("ゲート駆動状態 ");
        switch (mgecu1_p->getPWM())
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
        switch (mgecu1_p->getWorkingStatus())
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
        SERIAL_PORT_MONITOR.println(mgecu1_p->getMotorSpeed());

        SERIAL_PORT_MONITOR.print("モータ相電流 ");
        SERIAL_PORT_MONITOR.println(mgecu1_p->getMotorPhaseCurrent());

        SERIAL_PORT_MONITOR.print("入力直流電圧 ");
        SERIAL_PORT_MONITOR.println(mgecu1_p->getInputDCVoltage());

        SERIAL_PORT_MONITOR.print("異常状態 ");
        switch (mgecu1_p->getFailureStatus())
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
        SERIAL_PORT_MONITOR.println(mgecu2_p->getNormalInverterTemp(), BIN);
        SERIAL_PORT_MONITOR.print("maxMotorTorque = ");
        SERIAL_PORT_MONITOR.println(mgecu2_p->getNormalMaxAvailableMotorTorque(), BIN);
        SERIAL_PORT_MONITOR.print("maxGenerateTorque = ");
        SERIAL_PORT_MONITOR.println(mgecu2_p->getNormalMaxAvailableGenerateTorque(), BIN);
        SERIAL_PORT_MONITOR.print("motorTemp = ");
        SERIAL_PORT_MONITOR.println(mgecu2_p->getNormalMotorTemp(), BIN);

        SERIAL_PORT_MONITOR.print("インバータ温度 ");
        SERIAL_PORT_MONITOR.println(mgecu2_p->getInverterTemp());

        SERIAL_PORT_MONITOR.print("モーター上限制限トルク ");
        SERIAL_PORT_MONITOR.println(mgecu2_p->getMaxAvailableMotorTorque());

        SERIAL_PORT_MONITOR.print("モーター下限制限トルク ");
        SERIAL_PORT_MONITOR.println(mgecu2_p->getMaxAvailableGenerateTorque());

        SERIAL_PORT_MONITOR.print("モーター温度 ");
        SERIAL_PORT_MONITOR.println(mgecu2_p->getMotorTemp());

        SERIAL_PORT_MONITOR.println("--------------------------");
        SERIAL_PORT_MONITOR.println();
        break;

    default:
        break;
    }
};