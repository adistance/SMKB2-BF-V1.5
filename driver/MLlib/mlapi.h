/*
 * mlapi.h
 *
 *  Created on: 2020��9��24��
 *      Author: sks
 */

#ifndef MLAPI_H_
#define MLAPI_H_

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#include "rtl876x_lib_platform.h"

#define MLAPI_DEBUG_EN                  1

#ifndef MLAPI_DEBUG_EN
    #define MLAPI_DEBUG_EN  0
#endif

#if (MLAPI_DEBUG_EN == 1)
#define MLAPI_PRINTF(x)        Printf x
extern void Printf(char *fmt,...);
extern void dumpBuff(unsigned char *pBuff, unsigned int len);
#else
#define MLAPI_PRINTF(x)
#endif


/*****************************************************************************
 ��������  : API���صĴ�����
 �޸���ʷ      :
  1.��    ��   : 2019��08��20��
    ��    ��   : Alex
*****************************************************************************/
#define COMP_CODE_OK                            (0x00)  //��������
#define COMP_CODE_UNKOWN_CMD                    (0x01)  //δ֪����
#define COMP_CODE_CMD_DATA_LEN_ERROR            (0x02)  //�����ֶγ��ȷǷ�
#define COMP_CODE_CMD_DATA_ERROR                (0x03)  //�����ֶηǷ�
#define COMP_CODE_CMD_NOT_FINISHED              (0x04)  //��ǰ����������ִ�У�������������
#define COMP_CODE_NO_REQ_CMD                    (0x05)  //û�з��͸���������󣬾Ͳ�ѯ���
#define COMP_CODE_SYS_SOFT_ERROR                (0x06)  //ϵͳ����ϱ�����
#define COMP_CODE_HARDWARE_ERROR                (0x07)  //Ӳ������
#define COMP_CODE_NO_FINGER_DECTECT             (0x08)  //û�м�⵽��ָ��ѹ����ʱ�˳�
#define COMP_CODE_FINGER_EXTRACT_ERROR          (0x09)  //ָ����ȡ�������󣬿���ԭ��:ͼ���������ѵ�
#define COMP_CODE_FINGER_MATCH_ERROR            (0x0A)  //ָ��ƥ�䷢�����󣬿���ԭ��:û��ָ��
#define COMP_CODE_STORAGE_IS_FULL               (0x0B)  //�洢�ռ���
#define COMP_CODE_STORAGE_WRITE_ERROR           (0x0C)  //�洢д��ʧ��
#define COMP_CODE_STORAGE_READ_ERROR            (0x0D)  //�洢��ȡʧ��
#define COMP_CODE_UNQUALIFIED_IMAGE_ERROR       (0x0E)  //�ɼ���ָ��ͼ�񲻺ϸ�
#define COMP_CODE_STORAGE_REPEAT_FINGERPRINT    (0x0F)  //�ظ�ָ��
#define COMP_CODE_IMAGE_LOW_COVERAGE_ERROR      (0x10)  //��ͼ���С
#define COMP_CODE_CAPTURE_LARGE_MOVE            (0x11)  //�ƶ���Χ����
#define COMP_CODE_CAPTURE_NO_MOVE               (0x12)  //�ƶ���Χ��С
#define COMP_CODE_STORAGE_REPEAT_FP_INDEX_ERROR (0x13)  //ָ�������ű�ռ��
#define COMP_CODE_CAPTURE_IMAGE_FAIL            (0x14)  //��ͼʧ��
#define COMP_CODE_FORCE_QUIT                    (0x15)  //ǿ���˳�
#define COMP_CODE_NONE_UPDATE                   (0x16)  //û�и���
#define COMP_CODE_INVALID_FINGERPRINT_ID        (0x17)  //��Чָ��ID
#define COMP_CODE_ADJUST_GAIN_ERROR             (0x18)  //�Զ��������ʧ��
#define COMP_CODE_DATA_BUFFER_OVERFLOW          (0x19)  //���ݻ��������
#define COMP_CODE_CURRENT_SENSOR_SLEEP          (0x1A)  //��ǰ��������
#define COMP_CODE_PASSWORD_ERROR                (0x1B)  //У���������
#define COMP_CODE_CHECKSUM_ERROR                (0x1C)  //У��ʹ���
#define COMP_CODE_FINGER_PRESENT                (0x20)  //��ָ��λ��������ʧ�ܣ���ָŲ�����ٴγ��Խ�������
#define COMP_CODE_PARAMETER_ERROR               (0x21)  //��������
#define COMP_CODE_READ_FTR_ERROR                (0x22)  //��FTR����
#define COMP_CODE_FTR_CRC_ERR                   (0x23)  //FTRУ�����
#define COMP_CODE_FLASH_ID_ERROR                (0x24)  //�ⲿFlash ID����
#define COMP_CODE_FLASH_ADDR_ERROR              (0x25)  //�ⲿFlash ��ַ����
#define COMP_CODE_FLASH_LEN_ERROR               (0x26)  //�ⲿFlash ���ȴ���
#define COMP_CODE_GET_SHARE_MEMORY_ERROR        (0x27)  //��ȡ�����ڴ����
#define COMP_CODE_SENSOR_SELF_CHECK_ERROR       (0x28)  //�������Լ�ʧ��
#define COMP_CODE_LED_RESP_ERROR                (0x29)  //LEDӦ�����
#define COMP_CODE_ENROLL_LOW_MOISTNESS          (0x2A)  //ʪ��ָ
#define COMP_CODE_FLASH_RETRY_ERROR             (0x2B)
#define COMP_CODE_OTHER_ERROR                   (0xFF)  //��������

