#include "driver_flash.h"
#include "stdio.h"

#include "driver_spi.h"
#include "driver_delay.h"

#include <trace.h>
#include "driver_delay.h"
#include "flash_device.h"

#include "ftl.h"

#if 0
#define DFLASH_PRINT_INFO0   APP_PRINT_TRACE0
#define DFLASH_PRINT_INFO1   APP_PRINT_TRACE1
#define DFLASH_PRINT_INFO2   APP_PRINT_TRACE2
#define DFLASH_PRINT_INFO3   APP_PRINT_TRACE3
#define DFLASH_PRINT_INFO4   APP_PRINT_TRACE4
#else
#define DFLASH_PRINT_INFO0(...)
#define DFLASH_PRINT_INFO1(...)
#define DFLASH_PRINT_INFO2(...)
#define DFLASH_PRINT_INFO3(...)
#define DFLASH_PRINT_INFO4(...)
#endif

#define SPI_FLASH_WRITE_ENABLE          0x06
#define SPI_FLASH_WRITE_DISABLE         0x04
#define SPI_FLASH_READ_STATUS_REG_1     0x05
#define SPI_FLASH_WRITE_STATUS_REG_1    0x01
#define SPI_FLASH_READ_STATUS_REG_2     0x35
#define SPI_FLASH_WRITE_STATUS_REG_2    0x31
#define SPI_FLASH_PAGE_PROGRAM          0x02
#define SPI_FLASH_SECTOR_ERASE          0x20
#define SPI_FLASH_BLOCK_ERASE_32K       0x52
#define SPI_FLASH_BLOCK_ERASE_64K       0xD8
#define SPI_FLASH_CHIP_ERASE            0xC7
#define SPI_FLASH_POWER_DOWN            0xB9
#define SPI_FLASH_READ_DATA             0x03
#define SPI_FLASH_FAST_READ             0x0B
#define SPI_FLASH_RELEASE_POWER_DOWN    0xAB
#define SPI_FLASH_DEVICE_ID             0xAB
#define SPI_FLASH_MANU_ID               0x90
#define SPI_FLASH_JEDEC_ID              0x9F
#define SPI_FLASH_ENABLE_RESET          0x66
#define SPI_FLASH_RESET                 0x99

#define EWIP_FLAG                       0x01 

#define ExFlash_ReadStatusReg1          (0x05)
#define ExFlash_ReadStatusReg2          (0x35)
#define ExFlash_ReadStatusReg3          (0x15)
#define ExFlash_WriteStatusReg1         (0x01)
#define ExFlash_WriteStatusReg2         (0x31)
#define ExFlash_WriteStatusReg3         (0x11)

#define FLASH_PAGE_SIZE_OUTSIDE         (256)
#define FLASH_SECTOR_SIZE_OUTSIDE       (4096)

#define FLASH_RETRY_CNT                 (3)
#define FLASH_DELAY_CNT                 (200)

extern uint8_t spiFlashTransfer(unsigned char *txBuf, unsigned char *rxBuf, unsigned int len, bool leave_cs_asserted);

/***************************************************************
****************FLASH 芯片信息检测接口**************************
****************************************************************/
void driver_flash_read_id(Flash_ID_Type vFlashIdType)
{
    uint8_t send_buf[6] = {SPI_FLASH_JEDEC_ID, 0, 0, 0, 0, 0};
    uint8_t recv_buf[6];
    
    uint8_t recv_len = 3;
    
    switch (vFlashIdType)
    {
        case DEVICE_ID:
            send_buf[0] = SPI_FLASH_DEVICE_ID;
            recv_len = 4 + 1;
            break;
        case MF_DEVICE_ID:
            send_buf[0] = SPI_FLASH_MANU_ID;
            recv_len = 5 + 1;
            break;
        case JEDEC_ID:
            send_buf[0] = SPI_FLASH_JEDEC_ID;
            recv_len = 3 + 1;
            break;
        default:
            return;
    }
    
    spiFlashTransfer(send_buf, recv_buf, recv_len, false);
    
    APP_PRINT_TRACE2("[FLASH] FLASH info type:%d, ID: read_data = %b,", vFlashIdType, TRACE_BINARY(2, &recv_buf[4]));
    return;
}

