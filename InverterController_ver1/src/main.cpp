#include <Arduino.h>
#include "Inverter.hpp"

namespace MGECU1 = MG_ECU1;
namespace MGECU2 = MG_ECU2;

void setup()
{
  init_CAN();
}

void loop()
{
}

void checkBuf(void)
{
  Serial.println("----------buf----------");

  for (int i=0; i<8; i++)
  {
    for (int j=7; j>=0; j--)
    {

    }
  }

  Serial.println("-----------------------");
}