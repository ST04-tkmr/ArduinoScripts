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

Parameter::Parameter(unsigned short offset, float resolution, unsigned short minPhysical, unsigned short maxPhysical)
{
    this->offset = offset;
    this->resolution = resolution;
    this->minPhysical = minPhysical;
    this->maxPhysical = maxPhysical;
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

unsigned char MG_ECU1::ECU::setMsg(unsigned char *buf)
{
    for (int i=0; i<8; i++)
    {
        msg->msgs[i] = *(buf + i);
    }

    return 1;
};

void MG_ECU1::ECU::checkMGECU1(void)
{
    SERIAL_PORT_MONITOR.println("----------MGECU1----------");

    SERIAL_PORT_MONITOR.print("shutdownEnable = ");
    SERIAL_PORT_MONITOR.println(getShutdownEnable(), BIN);
    SERIAL_PORT_MONITOR.print("PWM = ");
    SERIAL_PORT_MONITOR.println(getPWM(), BIN);
    SERIAL_PORT_MONITOR.print("WorkingStatus = ");
    SERIAL_PORT_MONITOR.println(getWorkingStatus(), BIN);
    SERIAL_PORT_MONITOR.print("motorSpeed = ");
    SERIAL_PORT_MONITOR.println(getNormalMotorSpeed(), BIN);
    SERIAL_PORT_MONITOR.print("motorPhaseCurrent = ");
    SERIAL_PORT_MONITOR.println(getNormalMotorPhaseCurrent(), BIN);
    SERIAL_PORT_MONITOR.print("inputDCVoltage = ");
    SERIAL_PORT_MONITOR.println(getInputDCVoltage(), BIN);
    SERIAL_PORT_MONITOR.print("failureStatus = ");
    SERIAL_PORT_MONITOR.println(getFailureStatus(), BIN);

    if (getShutdownEnable())
    {
        SERIAL_PORT_MONITOR.println("シャットダウン許可有効");
    }
    else
    {
        SERIAL_PORT_MONITOR.println("シャットダウン許可無効");
    }

    SERIAL_PORT_MONITOR.print("ゲート駆動状態 ");
    switch (getPWM())
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
    switch (getWorkingStatus())
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
    if (getMotorSpeed() == 0x00)
    {
        SERIAL_PORT_MONITOR.println("調整中 0位置");
    }
    else
    {
        SERIAL_PORT_MONITOR.println(getMotorSpeed() - 14000);
    }

    SERIAL_PORT_MONITOR.print("モータ相電流 ");
    if (getMotorPhaseCurrent() == 0x00)
    {
        SERIAL_PORT_MONITOR.println("調整中 0位置");
    }
    else
    {
        SERIAL_PORT_MONITOR.println(getMotorPhaseCurrent());
    }

    SERIAL_PORT_MONITOR.print("入力直流電圧 ");
    if (getInputDCVoltage() == 0x00)
    {
        SERIAL_PORT_MONITOR.println("調整中 0位置");
    }
    else
    {
        SERIAL_PORT_MONITOR.println(getInputDCVoltage());
    }

    SERIAL_PORT_MONITOR.print("異常状態 ");
    switch (getFailureStatus())
    {
    case 0b000:
        SERIAL_PORT_MONITOR.println("エラーなし");
        break;
    case 0b001:
        SERIAL_PORT_MONITOR.println("負荷軽減");
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

unsigned char MG_ECU2::ECU::setMsg(unsigned char *buf)
{
    for (int i=0; i<8; i++)
    {
        msg->msgs[i] = *(buf + i);
    }

    return 1;
};

void MG_ECU2::ECU::checkMGECU2(void)
{
    SERIAL_PORT_MONITOR.println("----------MGECU2----------");

    SERIAL_PORT_MONITOR.print("inverterTemp = ");
    SERIAL_PORT_MONITOR.println(getNormalInverterTemp(), BIN);
    SERIAL_PORT_MONITOR.print("maxMotorTorque = ");
    SERIAL_PORT_MONITOR.println(getNormalMaxAvailableMotorTorque(), BIN);
    SERIAL_PORT_MONITOR.print("maxGenerateTorque = ");
    SERIAL_PORT_MONITOR.println(getNormalMaxAvailableGenerateTorque(), BIN);
    SERIAL_PORT_MONITOR.print("motorTemp = ");
    SERIAL_PORT_MONITOR.println(getNormalMotorTemp(), BIN);

    SERIAL_PORT_MONITOR.print("インバータ温度 ");
    SERIAL_PORT_MONITOR.println(getInverterTemp());

    SERIAL_PORT_MONITOR.print("モーター上限制限トルク ");
    SERIAL_PORT_MONITOR.println(getMaxAvailableMotorTorque());

    SERIAL_PORT_MONITOR.print("モーター下限制限トルク ");
    SERIAL_PORT_MONITOR.println(getMaxAvailableGenerateTorque());

    SERIAL_PORT_MONITOR.print("モーター温度 ");
    SERIAL_PORT_MONITOR.println(getMotorTemp());

    SERIAL_PORT_MONITOR.println("--------------------------");
    SERIAL_PORT_MONITOR.println();
};

void init_CAN(void)
{
    SERIAL_PORT_MONITOR.begin(115200);
    while (CAN_OK != CAN.begin(CAN_500KBPS))
    {
        SERIAL_PORT_MONITOR.println("CAN init fail, retry...");
        delay(100);
    }
    SERIAL_PORT_MONITOR.println("CAN init OK!");
};

unsigned char sendMsgToInverter(EV_ECU1::ECU *ecu)
{
    unsigned char buf[8];

    if (ecu->getID() == EV_ECU1_ID)
    {
        for (int i=0; i<8; i++)
        {
            buf[i] = ecu->getMsgByte(i);
        }

        CAN.sendMsgBuf(ecu->getID(), 0, 8, buf);

        return 1;
    }
    else
    {
        SERIAL_PORT_MONITOR.println("send CAN massage fail, please check ID");
        return 0;
    }
};

unsigned long readMsgFromInverter(MG_ECU1::ECU *ecu1, MG_ECU2::ECU *ecu2)
{
    if (CAN_MSGAVAIL == CAN.checkReceive())
    {
        unsigned char len = 0;
        unsigned char buf[8];
        CAN.readMsgBuf(&len, buf);
        unsigned long id = CAN.getCanId();

        if (id == ecu1->getID())
        {
            ecu1->setMsg(buf);
            return id;
        }
        else if  (id == ecu2->getID())
        {
            ecu2->setMsg(buf);
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