/***************************************************************
****************FLASH 内部寄存器操作接口************************
****************************************************************/
static unsigned char spi_flash_readSR(unsigned char regno)
{
    unsigned char command;
    unsigned char rx_dat;

    switch(regno)
    {
        case 1:
            command=ExFlash_ReadStatusReg1;
            break;
        case 2:
            command=ExFlash_ReadStatusReg2;
            break;
        case 3:
            command=ExFlash_ReadStatusReg3;
            break;
        default:
            command=ExFlash_ReadStatusReg1;
            break;
    }

    spiFlashTransfer(&command, &rx_dat, 1, true);
    command =0xFF;
    spiFlashTransfer(&command, &rx_dat, 1, false);

    return rx_dat;
}

static void spi_flash_writeEnable(void)
{
    uint16_t time_cnt  = FLASH_DELAY_CNT * 100;

    unsigned char commandBuffer[1] = {SPI_FLASH_WRITE_ENABLE};
    unsigned char rx_dat=0;

    while(1)
    {
        spiFlashTransfer(commandBuffer, &rx_dat, sizeof(commandBuffer), false);

        if(spi_flash_readSR(1)&0x02)
        {
            break;
        }

        if(0 >= time_cnt) 
            return;
        
        time_cnt --;
        delay_us_Ex(100);
    }
}

static void spi_flash_writeSR(unsigned char regno)
{
    uint16_t time_cnt  = FLASH_DELAY_CNT ;
    unsigned char command[2]={ExFlash_WriteStatusReg1,0};
    unsigned char rx_dat[2];

    switch(regno)
    {
        case 1:
            command[0] = ExFlash_WriteStatusReg1;
            break;
        case 2:
            command[0] = ExFlash_WriteStatusReg2;
            break;
        case 3:
            command[0] = ExFlash_WriteStatusReg3;
            break;
        default:
            command[0] = ExFlash_WriteStatusReg1;
            break;
    }
    
    while(1)
    {
        spi_flash_writeEnable();
        spiFlashTransfer(command, rx_dat, 2, false);
        delay_ms(10);
        if(spi_flash_readSR(1) == 0)
        {
            break;
        }
        if(0 >= time_cnt) 
            return;
        
        time_cnt --;
    }
}

static unsigned char spi_flash_waitForWriteEnd(void)
{
    unsigned char FLASH_Status = 0;
    unsigned char commandBuffer[1] = {ExFlash_ReadStatusReg1};
    unsigned char rx_dat;
    unsigned int time_cnt = 0;

    spiFlashTransfer(commandBuffer, &rx_dat, sizeof(commandBuffer), true);
    while(time_cnt++ < 5000)
    {
        rx_dat = 0;
        spiFlashTransfer(&rx_dat, &FLASH_Status, 1, true);
        if((FLASH_Status & 0x03) == 0)
        {
            spiFlashTransfer(&rx_dat, &FLASH_Status, 1, true);
            if((FLASH_Status & 0x03) == 0)
            {
                break;
            }
        }
        delay_us_Ex(10);
    }

    commandBuffer[0] = SPI_FLASH_WRITE_DISABLE;
    spiFlashTransfer(commandBuffer, &rx_dat, sizeof(commandBuffer), false);

    return FLASH_Status & 0x03;
}

/***************************************************************
****************FLASH 写操作内部接口****************************
****************************************************************/
static uint8_t spi_flash_read(uint8_t ReadCmd, uint8_t *pBuffer, uint32_t ReadAddr, uint16_t Length)
{
    uint8_t send_buf[5];  
    uint8_t send_len;
    
    if (SPI_FLASH_READ_DATA == ReadCmd)
    {
        send_len = 4;
    }
    else if (SPI_FLASH_FAST_READ == ReadCmd)
    {
        send_len = 5;
    }
    
    send_buf[0] = ReadCmd;
    send_buf[1] = (ReadAddr >> 16) & 0xFF;
    send_buf[2] = (ReadAddr >> 8) & 0xFF;
    send_buf[3] = (ReadAddr) & 0xFF;
    
    spiFlashTransfer(send_buf, NULL, send_len, true);
    spiFlashTransfer(NULL, pBuffer, Length, false);
    
    return 0;
}

