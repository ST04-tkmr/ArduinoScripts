#ifndef _AGTIMERR4_H_
#define _AGTIMERR4_H_

#include <Arduino.h>
#include "Inverter_dfs.hpp"

#ifdef ARDUINO_UNO_R4

#include <FspTimer.h>

class AGTimerR4
{
private:
    FspTimer fsp_timer;

    static void (*callback_func)();
    static void ourTimerCallback(timer_callback_args_t __attribute((unused)) *p_args);

public:
    void init(double freq_hz, void (*callback)());
    void init(int period_us, void (*callback)());
    void init(int period, timer_source_div_t sd, void (*callback)());
    bool start(void);
    bool stop(void);
};

extern AGTimerR4 AGTimer;

#endif

#endif