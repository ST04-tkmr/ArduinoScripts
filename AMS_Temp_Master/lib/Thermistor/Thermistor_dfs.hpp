#ifndef _THERMISTOR_DFS_H_
#define _THERMISTOR_DFS_H_

const unsigned char ecuNum = 1; // 温度監視ECUの個数
const unsigned char adrs[ecuNum] = {
    0b0000001,
    // 0b0000010,
    // 0b0000100,
    // 0b0001000
};
const unsigned char thmNum = 6; // 1つのマイコンで監視するサーミスタの本数（<= 6）
const float r0 = 10000.0f;      // 25℃の時のサーミスタの抵抗値
const float t0 = 25.0f;         // 基準温度
const float b = 3423.0f;        // B定数
const float rs = 10000.0f;      // サーミスタと直列につなぐ抵抗の値

#endif