/*
 * mlapi.h
 *
 *  Created on: 2020年9月24日
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
 功能描述  : API返回的错误码
 修改历史      :
  1.日    期   : 2019年08月20日
    作    者   : Alex
*****************************************************************************/
#define COMP_CODE_OK                            (0x00)  //正常返回
#define COMP_CODE_UNKOWN_CMD                    (0x01)  //未知命令
#define COMP_CODE_CMD_DATA_LEN_ERROR            (0x02)  //请求字段长度非法
#define COMP_CODE_CMD_DATA_ERROR                (0x03)  //请求字段非法
#define COMP_CODE_CMD_NOT_FINISHED              (0x04)  //当前有命令正在执行，不接受新命令
#define COMP_CODE_NO_REQ_CMD                    (0x05)  //没有发送该命令的请求，就查询结果
#define COMP_CODE_SYS_SOFT_ERROR                (0x06)  //系统软件上报错误
#define COMP_CODE_HARDWARE_ERROR                (0x07)  //硬件错误
#define COMP_CODE_NO_FINGER_DECTECT             (0x08)  //没有检测到手指按压，超时退出
#define COMP_CODE_FINGER_EXTRACT_ERROR          (0x09)  //指纹提取发生错误，可能原因:图像质量不佳等
#define COMP_CODE_FINGER_MATCH_ERROR            (0x0A)  //指纹匹配发生错误，可能原因:没有指纹
#define COMP_CODE_STORAGE_IS_FULL               (0x0B)  //存储空间满
#define COMP_CODE_STORAGE_WRITE_ERROR           (0x0C)  //存储写入失败
#define COMP_CODE_STORAGE_READ_ERROR            (0x0D)  //存储读取失败
#define COMP_CODE_UNQUALIFIED_IMAGE_ERROR       (0x0E)  //采集的指纹图像不合格
#define COMP_CODE_STORAGE_REPEAT_FINGERPRINT    (0x0F)  //重复指纹
#define COMP_CODE_IMAGE_LOW_COVERAGE_ERROR      (0x10)  //采图面积小
#define COMP_CODE_CAPTURE_LARGE_MOVE            (0x11)  //移动范围过大
#define COMP_CODE_CAPTURE_NO_MOVE               (0x12)  //移动范围过小
#define COMP_CODE_STORAGE_REPEAT_FP_INDEX_ERROR (0x13)  //指纹索引号被占用
#define COMP_CODE_CAPTURE_IMAGE_FAIL            (0x14)  //采图失败
#define COMP_CODE_FORCE_QUIT                    (0x15)  //强制退出
#define COMP_CODE_NONE_UPDATE                   (0x16)  //没有更新
#define COMP_CODE_INVALID_FINGERPRINT_ID        (0x17)  //无效指纹ID
#define COMP_CODE_ADJUST_GAIN_ERROR             (0x18)  //自动增益调整失败
#define COMP_CODE_DATA_BUFFER_OVERFLOW          (0x19)  //数据缓冲区溢出
#define COMP_CODE_CURRENT_SENSOR_SLEEP          (0x1A)  //当前正在休眠
#define COMP_CODE_PASSWORD_ERROR                (0x1B)  //校验密码错误
#define COMP_CODE_CHECKSUM_ERROR                (0x1C)  //校验和错误
#define COMP_CODE_FINGER_PRESENT                (0x20)  //手指在位导致休眠失败，手指挪开后将再次尝试进入休眠
#define COMP_CODE_PARAMETER_ERROR               (0x21)  //参数错误
#define COMP_CODE_READ_FTR_ERROR                (0x22)  //读FTR错误
#define COMP_CODE_FTR_CRC_ERR                   (0x23)  //FTR校验错误
#define COMP_CODE_FLASH_ID_ERROR                (0x24)  //外部Flash ID错误
#define COMP_CODE_FLASH_ADDR_ERROR              (0x25)  //外部Flash 地址错误
#define COMP_CODE_FLASH_LEN_ERROR               (0x26)  //外部Flash 长度错误
#define COMP_CODE_GET_SHARE_MEMORY_ERROR        (0x27)  //获取共享内存错误
#define COMP_CODE_SENSOR_SELF_CHECK_ERROR       (0x28)  //传感器自检失败
#define COMP_CODE_LED_RESP_ERROR                (0x29)  //LED应答错误
#define COMP_CODE_ENROLL_LOW_MOISTNESS          (0x2A)  //湿手指
#define COMP_CODE_FLASH_RETRY_ERROR             (0x2B)
#define COMP_CODE_OTHER_ERROR                   (0xFF)  //其他错误