#define ERASE_SINGLE_FINGER                     (0)     //����ɾ��
#define ERASE_ALL_FINGER                        (1)     //ȫ��ɾ��
#define ERASE_BATCH_FINGER                      (2)     //����ɾ��
#define ERASE_BLOCK_FINGER                      (3)     //��ɾ��
#define ERASE_NUMBER_FINGER						(3)		//ɾ����Ա�µ�����ָ��


typedef struct 
{
    uint8_t ucEnrollNum;                        //ע�����
    uint8_t ucIsSelfLearn;                      //��ѧϰ��־
    uint8_t ucIsRepeatCheck;                    //�ظ����
    uint8_t ucIsMergeControl;                   //ƴ������
    uint8_t ucIs360Identify;                    //360��ʶ��
    uint8_t ucReserve[7];                       //Ԥ��
}ST_SYS_PARA;

typedef struct 
{
    uint16_t usAlgoMainVer;                     //�㷨����ţ�        4λ������ 3.2.9��Ӧ3209
    uint16_t usAlgoSubVer;                      //�㷨�Ӱ�ţ�        4λ������ 0901��Ӧ901
    uint16_t usDriverVer;                       //������汾�ţ�3λ�����磺100
    uint8_t ucReserve[2];                       //Ԥ��
}ST_FP_VER,*PST_FP_VER;

typedef enum {
    SLEEP_NORMAL_MODE = 0,
    SLEEP_DEEP_MODE,
} SLEEP_MODE_T;

/**************mlapi.c�����ӿ�**********/

/*****************************************************************************
 �� �� ��: MLAPI_SystemInit
 ��������  : ϵͳ��ʼ��
 �������  : ST_SYS_PARA stSysPara
 �������  : ��
 �� �� ֵ: uint32_t 32 bits Error Code
 ���ú���  :
 ��������  :

 �޸���ʷ    :
  1.��    ��  : 2019��08��20��
    ��    ��  : Alex
    �޸�����   : �����ɺ���

*****************************************************************************/
extern uint32_t MLAPI_SystemInit(void);


