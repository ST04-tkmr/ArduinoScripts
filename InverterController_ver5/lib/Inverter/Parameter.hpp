#ifndef _PARAMETER_H_
#define _PARAMETER_H_

/**
 * Parameter for changing massage to physical
 * offset, resolution, physical(minPhysical~maxPhysical)
 */
class Parameter
{
private:
    const short offset;
    const float resolution;
    const short minPhysical, maxPhysical;

public:
    Parameter(short offset, float resolution, short minPhysical, short maxPhysical);

    inline short getOffset() { return offset; };
    inline float getResolution() { return resolution; };
    inline short getMinPhysical() { return minPhysical; };
    inline short getMaxPhysical() { return maxPhysical; };

    // Normal ValueからPhysical Valueを計算
    float calcPhysical(unsigned short normal);

    // Physical ValueからNormal Valueを計算
    unsigned short calcNormal(float physicalValue);
};

#endif