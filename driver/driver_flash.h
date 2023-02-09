#ifndef DRIVER_FLASH_H
#define DRIVER_FLASH_H

#include <rtl876x.h>
#include <app_task.h>

#include "mlapi.h"


/* Globals ------------------------------------------------------------------*/
//typedef enum 
//{
//    EM_FLASH_CTRL_INSIDE    = 0,                    //内部FLASH
//    EM_FLASH_CTRL_OUTSIDE   = 1                     //外部FLASH
//}EM_FLASH_CTRL_MODE;

typedef enum
{
    DEVICE_ID,
    MF_DEVICE_ID,
    JEDEC_ID
} Flash_ID_Type;

uint8_t flashReadBuffer(unsigned char *readBuffer, unsigned int address, unsigned int readLength, EM_FLASH_CTRL_MODE mode);
uint8_t flashWriteBuffer(unsigned char *writeBuffer, unsigned int address, unsigned int writeLength, EM_FLASH_CTRL_MODE mode);
uint8_t flashReadBuffer_match(unsigned char *readBuffer, unsigned int address, unsigned int readLength, EM_FLASH_CTRL_MODE mode);

void driver_flash_read_id(Flash_ID_Type vFlashIdType);

#endif