/*****************************************************************************
 �� �� ��: MLAPI_Enrollment
 ��������  : ע��ָ������
 �������  : uint8_t uEnrollNum
            **��ǰע�����
 �������  : uint16_t *uUnuseId
            **return COMP_CODE_OK���ȡ��ǰ��Сδʹ��ID
            **return COMP_CODE_STORAGE_REPEAT_FINGERPRINT���ȡ���ظ�ID
            uint8_t *uProgress ע�����
            uint16_t usTimeout ��ʱʱ��
            **return ok���ȡ��ǰע�����

 �� �� ֵ: uint32_t 32 bits Error Code
 ���ú���  :
 ��������  :

 �޸���ʷ    :
  1.��    ��  : 2019��08��20��
    ��    ��  : Alex
    �޸�����   : �����ɺ���
*****************************************************************************/
extern uint32_t MLAPI_Enrollment(uint8_t uEnrollNum, uint16_t *uUnuseId, uint8_t *uProgress, uint16_t usTimeout);

/*****************************************************************************
 �� �� ��: MLAPI_StorageFtr
 ��������  : ����ָ������
 �������  : uint16_t uStorageId
            **��Ҫ�����ID
 �������  : ��
 �� �� ֵ: uint32_t 32 bits Error Code
 ���ú���  :
 ��������  :

 �޸���ʷ  :
  1.��    ��  : 2019��08��20��
    ��    ��  : Alex
    �޸�����   : �����ɺ���

*****************************************************************************/
extern uint32_t MLAPI_StorageFtr(uint16_t uStorageId, uint16_t u16NumId);

/*****************************************************************************
 �� �� ��  : MLAPI_Match
 ��������  : ָ��ƥ��
 �������  : uint32_t usTimeout
            **��ָ��ѹ��ʱʱ��1000Ϊ1s
 �������  : uint16_t *uResult
            **��ȡƥ���� 1:�ɹ� 2��ʧ��
            uint16_t *uScore
            **��ȡƥ�����
            uint16_t *uMatchId
            **��ȡƥ�䵽��ID
 �� �� ֵ  : uint32_t 32 bits Error Code
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2019��08��20��
    ��    ��   : Alex
    �޸�����   : �����ɺ���

*****************************************************************************/
extern uint32_t MLAPI_Match(uint16_t *uResult, uint16_t *uScore, uint16_t *uMatchId, uint32_t usTimeout);

/*****************************************************************************
 �� �� ��: MLAPI_DeleteFTR
 ��������  : ���ָ������
 �������  : uint8_t uClearMode
            **���ģʽ 0������ɾ�� 1��ȫ��ɾ�� 2������ɾ��
            uint16_t uFpNum
            **����ɾ����ID����
            uint16_t* uFpIdx
            **����ɾ����һ��ID������ɾ����ID
 �������  : ��
 �� �� ֵ: uint32_t 32 bits Error Code
 ���ú���  :
 ��������  :

 �޸���ʷ  :
  1.��    ��  : 2019��08��20��
    ��    ��  : Alex
    �޸�����   : �����ɺ���

*****************************************************************************/
extern uint32_t MLAPI_DeleteFTR(uint8_t uClearMode, uint16_t uFpNum, uint16_t* uFpIdx);

/*****************************************************************************
 �� �� �� : MLAPI_UpdateFTR
 ��������  : ����ָ�����������ÿ�����ѧʱ����
 �������  : ��
 �������  : ��
 �� �� ֵ: uint32_t 32 bits Error Code

 �޸���ʷ  :
  1.��    ��	: 2019��08��20��
	��    ��	: Alex
	�޸�����   : �����ɺ���

*****************************************************************************/
extern uint32_t MLAPI_UpdateFTR(void);

/****************************************************************************
 �� �� ��: MLAPI_QueryFingerPresent
 ��������  : ��ѯ��ָ��λ״̬
 �������  : ��
 �������  : ��
 �� �� ֵ: 1:��ָ��λ 0����ָ����λ

 �޸���ʷ    :
 1.��    ��  : 2019��11��16��
   ��    ��  : Curry
   �޸�����   : �����ɺ���

*****************************************************************************/
extern uint8_t MLAPI_QueryFingerPresent(void);

/*****************************************************************************
 �� �� ��  : MLAPI_PowerSaving
 ��������  : ��������ģʽ
 �������  : uint8_t uSleepMode
            **0����ͨ���� 1���������
 �������  : ��
 �� �� ֵ  : uint32_t 32 bits Error Code
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2019��08��20��
    ��    ��   : Alex
    �޸�����   : �����ɺ���

*****************************************************************************/
uint32_t MLAPI_PowerSaving(uint8_t uSleepMode);

