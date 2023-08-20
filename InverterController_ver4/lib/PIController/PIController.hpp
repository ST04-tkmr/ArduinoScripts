#ifndef _PICONTROLLER_H_
#define _PICONTROLLER_H_

class PIDController
{
private:
    short currentOutput;    // 現在の出力
    short currentTarget;    // 現在の目標値
    short currentErr;       // 現在の誤差
    short pastErr;          // 過去の誤差


public:
    PIDController();

    void init(short output, short target);

    void setCurrentOutput(short output);

};

#endif