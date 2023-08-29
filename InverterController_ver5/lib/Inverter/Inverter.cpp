#include "Inverter.hpp"

/*
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
*/

#ifndef ARDUINO_UNO_R4
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
    Serial.begin(115200);
    while (!Serial)
    {
    }

#ifdef ARDUINO_UNO_R4
    if (!CAN.begin(CanBitRate::BR_500k))
    {
        Serial.println("CAN.begin(...) failed.");
        for (;;)
        {
        }
    }
#else
    while (CAN_OK != CAN.begin(CAN_500KBPS))
    {
        Serial.println("CAN init fail, retry...");
        delay(100);
    }
    Serial.println("CAN init OK!");
#endif
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
                // evecu1->setDischargeCommand(1);
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
                    // evecu1->setDischargeCommand(1);
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

int Inverter::sendMsgToInverter(unsigned char printFlag)
{
    unsigned char buf[8];

    for (int i = 0; i < 8; i++)
    {
        buf[i] = evecu1->getMsgByte(i);
    }

#ifdef ARDUINO_UNO_R4
    CanMsg msg(evecu1->getID(), sizeof(buf), buf);

    int const rc = CAN.write(msg);

    if (printFlag)
    {
        if (rc < 0)
        {
            Serial.print("CAN.write(...) failed with error code : ");
            Serial.println(rc);
            Serial.println();
        }
        else
        {
            Serial.println("send massage to inverter");
            Serial.println("----------Massage----------");
            checkBuf(buf);
            Serial.println("---------------------------");
            Serial.println();
        }
    }

    return rc;
#else
    unsigned char result = CAN.sendMsgBuf(evecu1->getID(), 0, 8, buf);

    if (printFlag)
    {
        switch (result)
        {
        case CAN_OK:
            Serial.println("send massage to inverter");
            Serial.println("----------Massage----------");
            checkBuf(buf);
            Serial.println("---------------------------");
            Serial.println();
            break;

        case CAN_GETTXBFTIMEOUT:
            Serial.println("get TXBF time out");
            break;

        case CAN_SENDMSGTIMEOUT:
            Serial.println("send MSG time out");
            break;

        default:
            Serial.println("send MSG fail");
            break;
        }
    }

    return result;
#endif
}

unsigned long Inverter::readMsgFromInverter(unsigned char printFlag)
{
#ifdef ARDUINO_UNO_R4
    if (CAN.available())
    {
        CanMsg const msg = CAN.read();

        unsigned long id = msg.id;
        unsigned char buf[8];

        memcpy(buf, msg.data, msg.data_length);

        if (id == mgecu1->getID())
        {
            if (printFlag)
            {
                Serial.println("set massage from MG-ECU1");
                Serial.print("ID = ");
                Serial.println(id, HEX);
                Serial.println("----------buf----------");
                checkBuf(buf);
                Serial.println("-----------------------");
                Serial.println();
            }

            mgecu1->setMsg(buf);
            return id;
        }
        else if (id == mgecu2->getID())
        {
            if (printFlag)
            {
                Serial.println("set massage from MG-ECU2");
                Serial.print("ID = ");
                Serial.println(id, HEX);
                Serial.println("----------buf----------");
                checkBuf(buf);
                Serial.println("-----------------------");
                Serial.println();
            }

            mgecu2->setMsg(buf);
            return id;
        }
        else
        {
            if (printFlag)
            {
                Serial.println("received ID is unknown");
            }
            return id;
        }
    }
    else
    {
        if (printFlag)
        {
            Serial.println("read massage fail");
        }
        return 0;
    }
#else
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
                    Serial.println("set massage from MG-ECU1");
                    Serial.print("ID = ");
                    Serial.println(id, HEX);
                    Serial.println("----------buf----------");
                    checkBuf(buf);
                    Serial.println("-----------------------");
                    Serial.println();
                }

                mgecu1->setMsg(buf);
                return id;
            }
            else if (id == mgecu2->getID())
            {
                if (printFlag)
                {
                    Serial.println("set massage from MG-ECU2");
                    Serial.print("ID = ");
                    Serial.println(id, HEX);
                    Serial.println("----------buf----------");
                    checkBuf(buf);
                    Serial.println("-----------------------");
                    Serial.println();
                }

                mgecu2->setMsg(buf);
                return id;
            }

            if (printFlag)
            {
                Serial.println("received ID is unknown");
            }
            return 0;
        }
        else if (result == CAN_NOMSG)
        {
            if (printFlag)
            {
                Serial.println("no massage");
            }
            return 0;
        }
    }

    if (printFlag)
    {
        Serial.println("read massage fail");
    }
    return 0;
#endif
}

void Inverter::checkBuf(unsigned char *buf)
{
    for (int i = 0; i < 8; i++)
    {
        Serial.print("buf");
        Serial.print(i);
        Serial.print(" = ");

        for (int j = 7; j >= 0; j--)
        {
            unsigned char bit = (*(buf + i) & (0x01 << j)) >> j;
            Serial.print(bit);
        }

        Serial.println();
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
        Serial.println("----------EVECU1Massage----------");
        break;

    case MG_ECU1_ID:
        for (int i = 0; i < 8; i++)
        {
            buf[i] = mgecu1->getMsgByte(i);
        }
        Serial.println("----------MGECU1Massage----------");
        break;

    case MG_ECU2_ID:
        for (int i = 0; i < 8; i++)
        {
            buf[i] = mgecu2->getMsgByte(i);
        }
        Serial.println("----------MGECU2Massage----------");
        break;

    default:
        Serial.print(id);
        Serial.println(" is invalid ID");
        return;
        break;
    }

    checkBuf(buf);

    Serial.println("---------------------------------");
    Serial.println();
}

