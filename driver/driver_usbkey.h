#ifndef _DRIVER_USBKEY_H_
#define _DRIVER_USBKEY_H_
//#include "n32g4fr.h"
#include <rtl876x.h>

#define MCU_LOCK_HAS_SET				                (0x55)
#define MCU_LOCK_NO_SET			     	                (0x66)
#define	ADDRESS_MCU_ID_DATA1							(0x1FFFF7F0)
#define	ADDRESS_MCU_ID_DATA2							(0x1FFFF7F4)
#define	ADDRESS_MCU_ID_DATA3							(0x1FFFF7F8)

#define	ADDRESS_MCU_SYSTEM_MEM_START					(0x1FFFF7F0)
#define	ADDRESS_MCU_SYSTEM_MEM_END						(0x1FFFF7F0 + 12)

#define ADDRESS_MCU_LOCK_DATA_STATE						(ADDR_FLASH_PAGE(48))                 //         96K
#define ADDRESS_USBKEY_INIT_INFO						(ADDRESS_MCU_LOCK_DATA_STATE + 1)             //          
#define USBKEY_INIT_INFO_LENGTH							(0x06)

#define PASS_WORD_NUM                                   (6)
#define USBKEY_MATCH_INFO_NUM							(0)
#define USB_CRC_NUM										(1)

#define USBKEY_NOT_MATCH								(0x55)
#define USBKEY_HAS_MATCH								(0x66)


#define USBKEY_MATCH_ERR                                (0x00)
#define USBKEY_MATCH_SUC                                (0x01)

typedef enum
{
	USB_KEY_NO_MATCH = 0,
	USB_KEY_HAS_MATCH = 1,
	USB_KEY_ERROR = 2
}USB_KEY_MATCH_STATE;

extern uint32_t g_usb_insert;
extern uint8_t g_none;
extern uint32_t g_mcu_match_key;
extern uint8_t g_board_sn[12];
extern uint8_t g_key_word[PASS_WORD_NUM + USB_CRC_NUM];

extern uint8_t DebugFlag,DebugFlag1;
extern uint8_t Debug_read_from_key_data[PASS_WORD_NUM + USB_CRC_NUM];
extern bool bInitFlag;

uint8_t UsbKeyStateJudge(void);
void SaveMcuLockInfo(void);
uint8_t McuHasMatch(void);

void UsbKeyCheck(void);
uint8_t UsbCheckResult(void);
void GetMcuUniqueId(void);////ªÒ»°Œ®“ªID
uint8_t UsbKeyHasMatch(void);
void McuMatchWithUsbKey(void);
int fileSys_getStoreFtrNum(void);
void GetMcuLockInfo(void);
void usbKeyWriteCreate(void);
void usbKeyCleanData(void);


#endif
