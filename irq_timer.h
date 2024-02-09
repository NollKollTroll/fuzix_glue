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

#ifndef IRQ_TIMER_H
#define IRQ_TIMER_H

int16_t irqTimerTicks;
uint8_t irqTimerTarget;

enum irqTimerState_e 
{
    RUNNING,
    TRIGGERED,
    PAUSED
};

irqTimerState_e irqTimerState;

static inline void irqTimerInit()
{
    irqTimerState = PAUSED;
    irqTimerTicks = 0;
    irqTimerTarget = UINT8_MAX;
}

static inline void irqTimerTick()
{
    if (irqTimerState != PAUSED) 
    {
        irqTimerTicks++;
    }
    if (irqTimerTicks >= irqTimerTarget) 
    {
        irqTimerTicks -= irqTimerTarget;
        irqTimerState = TRIGGERED;
        digitalWriteFast(GPIO_IRQ, 0);   
    }
}

static inline void irqTimerWrite(uint8_t cpuData)
{
    irqTimerTarget = cpuData;
    irqTimerTicks = 0;
    irqTimerState = RUNNING;                    
    digitalWriteFast(GPIO_IRQ, 1);    
}    
    
static inline void irqTimerReset()
{  
    // reset the timer
    irqTimerTicks = 0;
    irqTimerState = RUNNING;
    digitalWriteFast(GPIO_IRQ, 1);
}

static inline void irqTimerTrig()
{
    // trigger the timer
    irqTimerTicks = 0;
    irqTimerState = TRIGGERED;
    digitalWriteFast(GPIO_IRQ, 0); 
}

static inline void irqTimerPause()
{
    irqTimerState = PAUSED;
}

static inline void irqTimerCont()
{
    irqTimerState = RUNNING;
    digitalWriteFast(GPIO_IRQ, 1);    
}

static inline uint8_t irqTimerCountRead()
{
    return(irqTimerTicks);
}

#endif //IRQ_TIMER_H