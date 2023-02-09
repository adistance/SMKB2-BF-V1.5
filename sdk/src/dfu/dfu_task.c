/**
*****************************************************************************************
*     Copyright(c) 2016, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file    dfu_task.c
  * @brief   dfu task for normal ota.
  * @details
  * @author
  * @date
  * @version
  ******************************************************************************
  * @attention
  * <h2><center>&copy; COPYRIGHT 2016 Realtek Semiconductor Corporation</center></h2>
  ******************************************************************************
  */
#include "os_msg.h"
#include "os_task.h"
#include "gap_msg.h"
#include "app_msg.h"
#include "trace.h"
#include "dfu_application.h"
#include "dfu_main.h"
#include "dfu_task.h"
#include "board.h"
#include "rtl876x_uart.h"
#include "flash_device.h"
#include "rtl876x_wdg.h"
#include "os_sched.h"
#include <string.h>
#include "dfu_flash.h"
#include "driver_uart.h"
#include "aes256.h"
#include "driver_sensor.h"

static unsigned short ccitt_table[256] = {
0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};

uint8_t AesKeyBuf[32] = {  0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 
                        0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 
                        0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 
                        0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38};
uint8_t AesNonceBuf[16] = { 0x4d, 0x73, 0x74, 0x61, 0x72, 0x26, 0x4d, 0x4c,
                         0x4b, 0x65, 0x79, 0x5f, 0x41, 0x46, 0x49, 0x53 };
static uint8_t sectorId = 0;  //第几个sector，一个4096大小
uint8_t g_flash_old = 0xff;
static uint8_t upgradeSuc = 0; //升级成功标志


#if (SUPPORT_NORMAL_OTA == 1)
/*============================================================================*
 *                              Macros
 *============================================================================*/
#define DFU_TASK_PRIORITY               1
#define DFU_TASK_STACK_SIZE             512 * 8  //todo: sync with bee2, may modify

#define MAX_NUMBER_OF_GAP_MESSAGE       0x20
#define MAX_NUMBER_OF_IO_MESSAGE        0x20
#define MAX_NUMBER_OF_EVENT_MESSAGE     (MAX_NUMBER_OF_GAP_MESSAGE + MAX_NUMBER_OF_IO_MESSAGE)

void *dfu_task_handle;
void *dfu_evt_queue_handle;
void *dfu_io_queue_handle;

void *upgrade_task_handle;
void *upgrade_evt_queue_handle;
void *upgrade_io_queue_handle;

void dfu_main_task(void *p_param);
void upgrade_main_task(void *p_param);

bool unlock_flash(void)
{
    DFU_PRINT_TRACE0("**********Flash unlock BP***********");
    if (flash_sw_protect_unlock_by_addr_locked(0x00800000, &g_flash_old))
    {
        DFU_PRINT_TRACE1("<==Unlock Total Flash Success! prev_bp_lv=%d", g_flash_old);
        return true;
    }
    return false;
}

/**
*  @brief: lock flash after erase or write flash.
*/


bool app_send_msg_to_dfutask(T_IO_MSG *p_msg)
{
    uint8_t event = EVENT_IO_TO_APP;

    if (os_msg_send(dfu_io_queue_handle, p_msg, 0) == false)
    {
        APP_PRINT_ERROR0("send_io_msg_to_app fail");
        return false;
    }
    if (os_msg_send(dfu_evt_queue_handle, &event, 0) == false)
    {
        APP_PRINT_ERROR0("send_evt_msg_to_app fail");
        return false;
    }
    return true;
}

void dfu_task_init()
{
    os_task_create(&dfu_task_handle, "dfu", dfu_main_task, 0, DFU_TASK_STACK_SIZE,
                   DFU_TASK_PRIORITY);
}

void upgrade_task_init()
{
    os_task_create(&upgrade_task_handle, "upgrade", upgrade_main_task, 0, DFU_TASK_STACK_SIZE,
                   DFU_TASK_PRIORITY);
}

