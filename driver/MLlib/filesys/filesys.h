/*
 * filesys.h
 *
 *  Created on: 2020年8月17日
 *      Author: sks
 */

#ifndef APPLICATION_FILESYS_FILESYS_H_
#define APPLICATION_FILESYS_FILESYS_H_

#include "mlapi.h"

#define FILE_SYSTEM_OK                  (0)
#define FILE_SYSTEM_PARAMETER_ERROR     (1)
#define FILE_SYSTEM_FLASH_ID_ERROR      (2)
#define FILE_SYSTEM_STORAGE_FTR_FAILD   (3)
#define FILE_SYSTEM_STORAGE_FULL        (4)
#define FILE_SYSTEM_READ_FTR_ERROR      (5)
#define FILE_SYSTEM_NONE_UPDATE_SELF_BG (6)
#define FILE_SYSTEM_FTR_CRC_ERR         (7)
#define FILE_SYStEM_FTR_UPDATE          (8)

#define SETTING_STORAGE_OK              (0)
#define SETTING_STORAGE_CHK_ERR         (1)
#define SETTING_STORAGE_NULL            (2)

#define USE_FLAG                        (1)
#define DEFAULT_FLAG                    (0)
#define DEFAULT_INDEX                   (0)
#define DEFAULT_SECTOR                  (0xff)
#define DEFAULT_ID                      (0)
#define DEFAULT_LENGTH                  (0)

#define STORE_MAX_FTR                   (10) //指纹最大数量

#define SY_MAX_NOTE                     (16)
#define SY_NOTE_LEN                     (32)

typedef struct{
    unsigned char unFlag;               //变量命名前的 un 代表的是unsigned char
    unsigned char unSec;
    unsigned short unId;
    unsigned short unLength;
	unsigned short unNumID;				//成员ID，对于涂鸦平台的使用(范围1-100)
}__attribute__((packed)) STORAGE_FP_NODE;

typedef struct{
    STORAGE_FP_NODE ftrHead[STORE_MAX_FTR];                 //FTR head
    unsigned short updatetime[STORE_MAX_FTR];              //self leaning times
    unsigned char secHead[20];                              //sector head
    unsigned char idHead[32];                               //id head
    unsigned int ftrNum;                                    //FTR number
    unsigned char sy_note_Head[SY_MAX_NOTE][SY_NOTE_LEN];
    unsigned int flashEraNum;                               //Flash Erase Num
    unsigned int chk;                                       //CRC
}__attribute__((packed)) ST_STORAGE_FP_LINK;                //sum

#define FLASH_BASE_ADDR				(0x858000)
#define FLASH_FP_SAVE_ADDR			(0x0084C000) //指纹保存地址
#define FLASH_PAGE_SIZE             (0x1000)
#define FLASH_SECTOR_SIZE_8K        (0x2000)
#define FLASH_SECTOR_SIZE_12K       (0x3000)
#define FLASH_SECTOR_SIZE_16K       (0x4000)

#if 0
#define FTR_HEAD_ADDRESS            (FLASH_BASE_ADDR)
#define FTR_HEAD_BACKUP_ADDRESS     (FLASH_BASE_ADDR + FLASH_PAGE_SIZE)
#define FTR_HEAD_LENGTH             (sizeof(STORAGE_FP_NODE))
#define FTR_HEAD_ALL_LENGTH         (sizeof(ST_STORAGE_FP_LINK))

#define FTR_INFO_BASE_ADDRESS       (FLASH_BASE_ADDR + FLASH_PAGE_SIZE * 2)        //Ftr头信息的地址，占有4*2K
#define FPC_INIT_PARA_ADDRESS       (FLASH_BASE_ADDR + FLASH_PAGE_SIZE * 4)        //FPC数据
#define SYS_INIT_PARA_ADDRESS       (FLASH_BASE_ADDR + FLASH_PAGE_SIZE * 5)        //SYSTEM数据
#define SYS_INIT_PARA_BACK_ADDRESS  (FLASH_BASE_ADDR + FLASH_PAGE_SIZE * 6)

