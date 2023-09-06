#ifndef _CAN_TEMP_DFS_H_
#define _CAN_TEMP_DFS_H_

const unsigned long ACC_ID = 0x340;
const unsigned long SEG1_ID = 0x341;
const unsigned long SEG2_ID = 0x342;
const unsigned long SEG3_ID = 0x343;
const unsigned long SEG4_ID = 0x344;

enum Type
{
    AVR_TEMP,
    MAX_TEMP,
    MIN_TEMP
};

#endif