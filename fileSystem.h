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

#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <SD.h>

File fuzixHD;
uint8_t fsStatus;
uint8_t fsPrm0;
uint8_t fsPrm1;
uint8_t fsPrm2;
uint8_t fsPrm3;

void fsInit()
{
    // init filesystem
    if (!SD.begin(SD_CS))
    {
        SerDebug.println("SD-card missing?");
        fsStatus = DISK_STATUS_NOK;
    }
    else
    {
        fuzixHD = SD.open("disk.img", FILE_WRITE);
        fuzixHD.seek(0);
        if (fuzixHD.available())
        {
            SerDebug.println("disk.img opened");
            fsStatus = DISK_STATUS_OK;
        }
        else
        {
            SerDebug.println("disk.img failed to open");
            fsStatus = DISK_STATUS_NOK;
        }
    }
}

static inline void fsCmdWrite(uint8_t cpuData)
{
    if (cpuData == DISK_CMD_SELECT)
    {
        if (fuzixHD.available())
        {
            fsStatus = DISK_STATUS_OK;
        #if (DEBUG_IO == 1)
            SerDebug.println("FS:SELECT:OK");
        #endif
        }
        else
        {
            fsStatus = DISK_STATUS_NOK;
        #if (DEBUG_IO == 1)
            SerDebug.println("FS:SELECT:NOK_NOT_AVAIL");
        #endif
        }
    }
    else if (cpuData == DISK_CMD_SEEK)
    {
        if (fuzixHD.available())
        {
            uint32_t position = DISK_BLOCK_SIZE * (fsPrm0 + (fsPrm1 << 8) + (fsPrm2 << 16));
            bool result = fuzixHD.seek(position);
            if (result)
            {
                fsStatus = DISK_STATUS_OK;
            #if (DEBUG_IO == 1)
                SerDebug.print("FS:SEEK:OK ");
                SerDebug.println(position);
            #endif
            }
            else
            {
                fsStatus = DISK_STATUS_NOK;
            #if (DEBUG_IO == 1)
                SerDebug.println("FS:SEEK:NOK");
            #endif
            }
        }
        else
        {
            fsStatus = DISK_STATUS_NOK;
        #if (DEBUG_IO == 1)
            SerDebug.println("FS:SEEK:NOK_NOT_AVAIL");
        #endif
        }
    }
}

static inline uint8_t fsDataRead()
{
    uint8_t cpuData;
    if (fuzixHD.available())
    {
        cpuData = fuzixHD.read();
        fsStatus = DISK_STATUS_OK;
    }
    else
    {
        cpuData = 0;
        fsStatus = DISK_STATUS_NOK;
    }
    return cpuData;
}

static inline void fsDataWrite(uint8_t cpuData)
{
    if (fuzixHD.available())
    {
        bool result = fuzixHD.write(cpuData);
        if (result)
        {
            fsStatus = DISK_STATUS_OK;
        }
        else
        {
            fsStatus = DISK_STATUS_NOK;
        }
    }
    else
    {
        fsStatus = DISK_STATUS_NOK;
    }
}

#endif //FILESYSTEM_H