#define RECORD_ADDRESS				(FLASH_BASE_ADDR + FLASH_PAGE_SIZE * 7)		//指纹开锁记录
#define PASSWORD_ADDRESS			(FLASH_BASE_ADDR + FLASH_PAGE_SIZE * 8)		//保存密码
#else
//ftl空间暂时只有3056个字节
#define FTR_INFO_BASE_ADDRESS       0x0084A000       							//Ftr头信息的地址，占有4*2K
#define FTR_HEAD_ALL_LENGTH         (sizeof(ST_STORAGE_FP_LINK))				//776字节

#define FTR_HEAD_ADDRESS            (0)												//占776字节
#define FTR_HEAD_BACKUP_ADDRESS     (FTR_HEAD_ALL_LENGTH)						//占776字节

#define FPC_INIT_PARA_ADDRESS       (FTR_HEAD_ALL_LENGTH * 2)        			//FPC数据,占264字节

#define SYS_INIT_PARA_ADDRESS       (FTR_HEAD_ALL_LENGTH * 2 + 264)        		//SYSTEM数据，占256字节
#define SYS_INIT_PARA_BACK_ADDRESS  (FTR_HEAD_ALL_LENGTH * 2 + 264 + 256)		//256字节	

//tuya_ble_internal_config.h里面用了512字节，空间用到 2840了，
//tuya_ble_app_ota.cl里面需要用80字节，空间用到了2920, 剩下136
#define RECORD_ADDRESS				(2920)	//开锁记录


//#define RECORD_ADDRESS				(FTR_HEAD_ALL_LENGTH * 2 + 264 + 512 + 4)		//指纹开锁记录，占408字节
#define PASSWORD_ADDRESS			(FTR_HEAD_ALL_LENGTH * 2 + 264 + 512 + 408 + 4)	//保存密码，占用104字节

#endif

extern ST_STORAGE_FP_LINK g_stAllFtrHead;
extern unsigned short g_filesysAdminCount;

extern uint32_t CRC32_calc(uint8_t *Data , uint32_t Len);

extern int fileSys_getStoreFtrNum(void);
extern int fileSys_getUseIndex(unsigned char *index_buff, unsigned int *index_num);
extern unsigned short fileSys_getUnuseSmallestIndex(void);
extern unsigned short fileSys_indexToId(unsigned short index);
extern void fileSys_getIdDistribute(unsigned char * IdIndexBuff);

extern int fileSys_rankFtrHead(unsigned char *index_buff, unsigned int num);
extern int fileSys_storeFtrAndUpdateFtrHead(unsigned short id, unsigned short numId, unsigned char *ftrBuffer, unsigned int length);
extern int fileSys_Ftrinfo_StoreAndUpdate(unsigned short id, unsigned char *ftrBuffer, unsigned int leng);
extern int fileSys_readFtr(unsigned short index, unsigned char *readBuffer, unsigned int *length);
int fileSys_IdToIndex(unsigned short id);

extern uint32_t fileSys_deleteAllFtr(void);
extern uint32_t fileSys_deleteBatchFtr(unsigned short ausFPIdx[],unsigned short usFPNum);
extern uint32_t fileSys_deleteOneFtr(unsigned short id);

extern int fileSys_ReadSYNoteBook(unsigned char sec, unsigned char *buff, unsigned char length);
extern int fileSys_WriteSYNoteBook(unsigned char sec, unsigned char *buff, unsigned char length);
extern uint8_t fileSys_Is_Buff_Clear(unsigned char *buff, unsigned int num, unsigned int value);
    
extern int fileSys_Init(void);
unsigned int GetChksum(uint8_t *p, uint32_t num);
unsigned short fileSys_GetNumId(unsigned short id);
uint32_t fileSys_deleteNumberFtr(unsigned short id);
unsigned char fileSys_checkIDExist(unsigned short id);


#endif /* APPLICATION_FILESYS_FILESYS_H_ */