static uint8_t spi_flash_read_match(uint8_t ReadCmd, uint8_t *pBuffer, uint32_t ReadAddr, uint16_t Length)
{
    uint8_t send_buf[5];  
    uint8_t send_len;
    
    if (SPI_FLASH_READ_DATA == ReadCmd)
    {
        send_len = 4;
    }
    else if (SPI_FLASH_FAST_READ == ReadCmd)
    {
        send_len = 5;
    }
    
    send_buf[0] = ReadCmd;
    send_buf[1] = (ReadAddr >> 16) & 0xFF;
    send_buf[2] = (ReadAddr >> 8) & 0xFF;
    send_buf[3] = (ReadAddr) & 0xFF;
    
    spiFlashTransfer(send_buf, NULL, send_len, true);
    spiFlashTransfer_match(NULL, pBuffer, Length);
    
    return 0;
}

/***************************************************************
****************FLASH 擦除操作内部接口**************************
****************************************************************/
static uint16_t spi_flash_earse_num(uint32_t address, uint32_t length)
{
	uint16_t erasePageNum;
    uint16_t erasePageSize;  
    
    address = address;                          //用于区分区域，暂时不需使用
    
    erasePageSize = FLASH_SECTOR_SIZE_OUTSIDE;
    
	if(length < erasePageSize)
	{
		erasePageNum = 1;
	}
	else
	{
		erasePageNum = length/erasePageSize;
		if(length%erasePageSize)
		{
			erasePageNum += 1;
		}
	}

	return erasePageNum;
}

static uint8_t spi_flash_earse_sector(unsigned int earseAddress)
{
    uint8_t rv = 0;
    uint8_t retry_cnt = 0;
    uint8_t send_buf[4] = {SPI_FLASH_SECTOR_ERASE, 0, 0, 0};
    uint8_t recv_buf[4];
        
    send_buf[1] = ((earseAddress & 0xFF0000) >> 16);
    send_buf[2] = ((earseAddress & 0x00FF00) >> 8);
    send_buf[3] = ((earseAddress & 0x0000FF) >> 0);
    while(retry_cnt++ < FLASH_RETRY_CNT)
    {
        if(spi_flash_readSR(1) != 0)
        {
            spi_flash_writeSR(1);
        }
        spi_flash_writeEnable();

        spiFlashTransfer(send_buf, recv_buf, sizeof(send_buf), false);
        
        if(spi_flash_waitForWriteEnd() == 0)
        {
            break;
        }
        delay_ms(1);
    }
    
    if(retry_cnt >= FLASH_RETRY_CNT)
    {
        rv = 1;
    }
    
    return rv;
}

static uint8_t spi_flash_earse(uint32_t earseAddress, uint16_t earseNum)
{
    uint16_t i;
    uint8_t rv = 0;
    
    for(i=0; i<earseNum; i++)
    {
        rv = spi_flash_earse_sector(earseAddress + i*FLASH_SECTOR_SIZE_OUTSIDE); 
        if(rv != 0)
        {
            DFLASH_PRINT_INFO1("[FLASH] SPI_FLASH_earse: address = 0x%x,", earseAddress + i*FLASH_SECTOR_SIZE_OUTSIDE);
            break;
        }
    }

    return rv;
}

/***************************************************************
****************FLASH 写操作内部接口****************************
****************************************************************/
static uint8_t spi_flash_write_page(uint8_t *pBuffer, uint32_t writeAddress, uint16_t length)
{
    uint8_t rv = 0;
    uint8_t retry_cnt = 0;
    
    uint8_t send_buf[4] = {0};
    
    send_buf[0] = SPI_FLASH_PAGE_PROGRAM;
    send_buf[1] = (writeAddress >> 16) & 0xFF;
    send_buf[2] = (writeAddress >> 8) & 0xFF;
    send_buf[3] = (writeAddress) & 0xFF;
    
    while(retry_cnt++ < FLASH_RETRY_CNT)
    {
        if(spi_flash_readSR(1)!=0)
        {
            spi_flash_writeSR(1);
        }
        spi_flash_writeEnable();

        spiFlashTransfer(send_buf, NULL, sizeof(send_buf), true);
        spiFlashTransfer(pBuffer, NULL, length, false);

        if(spi_flash_waitForWriteEnd() == 0)
        {
            break;
        }
        delay_ms(1);
    }
    
    if(retry_cnt >= FLASH_RETRY_CNT)
    {
        rv = 1;
    }
    
    return rv;
}

