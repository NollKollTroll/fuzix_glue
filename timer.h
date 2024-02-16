/*
Written in 2022-24 by Adam Klotblixt (adam.klotblixt@gmail.com)

To the extent possible under law, the author have dedicated all
copyright and related and neighboring rights to this software to the
public domain worldwide.
This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication
along with this software. If not, see
<http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef TIMER_H
#define TIMER_H

int16_t timerTicks;
uint8_t timerTarget;
bool timerIrqPin;

enum timerState_e
{
    RUNNING,
    TRIGGERED,
    PAUSED
};

timerState_e timerState;

static inline void timerInit()
{
    timerState = PAUSED;
    timerTicks = 0;
    timerTarget = UINT8_MAX;
    timerIrqPin = 1;
}

static inline void timerTick()
{
    if (timerState != PAUSED)
    {
        timerTicks++;
    }
    if (timerTicks >= timerTarget)
    {
        timerTicks -= timerTarget;
        timerState = TRIGGERED;
        timerIrqPin = 0;
    }
}

static inline void timerWrite(uint8_t cpuData)
{
    timerTarget = cpuData;
    timerTicks = 0;
    timerState = RUNNING;
    timerIrqPin = 1;
}

static inline void timerReset()
{
    // reset the timer
    timerTicks = 0;
    timerState = RUNNING;
    timerIrqPin = 1;
}

static inline void timerTrig()
{
    // trigger the timer
    timerTicks = 0;
    timerState = TRIGGERED;
    timerIrqPin = 0;
}

static inline void timerPause()
{
    timerState = PAUSED;
}

static inline void timerCont()
{
    timerState = RUNNING;
    timerIrqPin = 1;
}

static inline uint8_t timerCountRead()
{
    return(timerTicks);
}

#endif //TIMER_H