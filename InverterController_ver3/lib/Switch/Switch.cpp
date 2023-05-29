#include "Switch.hpp"

Switch::Switch()
: swFlag(0)
, lastSW(0)
, SW(0)
, chatt{ 0, 0, 0}
{}

void Switch::updateState(unsigned char readValue)
{
    lastSW = SW;
    SW = (chatt[0] & chatt[1] & chatt[2]);
    chatt[2] = chatt[1];
    chatt[1] = chatt[0];
    chatt[0] = (readValue & 0x01);

    if (lastSW == 0 && SW == 1)
    {
        swFlag = 1;
    }
}

void Switch::resetFlag()
{
    swFlag = 0;
}