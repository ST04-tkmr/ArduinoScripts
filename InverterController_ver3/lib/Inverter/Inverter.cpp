#include "Inverter.hpp"

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
}

Inverter::~Inverter()
{
    delete (evecu1);
    delete (mgecu1);
    delete (mgecu2);
}

void Inverter::init(void)
{
    SERIAL_PORT_MONITOR.begin(115200);
    while (CAN_OK != CAN.begin(CAN_500KBPS))
    {
        SERIAL_PORT_MONITOR.println("CAN init fail, retry...");
        delay(100);
    }
    SERIAL_PORT_MONITOR.println("CAN init OK!");
}

void Inverter::runInverter(unsigned char *flags, unsigned short batVol, float torque)
{
    unsigned char *airFlag = flags;
    unsigned char *torqueControlFlag = flags + 1;
    const unsigned char shutdownFlag = *(flags + 2);
    const unsigned char driveFlag = shutdownFlag ? 0 : *(flags + 3);
    const unsigned char ws = mgecu1->getWorkingStatus();
    const unsigned short idcv = mgecu1->getInputDCVoltage();
    const float ms = fabs(mgecu1->getMotorSpeed());

    setBatVol(batVol);

    // if shutdown button is pushed
    if (shutdownFlag)
    {
        *airFlag = 0;
        *torqueControlFlag = 0;
        evecu1->setRequestTorque(0);
        evecu1->setEcuEnable(0);
        switch (ws)
        {
        case WORKING_RAPID_DISCHARGE:
            if (!evecu1->getDischargeCommand())
            {
                evecu1->setDischargeCommand(1);
            }
            break;

        case WORKING_STANDBY:
            if (evecu1->getDischargeCommand())
            {
                evecu1->setDischargeCommand(0);
            }
            break;
        }
    }
    else
    {
        switch (ws)
        {
        case WORKING_PRECHARGE:
            // 最低入力電圧チェック & バッテリー電圧チェック
            if (MINIMUM_INPUT_VOLTAGE <= idcv && batteryVoltage != 0)
            {
                // プリチャージ完了チェック
                if ((batteryVoltage - idcv) < (batteryVoltage / 10))
                {
                    // MG-ECU実行要求からAIR ON
                    if (!evecu1->setEcuEnable(1))
                    {
                        *airFlag = 1;
                    }
                }
            }
            break;

        case WORKING_TORQUE_CONTROL:
            // Ready to Drive
            if (driveFlag)
            {
                if (*torqueControlFlag)
                {
                    torqueRequest(torque);
                }
                else
                {
                    *torqueControlFlag = 1;
                }
            }
            break;

        case WORKING_RAPID_DISCHARGE:
            if (!evecu1->getDischargeCommand())
            {
                if (*airFlag || *torqueControlFlag)
                {
                    *airFlag = 0;
                    *torqueControlFlag = 0;
                }
                else
                {
                    evecu1->setDischargeCommand(1);
                }
            }
            break;

        case WORKING_STANDBY:
            if (*airFlag || *torqueControlFlag)
            {
                *airFlag = 0;
                torqueControlFlag = 0;
            }

            if (evecu1->getEcuEnable())
            {
                evecu1->setEcuEnable(0);
            }

            if (evecu1->getDischargeCommand())
            {
                evecu1->setDischargeCommand(0);
            }

            if (evecu1->getRequestTorque() != 0.0f)
            {
                evecu1->setRequestTorque(0.0f);
            }
            break;

        default:
            break;
        }
    }
}

unsigned char Inverter::setBatVol(unsigned short batVol)
{
    if (MINIMUM_BATTERY_VOLTAGE <= batVol && batVol <= MAXIMUM_BATTERY_VOLTAGE)
    {
        batteryVoltage = batVol;
        return 0;
    }

    batteryVoltage = 0;
    return 1;
}

unsigned char Inverter::setMgecuRequestON(unsigned short batVol)
{
    unsigned char ws = mgecu1->getWorkingStatus();
    unsigned short idcv = mgecu1->getInputDCVoltage();

    setBatVol(batVol);

    switch (ws)
    {
    case WORKING_PRECHARGE:

        // 入力最低電圧
        if (MINIMUM_INPUT_VOLTAGE <= idcv && batteryVoltage != 0)
        {

            // プリチャージ完了条件
            if ((batteryVoltage - idcv) < (batteryVoltage / 10))
            {
                return evecu1->setEcuEnable(1);
            }
        }
        break;

    default:
        break;
    }

    return 1;
}