/*****************************************************************************
 �� �� �� : MLAPI_AbortCommand
 ��������  : �жϵ�ǰ����
 �������  : ��
 �������  : ��
 �� �� ֵ: uint32_t 32 bits Error Code

 �޸���ʷ  :
  1.��    ��	: 2019��08��20��
	��    ��	: Alex
	�޸�����   : �����ɺ���

*****************************************************************************/
extern void MLAPI_AbortCommand(void);


/*****************************************************************************
 �� �� ��: MLAPI_GetFPVersion
 ��������  : ��ȡ�汾��Ϣ
 �������  : ST_FP_VER stVer  �汾��Ϣ
 �������  : PST_FP_VER�ṹ��
 �� �� ֵ: ��
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ    :
  1.��    ��  : 2019��11��16��
    ��    ��  : 
    �޸�����   : �����ɺ���

*****************************************************************************/
extern void MLAPI_GetFPVersion(PST_FP_VER stVer);

/*****************************************************************************
 �� �� ��: MLAPI_GetFTRNum
 ��������  : ��ȡָ����������
 �������  : uint16_t* pusFtrNum,               FTR����     
 �������  : ��
 �� �� ֵ: uint32_t 32 bits Error Code
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ    :
  1.��    ��  : 2019��11��16��
    ��    ��  : Curry
    �޸�����   : �����ɺ���

*****************************************************************************/
extern uint32_t MLAPI_GetFTRNum(uint16_t* pusFtrNum);

/*****************************************************************************
 �� �� ��: MLAPI_GetShareMemory
 ��������  : ��ȡ�����ڴ�
 �������  : uint8_t **ppMem,  ���ڷ���mem��ַ
           uint32_t *pLen,   ���ڷ���mem����
 �������  : ��
 �� �� ֵ: uint32_t 32 bits Error Code

 �޸���ʷ    :
 1.��    ��  : 2019��11��16��
   ��    ��  :
   �޸�����   : �����ɺ���

*****************************************************************************/
extern void* MLAPI_GetShareMemory(int nSize);

/*****************************************************************************
 �� �� ��: MLAPI_FreeShareMemory
 ��������  : �ͷŹ����ڴ�
 �������  : ��
 �������  : ��
 �� �� ֵ: uint32_t 32 bits Error Code

 �޸���ʷ    :
 1.��    ��  : 2019��11��16��
   ��    ��  :
   �޸�����   : �����ɺ���

*****************************************************************************/
extern uint32_t MLAPI_FreeShareMemory(void *pBuff);

/*****************************************************************************
 �� �� ��: MLAPI_SetAdminCount
 ��������  : ���ù���Ա��
 �������  : uint16_t count�� ����Ա����0-99��ָ����������100ö��
 �������  : ��
 �� �� ֵ: uint32_t 32 bits Error Code
         COMP_CODE_CMD_DATA_ERROR:�����쳣
 �޸���ʷ    :
 1.��    ��  : 2020��1111��146��
   ��    ��  :joho
   �޸�����   : �����ɺ���

*****************************************************************************/
extern uint32_t MLAPI_SetAdminCount(uint16_t count);

/**********************************************************************/
/**************�ڲ�ʹ�ýṹ��**********************************/
/**********************************************************************/
typedef struct{
    uint32_t pwd;
    uint8_t cmd1;
    uint8_t cmd2;
    uint32_t len;       //req�ε����ݳ���
    uint8_t req[136];     //req����
}ML_CMD_REQ_DATA, *P_ML_CMD_REQ_DATA;

typedef struct{
    uint8_t flag;      //��Ӧ��Ч��־��ȡ������֮�����
    uint32_t pwd;
    uint8_t ack1;
    uint8_t ack2;
    uint32_t comp_code;
    uint32_t len;       //��Ӧ���ݳ���
    uint8_t *resp;      //resp��������
}ML_CMD_RESP_DATA, *P_ML_CMD_RESP_DATA;

