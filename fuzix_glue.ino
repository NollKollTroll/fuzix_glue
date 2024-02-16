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

#define DEBUG_IO 0
#define DEBUG_MEM 0
#define DEBUG_STEP 0
#define DEBUG_SPEED 1

#define RAM_SIZE_KIB          512
#define RAM_SIZE              (RAM_SIZE_KIB * 1024)
#define NR_OF_BANKS           4
#define BANK_SIZE_KIB         16
#define BANK_SIZE             (BANK_SIZE_KIB * 1024)
#define NR_OF_BLOCKS          (RAM_SIZE_KIB / BANK_SIZE_KIB)
#define CPU_FREQUENCY         2000000 //Hz, 65C02
#define FRAME_RATE            60 //Hz
#define FRAME_TIME_US         (1000000 / FRAME_RATE) //microseconds
#define IRQ_TIMER_FREQ        900 //Hz, multiple of 50 & 60Hz
#define SerPC0                Serial
#define SerPC1                SerialUSB1
#define SerDebug              SerialUSB2

EXTMEM uint8_t ram6502[RAM_SIZE]; //requires soldered qspi ram
uint8_t bankReg[4];
uint32_t frameTimer;
uint32_t frameCounter;

#include <SD.h>
#include <SPI.h>
#include "portdefs.h"
#include "pindefs.h"
#include "fast_addr_data.h"
#include "timer.h"
#include "fileSystem.h"
#include "fuzix_kernel.h"
#include "trace_debug.h"

uint8_t cpuData;
uint16_t cpuAddr;

