
/*
 *  Routines to access hardware
 *
 *  Copyright (c) 2013 Realtek Semiconductor Corp.
 *
 *  This module is a confidential and proprietary property of RealTek and
 *  possession or use of this module requires written permission of RealTek.
 */
#ifndef _DFU_TASK_H_
#define _DFU_TASK_H_
#include <stdint.h>
#include <stdbool.h>
#include "app_msg.h"

#define TMP_OTA_ADDR 0x0084C000  //升级用到的分区

#define SECTOR_SIZE 4096   //一个扇区的大小

//命令字2，在线升级类命令(私有命令)
#define CMD_UPDATE_START_DOWNLOAD               (0xFE01)  //启动升级
#define CMD_UPDATE_FILE_INFO                    (0xFE02)  //下发文件信息(文件长度，文件CRC)
#define CMD_UPDATE_TRANSFER_FILE                (0xFE03)  //文件传输
#define CMD_UPDATE_BLOCK_CRC                    (0xFE04)  //当前Block CRC
#define CMD_UPDATE_TRANS_COMPLETE               (0xFE05)  //文件传输完成
#define CMD_UPDATE_ERASE_SYS_PARA               (0xFE06)  //擦除系统参数
#define CMD_UPDATE_SET_SYS_PARA               	(0xFE0F)  //设置系统参数

typedef enum
{
	E_UPDATE_START = 0x01,		//启动升级
	E_UPDATE_INFO = 0x02,		//下发文件信息(文件长度，文件CRC)
	E_UPDATE_TRANSFER = 0x03,	//文件传输
	E_UPDATE_BLOCK_CRC = 0x04,	//当前Block CRC
	E_UPDATE_COMPLETE = 0x05,	//文件传输完成
	E_UPDATA_CLEAN_PARAM = 0x06, //擦除系统参数
}update_e;

typedef enum
{
	E_SECTOR_CONFIG,
	E_SECTOR_OTA,
	E_SECTOR_LOADER,
	E_SECTOR_ROM,
	E_SECTOR_BT,
	E_SECTOR_APP,
}sector_e;

void dfu_task_init(void);
bool app_send_msg_to_dfutask(T_IO_MSG *p_msg);
void upgrade_task_init(void);
unsigned short crc_ccitt(unsigned char *q, int len);

#endif

