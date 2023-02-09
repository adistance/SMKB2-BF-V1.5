#include "driver_usbkey.h"
#include "driver_flash.h"
#include "driver_I2C.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "system_setting.h"
#include "driver_uart.h"
#include "driver_delay.h"
#include "trace.h"
#include "driver_led.h"
#include "rtl876x_gpio.h"
#include "rtl876x_rcc.h"
#include "system_setting.h"
#include "driver_led.h"

uint32_t g_McuLockInfo = 0;
USB_KEY_MATCH_STATE g_usb_key_match_state = USB_KEY_NO_MATCH;
uint8_t g_board_sn[12] = {0};
uint8_t g_key_word[PASS_WORD_NUM + USB_CRC_NUM] = {0};
uint8_t g_mcu_word[PASS_WORD_NUM] = {0};
uint8_t g_mcu_word_temp[7] = {0};
bool AgingTestFlag = false; //老化标志
bool bInitFlag = false; //回复出厂设置标志
bool bKeyOn = false;  //usbkey插上标志
uint32_t g_mcu_match_key = 0;  //插入了正确的usbkey会被置一
uint8_t g_passworld_crc[USB_CRC_NUM] = {0};

uint8_t g_none = 0;
uint8_t g_fixed_value[4] = {1, 1, 1, 1};

uint32_t RebindUsbkeyFlag = 0;

uint8_t DebugFlag = 0, DebugFlag1 = 21;
//uint32_t ftr_num_new = 21;

//uint8_t Debug_read_from_key_data[PASS_WORD_NUM + USB_CRC_NUM] = {0};

uint8_t read_from_key_data[PASS_WORD_NUM + USB_CRC_NUM];


uint8_t UsbKeyStateJudge(void)/*检验USBKEY是否插上，插上返回1*/
{
	uint8_t readByte[PASS_WORD_NUM];
	uint8_t i = 0;
	memset(readByte, 0xcc,sizeof(readByte));
	while((I2C_ReadnByte_2(0xA0,readByte,PASS_WORD_NUM,0x00) == 0) && (i < 7))
	{
		i++;
		delay_us(2000);
	}
	if(i < 7)
	{
		return 1;
	}
	else
	{
		return 0;
	}
		
}


void GetMcuLockInfo(void)/*查看MCU是否配对*/
{
//	flashReadBuffer((uint8_t *)&g_McuLockInfo,ADDRESS_MCU_LOCK_DATA_STATE , 1);
    if((g_sysSetting.usbkey_password[0] != MCU_LOCK_HAS_SET) && (g_sysSetting.usbkey_password[0] != MCU_LOCK_NO_SET))
    {
        g_sysSetting.usbkey_password[0] = MCU_LOCK_NO_SET;
	
    }
	else if(g_sysSetting.usbkey_password[0] == MCU_LOCK_HAS_SET)
	{
	
	}
	
}
void SaveMcuLockInfo(void)/*保存MCU配对的状态*/
{
//	flashReadBuffer( (uint8_t *)&g_McuLockInfo , ADDRESS_MCU_LOCK_DATA_STATE , 1);
	SYSSET_SyncToFlash();
}

//void SaveUsbKeyInitInfo(void)/*保存MCU配对的状态*/
//{
//	flashReadBuffer(g_key_word , ADDRESS_USBKEY_INIT_INFO , USBKEY_INIT_INFO_LENGTH);
//}
//void SaveAllInfo(uint8_t len,uint8_t data[])/*保存MCU配对的状态*/
//{
//	flashReadBuffer(data , ADDRESS_MCU_LOCK_DATA_STATE , len);
//}
//
//void GetAllInfo(uint8_t len,uint8_t data[])/*保存MCU配对的状态*/
//{
//	flashReadBuffer(data , ADDRESS_MCU_LOCK_DATA_STATE , len);
//}


void GetUsbKeyInitInfo(void)
{
	int i = 0;
	for(i = 0;i<PASS_WORD_NUM;i++)
	{
		g_key_word[i] = g_sysSetting.usbkey_password[i+1];
	}
//	flashReadBuffer(g_key_word , ADDRESS_USBKEY_INIT_INFO , USBKEY_INIT_INFO_LENGTH);
}
uint8_t McuHasMatch(void)
{
	GetMcuLockInfo();
	if(g_sysSetting.usbkey_password[0] != MCU_LOCK_HAS_SET)
	{
        DebugFlag = 4;
		return 0;
	}
	else
	{
        DebugFlag = 5;
		return 1;
	}
}

