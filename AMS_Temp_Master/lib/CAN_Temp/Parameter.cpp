#include "Parameter.hpp"

Parameter::Parameter(short offset, float resolution, short minPhysical, short maxPhysical)
    : offset(offset), resolution(resolution), minPhysical(minPhysical), maxPhysical(maxPhysical) {}

float Parameter::calcPhysical(unsigned short normal)
{
    return (normal * resolution) + offset;
}

unsigned short Parameter::calcNormal(float physicalValue)
{
    return static_cast<unsigned short>((physicalValue - offset) / resolution);
}