static uint8_t spi_flash_write_buffer(uint8_t *writeBuffer, uint32_t writeAddress, uint16_t length)
{
    unsigned char numOfPage = 0, Addr = 0;
    unsigned int  numOfSingle = 0, count = 0, temp = 0;

    Addr = writeAddress % FLASH_PAGE_SIZE_OUTSIDE;           //计算该地址是某页的第多少个字节
    count = FLASH_PAGE_SIZE_OUTSIDE - Addr;                  //计算该地址所在的页还有多少的字节需要写
    numOfPage = length / FLASH_PAGE_SIZE_OUTSIDE;       //计算有该长度有多少页
    numOfSingle = length % FLASH_PAGE_SIZE_OUTSIDE;      //计算除去整页还剩多少字节

    if(0 == Addr)                                       //编程地址是某页的起始地址
    {
        if(0 == numOfPage)                              //长度小于一页
        {
            spi_flash_write_page(writeBuffer, writeAddress, length);
        }
        else                                            //长度大于一页
        {
            while(numOfPage--)
            {
                spi_flash_write_page(writeBuffer, writeAddress, FLASH_PAGE_SIZE_OUTSIDE);
                
                writeAddress += FLASH_PAGE_SIZE_OUTSIDE;
                writeBuffer  += FLASH_PAGE_SIZE_OUTSIDE;
                
            }
            if(numOfSingle)
            {
                spi_flash_write_page(writeBuffer, writeAddress, numOfSingle);     //写剩余的一部分
            }
        }
    }
    else                        //编程地址不是某页的起始地址
    {
        if(0 == numOfPage)      //长度小于一页
        {
            if(numOfSingle > count)     //除去整页剩下的字节 > 该页还可以写的长度
            {
                temp = numOfSingle - count;
                spi_flash_write_page(writeBuffer, writeAddress, count);

                writeAddress += count;
                writeBuffer  += count;

                spi_flash_write_page(writeBuffer, writeAddress, temp);
            }
            else
            {
                spi_flash_write_page(writeBuffer, writeAddress, length);
            }
        }
        else
        {
            length -= count;
            numOfPage   = length / FLASH_PAGE_SIZE_OUTSIDE;
            numOfSingle = length % FLASH_PAGE_SIZE_OUTSIDE;

            spi_flash_write_page(writeBuffer, writeAddress, count);

            writeAddress += count;
            writeBuffer  += count;

            while(numOfPage--)
            {
                spi_flash_write_page(writeBuffer, writeAddress, FLASH_PAGE_SIZE_OUTSIDE);

                writeAddress += FLASH_PAGE_SIZE_OUTSIDE;
                writeBuffer  += FLASH_PAGE_SIZE_OUTSIDE;
            }

            if(numOfSingle != 0)
            {
                spi_flash_write_page(writeBuffer, writeAddress, numOfSingle);
            }
        }
    }
    
    return 0;
}

/***************************************************************
****************FLASH 对外操作接口******************************
****************************************************************/
uint8_t flashEarseBuffer(unsigned int address, unsigned int writeLength)
{
    uint16_t earseNum;
    
    if(writeLength == 0)
    {
        return 1;
    }
    
    earseNum = spi_flash_earse_num(address, writeLength);    
    DFLASH_PRINT_INFO4("[FLASH] spi_flash_earse_num addr:0x%x(%dk), len:%d, earseNum:%d", address, (address/0x400), writeLength, earseNum);
    
    return spi_flash_earse(address, earseNum);
}