#define ERASE_SINGLE_FINGER                     (0)     //单个删除
#define ERASE_ALL_FINGER                        (1)     //全部删除
#define ERASE_BATCH_FINGER                      (2)     //批量删除
#define ERASE_BLOCK_FINGER                      (3)     //块删除
#define ERASE_NUMBER_FINGER						(3)		//删除成员下的所有指纹


typedef struct 
{
    uint8_t ucEnrollNum;                        //注册次数
    uint8_t ucIsSelfLearn;                      //自学习标志
    uint8_t ucIsRepeatCheck;                    //重复检查
    uint8_t ucIsMergeControl;                   //拼接限制
    uint8_t ucIs360Identify;                    //360°识别
    uint8_t ucReserve[7];                       //预留
}ST_SYS_PARA;

typedef struct 
{
    uint16_t usAlgoMainVer;                     //算法主板号，        4位，例如 3.2.9对应3209
    uint16_t usAlgoSubVer;                      //算法子板号，        4位，例如 0901对应901
    uint16_t usDriverVer;                       //驱动库版本号，3位，例如：100
    uint8_t ucReserve[2];                       //预留
}ST_FP_VER,*PST_FP_VER;

typedef enum {
    SLEEP_NORMAL_MODE = 0,
    SLEEP_DEEP_MODE,
} SLEEP_MODE_T;

/**************mlapi.c操作接口**********/

/*****************************************************************************
 函 数 名: MLAPI_SystemInit
 功能描述  : 系统初始化
 输入参数  : ST_SYS_PARA stSysPara
 输出参数  : 无
 返 回 值: uint32_t 32 bits Error Code
 调用函数  :
 被调函数  :

 修改历史    :
  1.日    期  : 2019年08月20日
    作    者  : Alex
    修改内容   : 新生成函数

*****************************************************************************/
extern uint32_t MLAPI_SystemInit(void);


/*****************************************************************************
 函 数 名: MLAPI_Enrollment
 功能描述  : 注册指纹特征
 输入参数  : uint8_t uEnrollNum
            **当前注册次数
 输出参数  : uint16_t *uUnuseId
            **return COMP_CODE_OK后获取当前最小未使用ID
            **return COMP_CODE_STORAGE_REPEAT_FINGERPRINT后获取的重复ID
            uint8_t *uProgress 注册进度
            uint16_t usTimeout 超时时间
            **return ok后获取当前注册进度

 返 回 值: uint32_t 32 bits Error Code
 调用函数  :
 被调函数  :

 修改历史    :
  1.日    期  : 2019年08月20日
    作    者  : Alex
    修改内容   : 新生成函数
*****************************************************************************/
extern uint32_t MLAPI_Enrollment(uint8_t uEnrollNum, uint16_t *uUnuseId, uint8_t *uProgress, uint16_t usTimeout);

/*****************************************************************************
 函 数 名: MLAPI_StorageFtr
 功能描述  : 保存指纹特征
 输入参数  : uint16_t uStorageId
            **需要保存的ID
 输出参数  : 无
 返 回 值: uint32_t 32 bits Error Code
 调用函数  :
 被调函数  :

 修改历史  :
  1.日    期  : 2019年08月20日
    作    者  : Alex
    修改内容   : 新生成函数

*****************************************************************************/
extern uint32_t MLAPI_StorageFtr(uint16_t uStorageId, uint16_t u16NumId);

/*****************************************************************************
 函 数 名  : MLAPI_Match
 功能描述  : 指纹匹配
 输入参数  : uint32_t usTimeout
            **手指按压超时时间1000为1s
 输出参数  : uint16_t *uResult
            **获取匹配结果 1:成功 2：失败
            uint16_t *uScore
            **获取匹配分数
            uint16_t *uMatchId
            **获取匹配到的ID
 返 回 值  : uint32_t 32 bits Error Code
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2019年08月20日
    作    者   : Alex
    修改内容   : 新生成函数

*****************************************************************************/
extern uint32_t MLAPI_Match(uint16_t *uResult, uint16_t *uScore, uint16_t *uMatchId, uint32_t usTimeout);