void Inverter::checkMsg(unsigned long id)
{
    switch (id)
    {
    case EV_ECU1_ID:
        Serial.println("----------EVECU1----------");

        Serial.print("ecuEnable = ");
        Serial.println(evecu1->getEcuEnable(), BIN);
        Serial.print("dischargeCommand = ");
        Serial.println(evecu1->getDischargeCommand(), BIN);
        Serial.print("requestTorque = ");
        Serial.println(evecu1->getNormalRequestTorque(), BIN);

        if (evecu1->getEcuEnable())
        {
            Serial.println("MG-ECU Enable");
        }
        else
        {
            Serial.println("MG-ECU Disable");
        }

        if (evecu1->getDischargeCommand())
        {
            Serial.println("Rapid Discharge Command Active");
        }
        else
        {
            Serial.println("Rapid Discharge Command Inactive");
        }

        Serial.print("Torque Request ");
        Serial.println(evecu1->getRequestTorque());

        Serial.println("--------------------------");
        Serial.println();
        break;

    case MG_ECU1_ID:
        Serial.println("----------MGECU1----------");

        Serial.print("shutdownEnable = ");
        Serial.println(mgecu1->getShutdownEnable(), BIN);
        Serial.print("PWM = ");
        Serial.println(mgecu1->getPWM(), BIN);
        Serial.print("WorkingStatus = ");
        Serial.println(mgecu1->getWorkingStatus(), BIN);
        Serial.print("motorSpeed = ");
        Serial.println(mgecu1->getNormalMotorSpeed(), BIN);
        Serial.print("motorPhaseCurrent = ");
        Serial.println(mgecu1->getNormalMotorPhaseCurrent(), BIN);
        Serial.print("inputDCVoltage = ");
        Serial.println(mgecu1->getInputDCVoltage(), BIN);
        Serial.print("failureStatus = ");
        Serial.println(mgecu1->getFailureStatus(), BIN);

        if (mgecu1->getShutdownEnable())
        {
            Serial.println("Shutdown Enable");
        }
        else
        {
            Serial.println("Shutdown Not Enable");
        }

        Serial.print("PWM ");
        switch (mgecu1->getPWM())
        {
        case PWM_SHORT_CIRCUIT:
            Serial.println("Short Circuit");
            break;

        case PWM_FREE_WHEEL:
            Serial.println("Free Wheel");
            break;

        case PWM_RUN:
            Serial.println("PWM Run");
            break;

        default:
            Serial.println("error");
            break;
        }

        Serial.print("Working Status ");
        switch (mgecu1->getWorkingStatus())
        {
        case WORKING_INIT:
            Serial.println("init");
            break;

        case WORKING_PRECHARGE:
            Serial.println("precharge");
            break;

        case WORKING_STANDBY:
            Serial.println("standby");
            break;

        case WORKING_TORQUE_CONTROL:
            Serial.println("torque control");
            break;

        case WORKING_RAPID_DISCHARGE:
            Serial.println("rapid discharge");
            break;

        default:
            Serial.println("error");
            break;
        }

        Serial.print("Motor Speed ");
        Serial.println(mgecu1->getMotorSpeed());

        Serial.print("Motor Phase Current ");
        Serial.println(mgecu1->getMotorPhaseCurrent());

        Serial.print("Input DC Voltage ");
        Serial.println(mgecu1->getInputDCVoltage());

        Serial.print("Failure Status ");
        switch (mgecu1->getFailureStatus())
        {
        case FAILURE_NO_ERROR:
            Serial.println("No Error");
            break;

        case FAILURE_DERATING:
            Serial.println("Derating");
            break;

        case FAILURE_WARNING:
            Serial.println("Warning");
            break;

        case FAILURE_ERROR:
            Serial.println("Error");
            break;

        case FAILURE_CRITICAL_ERROR:
            Serial.println("Critical Error");

        default:
            Serial.println("error");
            break;
        }

        Serial.println("--------------------------");
        Serial.println();
        break;

    case MG_ECU2_ID:
        Serial.println("----------MGECU2----------");

        Serial.print("inverterTemp = ");
        Serial.println(mgecu2->getNormalInverterTemp(), BIN);
        Serial.print("maxMotorTorque = ");
        Serial.println(mgecu2->getNormalMaxAvailableMotorTorque(), BIN);
        Serial.print("maxGenerateTorque = ");
        Serial.println(mgecu2->getNormalMaxAvailableGenerateTorque(), BIN);
        Serial.print("motorTemp = ");
        Serial.println(mgecu2->getNormalMotorTemp(), BIN);

        Serial.print("Inverter Temperature ");
        Serial.println(mgecu2->getInverterTemp());

        Serial.print("Maximum Available Motoring Torque ");
        Serial.println(mgecu2->getMaxAvailableMotorTorque());

        Serial.print("Maximum Available Generating Torque ");
        Serial.println(mgecu2->getMaxAvailableGenerateTorque());

        Serial.print("Motor Temperature ");
        Serial.println(mgecu2->getMotorTemp());

        Serial.println("--------------------------");
        Serial.println();
        break;

    default:
        break;
    }
}