static uint8_t FP_protocol_get_checksum(uint8_t * data, uint32_t length)
{
    uint32_t i = 0;
    uint8_t sum = 0;
    
    for(i = 0; i < length; i++)
        sum += data[i];

    return (uint8_t)((~sum)+1);
}


void request_upgrade_cmd(uint16_t cmd_type, uint8_t *data, uint8_t dataLen)
{
	uint8_t buf[255] = {0};
	uint8_t bufLen = 0;
	uint8_t MLframeHead[8] = {0xF1, 0x1F, 0xE2, 0x2E, 0xB6, 0x6B, 0xA8, 0x8A};
	uint8_t *p = buf;
	uint8_t *start = buf;
	uint8_t chksum = 0;
	int i = 0;

	memcpy(p, MLframeHead, sizeof(MLframeHead));
	p += sizeof(MLframeHead);
	bufLen += sizeof(MLframeHead);

	//data len
	*p++ = 0;
	*p++ = dataLen+7;
	bufLen += 2;

	//head checksum
	chksum = FP_protocol_get_checksum(start, 10);
	*p++ = chksum;
	bufLen++;

	//check password
	p += 4;
	bufLen += 4;

	//cmd
	*p++ = cmd_type>>8;
	*p++ = cmd_type;
	bufLen += 2;

	//app data
	if(dataLen != 0)
	{
		memcpy(p, data, dataLen);
		p += dataLen;
		bufLen += dataLen;
	}

	chksum = FP_protocol_get_checksum(start+11, 6+dataLen);
	*p++ = chksum;
	bufLen++;
	
	//APP_PRINT_INFO2("send data len:%d, data:%b", bufLen, TRACE_BINARY(bufLen, buf));
	for(i = 0; i < bufLen; i++)
	{
		USART_SendData(buf+i, 1);
	}
	
}

unsigned short crc_ccitt(unsigned char *q, int len)
{
    unsigned short crc = 0;

    while (len-- > 0)
    crc = ccitt_table[(crc >> 8 ^ *q++) & 0xff] ^ (crc << 8);
    return ~crc;
}


static uint32_t flash_erase(uint32_t addr)
{
    uint32_t ret = 0;
    bool result = false;
    result = flash_erase_locked(FLASH_ERASE_SECTOR, addr);
    if (!result)
    {
        ret = __LINE__;
    }
    APP_PRINT_INFO2("<==dfu_flash_erase_sector: addr=%x, result=%d", addr, result);
    return ret;
}

int upgrade_write_flash()
{
	int i = 0, ret = 0;
	uint32_t s_val = 0;
	uint32_t *p = 0;
	uint32_t app_base = TMP_OTA_ADDR;//app分区的起始地址，这个从flash_map.ini文件里面确认
	uint32_t earse_base = TMP_OTA_ADDR;  
	uint16_t write_size = SECTOR_SIZE;
	uint32_t id = 0;

	id = sectorId;
	p = (uint32_t *)g_ImgBuf;
	
	unlock_flash_bp_all();
	APP_PRINT_INFO1("-----upgrade_write_flash %d ------", sectorId);	
	flash_erase(earse_base+sectorId*SECTOR_SIZE);
		
	for(i = 0; i < write_size; i += 4)
	{		
		//前面4个字节不能这么快写，等全部数据写完了这4个字节才可以写，这里是为了防止升级中掉电的问题
		//bootloader 根据这个数据来判断是否把ota_tmp数据拷贝到app分区
		if(id == 0 && i == 0)
		{
			p++;
			continue;
		}
		ret = flash_auto_write_locked(app_base+id*SECTOR_SIZE+i, *(uint32_t *)p);
		if(ret != true)
			APP_PRINT_INFO0("-----flash_auto_write_locked error *********------");	
		flash_auto_read_locked(app_base+id*SECTOR_SIZE + i | FLASH_OFFSET_TO_NO_CACHE, &s_val);
        if (s_val != *(uint32_t *)p)
        {
            APP_PRINT_INFO3("<==dfu_update: ERROR! w_data=0x%x, r_data=0x%x, addr=0x%x",
                             *(uint32_t *)p, s_val, app_base+id*SECTOR_SIZE + i);
        }
        else
        {
            p++;
        }
	}
	lock_flash_bp();
	return 0;
}