/*****************************************************************************
 函 数 名: MLAPI_DeleteFTR
 功能描述  : 清除指纹特征
 输入参数  : uint8_t uClearMode
            **清除模式 0：单个删除 1：全部删除 2：批量删除
            uint16_t uFpNum
            **批量删除的ID个数
            uint16_t* uFpIdx
            **单个删除的一个ID或批量删除的ID
 输出参数  : 无
 返 回 值: uint32_t 32 bits Error Code
 调用函数  :
 被调函数  :

 修改历史  :
  1.日    期  : 2019年08月20日
    作    者  : Alex
    修改内容   : 新生成函数

*****************************************************************************/
extern uint32_t MLAPI_DeleteFTR(uint8_t uClearMode, uint16_t uFpNum, uint16_t* uFpIdx);

/*****************************************************************************
 函 数 名 : MLAPI_UpdateFTR
 功能描述  : 更新指纹特征，配置开启自学时调用
 输入参数  : 无
 输出参数  : 无
 返 回 值: uint32_t 32 bits Error Code

 修改历史  :
  1.日    期	: 2019年08月20日
	作    者	: Alex
	修改内容   : 新生成函数

*****************************************************************************/
extern uint32_t MLAPI_UpdateFTR(void);

/****************************************************************************
 函 数 名: MLAPI_QueryFingerPresent
 功能描述  : 查询手指在位状态
 输入参数  : 无
 输出参数  : 无
 返 回 值: 1:手指在位 0：手指不在位

 修改历史    :
 1.日    期  : 2019年11月16日
   作    者  : Curry
   修改内容   : 新生成函数

*****************************************************************************/
extern uint8_t MLAPI_QueryFingerPresent(void);

/*****************************************************************************
 函 数 名  : MLAPI_PowerSaving
 功能描述  : 设置休眠模式
 输入参数  : uint8_t uSleepMode
            **0：普通休眠 1：深度休眠
 输出参数  : 无
 返 回 值  : uint32_t 32 bits Error Code
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2019年08月20日
    作    者   : Alex
    修改内容   : 新生成函数

*****************************************************************************/
uint32_t MLAPI_PowerSaving(uint8_t uSleepMode);

/*****************************************************************************
 函 数 名 : MLAPI_AbortCommand
 功能描述  : 中断当前操作
 输入参数  : 无
 输出参数  : 无
 返 回 值: uint32_t 32 bits Error Code

 修改历史  :
  1.日    期	: 2019年08月20日
	作    者	: Alex
	修改内容   : 新生成函数

*****************************************************************************/
extern void MLAPI_AbortCommand(void);


/*****************************************************************************
 函 数 名: MLAPI_GetFPVersion
 功能描述  : 获取版本信息
 输入参数  : ST_FP_VER stVer  版本信息
 输出参数  : PST_FP_VER结构体
 返 回 值: 无
 调用函数  : 
 被调函数  : 
 
 修改历史    :
  1.日    期  : 2019年11月16日
    作    者  : 
    修改内容   : 新生成函数

*****************************************************************************/
extern void MLAPI_GetFPVersion(PST_FP_VER stVer);

/*****************************************************************************
 函 数 名: MLAPI_GetFTRNum
 功能描述  : 获取指纹特征数量
 输入参数  : uint16_t* pusFtrNum,               FTR数量     
 输出参数  : 无
 返 回 值: uint32_t 32 bits Error Code
 调用函数  : 
 被调函数  : 
 
 修改历史    :
  1.日    期  : 2019年11月16日
    作    者  : Curry
    修改内容   : 新生成函数

*****************************************************************************/
extern uint32_t MLAPI_GetFTRNum(uint16_t* pusFtrNum);

/*****************************************************************************
 函 数 名: MLAPI_GetShareMemory
 功能描述  : 获取共享内存
 输入参数  : uint8_t **ppMem,  用于返回mem地址
           uint32_t *pLen,   用于返回mem长度
 输出参数  : 无
 返 回 值: uint32_t 32 bits Error Code

 修改历史    :
 1.日    期  : 2019年11月16日
   作    者  :
   修改内容   : 新生成函数

*****************************************************************************/
extern void* MLAPI_GetShareMemory(int nSize);

