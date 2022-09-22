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

#ifndef PORTDEFS_H
#define PORTDEFS_H

// Port definitions

// ******** Bank registers ********
#define PORT_BANK_0             0xFE00
#define PORT_BANK_1             0xFE01
#define PORT_BANK_2             0xFE02
#define PORT_BANK_3             0xFE03
        
// ******** Serial ********
#define PORT_SERIAL_0_FLAGS     0xFE10 // bit 7: out-buffer full, bit 6: in-data available
#define PORT_SERIAL_0_IN        0xFE11
#define PORT_SERIAL_0_OUT       0xFE12
#define PORT_SERIAL_1_FLAGS     0xFE18 // bit 7: out-buffer full, bit 6: in-data available
#define PORT_SERIAL_1_IN        0xFE19
#define PORT_SERIAL_1_OUT       0xFE1A
#define SERIAL_FLAGS_OUT_FULL      128
#define SERIAL_FLAGS_IN_AVAIL       64

// ******** File system ********
#define PORT_FILE_SYSTEM_CMD    0xFE60
#define PORT_FILE_SYSTEM_PRM_0  0xFE61
#define PORT_FILE_SYSTEM_PRM_1  0xFE62
#define PORT_FILE_SYSTEM_DATA   0xFE63
#define PORT_FILE_SYSTEM_STATUS 0xFE64
#define FILE_SYSTEM_BLOCK_SIZE  512
enum class fileSystemCmd_e
{
    SELECT = 0,
    SEEK = 1
};

enum class fileSystemStatus_e
{
    OK = 0,
    NOK = 1
};

// ******** Timer ********
#define PORT_IRQ_TIMER_TARGET   0xFE80  //writing sets the target and enables timer
#define PORT_IRQ_TIMER_COUNT    0xFE81  //current counter value (read only)
#define PORT_IRQ_TIMER_RESET    0xFE82  //(timer = 0) so it can count a full cycle again
#define PORT_IRQ_TIMER_TRIG     0xFE83  //(timer = timer target) so it triggers an irq
#define PORT_IRQ_TIMER_PAUSE    0xFE84  //pause the timer indefinitely
#define PORT_IRQ_TIMER_CONT     0xFE85  //continue the timer after a pause, also acks a triggered timer

#endif //PORTDEFS_H