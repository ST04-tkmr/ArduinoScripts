#include "CAN_Temp.hpp"

mcp2515_can CAN(SPI_CS_PIN); // Set CS pin

CAN_Temp_MSG::CAN_Temp_MSG()
    : msg{0, 0, 0, 0, 0, 0, 0, 0}
{
}

CAN_Temp::CAN_Temp(const unsigned long id)
    : id(id)
{
    msg = new CAN_Temp_MSG();
    tempPara = new Parameter(-25, 0.5f, -25, 100);
}

void CAN_Temp::init(void)
{
    while (CAN_OK != CAN.begin(CAN_500KBPS))
    {
        SERIAL_PORT_MONITOR.println("CAN init fail, retry...");
        delay(100);
    }
    SERIAL_PORT_MONITOR.println("CAN init OK!");
}

unsigned char CAN_Temp::sendTempMsg(unsigned char printFlag)
{
    unsigned char buf[8];

    for (int i = 0; i < 8; i++)
    {
        buf[i] = msg->msg[i];
    }

    unsigned char result = CAN.sendMsgBuf(id, 0, 8, buf);

    if (printFlag)
    {
        switch (result)
        {
        case CAN_OK:
            SERIAL_PORT_MONITOR.println("send message");
            SERIAL_PORT_MONITOR.println("----------Massage----------");
            checkBuf(buf);
            SERIAL_PORT_MONITOR.println("---------------------------");
            SERIAL_PORT_MONITOR.println();
            break;

        case CAN_GETTXBFTIMEOUT:
            SERIAL_PORT_MONITOR.println("get TXBF time out");
            SERIAL_PORT_MONITOR.println();
            break;

        case CAN_SENDMSGTIMEOUT:
            SERIAL_PORT_MONITOR.println("send MSG time out");
            SERIAL_PORT_MONITOR.println();
            break;

        default:
            SERIAL_PORT_MONITOR.println("send MSG fail");
            SERIAL_PORT_MONITOR.println();
            break;
        }
    }

    return result;
}

void CAN_Temp::checkBuf(unsigned char *buf)
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