int write_flash(uint32_t addr, uint32_t *data, uint16_t writeLen)
{
	int i = 0, ret = 0;
	uint32_t s_val = 0;
	
	unlock_flash_bp_all();
	APP_PRINT_INFO1("-----flash_auto_write_locked addr 0x%x *********------", addr);		
	for(i = 0; i < writeLen; i += 4)
	{
		ret = flash_auto_write_locked(addr+i, *data);
		if(ret != true)
			APP_PRINT_INFO0("-----flash_auto_write_locked error *********------");	
		flash_auto_read_locked(addr+i | FLASH_OFFSET_TO_NO_CACHE, &s_val);
        if (s_val != *data)
        {
            APP_PRINT_INFO3("<==dfu_update: ERROR! w_data=0x%x, r_data=0x%x, addr=0x%x",
                             *data, s_val, addr+i);
        }
        else
        {
            data++;
        }
	}
	lock_flash_bp();
	
	return 0;
}


int cmd_process(uint8_t data)
{	
	uint8_t MLframeHead[8] = {0xF1, 0x1F, 0xE2, 0x2E, 0xB6, 0x6B, 0xA8, 0x8A}; 
	static uint8_t frameId = 0;
	static uint8_t tmpBuf[512] = {0};  //存放每一帧的数据，升级每次传输最大276个字节
	static uint16_t tmpLen = 0;			//缓存数据的长度
	static uint8_t frameHeadTag = 0;  //协议头部校验标志
	static uint16_t dataLen = 0;
	static uint32_t file_len = 0;	//整个升级文件的长度
	static uint16_t file_crc = 0;	//整个升级文件的CRC
	//uint8_t i = 0;
	uint16_t cmd_type = 0;
	static uint16_t index = 0;    //记录获取了多少个256数据
	uint8_t send_data[50] = {0};
	
	uint16_t usSourceCRC = 0;		//每个扇区收到的CRC
	uint16_t calCrc = 0;			//每个扇区计算出来的CRC
	static uint16_t pos = 0;

	static uint8_t fail_cnt = 0;  //升级失败最大次数(包括接口数据错误)

	if(!frameHeadTag) //校验协议头部
	{
		if(data == MLframeHead[frameId])
		{
			frameId++;
			if(frameId == sizeof(frameId))
			{
				frameId = 0;
				frameHeadTag = 1;
			}
			tmpBuf[tmpLen] = data;
			tmpLen++;
		}
		else
		{
			goto clean;
		}
	}
	else //协议的头文件校验过了
	{
		tmpBuf[tmpLen] = data;	
		
		if(tmpLen == 8)
			dataLen |= (data << 8);
		else if(tmpLen == 9)
		{
			dataLen |= data;
			
			//APP_PRINT_INFO1("data len is %d\n", dataLen);
		}		
		else if(tmpLen == 10) //帧头部校验
		{
			if(data != FP_protocol_get_checksum(tmpBuf, 10))
			{
				APP_PRINT_INFO2("frame head check error, %x %x", data, FP_protocol_get_checksum(tmpBuf, 10));
				fail_cnt++;	
				if(fail_cnt > 5)
				{
					fail_cnt = 0;
					goto upgradeFail;
				}
				goto clean;
			}
		}		
		
		tmpLen++;
		if(dataLen + 11 == tmpLen) //全部数据收齐
		{			
			if(tmpLen < 100)
				APP_PRINT_INFO2("uart recv buf len:%d, data:%b", tmpLen, TRACE_BINARY(tmpLen, tmpBuf));
			else 
			{
				//for(i = 0; i < 16; i++)
				//	APP_PRINT_INFO2("uart recv i:%d, data:%b", i, TRACE_BINARY(16, tmpBuf+19+16*i));
			}
			
			//校验数据部分
			if(data != FP_protocol_get_checksum(tmpBuf+11, dataLen-1))
			{
				APP_PRINT_INFO2("frame head check error, %x %x", data, FP_protocol_get_checksum(tmpBuf+11, tmpLen-1));
				goto clean;
			}
			cmd_type = (tmpBuf[15] << 8) | tmpBuf[16];
			//APP_PRINT_INFO1("cmd_type is 0x%x", cmd_type);
			switch(cmd_type)
			{
				case CMD_UPDATE_START_DOWNLOAD:					
					request_upgrade_cmd(cmd_type, send_data, 4);
					os_delay(1);
					request_upgrade_cmd(0xFE02, send_data, 0); 
					index = 0;
					pos = 0;
					sectorId = 0;
					break;
				case CMD_UPDATE_FILE_INFO:
					file_len = (tmpBuf[17] << 24) | (tmpBuf[18] << 16) | (tmpBuf[19] << 8) | tmpBuf[20];
					file_crc = (tmpBuf[21] << 8) | tmpBuf[22];
					APP_PRINT_INFO3("file_len is %d(%x), file_crc is 0x%x", file_len, file_len, file_crc);
					request_upgrade_cmd(0xFE03, send_data, 2);
					break;
				
				case CMD_UPDATE_TRANSFER_FILE:
					if(index < file_len / 256)
					{
						memcpy(g_ImgBuf+pos*256, tmpBuf+19, 256);	
						//for(i = 0; i < 16; i++)
						//	APP_PRINT_INFO2("i:%d, g_ImgBuf:%b", i, TRACE_BINARY(16, g_ImgBuf+a*256+16*i));
						pos++;
						//数据有误，重新接受4096个数据
						if (((index+1) * 256)% 0x1000 == 0 && (index != 0))
						{							
							send_data[0] = sectorId;
							request_upgrade_cmd(0xFE04, send_data, 1);
							break;
						}
						
						index++;
						//APP_PRINT_INFO1("---index %d----", index);
						send_data[0] = (index >> 8) & 0xFF;
						send_data[1] = index & 0xFF;
						request_upgrade_cmd(0xFE03, send_data, 2);						
					}					
					break;

				case CMD_UPDATE_BLOCK_CRC:
					usSourceCRC = (tmpBuf[17] << 8) | (tmpBuf[18]);
					
					AesKeyBuf[0] =  0x31+1+sectorId;			
					aes256_ctr(AesKeyBuf, AesNonceBuf, g_ImgBuf, SECTOR_SIZE);
					calCrc = crc_ccitt(g_ImgBuf, SECTOR_SIZE);	
					if(calCrc != usSourceCRC)
					{
						APP_PRINT_INFO3("****usSourceCRC is %d(%x), cal crc is %x", usSourceCRC, usSourceCRC, calCrc);
						pos = 0;
						index -= 15;
						APP_PRINT_INFO1("---index %d----", index);
						send_data[0] = (index >> 8) & 0xFF;
						send_data[1] = index & 0xFF;
						request_upgrade_cmd(0xFE03, send_data, 2);
						fail_cnt++;	
						if(fail_cnt > 5)
						{
							fail_cnt = 0;
							goto upgradeFail;
						}
						break;
					}	
					else
					{
						upgrade_write_flash();						

						sectorId++;	
						memset(g_ImgBuf, 0, SECTOR_SIZE);
						pos = 0;
						
						if(index < (file_len / 256) - 1)
						{															
							index++;
							//APP_PRINT_INFO2("---index %d %d----", index, file_len / 256 - 1);
							send_data[0] = (index >> 8) & 0xFF;
							send_data[1] = index & 0xFF;
							request_upgrade_cmd(0xFE03, send_data, 2);
							
						}
						else
						{	
							uint32_t data = 0x09000109; //bootloader 根据这个数据来判断是否把ota_tmp数据拷贝到app分区
							send_data[0] = 0;
							flash_auto_write_locked(TMP_OTA_ADDR, data);
							//APP_PRINT_INFO0("-------upgrade success!-------");
							request_upgrade_cmd(0xFE05, send_data, 1);	
							upgradeSuc = 1;	
							//dfu_fw_reboot(true);
							//WDG_SystemReset(RESET_ALL_EXCEPT_AON, SWITCH_TEST_MODE);
							//return 0;
						}
					}									
					break;
					
				case CMD_UPDATE_TRANS_COMPLETE:
					break;

				case CMD_UPDATE_SET_SYS_PARA:
					request_upgrade_cmd(0xFE0F, send_data, 4);
					break;
				case 0x0302: //write id
					request_upgrade_cmd(cmd_type, send_data, 4);
					break;	
				case 0x0306: //获取软件版本号
					request_upgrade_cmd(cmd_type, send_data, 4+4);
					break;
				case 0x0308: //获取sn号
					request_upgrade_cmd(cmd_type, send_data, 16+4);
					break;
				case 0x03FA: //read board series num
					request_upgrade_cmd(cmd_type, send_data, 24+4);	
					break;
				case 0x03FB: //write board series num
					request_upgrade_cmd(cmd_type, send_data, 4);					
					break;
			}
			
			goto clean;
		}
		
	}

	return 0;
	
upgradeFail:
	flash_erase(TMP_OTA_ADDR);
	send_data[0] = 1;
	APP_PRINT_INFO0("-------upgrade fail!-------");
	request_upgrade_cmd(0xFE05, send_data, 1);	
clean:
	//APP_PRINT_INFO0("*****************clean data***********************");
	dataLen = 0;
	frameId = 0;
	tmpLen = 0;
	memset(tmpBuf, 0 , sizeof(tmpBuf));
	frameHeadTag = 0;

	return 0;
}


