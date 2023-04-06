#include "Thermistor.hpp"

Thermistor::Thermistor() {
    val = 0;
    r = 0.0f;
    temp = 0.0f;
}

char Thermistor::setVal(int val) {
    if (0 <= val && val <= 2023) {
        this->val = val;
        r = this->calcR(this->val);
        temp = this->calcTemp(this->r);
        return 1;
    } else {
        return 0;
    }
}

float Thermistor::calcR(int val) {
    const float r = 10000.0f;   //サーミスタと直列につなぐ抵抗の値
    float vd, thmR; //実際の電圧値, サーミスタの抵抗値
    vd = val * 0.0049f; //実際の電圧値に変換
    thmR = (r * vd) / (5 - vd); //サーミスタの抵抗値算出
    return thmR;
}

float Thermistor::calcTemp(float thmR) {
    const float r0 = 10000.0f;  //25℃の時のサーミスタの抵抗値
    const float t0 = 25.0f; //基準温度
    const float b = 3380.0f;    //B定数
    float logOfthmR = static_cast<float>(log(thmR / r0));   //(サーミスタの抵抗値/r0)の常用対数をとる
    float temp = 1 / ((logOfthmR / b) + (1 / (t0 + 273.0f))) - 273.0f;  //抵抗値から温度を算出
    return temp;
}