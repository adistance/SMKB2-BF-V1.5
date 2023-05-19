#ifndef __SYSTEM_SETTING__
#define __SYSTEM_SETTING__

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "filesys.h"
#include "action.h"

#define SETTING_USE_DEFAULT(stoStat, updFlag)   \
            (((stoStat) != SETTING_STORAGE_OK)  \
            || ((updFlag) != CONTENT_USE_UPDATE))
            
#define CONTENT_USE_DEFAULT                     (0x00)
#define CONTENT_USE_UPDATE                      (0x55)

#define UART_BAUD_RATE_9600                     (9600)
#define UART_BAUD_RATE_19200                    (19200)
#define UART_BAUD_RATE_38400                    (38400)
#define UART_BAUD_RATE_57600                    (57600)
#define UART_BAUD_RATE_115200                   (115200)

#define DEFAULT_BOARD_ID                        "SMKB2-BF-0101"   //16Bytes			"9318_FP_zdxm0101"

#define DEFAULT_PROTO_PWD                       (0x00000000)
#define DEFAULT_BAUD_RATE                       UART_BAUD_RATE_115200
#define DEFAULT_LOADER_BAUD_RATE                UART_BAUD_RATE_115200

#define DEFAULT_FPSEN_SHIFT                     (16)
#define DEFAULT_FPSEN_GAIN                      (8)
#define DEFAULT_FPSEN_PXLCTRL                   (7)

#define DEFAULT_MATCH_THRESHOLD                 (4600)
#define DEFAULT_MERGE_THRESHOLD                 (3000)

#define ICN7000_DETECT_FP_ADC_SHIFT             (10)
#define ICN7000_DETECT_FP_ADC_GAIN              (4)
#define ICN7000_DETECT_FP_ADC_PXLCTRL           (16)


#define DEFAULT_ALG_QULITY_LEVEL                (2)
#define DEFAULT_ALG_COVERAGE_RATE               (50)
#define DEFAULT_ALG_ENROLL_NUM                  (6) //默认6次注册

#define DEFAULT_SYSTEM_POLICY                   (SYS_POLICY_REPEAT_CHECK  | SYS_POLICY_SELF_LEARN | SYS_POLICY_360_IDENTIFY)

#define DEFAULT_SY_CHIP_ADDRESS					(0xffffffff)
#define DEFAULT_FREQUENCY_CAILBRATION_VALUE		(0) //(((RCU_CTL >> 3) & 0x1f))

#define DEFAULT_PIN_VALUE						(0x00000000)
#define DEFAULT_PIN_ERROR_TIMES					(0)

#define HARDWARE_POLICY_TOUCH                   (0)
#define HARDWARE_POLICY_LED                     (0)
#define HARDWARE_POLICY_DX81_ON                 (0)
#define HARDWARE_POLICY_STARTUP_ON              (1)

#define BLUETOOTH_CLOSE							(0) //关闭蓝牙功能

typedef struct ftr_msg_info
{
    unsigned char ftrIndex;
    unsigned char useflag;
}__attribute__((packed))FTR_MSG_INFO;

typedef struct pic_msg_info  
{
    unsigned char matchNum;
    unsigned char matchIndex;
    unsigned char enrollNum;
    unsigned char ftrNum;
    FTR_MSG_INFO FtrInfo[8];
    FTR_MSG_INFO UpdateInfo[10];
    unsigned char updateNum;
}__attribute__((packed))PIC_MSG_INFO, *PPIC_MSG_INFO;

#define PIC_INFO_SIZE       (sizeof(PIC_MSG_INFO))

typedef struct {
    uint8_t head[4];
    uint8_t ver[4];
    
    //板名
    uint8_t board_id_update;
    uint8_t board_id[32];   //板名+SN号，前面16个字节是板名，后面16个字节是SN号

    //通信协议密码
    uint8_t pwd_update;
    uint32_t pwd;

    //串口波特率
    uint8_t uart_baudrate_update;
    uint32_t uart_baudrate;
    uint32_t loader_uart_baudrate;

    //指纹传输器增益
    uint8_t fp_gain_update;
    uint8_t fp_shift;
    uint8_t fp_gain;
    uint8_t fp_pxlctrl;

    //指纹匹配分数
    uint8_t fp_score_update;
    uint16_t fp_score;
    uint16_t fp_IsSingleScore;

    //在位检测增益值
    uint8_t fp_detect_shift;
    uint8_t fp_detect_gain;
    uint8_t fp_detect_pxlctrl;

    //算法参数值
    uint8_t alg_para_update;
    uint8_t alg_quality_level;
    uint8_t alg_coverage_rate;
    uint8_t alg_enroll_num;

    //系统策略
    uint8_t sys_policy_update;
    uint32_t sys_policy;

	//晟元芯片地址
	uint8_t chip_address_update;
	uint32_t chip_address;

	//主频校准参数
	uint8_t frequency_cailbration_value_update;
	uint8_t frequency_cailbration_value;

	//PIN码
	uint8_t pin_value_update;
	uint32_t pin_value_h;
	uint32_t pin_value_l;
	uint8_t pin_error_times;
    
    //系统序列号
	uint8_t board_serial_number_update;
	uint8_t board_serial_number[24];   
    /*{ 0x31, 0x33, 0x07, 0x12, 0x08, 0x6F, 0x53, 0x03, 0xF2, 0x70, 0xDE, 0x6B, 
        0x2B, 0x3E, 0x53, 0xAE, 0x16, 0x49, 0xB3, 0x38, 0xCB, 0xEF, 0x28, 0x37};*/

    //TZ密码—泽谚用
    uint8_t tz_password_update;
	uint32_t tz_password;

    //ML协议rootKey—云丁用
    uint8_t YD_rootkey_update;
    uint8_t YD_rootkey[4];
    
    //MCU UID
	uint8_t StartUpCheck_update;
    uint8_t StartUpCheck[4];    
    		
    uint8_t usbkey_clear;                               //清空
    uint8_t usbkey_password[8];
	
    uint8_t rsv[113];
    
    uint8_t chksum;
}__attribute__((packed)) SYS_SETTING, *PSYS_SETTING;

typedef enum
{
	E_PRODUCT_id,
	E_ROOTKEY,
	E_MAX,
}eSerialType;


extern SYS_SETTING g_sysSetting;
uint8_t SYSSET_SetUartBaudRate(uint32_t baudrate);
unsigned char SYSSET_SetChipAddress(const unsigned int address);
uint8_t SYSSET_SetEnrollNum(uint32_t unEnrollNum);
uint8_t SYSSET_SetBoardId(uint8_t *p, uint32_t len);
uint8_t SYSSET_SetSystemPolicy(uint32_t policy);
uint8_t SYSSET_SetBoardSerialNumber(uint8_t *p, uint32_t len);
uint8_t SYSSET_UpdateSystemPara(unsigned char *p);
uint8_t SYSSET_GetSystemPolicy(void);
void SYSSET_Init(void);
uint8_t SYSSET_GetBoardSerialNumber(uint8_t *out, eSerialType type);
uint8_t SYSSET_GetChksum(uint8_t *p, uint32_t num);
uint8_t SYSSET_SetProtoPwd(uint32_t pwd);
uint8_t SYSSET_GetEnrollNum(void);
void SYSSET_SyncToFlash(void);


#ifdef __cplusplus
}
#endif

#endif
