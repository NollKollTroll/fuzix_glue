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

#ifndef PORTDEFS_H
#define PORTDEFS_H

// Port definitions

// ******** Bank registers ********
#define IO_BANK_0               0xFE00
#define IO_BANK_1               0xFE01
#define IO_BANK_2               0xFE02
#define IO_BANK_3               0xFE03
        
// ******** Serial ********
#define IO_SERIAL_0_FLAGS       0xFE10 // bit 7: out-buffer full, bit 6: in-data available
#define IO_SERIAL_0_IN          0xFE11
#define IO_SERIAL_0_OUT         0xFE12
#define IO_SERIAL_1_FLAGS       0xFE18 // bit 7: out-buffer full, bit 6: in-data available
#define IO_SERIAL_1_IN          0xFE19
#define IO_SERIAL_1_OUT         0xFE1A
#define SERIAL_FLAGS_OUT_FULL      128
#define SERIAL_FLAGS_IN_AVAIL       64

// ******** File system ********
#define IO_FS_CMD               0xFE60
#define IO_FS_PRM_0             0xFE61
#define IO_FS_PRM_1             0xFE62
#define IO_FS_PRM_2             0xFE63
#define IO_FS_PRM_3             0xFE64
#define IO_FS_DATA              0xFE65
#define IO_FS_STATUS            0xFE66
#define FS_BLOCK_SIZE              512
#define FS_CMD_SELECT                0
#define FS_CMD_SEEK                  1
#define FS_STATUS_OK                 0
#define FS_STATUS_NOK                1

// ******** Timer ********
#define IO_TIMER_TARGET         0xFE80  //writing sets the target and enables timer
#define IO_TIMER_COUNT          0xFE81  //current counter value (read only)
#define IO_TIMER_RESET          0xFE82  //(timer = 0) so it can count a full cycle again
#define IO_TIMER_TRIG           0xFE83  //(timer = timer target) so it triggers an irq
#define IO_TIMER_PAUSE          0xFE84  //pause the timer indefinitely
#define IO_TIMER_CONT           0xFE85  //continue the timer after a pause, also acks a triggered timer

#endif //PORTDEFS_H