uint32_t MLAPI_EditFP(uint8_t uEnrollNum, uint16_t *uUnuseId, uint8_t *uProgress, uint16_t usTimeout);


/**********************************************************************/
/***************��Ҫ�ⲿʵ�ֵĽӿ�***************************/
/**********************************************************************/

/**************sensor.c*********************/
extern void Sensor_PinInit(void);
extern void Sensor_Reset(void);
extern uint8_t SensorIntValueGet(void);

/**************delay.c�����ӿ�**********/
extern void delay_us(unsigned int us);
extern void delay_ms(unsigned int ms);
extern uint32_t GetSysTick(void);

/***************protocol.c(����������ʱ����)**************************/
extern volatile uint8_t g_recvFlag;
//extern void USART_SendData(uint8_t ch);
extern void USART_SendData(uint8_t *data, uint16_t vCount);
extern void USART_RecvData(uint8_t data);
extern void protocol_ReqProc(void);
extern void ML_COMMAND_Task(void);


/**********************************************************************/
/***************�������ⲿʵ�ֵĽӿ�***********************/
/**********************************************************************/

/*******************spi�Ĳ����ӿ�ʵ��************************************/
extern uint8_t spiInit(uint8_t t_mode);
extern void SensorRSTControl(bool status);
extern void spiFlashCSControl(unsigned char status);
extern void spiSensorCSControl(unsigned char status);
extern uint8_t spiTransWait(uint32_t delay_time);
extern uint8_t spiSensorTransfer(unsigned char *txBuf, unsigned char *rxBuf, unsigned int len, bool leave_cs_asserted);
extern uint8_t spiFlashTransfer(unsigned char *txBuf, unsigned char *rxBuf, unsigned int len, bool leave_cs_asserted);
extern uint8_t spiFlashTransfer_match(unsigned char *txBuf, unsigned char *rxBuf, unsigned int len);
extern uint8_t spiFlashTransfer_match_wait(void);

/******************flash�Ĳ����ӿ�ʵ��*********************/
typedef enum 
{
    EM_FLASH_CTRL_INSIDE    = 0,                    //�ڲ�FLASH
    EM_FLASH_CTRL_OUTSIDE   = 1,                     //�ⲿFLASH
    EM_FLASH_CTRL_FTL		= 2,			//�ڲ�FLASH�������Ǳ�����FTL�Ŀռ��ڣ�ֻ��3056�ֽڴ�С
}EM_FLASH_CTRL_MODE;

//extern uint8_t flashInit(void);
//extern uint8_t flashReadBuffer(uint8_t handle, uint8_t *readBuffer, uint32_t readAddress, uint32_t readLength);
//extern uint8_t flashWriteBuffer(uint8_t handle, uint8_t *writeBuffer,  uint32_t writeAddress, uint32_t writeLength);

//extern bool ExFlash_ReadFlashID(void);
//extern void ExFlashReadPage(unsigned char *readBuffer, unsigned int readAddress, unsigned int readLength);
//extern unsigned int ExFlashEarse(unsigned int earseAddress, unsigned char earseNum);
//extern unsigned int ExFlashWriteBuffer(unsigned char *writeBuffer, unsigned int writeAddress, unsigned int writeLength);
//extern unsigned int ExFlasReadPageMatch(unsigned char *readBuffer, unsigned int readAddress, unsigned int readLength);
//extern unsigned int ExFlashReadPageWait(void);

extern uint8_t flashReadBuffer(unsigned char *readBuffer, unsigned int address, unsigned int readLength, EM_FLASH_CTRL_MODE mode);
extern uint8_t flashWriteBuffer(unsigned char *writeBuffer, unsigned int address, unsigned int writeLength, EM_FLASH_CTRL_MODE mode);


#define TEST_FPS_SPEED   1
#if defined TEST_FPS_SPEED
    #if(TEST_FPS_SPEED == 1)
        extern void timerStart(char step);
        extern uint32_t timerEnd(void);
    #else

    #endif
#else
    #error "not define TEST_FPS_SPEED!!!"
#endif

#endif /* MLAPI_H_ */
