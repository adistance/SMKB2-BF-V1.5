#include "driver_button.h"
#include <trace.h>

#include "rtl876x_rcc.h"
#include "rtl876x_gpio.h"
#include "rtl876x_nvic.h"

#include "board.h"
#include "driver_led.h"
#include "driver_spi.h"
#include "driver_flash.h"
#include "driver_uart.h"
#include "driver_rtc.h"
#include "password.h"
#include "record.h"
#include "os_sched.h"
#include "driver_motor.h"
#include "fingerprint.h"
#include "tuya_ble_storage.h"
#include "driver_usbkey.h"
#include "driver_hal.h"

static bool pair_press = false;
extern bool bKeyOn;

extern void delay_ms(unsigned int nms);

void gpio_button_pinmux_config(void)
{
    Pinmux_Config(PAIR_BUTTON, DWGPIO);
}

void gpio_button_pad_config(void)
{
    Pad_Config(PAIR_BUTTON, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);
}

void gpio_button_enter_dlps_config(void)
{
    Pad_Config(PAIR_BUTTON, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);
	System_WakeUpPinEnable(PAIR_BUTTON, PAD_WAKEUP_POL_LOW, PAD_WK_DEBOUNCE_DISABLE);
}

void gpio_button_exit_dlps_config(void)
{
    Pad_Config(PAIR_BUTTON, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);
}

void driver_button_init(void)
{ 
	Pad_Config(PAIR_BUTTON, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);
	Pinmux_Config(PAIR_BUTTON, DWGPIO);
	
    RCC_PeriphClockCmd(APBPeriph_GPIO, APBPeriph_GPIO_CLOCK, ENABLE);
 
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin        = GPIO_GetPin(PAIR_BUTTON);
    GPIO_InitStruct.GPIO_Mode       = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_ITCmd      = ENABLE;
    GPIO_InitStruct.GPIO_ITTrigger  = GPIO_INT_Trigger_EDGE;
    GPIO_InitStruct.GPIO_ITPolarity = GPIO_INT_POLARITY_ACTIVE_LOW;
    GPIO_InitStruct.GPIO_ITDebounce = GPIO_INT_DEBOUNCE_ENABLE;
    GPIO_InitStruct.GPIO_DebounceTime = 10;
    GPIO_Init(&GPIO_InitStruct);

    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = PAIR_BUTTON_IRQ;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
    
    /* Enable interrupt */
    GPIO_MaskINTConfig(GPIO_GetPin(PAIR_BUTTON), DISABLE);
    GPIO_INTConfig(GPIO_GetPin(PAIR_BUTTON), ENABLE);
}


bool button_get_press_status(void)
{
	return pair_press;
}

//void GPIO8_Handler(void)
//{
//    GPIO_INTConfig(GPIO_GetPin(PAIR_BUTTON), DISABLE);
//    GPIO_MaskINTConfig(GPIO_GetPin(PAIR_BUTTON), ENABLE);
//	menu_sleep_event_control(MENU_SLEEP_EVENT_WAKEUP_BIT, true);
//    if (GPIO_ReadInputDataBit(GPIO_GetPin(PAIR_BUTTON)))        //release
//    {
//		APP_PRINT_INFO0("release");
//    	pair_press = false;
//        GPIO->INTPOLARITY &= ~GPIO_GetPin(PAIR_BUTTON);  //Polarity Low
//    }
//    else														//press
//    {
//		APP_PRINT_INFO0("press");
//    	pair_press = true;
//        GPIO->INTPOLARITY |= GPIO_GetPin(PAIR_BUTTON);   //Polarity High
//    }
//  
//    GPIO_ClearINTPendingBit(GPIO_GetPin(PAIR_BUTTON));
//    GPIO_MaskINTConfig(GPIO_GetPin(PAIR_BUTTON), DISABLE);
//    GPIO_INTConfig(GPIO_GetPin(PAIR_BUTTON), ENABLE);
//}

//ÉèÖÃÖ¸ÎÆ²Ù×÷(subtype T_MENU_MSG_BUTTON_SUBTYPE)
void background_msg_set_button(unsigned char subtype)
{
	T_MENU_MSG menMsg = {0};
	menMsg.type = MENU_MSG_TYPE_BUTTON;
	menMsg.subtype = subtype;
	menu_task_msg_send(&menMsg);
}

uint32_t button_handle_msg(T_MENU_MSG *button_msg)
{
	if(button_msg == NULL)
	{
		APP_PRINT_INFO0("button_handle_msg input param is NULL");
		return 0;
	}
	uint16_t deleteId = 0;
	
	switch(button_msg->subtype)
	{
		case MENU_MSG_BUTTON_SUBTYPE_RESET:					
//			suspend_task();
			BleRecordInfoReset();
			MLAPI_DeleteFTR(ERASE_ALL_FINGER, 0, &deleteId);
			if(bKeyOn)
				usbKeyCleanData();
//			background_msg_set_led(BACKGROUND_MSG_LED_SUBTYPE_RESET_INIT);
			tuya_ble_storage_reset();
//			resume_task(false);
//			set_door_open_status(E_OPEN_SUC);
//			Set_hal_door_status(true);
			break;
			
		default:
			break;
	}

	return 0;
}