void inside_flash_write(unsigned char *writeBuffer, unsigned int address, unsigned int writeLength)
{
	uint32_t *p = (uint32_t *)writeBuffer;
	uint8_t earseNum;
	int i = 0; 
	uint32_t s_val = 0;
	
	//APP_PRINT_INFO2("len %d, data is %b", writeLength, TRACE_BINARY(writeLength, writeBuffer));

	earseNum = (writeLength / 4096) + ((writeLength % 4096 > 0) ? (1) : (0));
	//APP_PRINT_INFO1("earseNum is %d", earseNum);	

	for(i = 0; i < earseNum; i++)
		flash_erase_locked(FLASH_ERASE_SECTOR, address+i*4096);
	
	for(i = 0; i < writeLength; i+=4)
	{
		flash_auto_write_locked(address+i, *p);
		flash_auto_read_locked(address + i | FLASH_OFFSET_TO_NO_CACHE, &s_val);
        if (s_val != *(uint32_t *)p)
        {
           // APP_PRINT_INFO3("<==dfu_update: ERROR! w_data=0x%x, r_data=0x%x, addr=0x%x",
           //                  *(uint32_t *)p, s_val, address + i);
        }
		p++;
	}
		
}

uint8_t flashWriteBuffer(unsigned char *writeBuffer, unsigned int address, unsigned int writeLength, EM_FLASH_CTRL_MODE mode)
{
    uint8_t rv = 0;
    switch(mode)
    {
        case EM_FLASH_CTRL_INSIDE:
			if((address < 0x0082A000) || (address + writeLength > 0x00880000))
			{
				APP_PRINT_INFO2("write inside flash error ddr(0x%x), len(%d)", address, writeLength);
				return 1;
			}
            inside_flash_write(writeBuffer, address, writeLength);
            break;
        
        case EM_FLASH_CTRL_OUTSIDE:
            rv = flashEarseBuffer(address, writeLength);
            if(rv != 0)
            {
            	APP_PRINT_INFO0("flashEarseBuffer error ###########!");
            	break;
            }                
            
            spi_flash_write_buffer(writeBuffer, address, writeLength);
            break;
			
        case EM_FLASH_CTRL_FTL:
			if(address + writeLength > 3056)
			{
				APP_PRINT_INFO2("write ftl flash error ddr(0x%x), len(%d)", address, writeLength);
				return 1;
			}
			ftl_save(writeBuffer, address, writeLength);
			break;
		
        default:
            break;
    }
    
    return rv;
}

void inside_flash_read(unsigned char *readBuffer, unsigned int address, unsigned int readLength)
{
	uint32_t data = 0;
	int i = 0; 

	//APP_PRINT_INFO1("address 0x%x", address);
	for(i = 0; i < readLength; i+=4)
	{
		flash_auto_read_locked(address+i|FLASH_OFFSET_TO_NO_CACHE, &data);
		//if(i == 0)
		//	APP_PRINT_INFO2("read addr 0x%x, data is 0x%x", address+i, data);
		readBuffer[i] = data & 0xFF;
		readBuffer[i+1] = (data >> 8) & 0xFF;
		readBuffer[i+2] = (data >> 16) & 0xFF;
		readBuffer[i+3] = (data >> 24) & 0xFF;
	}

	//APP_PRINT_INFO2("read len %d, data is %b", readLength, TRACE_BINARY(readLength, readBuffer));
		
}

uint8_t flashReadBuffer(unsigned char *readBuffer, unsigned int address, unsigned int readLength, EM_FLASH_CTRL_MODE mode)
{   

    switch(mode)
    {
        case EM_FLASH_CTRL_INSIDE:
            inside_flash_read(readBuffer, address, readLength);
            break;
        
        case EM_FLASH_CTRL_OUTSIDE:
            spi_flash_read(SPI_FLASH_READ_DATA, readBuffer, address, readLength);
            break;

		case EM_FLASH_CTRL_FTL:
			ftl_load(readBuffer, address, readLength);
			break;
        
        default:
            break;
    }
    return 0;
}

uint8_t flashReadBuffer_match(unsigned char *readBuffer, unsigned int address, unsigned int readLength, EM_FLASH_CTRL_MODE mode)
{
    switch(mode)
    {
        case EM_FLASH_CTRL_INSIDE:
            inside_flash_read(readBuffer, address, readLength);
            break;
        
        case EM_FLASH_CTRL_OUTSIDE:
            spi_flash_read_match(SPI_FLASH_READ_DATA, readBuffer, address, readLength);
            break;
        
        default:
            break;
    }
    return 0;
}