//void GetMcuUniqueId(void)////获取唯一ID
//{
//	memset(g_board_sn,0xff,sizeof(g_board_sn));
//	flashReadBuffer(g_board_sn,ADDRESS_MCU_ID_DATA1,4);
//	flashReadBuffer(&g_board_sn[4],ADDRESS_MCU_ID_DATA2,4);
//	flashReadBuffer(&g_board_sn[8],ADDRESS_MCU_ID_DATA3,4);
//	
//	
//}

void GetUsbKeyCrc(uint8_t write_to_key_data[], uint8_t n)
{
//	uint32_t crc_value = 0;
//	crc_value = CRC32_calc(write_to_key_data,n);
//	 g_passworld_crc[0] =  crc_value & 0x000000ff;
//	 g_passworld_crc[1] =  ((crc_value & 0x0000ff00)>>8);
//	 g_passworld_crc[2] =  ((crc_value & 0x00ff0000)>>16);
//	 g_passworld_crc[3] =  ((crc_value & 0xff000000)>>24);
}
uint8_t UsbKeyHasInit(uint8_t readByte[],uint8_t n,uint8_t match_bit)
{
//	int i;
//	uint8_t temp[USB_CRC_NUM];
//	
//	uint32_t sum = 0;
//	if(match_bit == 0)
//	{
//		for(i = 0; i < PASS_WORD_NUM ; i++)
//		{
//			sum = sum +  readByte[i];
//			sum = sum << 2;
//            printf("%x ", readByte[i]);
//		}
//		temp[0] =  sum & 0x000000ff;
//	 	temp[1] =  ((sum & 0x0000ff00)>>8);
//		temp[2] =  ((sum & 0x00ff0000)>>16);
//	 	temp[3] =  ((sum & 0xff000000)>>24);
//		for(i = 0; i < USB_CRC_NUM; i++)
//		{
//			temp[i] = temp[i]/g_fixed_value[i] + g_fixed_value[i];
//		}
//		for(i = 0; i < USB_CRC_NUM; i++)
//		{
//			if(temp[i] != readByte[USBKEY_MATCH_INFO_NUM + PASS_WORD_NUM + i])
//			{
//				break;
//			}
//		}
//		if(i == USB_CRC_NUM)
//		{
//			return 1;
//		}
//		else
//		{
//			return 0;
//		}
//	}
//	else
//	{
//		if(readByte[0] != USBKEY_NOT_MATCH)
//		{
//		  return 0;
//		}
//		else
//		{
//			for(i = 0; i < (n - USB_CRC_NUM) ; i++)
//			{
//				sum = sum +  readByte[i];
//				sum = sum << 2;
//			}
//			temp[0] =  sum & 0x000000ff;
//	 		temp[1] =  ((sum & 0x0000ff00)>>8);
//			temp[2] =  ((sum & 0x00ff0000)>>16);
//	 		temp[3] =  ((sum & 0xff000000)>>24);
//			for(i = 0; i < USB_CRC_NUM; i++)
//			{
//				temp[i] = (temp[i]/g_fixed_value[i]) + g_fixed_value[i];
//				//printf("temp[%d]: %d",i,temp[i]);
//			}
//			for(i = 0; i < USB_CRC_NUM; i++)
//			{
//				if(temp[i] != readByte[USBKEY_MATCH_INFO_NUM + PASS_WORD_NUM + i])
//				{
//					break;
//				}
//			}
//			if(i == USB_CRC_NUM)
//			{
//				return 1;
//			}
//			else
//			{
//				return 0;
//			}
//		}
//	}
	return 0;
}