/*****************************************************************************
 函 数 名: MLAPI_FreeShareMemory
 功能描述  : 释放共享内存
 输入参数  : 无
 输出参数  : 无
 返 回 值: uint32_t 32 bits Error Code

 修改历史    :
 1.日    期  : 2019年11月16日
   作    者  :
   修改内容   : 新生成函数

*****************************************************************************/
extern uint32_t MLAPI_FreeShareMemory(void *pBuff);

/*****************************************************************************
 函 数 名: MLAPI_SetAdminCount
 功能描述  : 设置管理员数
 输入参数  : uint16_t count， 管理员数：0-99（指纹总容量：100枚）
 输出参数  : 无
 返 回 值: uint32_t 32 bits Error Code
         COMP_CODE_CMD_DATA_ERROR:传参异常
 修改历史    :
 1.日    期  : 2020年1111月146日
   作    者  :joho
   修改内容   : 新生成函数

*****************************************************************************/
extern uint32_t MLAPI_SetAdminCount(uint16_t count);

/**********************************************************************/
/**************内部使用结构体**********************************/
/**********************************************************************/
typedef struct{
    uint32_t pwd;
    uint8_t cmd1;
    uint8_t cmd2;
    uint32_t len;       //req段的数据长度
    uint8_t req[136];     //req数据
}ML_CMD_REQ_DATA, *P_ML_CMD_REQ_DATA;

typedef struct{
    uint8_t flag;      //响应有效标志，取完数据之后清空
    uint32_t pwd;
    uint8_t ack1;
    uint8_t ack2;
    uint32_t comp_code;
    uint32_t len;       //响应数据长度
    uint8_t *resp;      //resp数据所在
}ML_CMD_RESP_DATA, *P_ML_CMD_RESP_DATA;

uint32_t MLAPI_EditFP(uint8_t uEnrollNum, uint16_t *uUnuseId, uint8_t *uProgress, uint16_t usTimeout);


/**********************************************************************/
/***************需要外部实现的接口***************************/
/**********************************************************************/

/**************sensor.c*********************/
extern void Sensor_PinInit(void);
extern void Sensor_Reset(void);
extern uint8_t SensorIntValueGet(void);

/**************delay.c操作接口**********/
extern void delay_us(unsigned int us);
extern void delay_ms(unsigned int ms);
extern uint32_t GetSysTick(void);

/***************protocol.c(如需适配延时工具)**************************/
extern volatile uint8_t g_recvFlag;
//extern void USART_SendData(uint8_t ch);
extern void USART_SendData(uint8_t *data, uint16_t vCount);
extern void USART_RecvData(uint8_t data);
extern void protocol_ReqProc(void);
extern void ML_COMMAND_Task(void);


/**********************************************************************/
/***************公开给外部实现的接口***********************/
/**********************************************************************/

/*******************spi的操作接口实现************************************/
extern uint8_t spiInit(uint8_t t_mode);
extern void SensorRSTControl(bool status);
extern void spiFlashCSControl(unsigned char status);
extern void spiSensorCSControl(unsigned char status);
extern uint8_t spiTransWait(uint32_t delay_time);
extern uint8_t spiSensorTransfer(unsigned char *txBuf, unsigned char *rxBuf, unsigned int len, bool leave_cs_asserted);
extern uint8_t spiFlashTransfer(unsigned char *txBuf, unsigned char *rxBuf, unsigned int len, bool leave_cs_asserted);
extern uint8_t spiFlashTransfer_match(unsigned char *txBuf, unsigned char *rxBuf, unsigned int len);
extern uint8_t spiFlashTransfer_match_wait(void);

/******************flash的操作接口实现*********************/
typedef enum 
{
    EM_FLASH_CTRL_INSIDE    = 0,                    //内部FLASH
    EM_FLASH_CTRL_OUTSIDE   = 1,                     //外部FLASH
    EM_FLASH_CTRL_FTL		= 2,			//内部FLASH，不过是保存在FTL的空间内，只有3056字节大小
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