static inline void cpuTick(void)
{
    // entering CLK phase low
    // lots of delay already because of looping
    // but the low CLK must be at least tPWL long
    delayNanoseconds(63); // tPWL >= 63ns @3.3V
    // begin CLK high phase
    digitalWriteFast(GPIO_CLK, 1);
    // get 65C02 addr
    cpuAddr = addrBusRead();
    if (digitalReadFast(GPIO_RW) == 1)
    {   // 65C02 read operation
        if (cpuAddr >= 0xFF00)
        {   // top page memory read, no banking
            cpuData = ram6502[cpuAddr];
            dataBusWrite(cpuData);
            dataBusOutput();
            debugMemRead();
            delayNanoseconds(62); // tPWH >= 62ns @3.3V
            digitalWriteFast(GPIO_CLK, 0);
            delayNanoseconds(10); // tDHR >=10ns @3.3V
            dataBusInput();
        }
        if (cpuAddr >= 0xFE00)
        {   // I/O-device read
            switch (cpuAddr)
            {
                // ******** Memory bank registers ********
                case IO_BANK_0:
                    cpuData = bankReg[0];
                    break;
                case IO_BANK_1:
                    cpuData = bankReg[1];
                    break;
                case IO_BANK_2:
                    cpuData = bankReg[2];
                    break;
                case IO_BANK_3:
                    cpuData = bankReg[3];
                    break;
                // ******** Serial port registers ********
                case IO_SERIAL_0_FLAGS:
                    cpuData = 0;
                    if (SerPC0.available()) 
                    {
                        cpuData |= 64;
                    }
                    if (!SerPC0.availableForWrite()) 
                    {
                        cpuData |= 128;
                    }
                    break;
                case IO_SERIAL_0_IN:
                    if (SerPC0.available()) 
                    {
                        cpuData = SerPC0.read();
                    }
                    else
                    {
                        cpuData = 0;
                    }
                    break;
                case IO_SERIAL_1_FLAGS:
                    cpuData = 0;
                    if (SerPC1.available()) 
                    {
                        cpuData |= 64;
                    }
                    if (!SerPC1.availableForWrite()) 
                    {
                        cpuData |= 128;
                    }
                    break;
                case IO_SERIAL_1_IN:
                    if (SerPC1.available()) 
                    {
                        cpuData = SerPC1.read();
                    }
                    else 
                    {
                        cpuData = 0;
                    }
                    break;
                // ******** File system registers ********
                //case IO_FS_CMD:
                //case IO_FS_PRM_0:
                //case IO_FS_PRM_1:
                //case IO_FS_PRM_2:
                //case IO_FS_PRM_3:
                case IO_FS_DATA:
                    cpuData = fsDataRead();
                    break;
                case IO_FS_STATUS:
                    cpuData = fsStatus;
                    break;
                // ******** Irq timer registers ********
                case IO_TIMER_TARGET:
                    cpuData = timerTarget;
                    break;
                case IO_TIMER_COUNT:
                    cpuData = timerCountRead();
                    break;
                //case IO_TIMER_RESET:
                //case IO_TIMER_TRIG:
                //case IO_TIMER_PAUSE:
                //case IO_TIMER_CONT:
                // ******** All other I/O registers ********
                default:
                    cpuData = 0xFF;
                    break;
            }
            dataBusWrite(cpuData);
            dataBusOutput();
            debugIORead();
            delayNanoseconds(62); // tPWH >= 62ns @3.3V
            digitalWriteFast(GPIO_CLK, 0);    
            delayNanoseconds(10); // tDHR >=10ns @3.3V
            dataBusInput();
        }
        else
        {   // banked memory read
            cpuData = banked6502Read(cpuAddr);
            dataBusWrite(cpuData);
            dataBusOutput();
            debugMemRead();
            delayNanoseconds(62); // tPWH >= 62ns @3.3V
            digitalWriteFast(GPIO_CLK, 0);    
            delayNanoseconds(10); // tDHR >=10ns @3.3V
            dataBusInput();
       }
    } 
    else
    {   // 65C02 write operation
        if (cpuAddr >= 0xFF00)
        {   // top page memory write, no banking
            delayNanoseconds(40); // tMDS >= 40ns @3.3V
            cpuData = dataBusRead();
            ram6502[cpuAddr] = cpuData;
            debugMemWrite();
            delayNanoseconds(62 - 40); // tPWH >= 62ns @3.3V
            digitalWriteFast(GPIO_CLK, 0);
            delayNanoseconds(10); // tDHW >=10ns @3.3V
        }
        if (cpuAddr >= 0xFE00)
        {   // I/O-device write
            delayNanoseconds(40); // tMDS >= 40ns @3.3V
            cpuData = dataBusRead();
            switch (cpuAddr)
            {
                // ******** Memory bank registers ********
                case IO_BANK_0:
                    bankReg[0] = cpuData;
                    break;
                case IO_BANK_1:
                    bankReg[1] = cpuData;
                    break;
                case IO_BANK_2:
                    bankReg[2] = cpuData;
                    break;
                case IO_BANK_3:
                    bankReg[3] = cpuData;
                    break;
                // ******** Serial port registers ********
                case IO_SERIAL_0_OUT:
                    SerPC0.write(cpuData);
                    break;
                case IO_SERIAL_1_OUT:
                    SerPC1.write(cpuData);
                    break;
                // ******** File system registers ********
                case IO_FS_CMD:
                    fsCmdWrite(cpuData);
                    break;
                case IO_FS_PRM_0:
                    fsPrm0 = cpuData;
                    break;
                case IO_FS_PRM_1:
                    fsPrm1 = cpuData;
                    break;
                case IO_FS_PRM_2:
                    fsPrm2 = cpuData;
                    break;
                case IO_FS_PRM_3:
                    fsPrm3 = cpuData;
                    break;
                case IO_FS_DATA:
                    fsDataWrite(cpuData);
                    break;
                //case IO_FS_STATUS:
                // ******** Irq timer registers ********
                case IO_TIMER_TARGET:
                    timerWrite(cpuData);
                    digitalWriteFast(GPIO_IRQ, timerIrqPin);
                    break;
                //case IO_TIMER_COUNT:
                case IO_TIMER_RESET:
                    timerReset();
                    digitalWriteFast(GPIO_IRQ, timerIrqPin);
                    break;
                case IO_TIMER_TRIG:
                    timerTrig();
                    digitalWriteFast(GPIO_IRQ, timerIrqPin);
                    break;
                case IO_TIMER_PAUSE:
                    timerPause();
                    break;
                case IO_TIMER_CONT:
                    timerCont();
                    digitalWriteFast(GPIO_IRQ, timerIrqPin);
                    break;
                // ******** All other I/O registers ********
                default: 
                    break;
            }
            debugIOWrite();
            delayNanoseconds(62 - 40); // tPWH >= 62ns @3.3V
            digitalWriteFast(GPIO_CLK, 0);
            delayNanoseconds(10); // tDHW >=10ns @3.3V
        }
        else
        {   // banked memory write
            delayNanoseconds(40); // tMDS >= 40ns @3.3V
            cpuData = dataBusRead();
            banked6502Write(cpuAddr, cpuData);
            debugMemWrite();
            delayNanoseconds(62 - 40); // tPWH >= 62ns @3.3V
            digitalWriteFast(GPIO_CLK, 0);
            delayNanoseconds(10); // tDHW >=10ns @3.3V
        }
    }
    debugStep();
}