uint8_t UsbKeyHasMatch(void)
{
	uint8_t readByte[USBKEY_MATCH_INFO_NUM + PASS_WORD_NUM + USB_CRC_NUM];
	//uint8_t read_id[PASS_WORD_NUM + PASS_WORD_NUM+4];
	uint8_t i = 0;
//	uint32_t crc_value = 0;
//	uint8_t temp[USB_CRC_NUM];
//	memset(temp,0,USB_CRC_NUM);
	memset(readByte, 0xcc,sizeof(readByte));
	I2C_ReadnByte_2(0xA0,readByte,USBKEY_MATCH_INFO_NUM + PASS_WORD_NUM + USB_CRC_NUM,0x00);
	for(i = 0; i < (USBKEY_MATCH_INFO_NUM + PASS_WORD_NUM + USB_CRC_NUM); i++)
	{
		if(readByte[i] != 0xcc)//读成功
		{
			break;
		}
	}
	if(i == USBKEY_MATCH_INFO_NUM + PASS_WORD_NUM + USB_CRC_NUM) //没有读成功
	{
		g_usb_key_match_state = USB_KEY_ERROR;
		g_none = 1;
		return (uint8_t)g_usb_key_match_state;
	}
    
	for(i = 0; i < USBKEY_MATCH_INFO_NUM + PASS_WORD_NUM + USB_CRC_NUM; i++)
	{
		if(readByte[i] != 0xff)
		{
			break;
		}
	}
	if(i == (USBKEY_MATCH_INFO_NUM + PASS_WORD_NUM + USB_CRC_NUM))              //是一块空板
	{
		g_usb_key_match_state = USB_KEY_ERROR;
		g_none = 2;
		return (uint8_t)g_usb_key_match_state;
	}
	else//不是空板
	{
		g_usb_key_match_state = USB_KEY_HAS_MATCH;
		g_none = 4;
		return (uint8_t)g_usb_key_match_state;
	}
}