unsigned char Inverter::setMgecuRequestOFF()
{
    unsigned char ws = mgecu1->getWorkingStatus();
    float ms = fabs(mgecu1->getMotorSpeed());

    switch (ws)
    {
    case WORKING_TORQUE_CONTROL:
        // torque control終了条件
        if (ms < ECU_DISABLE_MOTOR_SPEED)
        {
            return evecu1->setEcuEnable(0);
        }
        break;

    default:
        break;
    }

    return 1;
}

unsigned char Inverter::setRapidDischargeRequestON()
{
    unsigned char ws = mgecu1->getWorkingStatus();

    if (ws == WORKING_RAPID_DISCHARGE)
    {
        return evecu1->setDischargeCommand(1);
    }

    return 1;
}

unsigned char Inverter::setRapidDischargeRequestOFF()
{
    return evecu1->setDischargeCommand(0);
}

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
}

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
}

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
}

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
}

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
}

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
            SERIAL_PORT_MONITOR.println("MG-ECU Enable");
        }
        else
        {
            SERIAL_PORT_MONITOR.println("MG-ECU Disable");
        }

        if (evecu1->getDischargeCommand())
        {
            SERIAL_PORT_MONITOR.println("Rapid Discharge Command Active");
        }
        else
        {
            SERIAL_PORT_MONITOR.println("Rapid Discharge Command Inactive");
        }

        SERIAL_PORT_MONITOR.print("Torque Request ");
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
            SERIAL_PORT_MONITOR.println("Shutdown Enable");
        }
        else
        {
            SERIAL_PORT_MONITOR.println("Shutdown Not Enable");
        }

        SERIAL_PORT_MONITOR.print("PWM ");
        switch (mgecu1->getPWM())
        {
        case PWM_SHORT_CIRCUIT:
            SERIAL_PORT_MONITOR.println("Short Circuit");
            break;

        case PWM_FREE_WHEEL:
            SERIAL_PORT_MONITOR.println("Free Wheel");
            break;

        case PWM_RUN:
            SERIAL_PORT_MONITOR.println("PWM Run");
            break;

        default:
            SERIAL_PORT_MONITOR.println("error");
            break;
        }

        SERIAL_PORT_MONITOR.print("Working Status ");
        switch (mgecu1->getWorkingStatus())
        {
        case WORKING_INIT:
            SERIAL_PORT_MONITOR.println("init");
            break;

        case WORKING_PRECHARGE:
            SERIAL_PORT_MONITOR.println("precharge");
            break;

        case WORKING_STANDBY:
            SERIAL_PORT_MONITOR.println("standby");
            break;

        case WORKING_TORQUE_CONTROL:
            SERIAL_PORT_MONITOR.println("torque control");
            break;

        case WORKING_RAPID_DISCHARGE:
            SERIAL_PORT_MONITOR.println("rapid discharge");
            break;

        default:
            SERIAL_PORT_MONITOR.println("error");
            break;
        }

        SERIAL_PORT_MONITOR.print("Motor Speed ");
        SERIAL_PORT_MONITOR.println(mgecu1->getMotorSpeed());

        SERIAL_PORT_MONITOR.print("Motor Phase Current ");
        SERIAL_PORT_MONITOR.println(mgecu1->getMotorPhaseCurrent());

        SERIAL_PORT_MONITOR.print("Input DC Voltage ");
        SERIAL_PORT_MONITOR.println(mgecu1->getInputDCVoltage());

        SERIAL_PORT_MONITOR.print("Failure Status ");
        switch (mgecu1->getFailureStatus())
        {
        case FAILURE_NO_ERROR:
            SERIAL_PORT_MONITOR.println("No Error");
            break;

        case FAILURE_DERATING:
            SERIAL_PORT_MONITOR.println("Derating");
            break;

        case FAILURE_WARNING:
            SERIAL_PORT_MONITOR.println("Warning");
            break;

        case FAILURE_ERROR:
            SERIAL_PORT_MONITOR.println("Error");
            break;

        case FAILURE_CRITICAL_ERROR:
            SERIAL_PORT_MONITOR.println("Critical Error");

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

        SERIAL_PORT_MONITOR.print("Inverter Temperature ");
        SERIAL_PORT_MONITOR.println(mgecu2->getInverterTemp());

        SERIAL_PORT_MONITOR.print("Maximum Available Motoring Torque ");
        SERIAL_PORT_MONITOR.println(mgecu2->getMaxAvailableMotorTorque());

        SERIAL_PORT_MONITOR.print("Maximum Available Generating Torque ");
        SERIAL_PORT_MONITOR.println(mgecu2->getMaxAvailableGenerateTorque());

        SERIAL_PORT_MONITOR.print("Motor Temperature ");
        SERIAL_PORT_MONITOR.println(mgecu2->getMotorTemp());

        SERIAL_PORT_MONITOR.println("--------------------------");
        SERIAL_PORT_MONITOR.println();
        break;

    default:
        break;
    }
}