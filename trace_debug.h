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

#ifndef TRACE_DEBUG_H
#define TRACE_DEBUG_H

extern uint16_t cpuAddr;
extern uint8_t cpuData;

#if (DEBUG_SPEED == 1)
int32_t frameTimeElapsed;
int32_t frameTimeMin;
int32_t frameTimeMax;
int32_t frameTimeAdded;
#endif

static inline void debugAddrData()
{
    char buf[10];
    sprintf(buf, "%04X : %02X", cpuAddr, cpuData);
    SerDebug.println(buf);
}

static inline void debugMemRead()
{
    #if (DEBUG_MEM == 1)
    if (digitalReadFast(GPIO_SYNC) == 1)
    {
        SerDebug.print("*r ");
    }
    else
    {
        SerDebug.print(" r ");
    }
    debugAddrData();
    #endif
}

static inline void debugMemWrite()
{
    #if (DEBUG_MEM == 1)
    SerDebug.print(" w ");
    debugAddrData();
    #endif
}

static inline void debugIORead()
{
    #if (DEBUG_IO == 1)
    SerDebug.print("ior ");
    debugAddrData();
    #endif
}

static inline void debugIOWrite()
{
    #if (DEBUG_IO == 1)
    SerDebug.print("iow ");
    debugAddrData();
    #endif
}

static inline void debugStep()
{
    #if (DEBUG_STEP == 1)
    //wait for any key
    while (!SerDebug.available());
    SerDebug.read();
    #endif
}

#endif //TRACE_DEBUG_H