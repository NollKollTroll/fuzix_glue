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

#ifndef PINDEFS_H
#define PINDEFS_H

#define GPIO_D0       9   // 65C02 pin 33
#define GPIO_D1       32  // 65C02 pin 32
#define GPIO_D2       8   // 65C02 pin 31
#define GPIO_D3       7   // 65C02 pin 30
#define GPIO_D4       36  // 65C02 pin 29
#define GPIO_D5       37  // 65C02 pin 28
#define GPIO_D6       35  // 65C02 pin 27
#define GPIO_D7       34  // 65C02 pin 26

#define GPIO_A0       19  // 65C02 pin 09
#define GPIO_A1       18  // 65C02 pin 10
#define GPIO_A2       14  // 65C02 pin 11
#define GPIO_A3       15  // 65C02 pin 12
#define GPIO_A4       40  // 65C02 pin 13
#define GPIO_A5       41  // 65C02 pin 14
#define GPIO_A6       17  // 65C02 pin 15
#define GPIO_A7       16  // 65C02 pin 16
#define GPIO_A8       22  // 65C02 pin 17
#define GPIO_A9       23  // 65C02 pin 18
#define GPIO_A10      20  // 65C02 pin 19
#define GPIO_A11      21  // 65C02 pin 20
#define GPIO_A12      38  // 65C02 pin 22
#define GPIO_A13      39  // 65C02 pin 23
#define GPIO_A14      26  // 65C02 pin 24
#define GPIO_A15      27  // 65C02 pin 25

#define GPIO_CLK      2   // 65C02 pin 37
#define GPIO_RST      3   // 65C02 pin 40
#define GPIO_RW       4   // 65C02 pin 34
#define GPIO_NMI      5   // 65C02 pin 06
#define GPIO_SYNC     28  // 65C02 pin 07
#define GPIO_BE       29  // 65C02 pin 36
#define GPIO_IRQ      33  // 65C02 pin 04

#define SD_CS         BUILTIN_SDCARD

#endif //PINDEFS_H                                  