void ClearUsbKey(void)
{
   uint16_t i;
   uint8_t read_buffer[256]; 
   while(1)
	{
		for(i = 0; i < 255; i++)
		{
			I2C_WriteByte_2(i,0xFF,0xA0);//密码，0-11个字节
			
		}
		delay_ms(100);
		I2C_ReadnByte_2(0xA0,read_buffer,255,0);
		delay_ms(200);
		for(i = 0; i < 255; i++)
		{
			if(read_buffer[i] != 0xff)
			{
				break;
			}
		}
		if(i != 255)
		{
			continue;
		}
		else
		{
			break;
		}
	}
	
	
}
uint8_t array_cmp(uint8_t a[],uint8_t b[],int n)
{
	uint8_t i;

	for(i = 0; i < n; i++)
	{
		if(a[i] != b[i])
		{
			break;
		}
	}
	if(i != n)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

void McuMatchWithUsbKey(void)
{
	uint8_t i = 0;
	uint8_t ZeroCount = 0;
	uint8_t AgingTestPassword[7] = {0xAA,0x21,0x43,0x65,0x87,0x09,0x59};//0x59是校验码，是前面5个字节的和
	uint8_t AgingTestKeyword[7] = {0};
	uint8_t AgingTestPasswordCrc = 0;
	//0xaa 头部，0xFF是检验和，是0x11到0x55的和
	uint8_t u8InitData[7] = {0xAA,0x11,0x22,0x33,0x44,0x55, 0xFF}; 

	if(UsbKeyHasMatch() == 1)
	{
		I2C_ReadnByte_2(0xA0,AgingTestKeyword,USB_CRC_NUM + PASS_WORD_NUM,0x00);

		DebugFlag = 0;
		
		//计算校验和
		for(i = 1; i < PASS_WORD_NUM; i++)
		{
			AgingTestPasswordCrc += AgingTestKeyword[i];		
		}

		if(AgingTestPasswordCrc == AgingTestKeyword[PASS_WORD_NUM]) //检验和通过
		{
			for(i = 0; i < PASS_WORD_NUM; i++)
			{
				//老化数据不相同，就或1
				if(AgingTestPassword[i] != AgingTestKeyword[i])
				{
					DebugFlag |= 1;
				}
				//恢复出厂数据不相同，就或2
				if(u8InitData[i] != AgingTestKeyword[i])
				{
					DebugFlag |= 2;
				}				
			}

			//恢复出厂设置
			if(DebugFlag == 1)
			{
				bInitFlag = true;
				bKeyOn = true;
				return ;
			}
			else if(DebugFlag == 2) //老化程序设置
			{
				AgingTestFlag = true;
				return ;
			}
			
		}
		else //检验和不通过
		{}
							
	}
	
	if(UsbKeyHasMatch() == 1)
	{
		I2C_ReadnByte_2(0xA0,AgingTestKeyword,USB_CRC_NUM + PASS_WORD_NUM,0x00);
		for(i = 1; i < PASS_WORD_NUM; i++)
		{
			AgingTestPasswordCrc += AgingTestKeyword[i];
			if((AgingTestPassword[i-1]) !=	AgingTestKeyword[i])
			{				 
				DebugFlag = 23;
				break;
			}
		}
		if(i !=  PASS_WORD_NUM)//密码不通过
		{
			AgingTestFlag = false;
			DebugFlag = 24;
		}
		else
		{
			if(AgingTestPasswordCrc == AgingTestKeyword[PASS_WORD_NUM + USB_CRC_NUM - 1])//校验码通过
			{
				AgingTestFlag = true;
				return;
			}
			else//校验码不通过
			{
				AgingTestFlag = false;
			}
		}
	}


    if((McuHasMatch() == 0) && (UsbKeyHasMatch() == 1))//MCU没有匹配过,USBKEY初始化好了
	{
		 I2C_ReadnByte_2(0xA0,g_key_word,USBKEY_MATCH_INFO_NUM + PASS_WORD_NUM + USB_CRC_NUM,0x00);
		 g_passworld_crc[0] = 0;
		 for(i = 0; i < PASS_WORD_NUM; i++)
		 {
		 	if(i != 0)
		 		g_passworld_crc[0] += g_key_word[i];
			 if(g_key_word[i] == 0x00)
			 {
				ZeroCount++;
			 }
		 }
		 if(ZeroCount >= 6 || g_passworld_crc[0] != g_key_word[PASS_WORD_NUM + USB_CRC_NUM - 1])
		 {
			g_mcu_match_key = USBKEY_MATCH_ERR;
			return;
		 }
		 g_sysSetting.usbkey_password[0] = MCU_LOCK_HAS_SET;
//		 temp[0] = g_sysSetting.usbkey_password[0];
		 for(i = 0; i < PASS_WORD_NUM + USB_CRC_NUM;i++)
	 	 {
	 		g_sysSetting.usbkey_password[i+1] = g_key_word[i];
	 	 }
		 SYSSET_SyncToFlash();
//		 SaveAllInfo(PASS_WORD_NUM + 1,temp);
		 g_mcu_match_key = 1;
         DebugFlag = 11;

	}
	else if((McuHasMatch() == 1) && (UsbKeyHasMatch() == 1))//锁和钥匙都匹配
	{
//		GetMcuUniqueId();
		if(RebindUsbkeyFlag == 1)
		{
			RebindUsbkeyFlag = 0;
			g_FtrUpdateFlag = false;
	        g_FtrInfoUpdateFlag = false;
	        g_sysSetting.usbkey_password[0] = MCU_LOCK_NO_SET;
	        SaveMcuLockInfo();	        
	        UsbKeyCheck();
			return;
		}
//		flashReadBuffer(g_mcu_word_temp , ADDRESS_MCU_LOCK_DATA_STATE , 7);
		for(i = 0;i < PASS_WORD_NUM;i++)//去掉前面的MCU匹配标志位（0x55或0x66）
		{
			g_mcu_word[i] = g_sysSetting.usbkey_password[i+1];
		}
//		GetUsbKeyInitInfo();
		I2C_ReadnByte_2(0xA0,g_key_word,PASS_WORD_NUM + USB_CRC_NUM, 0);
		if((g_mcu_word[0] != 0xaa) || (g_key_word[0] != 0xaa))
		{
			DebugFlag = 20;
			return;
		}
		g_passworld_crc[0] = 0;
		for(i = 1; i < PASS_WORD_NUM; i++)
		{
			g_passworld_crc[0] += g_mcu_word[i];
			if((g_mcu_word[i]) !=  g_key_word[i])
			{                
                DebugFlag = 21;
				break;
			}
		}
		if(i !=  PASS_WORD_NUM)//密码不通过
		{
			g_mcu_match_key = USBKEY_MATCH_ERR;
			DebugFlag = 22;
			DebugFlag1 = i;
			return;
		}
		else
		{
			DebugFlag = g_passworld_crc[0];
			DebugFlag1 = g_key_word[PASS_WORD_NUM + USB_CRC_NUM - 1];
			if(g_passworld_crc[0] == g_key_word[PASS_WORD_NUM + USB_CRC_NUM - 1])//校验码通过
			{
				g_mcu_match_key = USBKEY_MATCH_SUC;
			}
			else//校验码不通过
			{
				g_mcu_match_key = USBKEY_MATCH_ERR;
			}
			return;
		}
	}
//    else if((McuHasMatch() == 1) && (UsbKeyHasMatch() == 0))//锁匹配过，钥匙没匹配过
//    {
//    	RebindUsbkeyFlag = 1;
//    }
	else//锁和钥匙一个匹配一个没有匹配
	{
		
        DebugFlag1 = UsbKeyHasMatch();
		g_mcu_match_key = USBKEY_MATCH_ERR;
		return;
	}
	
}

void UsbKeyCheck(void)
{
	driver_uart_int_disable();
    IIC_Init_2();
	g_mcu_match_key = 0;
    DebugFlag = 3;
	if(UsbKeyStateJudge())
	{
		bKeyOn = true;
        DebugFlag = 2;
		McuMatchWithUsbKey();
	}
	else
		bKeyOn = false;
    
	board_uart_init();
	driver_uart_init();
	APP_PRINT_INFO1("222g_sysSetting.usbkey_password = %b", TRACE_BINARY(8 , g_sysSetting.usbkey_password));	
	APP_PRINT_INFO1("read_from_key_data = %b", TRACE_BINARY(7, g_key_word));	
	APP_PRINT_INFO1("g_none : %d",g_none);
	APP_PRINT_INFO1("DebugFlag : %d",DebugFlag);
	APP_PRINT_INFO1("DebugFlag1 : %d",DebugFlag1 );
	APP_PRINT_INFO1("g_mcu_match_key : %d",g_mcu_match_key);
	APP_PRINT_INFO1("222AgingTestFlag : %d",AgingTestFlag);
}

uint8_t UsbCheckResult(void)
{
//    if(g_usb_insert == 1)
//    {
//        if(g_none == 0)
//        {   
//            printf("UsbKey no Match\r\n");
//        }   
//        else if(g_none == 1)
//        {
//        	printf("UsbKey Read Err\r\n");
//        }
//        else if(g_none == 2)
//        {
//            printf("UsbKey No Init\r\n");
//        }
//        else if(g_none == 3)
//        {
//        	printf("UsbKey Init Data Err\r\n");
//        }
//        else if(g_none == 4)
//    	{
//    		printf("UsbKey has match\r\n");
//    	}
//        if(g_mcu_match_key == USBKEY_MATCH_SUC)
//		{
//			printf("g_mcu_match_key == USBKEY_MATCH_SUC\r\n");
//            //SYSSET_SetUsbKeyStatus(USBKEY_IS_MATCH);
//			return 0;
//		}
//        else
//        {
//            if(McuHasMatch() == 0)
//            {
//                printf("Mcu No Match\r\n");
//            }
//            else
//            {
//                printf("Mcu Match\r\n");
//            }        
//        }
//    }
//	else
//	{
//		printf("UsbKey no insert\r\n");
//	}
    
    return 1;
}

void usbKeyCleanData(void)
{
	memset(g_sysSetting.usbkey_password, 0, sizeof(g_sysSetting.usbkey_password));
	SYSSET_SyncToFlash();
}

//写入usgkey的数据，用于生成usbkey
void usbKeyWriteCreate(void)
{
	driver_uart_int_disable();
    IIC_Init_2();
			
	if(UsbKeyStateJudge())
	{
		uint8_t key[7] = {0xAA,0x11,0x22,0x33,0x44,0x55, 0xFF};
		int nRand = rand();
		
		#if 0
		int i = 0;
		key[0] = 0xaa;
		key[1] = (nRand >> 24) & 0xFF;
		key[2] = (nRand >> 16) & 0xFF;
		key[3] = (nRand >> 8) & 0xFF;
		key[4] = (nRand) & 0xFF;
		key[5] = (nRand >> 12) & 0xFF;

		for(i = 1; i < 6; i++)
			key[6] += key[i];
		#endif
		APP_PRINT_INFO1("i2c send is %b", TRACE_BINARY(7, key));

		I2C_WritenByte_2(0xA0, key, 7, 0);
		
		McuMatchWithUsbKey();

		APP_PRINT_INFO1("read_from_key_data = %b", TRACE_BINARY(7, g_key_word));	

		if(memcmp(g_key_word, key, 7) == 0)
		{
			background_msg_set_led(BACKGROUND_MSG_LED_SUBTYPE_MATCH_SUCCESS);
			APP_PRINT_INFO0("create ok");
		}
			
	}

	board_uart_init();
	driver_uart_init();
}



