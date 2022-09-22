/*
Written in 2022 by Adam Klotblixt (adam.klotblixt@gmail.com)

To the extent possible under law, the author have dedicated all
copyright and related and neighboring rights to this software to the
public domain worldwide.
This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication
along with this software. If not, see
<http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#define DEBUG_SPEED 0
#define DEBUG_IO 1
#define TRACE_CLK 0

#define RAM_SIZE_KIB          1024 //requires soldered qspi ram
#define RAM_SIZE              (RAM_SIZE_KIB * 1024)
#define BANK_SIZE_KIB         16
#define BANK_SIZE             (BANK_SIZE_KIB * 1024)
#define NR_OF_BLOCKS          (RAM_SIZE_KIB / BANK_SIZE_KIB)
#define NR_OF_BANKS           (64 / BANK_SIZE_KIB)
#define CPU_FREQUENCY         2000000 //Hz, 65C02
#define FRAME_RATE            60 //Hz
#define FRAME_TIME_US         (1000000 / FRAME_RATE) //microseconds
#define IRQ_TIMER_FREQ        900 //Hz, multiple of 50 & 60Hz
#define SerPC0                Serial
#define SerPC1                SerialUSB1
#define SerDebug              SerialUSB2

#include <SD.h>
#include <SPI.h>
#include "portdefs.h"
#include "pindefs.h"
#include "fast_addr_data.h"
#include "irq_timer.h"
#include "fuzix_kernel.h"

uint32_t cpuTicks;
uint32_t startTime;

#define SD_CS BUILTIN_SDCARD
File fuzixHD;

EXTMEM uint8_t memory6502[RAM_SIZE];
uint8_t io6502[256];

static inline uint8_t io6502Read(uint16_t cpuAddr)
{
    return io6502[cpuAddr & 255];
}

static inline void io6502Write(uint16_t cpuAddr , uint8_t cpuData)
{
    io6502[cpuAddr & 255] = cpuData;
}

inline uint8_t banked6502Read(uint16_t cpuAddr)
{
    uint32_t block = cpuAddr / BANK_SIZE;
    uint32_t base = cpuAddr & (BANK_SIZE - 1);
    return(memory6502[(io6502Read(PORT_BANK_0 + block) * BANK_SIZE) + base]);
}

inline void banked6502Write(uint16_t cpuAddr, uint8_t cpuData)
{
    uint32_t block = cpuAddr / BANK_SIZE;
    uint32_t base = cpuAddr & (BANK_SIZE - 1);
    memory6502[(io6502Read(PORT_BANK_0 + block) * BANK_SIZE) + base] = cpuData;
}

void cpuTick(void)
{ 
    uint8_t cpuData;
    uint16_t cpuAddr;

    // entering CLK phase low
    // first wait for tADS before addr and R/W are stable
    // lots of delay already because of looping
    DELAY_40(); // tADS >= 40ns @3.3V
    // get 65C02 addr
    cpuAddr = addrBusRead();
    //debugHW("CLK 0 0x" + String(addrBusRead(), HEX) + " 0x" + String(dataBusRead(), HEX));
    // begin CLK high phase
    digitalWriteFast(GPIO_CLK, 1);    
    
    if (digitalReadFast(GPIO_RW) == 1)
    // 65C02 read operation
    {
        #if TRACE_CLK == 1
            if (digitalReadFast(GPIO_SYNC) == 1)
            {
                SerDebug.print("op r ");
            }
            else
            {
                SerDebug.print(" r ");
            }
        #endif
        
        // read data from memory or device?        
        if ((cpuAddr & 0xFF00) == 0xFE00)
        {   // I/O-device read
            switch (cpuAddr)
            {
                // ******** Memory bank registers ********
                /* used like default, see below
                case PORT_BANK_0:
                case PORT_BANK_1:
                case PORT_BANK_2:
                case PORT_BANK_3:
                */
                // ******** Serial port registers ********
                case PORT_SERIAL_0_FLAGS: 
                {
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
                }
                case PORT_SERIAL_0_IN: 
                {
                    if (SerPC0.available()) 
                    {
                        cpuData = SerPC0.read();
                    }
                    else
                    {
                        cpuData = 0;
                    }
                    break;
                }
                case PORT_SERIAL_1_FLAGS: 
                {
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
                }
                case PORT_SERIAL_1_IN: 
                {
                    if (SerPC1.available()) 
                    {
                        cpuData = SerPC1.read();
                    }
                    else 
                    {
                        cpuData = 0;
                    }
                    break;
                }
                // ******** File system registers ********
                /* used like default, see below
                case PORT_FILE_SYSTEM_CMD:
                */                    
                case PORT_FILE_SYSTEM_DATA:
                {
                    if (fuzixHD.available())
                    {
                        cpuData = fuzixHD.read();                        
                        io6502Write(PORT_FILE_SYSTEM_STATUS, (uint8_t)fileSystemStatus_e::OK);
                    #if (DEBUG_IO == 1)
                        //SerDebug.print("FS:READ:OK ");
                        //SerDebug.println(cpuData, HEX);
                        //SerDebug.print("    ");
                    #endif
                    }
                    else
                    {
                        cpuData = 0;
                        io6502Write(PORT_FILE_SYSTEM_STATUS, (uint8_t)fileSystemStatus_e::NOK);
                    #if (DEBUG_IO == 1)
                        SerDebug.println("FS:READ:NOK");
                    #endif
                    }
                    break;
                }
                case PORT_FILE_SYSTEM_STATUS: 
                {
                    cpuData = io6502Read(PORT_FILE_SYSTEM_STATUS);
                    break;
                }
                // ******** Irq timer registers ********
                case PORT_IRQ_TIMER_TARGET:
                {
                    cpuData = irqTimerTarget;
                    break;
                }
                case PORT_IRQ_TIMER_COUNT:
                {
                    cpuData = readTimerCount();
                    break;
                }
                /* used like default, see below
                case PORT_IRQ_TIMER_RESET:
                case PORT_IRQ_TIMER_TRIG:
                case PORT_IRQ_TIMER_PAUSE:
                case PORT_IRQ_TIMER_CONT:
                */
                // ******** All other I/O registers ********
                default:
                {
                    cpuData = io6502Read(cpuAddr);
                #if (DEBUG_IO == 1)
                    SerDebug.print("ior ");
                    SerDebug.print(cpuAddr, HEX);
                    SerDebug.print(":");
                    SerDebug.println(cpuData, HEX);
                #endif
                    break;
                }
            }   
            dataBusWrite(cpuData);
            dataBusOutput();
            digitalWriteFast(GPIO_CLK, 0);    
            DELAY_10();
            dataBusInput();
        }
        else
        {   // normal banked memory read
            cpuData = banked6502Read(cpuAddr);
            dataBusWrite(cpuData);
            dataBusOutput();
            digitalWriteFast(GPIO_CLK, 0);    
            DELAY_10();
            dataBusInput();
        }
    } 
    else
    // 65C02 write operation
    {
        #if TRACE_CLK == 1
            SerDebug.print(" w ");
        #endif
        
        // write data to memory or device?            
        if ((cpuAddr & 0xFF00) == 0xFE00)
        {   // I/O-device write
            DELAY_40(); // tMDS >= 40ns @3.3V
            cpuData = dataBusRead();
            // mirror all I/O-data
            io6502Write(cpuAddr, cpuData);
            switch (cpuAddr)
            {
                // ******** Memory bank registers ********
                /* used like default, see below
                case PORT_BANK_0:
                case PORT_BANK_1:
                case PORT_BANK_2:
                case PORT_BANK_3:
                */
                // ******** Serial port registers ********
                case PORT_SERIAL_0_OUT: 
                {
                    SerPC0.write(cpuData);
                    break;
                }
                case PORT_SERIAL_1_OUT: 
                {
                    SerPC1.write(cpuData);
                    break;
                }                    
                // ******** Frame counter & CPU counter ********
                /*
                case PORT_COUNTER_50HZ:
                case PORT_COUNTER_60HZ:
                case PORT_CPU_COUNTER_0:
                case PORT_CPU_COUNTER_1:
                case PORT_CPU_COUNTER_2:
                case PORT_CPU_COUNTER_3:
                */
                // ******** File system registers ********
                case PORT_FILE_SYSTEM_CMD:
                {
                    if (cpuData == (uint8_t)fileSystemCmd_e::SELECT) 
                    {
                        if (fuzixHD.available())
                        {
                            io6502Write(PORT_FILE_SYSTEM_STATUS, (uint8_t)fileSystemStatus_e::OK);
                        #if (DEBUG_IO == 1)
                            SerDebug.println("FS:SELECT:OK");
                        #endif
                            
                        }
                        else
                        {
                            io6502Write(PORT_FILE_SYSTEM_STATUS, (uint8_t)fileSystemStatus_e::NOK);
                        #if (DEBUG_IO == 1)
                            SerDebug.println("FS:SELECT:NOK_NOT_AVAIL");
                        #endif
                        }
                    }
                    else if (cpuData == (uint8_t)fileSystemCmd_e::SEEK) 
                    {
                        if (fuzixHD.available())
                        {
                            uint32_t position = 
                                (
                                    (io6502Read(PORT_FILE_SYSTEM_PRM_1) << 8) + 
                                    io6502Read(PORT_FILE_SYSTEM_PRM_0)
                                ) * FILE_SYSTEM_BLOCK_SIZE;
                            bool result = fuzixHD.seek(position);
                            if (result)
                            {
                                io6502Write(PORT_FILE_SYSTEM_STATUS, (uint8_t)fileSystemStatus_e::OK);
                            #if (DEBUG_IO == 1)
                                SerDebug.print("FS:SEEK:OK ");
                                SerDebug.println(position);
                            #endif
                            }
                            else
                            {
                                io6502Write(PORT_FILE_SYSTEM_STATUS, (uint8_t)fileSystemStatus_e::NOK);
                            #if (DEBUG_IO == 1)
                                SerDebug.println("FS:SEEK:NOK");
                            #endif
                            }
                        }
                        else
                        {
                            io6502Write(PORT_FILE_SYSTEM_STATUS, (uint8_t)fileSystemStatus_e::NOK);
                        #if (DEBUG_IO == 1)
                            SerDebug.println("FS:SEEK:NOK_NOT_AVAIL");
                        #endif
                        }
                    }
                    break;
                }
                case PORT_FILE_SYSTEM_DATA:
                {
                    if (fuzixHD.available())
                    {
                        bool result = fuzixHD.write(cpuData);
                        if (result)
                        {
                            io6502Write(PORT_FILE_SYSTEM_STATUS, (uint8_t)fileSystemStatus_e::OK);
                        }
                        else
                        {
                            io6502Write(PORT_FILE_SYSTEM_STATUS, (uint8_t)fileSystemStatus_e::NOK);
                        }
                    }
                    else
                    {
                        io6502Write(PORT_FILE_SYSTEM_STATUS, (uint8_t)fileSystemStatus_e::NOK);
                    }
                    break;
                }
                // ******** Irq timer registers ********
                case PORT_IRQ_TIMER_TARGET:
                {   // write target
                    irqTimerWrite(cpuData);
                    break;
                }
                /*
                case PORT_IRQ_TIMER_COUNT:
                */
                case PORT_IRQ_TIMER_RESET:
                {   // reset the timer
                    irqTimerReset();
                    break;
                }
                case PORT_IRQ_TIMER_TRIG:
                {   // trigger the timer
                    irqTimerTrig();
                    break;
                }
                case PORT_IRQ_TIMER_PAUSE:
                {   // pause the timer
                    irqTimerPause();
                    break;
                }
                case PORT_IRQ_TIMER_CONT:
                {   // unpause the timer
                    irqTimerCont();
                    break;
                }
                // ******** All other I/O registers ********
                default: 
                {         
                #if 0//(DEBUG_IO == 1)
                    SerDebug.print("iow ");
                    SerDebug.print(cpuAddr, HEX);
                    SerDebug.print(":");
                    SerDebug.println(cpuData, HEX);
                #endif
                    break;
                }
            }
            digitalWriteFast(GPIO_CLK, 0);
            DELAY_10();
        }
        else
        {   // normal banked memory write
            DELAY_40(); // tMDS >= 40ns @3.3V
            cpuData = dataBusRead();
            banked6502Write(cpuAddr, cpuData);
            digitalWriteFast(GPIO_CLK, 0);
            DELAY_10();
        }
    }
    
    #if TRACE_CLK == 1
        char buf[9];
        sprintf(buf, "%04X : %02X", cpuAddr, cpuData);
        SerDebug.println(buf);
        while (!SerDebug.available());
        SerDebug.read();
    #endif
    cpuTicks++;
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
    {              
        // ...or 2.5s timeout
        if (micros() >= (startTime + 2500))
        {
            break;
        }
    }
    delay(10);
    SerDebug.println("\n########\n\rStarting Teensy glue logic...");
        
    // load Fuzix kernel, starting @1KiB
    memcpy(&memory6502[1024], &fuzix_kernel, sizeof(fuzix_kernel));
    // load it @512+1KiB as well
    memcpy(&memory6502[(512 * 1024) + 1024], &fuzix_kernel, sizeof(fuzix_kernel));
    
    // init bank registers
    io6502Write(PORT_BANK_0, 32);
    io6502Write(PORT_BANK_1, 33);
    io6502Write(PORT_BANK_2, 34);
    io6502Write(PORT_BANK_3, 35);

    cpuTicks = 0;
    irqTimerInit();
    
    // init filesystem
    if (!SD.begin(SD_CS)) 
    {
        SerDebug.println("SD-card missing?");
    }
    else
    {
        fuzixHD = SD.open("filesys.img", FILE_READ);
        if (fuzixHD.available())
        {
            SerDebug.println("filesys.img opened");
        }
        else
        {
            SerDebug.println("filesys.img failed to open");
        }        
    }
    
    SerDebug.println("Teensy started");
    startTime = micros();
}
    
void loop() 
{
    #if DEBUG_SPEED == 1
        uint32_t cpuTimeBegin = micros();
    #endif
    
    for(int i = 0; i < (IRQ_TIMER_FREQ / FRAME_RATE); i++)
    {
        for(int j = 0; j < (CPU_FREQUENCY / IRQ_TIMER_FREQ); j++)
        {
            cpuTick();
        }
        irqTimerTick();
    }
    
    #if DEBUG_SPEED == 1
        int32_t cpuTime = micros() - cpuTimeBegin;
        SerDebug.println(cpuTime);
    #endif
    
    while (micros() < (startTime + FRAME_TIME_US));
    startTime += FRAME_TIME_US;
}

/* 

*** count CPU-cycles ***
uint32_t countCycles = ARM_DWT_CYCCNT;
...
countCycles = ARM_DWT_CYCCNT - countCycles - 2;
SerDebug.println(countCycles);

minicom -D /dev/ttyACM0
minicom -D /dev/ttyACM1
minicom -D /dev/ttyACM2
arduino --upload digital_test.ino
arduino --verify digital_test.ino
*/
