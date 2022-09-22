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

#ifndef FAST_ADDR_DATA_H
#define FAST_ADDR_DATA_H

extern uint8_t memory6502[];

static inline void dataBusInput(void)
{
    GPIO7_GDIR = GPIO7_GDIR & 0b11001111111100001110011111111111;
}

static inline void dataBusInit(void)
{
    pinMode(GPIO_D0, INPUT);
    pinMode(GPIO_D1, INPUT);
    pinMode(GPIO_D2, INPUT);
    pinMode(GPIO_D3, INPUT);
    pinMode(GPIO_D4, INPUT);
    pinMode(GPIO_D5, INPUT);
    pinMode(GPIO_D6, INPUT);
    pinMode(GPIO_D7, INPUT);
}

static inline void dataBusOutput(void)
{
    GPIO7_GDIR = GPIO7_GDIR | 0b00110000000011110001100000000000;
}

static inline uint8_t dataBusRead(void)
{
    uint32_t data = GPIO7_DR;
    return
    (
        ((data >> 11) & 0b00000011) |
        ((data >> 14) & 0b00111100) |
        ((data >> 22) & 0b11000000)
    );
}

static inline void dataBusWrite(uint8_t data)
{
    GPIO7_DR = 
    (
        (GPIO7_DR & 0b11001111111100001110011111111111)
        | ((data & 0b00000011) << 11)
        | ((data & 0b00111100) << 14)
        | ((data & 0b11000000) << 22)
    );
}

static inline void addrBusInput(void)
{    
    GPIO6_GDIR = GPIO6_GDIR & 0x0000ffff;
}

static inline void addrBusInit(void)
{    
    pinMode(GPIO_A0,  INPUT);
    pinMode(GPIO_A1,  INPUT);
    pinMode(GPIO_A2,  INPUT);
    pinMode(GPIO_A3,  INPUT);
    pinMode(GPIO_A4,  INPUT);
    pinMode(GPIO_A5,  INPUT);
    pinMode(GPIO_A6,  INPUT);
    pinMode(GPIO_A7,  INPUT);
    pinMode(GPIO_A8,  INPUT);
    pinMode(GPIO_A9,  INPUT);
    pinMode(GPIO_A10, INPUT);
    pinMode(GPIO_A11, INPUT);
    pinMode(GPIO_A12, INPUT);
    pinMode(GPIO_A13, INPUT);
    pinMode(GPIO_A14, INPUT);
    pinMode(GPIO_A15, INPUT);
}

static inline void addrBusOutput(void)
{
    GPIO6_GDIR = GPIO6_GDIR | 0xffff0000;
}

static inline uint16_t addrBusRead(void)
{    
    return ((GPIO6_DR >> 16) & 0x0000ffff);    
}

static inline void addrBusWrite(uint16_t addr)
{    
    GPIO6_DR = (GPIO6_DR & 0x0000ffff) | (addr << 16);
}

#endif //FAST_ADDR_DATA_H