static inline void cpuReset(void)
{
    addrBusInit();
    dataBusInit();
    
    pinMode(GPIO_CLK,      OUTPUT);
    pinMode(GPIO_RST,      OUTPUT);
    pinMode(GPIO_RW,       INPUT);
    pinMode(GPIO_IRQ,      OUTPUT);
    pinMode(GPIO_NMI,      OUTPUT);
    pinMode(GPIO_BE,       OUTPUT);    
    pinMode(GPIO_SYNC,     INPUT);
    
    digitalWriteFast(GPIO_RST, 0);
    digitalWriteFast(GPIO_IRQ, 1);
    digitalWriteFast(GPIO_NMI, 1);
    digitalWriteFast(GPIO_BE,  1);

    for(int i = 0; i < 32; i++)
    {
        digitalWriteFast(GPIO_CLK, 1);
        delayMicroseconds(1);
        digitalWriteFast(GPIO_CLK, 0);
        delayMicroseconds(1);
    }

    digitalWriteFast(GPIO_RST, 1);
}

void setup()
{
    cpuReset();

    SerPC0.begin(115200);
    SerPC1.begin(115200);
    SerDebug.begin(115200);
    uint32_t startTime = millis();
    while (!SerDebug) // wait for SerDebug connection...
    {   // ...or 2.5s timeout
        if (micros() >= (startTime + 2500))
        {
            break;
        }
    }
    delay(10);
    SerDebug.println("\n\n\n\n###\n\rStarting Teensy glue logic...");

    SerDebug.println("Putting Fuzix code into ram");
    // load Fuzix kernel, skipping ZP & stack
    memcpy(&ram6502[512], &fuzix_kernel[512], sizeof(fuzix_kernel) - 512);
    
    // init bank registers
    bankReg[0] = 0;
    bankReg[1] = 1;
    bankReg[2] = 2;
    bankReg[3] = 3;

    timerInit();
    
    fsInit();
    
    SerDebug.println("Teensy started");
    #if (DEBUG_SPEED == 1)
    frameCounter = 0;
    frameTimeAdded = 0;
    frameTimeMin = INT32_MAX;
    frameTimeMax = INT32_MIN;
    #endif
    frameTimer = micros() + FRAME_TIME_US;
}
    
void loop() 
{
    // sync with next frame start
    while (micros() < frameTimer)
    {
    }
    
    // run cpu for a whole frame
    for(int i = 0; i < (IRQ_TIMER_FREQ / FRAME_RATE); i++)
    {   // run cpu for a timer-tick
        for(int j = 0; j < (CPU_FREQUENCY / IRQ_TIMER_FREQ); j++)
        {
            cpuTick();
        }
        timerTick();
        digitalWriteFast(GPIO_IRQ, timerIrqPin);
    }

    #if (DEBUG_SPEED == 1)
    // statistics
    frameTimeElapsed = micros() - frameTimer;
    frameTimeAdded += frameTimeElapsed;
    frameTimeMin = min(frameTimeElapsed, frameTimeMin);
    frameTimeMax = max(frameTimeElapsed, frameTimeMax);
    if ((frameCounter % FRAME_RATE) == 0)
    {   // once per second print average/min/max cpu-load
        SerDebug.print((frameTimeAdded / FRAME_RATE) * 100.0 / FRAME_TIME_US);
        SerDebug.print("% ");
        SerDebug.print(frameTimeMin * 100.0 / FRAME_TIME_US);
        SerDebug.print("% ");
        SerDebug.print(frameTimeMax * 100.0 / FRAME_TIME_US);
        SerDebug.println("%");
        frameTimeAdded = 0;
        frameTimeMin = INT32_MAX;
        frameTimeMax = INT32_MIN;
    }
    #endif

    frameTimer += FRAME_TIME_US;
    frameCounter++;
}