void upgrade_main_task(void *p_param)
{
	uint8_t buf[50] = {0};
	uint32_t cnt = 0;
	
	uart_mode_set(true);
	request_upgrade_cmd(0xFE02, buf, 0);  	
    while (1)
    { 	
		if(UART_GetFlagStatus(UART0, UART_FLAG_RX_DATA_AVA) == 1)
		{
			UART_ReceiveData(UART0, buf, 1);
			cmd_process(buf[0]);
			cnt = 0;
		}
		else
		{
			cnt++;
			if(cnt == 0xFFFFF && upgradeSuc)
			{				
				dfu_fw_reboot(true);
				WDG_SystemReset(RESET_ALL_EXCEPT_AON, SWITCH_TEST_MODE);
			}
		}
    }
}


/**
* @brief
*
*
* @param   pvParameters
* @return  void
*/
void dfu_main_task(void *p_param)
{
    uint8_t event;

    os_msg_queue_create(&dfu_io_queue_handle, MAX_NUMBER_OF_IO_MESSAGE, sizeof(T_IO_MSG));
    os_msg_queue_create(&dfu_evt_queue_handle, MAX_NUMBER_OF_EVENT_MESSAGE, sizeof(uint8_t));

    gap_start_bt_stack(dfu_evt_queue_handle, dfu_io_queue_handle, MAX_NUMBER_OF_GAP_MESSAGE);

    //dfu_timer_init();

    while (true)
    {
        if (os_msg_recv(dfu_evt_queue_handle, &event, 0xFFFFFFFF) == true)
        {
            //DBG_DIRECT("***os_msg_recv***");
            if (event == EVENT_IO_TO_APP)
            {
                T_IO_MSG io_msg;
                if (os_msg_recv(dfu_io_queue_handle, &io_msg, 0) == true)
                {
                    //dfu_handle_io_msg(io_msg);
                }
            }
            else
            {
                gap_handle_msg(event);
            }
        }
    }
}
#endif //end SUPPORT_NORMAL_OTA
