
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

#define TMP_OTA_ADDR 0x0084C000  //�����õ��ķ���

#define SECTOR_SIZE 4096   //һ�������Ĵ�С

//������2����������������(˽������)
#define CMD_UPDATE_START_DOWNLOAD               (0xFE01)  //��������
#define CMD_UPDATE_FILE_INFO                    (0xFE02)  //�·��ļ���Ϣ(�ļ����ȣ��ļ�CRC)
#define CMD_UPDATE_TRANSFER_FILE                (0xFE03)  //�ļ�����
#define CMD_UPDATE_BLOCK_CRC                    (0xFE04)  //��ǰBlock CRC
#define CMD_UPDATE_TRANS_COMPLETE               (0xFE05)  //�ļ��������
#define CMD_UPDATE_ERASE_SYS_PARA               (0xFE06)  //����ϵͳ����
#define CMD_UPDATE_SET_SYS_PARA               	(0xFE0F)  //����ϵͳ����

typedef enum
{
	E_UPDATE_START = 0x01,		//��������
	E_UPDATE_INFO = 0x02,		//�·��ļ���Ϣ(�ļ����ȣ��ļ�CRC)
	E_UPDATE_TRANSFER = 0x03,	//�ļ�����
	E_UPDATE_BLOCK_CRC = 0x04,	//��ǰBlock CRC
	E_UPDATE_COMPLETE = 0x05,	//�ļ��������
	E_UPDATA_CLEAN_PARAM = 0x06, //����ϵͳ����
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

