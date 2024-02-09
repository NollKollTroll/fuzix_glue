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

void fsInit()
{
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
    fsStatus = FS_STATUS_NOK;
}

static inline void fsCmdWrite(uint8_t cpuData)
{
    if (cpuData == FS_CMD_SELECT)
    {
        if (fuzixHD.available())
        {
            fsStatus = FS_STATUS_OK;
        #if (DEBUG_IO == 1)
            SerDebug.println("FS:SELECT:OK");
        #endif
        }
        else
        {
            fsStatus = FS_STATUS_NOK;
        #if (DEBUG_IO == 1)
            SerDebug.println("FS:SELECT:NOK_NOT_AVAIL");
        #endif
        }
    }
    else if (cpuData == FS_CMD_SEEK)
    {
        if (fuzixHD.available())
        {
            uint32_t position = ((fsPrm1 << 8) + fsPrm0) * FS_BLOCK_SIZE;
            bool result = fuzixHD.seek(position);
            if (result)
            {
                fsStatus = FS_STATUS_OK;
            #if (DEBUG_IO == 1)
                SerDebug.print("FS:SEEK:OK ");
                SerDebug.println(position);
            #endif
            }
            else
            {
                fsStatus = FS_STATUS_NOK;
            #if (DEBUG_IO == 1)
                SerDebug.println("FS:SEEK:NOK");
            #endif
            }
        }
        else
        {
            fsStatus = FS_STATUS_NOK;
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
        fsStatus = FS_STATUS_OK;
    }
    else
    {
        cpuData = 0;
        fsStatus = FS_STATUS_NOK;
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
            fsStatus = FS_STATUS_OK;
        }
        else
        {
            fsStatus = FS_STATUS_NOK;
        }
    }
    else
    {
        fsStatus = FS_STATUS_NOK;
    }
}

#endif //FILESYSTEM_H
