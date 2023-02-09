#ifndef RECORD_H
#define RECORD_H

#include "filesys.h"

//���ż�¼������
#define BLE_RECORD_MAX_NUM	(15) 

#define BLE_RECORD_PAGE_MAX_SIZE 13 //����С�����ȡ��¼ÿ�ζ�ȡ13����¼

#define BLE_RECORD_HEAD_ADDRESS	        	 	(RECORD_ADDRESS)
#define BLE_RECORD_HEAD_ALL_LENGTH         		(sizeof(ST_STORAGE_BLE_RECORD_LINK))

typedef enum mode
{
	ModeNull     = 0,
	ModePassword,
	ModeDoor,
	ModeFp,
	ModeInit,
	ModeBle,
	ModeKey,
	ModeOther
}eModeCtrl;

#if 1
typedef struct{
    unsigned char ucNumId;	//����ID
	unsigned char ucType;	//��������
    bool bUseFlag;		//Ĭ��true���ϴ�������λfalse
    unsigned char ucReserve[1];
	unsigned int utcTime;
}__attribute__((packed)) STORAGE_RECORD_NODE;

typedef struct{
	STORAGE_RECORD_NODE record[BLE_RECORD_MAX_NUM];
    unsigned char recordNum;
	unsigned char ucReserve[3];
    unsigned int chk;
}__attribute__((packed)) ST_STORAGE_BLE_RECORD_LINK;
#else
typedef struct{
    unsigned char mode_h;
    unsigned char id_h;
    unsigned char mode_l;
    unsigned char id_l;
	unsigned int utcTime;
}__attribute__((packed)) STORAGE_RECORD_NODE;

typedef struct{
	STORAGE_RECORD_NODE record[BLE_RECORD_MAX_NUM];
    unsigned short recordNum;
    unsigned char ucReserve[2];                       //??
    unsigned int chk;
}__attribute__((packed)) ST_STORAGE_BLE_RECORD_LINK;

#endif
typedef struct{
	uint8_t setCloseDoorTime;               //12
	uint8_t setOpenDoorDirection;           //0x4D - ת������
	uint8_t setLatchBoltRevertTime;         //0x4F - ���෴תʱ��1-100ms 2-150ms... 17-900ms
	uint8_t setOpenDoorState;               //0x50 - ��������
	uint8_t setDoorStateFlag;               //��ǰ״̬
    unsigned int chk;    
}__attribute__((packed)) ST_STORAGE_BLE_CONFIG_LINK;


void BleRecordSet(eModeCtrl moid, uint16_t index);
void BleRecordInfoReset(void);
void BleRecordUpdata(void);
void BleProductInfoInit(void);
int bleRecordGet(uint8_t *data, uint8_t *len, uint8_t nPage);

void GetFpList(uint8_t* listBuf);
void BleRecordSync(bool bClean);

#endif

