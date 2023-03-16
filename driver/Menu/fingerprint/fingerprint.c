#include "fingerprint.h"
#include "trace.h"
#include "mlapi.h"
#include "driver_sensor.h"
#include "menu_manage.h"
#include "bluetooth_menu.h"
#include "filesys.h"
#include "test_mode.h"
#include "driver_led.h"
#include "driver_motor.h"
#include "driver_usbkey.h"
#include "tuya_ble_app_demo.h"
#include "driver_button.h"
#include "gap_adv.h"
#include "tuya_ble_main.h"
#include "os_sched.h"
#include "driver_hal.h"
#include "tuya_ble_app_demo.h"
#include "rtl876x_gpio.h"
#include <board.h>


#if 1
#define FINGERPRINT_PRINT_INFO0   APP_PRINT_TRACE0
#define FINGERPRINT_PRINT_INFO1   APP_PRINT_TRACE1
#define FINGERPRINT_PRINT_INFO2   APP_PRINT_TRACE2
#define FINGERPRINT_PRINT_INFO3   APP_PRINT_TRACE3
#define FINGERPRINT_PRINT_INFO4   APP_PRINT_TRACE4
#else
#define FINGERPRINT_PRINT_INFO0(...)
#define FINGERPRINT_PRINT_INFO1(...)
#define FINGERPRINT_PRINT_INFO2(...)
#define FINGERPRINT_PRINT_INFO3(...)
#define FINGERPRINT_PRINT_INFO4(...)
#endif

bool bAdminFpFlag = false; //管理员指纹标志

int fp_enroll()
{
    uint32_t ML_Error;
    uint8_t progress = 0, uEnrollNum = 1 ,uPressErrorNum = 0;
	int FtrNum = 0;
    uint16_t index = 0xFFFF;		//保存的ID号	
	
	progress = 0;
	uEnrollNum = 1;	
		
    while(progress < 100)
    {
		background_msg_set_led(BACKGROUND_MSG_LED_SUBTYPE_BLUE_ON);

		menu_sleep_event_control(MENU_SLEEP_EVENT_WAKEUP_BIT, true);
    	ML_Error = MLAPI_Enrollment(uEnrollNum, &index, &progress, 2500);//progress :注册进度
		
        if(COMP_CODE_OK != ML_Error) 
        {
        	FINGERPRINT_PRINT_INFO1("ML_Error:%d\r\n",ML_Error);
            if(COMP_CODE_NO_FINGER_DECTECT == ML_Error)
            {
        	    FINGERPRINT_PRINT_INFO1("DEMO:Registration failed,errorCode:%d\r\n", ML_Error);//可能本次质量不佳或面积过小等，需尝试本次再次注册，这里不做处理。
				goto ErrorOut;
            }
            else if(COMP_CODE_STORAGE_IS_FULL == ML_Error)
            {
				goto ErrorOut;
            }
            else if(COMP_CODE_STORAGE_REPEAT_FINGERPRINT == ML_Error)
            {
              background_msg_set_led(BACKGROUND_MSG_LED_SUBTYPE_PRESS_FAIL);
              uPressErrorNum++;
              if(uPressErrorNum >= 3)
              {
                uPressErrorNum = 0;
                goto ErrorOut;
              }
              else
              {
                while(MLAPI_QueryFingerPresent());
                continue;
              }
            }
            else if(COMP_CODE_FORCE_QUIT == ML_Error)
            {
            	FINGERPRINT_PRINT_INFO0("quit add fp suc");
                goto ErrorOut;
            }
            else
        	{
        		continue;
        	}
        	
ErrorOut:
			background_msg_set_led(BACKGROUND_MSG_LED_SUBTYPE_ENROLL_FAIL);
			while(MLAPI_QueryFingerPresent());
			return 0;
			
		}
		else
		{
			FINGERPRINT_PRINT_INFO1("DEMO:This press[%d] registration is successful.\r\n", uEnrollNum);
            if(progress != 100)//按下最后一次注册的指纹后不闪灯，直接下一步保存
            {
				background_msg_set_led(BACKGROUND_MSG_LED_SUBTYPE_PRESS_SUC);
				background_msg_set_beep(50, 1);
                uEnrollNum ++;   					
            }			
		}
		if(progress != 100)	//按下最后一次注册的指纹后不等待手指离开，直接下一步保存
        	while(MLAPI_QueryFingerPresent());      //等待手指离开
    }
	
	
    ML_Error = MLAPI_StorageFtr(index, 1);//保存指纹特征
    if(COMP_CODE_OK == ML_Error)
    {
        FINGERPRINT_PRINT_INFO1("DEMO:Save success., index is %d\r\n", index);
		background_msg_set_led(BACKGROUND_MSG_LED_SUBTYPE_ENROLL_SUCCESS);
		background_msg_set_beep(100, 2);

		FtrNum = fileSys_getStoreFtrNum();//注册完成继续注册
		if(STORE_MAX_FTR == FtrNum)                    //指纹容量已满
        {
			background_msg_set_led(BACKGROUND_MSG_LED_SUBTYPE_ENROLL_FAIL);
        }
//		else
//		{
//			background_msg_set_fp(FP_MSG_SUBTYPE_ENROLL);
//		}
    }
	else
	{
		FINGERPRINT_PRINT_INFO1("DEMO:Save failed[%d].\r\n", ML_Error);		
		background_msg_set_led(BACKGROUND_MSG_LED_SUBTYPE_ENROLL_FAIL);
	}
	while(MLAPI_QueryFingerPresent());      //等待手指离开

    return 0;
}

/*****************************************************************************
 函 数 名: fp_admin_match
 功能描述  : 校验管理员指纹(每一次校验有3次机会)
 
 输入参数  : uint8_t u8OkCnt
            需要验证的次数
 输出参数  : 
			
 返 回 值: 是否校验管理员，true校验，false不校验

****************************************************************************/
bool fp_admin_match(uint8_t u8OkCnt)
{
    uint16_t uResult = 0, uScore = 0, uMatchId = 0xFFFF;
	bool bResult = false;
	uint8_t i, j = 0;
	uint32_t ret = 0;

	if(fileSys_getStoreFtrNum() == 0)
	{
		background_msg_set_led(BACKGROUND_MSG_LED_SUBTYPE_CHECK_ADMIN_SUC);
		return true;
	}

	for(i= 0; i < u8OkCnt * 3; i++)
	{
		background_msg_set_led(BACKGROUND_MSG_LED_SUBTYPE_CHECK_ADMIN);
		
		ret = MLAPI_Match(&uResult, &uScore, &uMatchId, 5000);

		//超时没检测到手指，直接退出
		if(COMP_CODE_NO_FINGER_DECTECT == ret)
		{
			background_msg_set_led(BACKGROUND_MSG_LED_SUBTYPE_STOP_ALL);
			return false;
		}			

		// uResult = 获取匹配结果 1:成功;  0：失败  			
		if(uMatchId == 0 || uMatchId == 1)
		{					
			background_msg_set_led(BACKGROUND_MSG_LED_SUBTYPE_CHECK_ADMIN_SUC);
			j++;
			if(j == u8OkCnt)
			{
				bResult = true;
				goto out;
			}
		}		  
		else
		{
			background_msg_set_led(BACKGROUND_MSG_LED_SUBTYPE_CHECK_ADMIN_FAIL);
		}

		//前面3次假设一次都没通过，直接退出
		if(i == 2 && j == 0)
			goto out; 
		
		while(MLAPI_QueryFingerPresent()){}
	}

	
		
    return bResult;

out:
	while(MLAPI_QueryFingerPresent()){}
	return bResult;
}


static void fp_match(T_MENU_MSG *p_menu_msg)
{
    int FtrNum = 0;
    uint16_t uResult = 0, uScore = 0, uMatchId = 0xFFFF;
	bAdminFpFlag = false;
	
    FtrNum = fileSys_getStoreFtrNum();
    if(0 == FtrNum && tuya_ble_current_para.sys_settings.bound_flag == 0)		//判断设备是否被绑定
    {
        uResult = 1;	
		bAdminFpFlag = true;
    }
    else if (0 == FtrNum && tuya_ble_current_para.sys_settings.bound_flag == 1)
    {
    	//background_msg_set_led(BACKGROUND_MSG_LED_SUBTYPE_MATCH_FAIL);
		//menu_set_press_status(false);
		//while(MLAPI_QueryFingerPresent());
        //MLAPI_Match(&uResult, &uScore, &uMatchId, 100);
		
    }
	else 
	{		
		MLAPI_Match(&uResult, &uScore, &uMatchId, 100);
	}
	
	if(uResult)             // uResult = 获取匹配结果 1:成功;  0：失败
    {    			
    	
		if(door_open_status() == E_OPEN_NONE || door_open_status() == E_OPEN_SUC)
		set_door_open_status(E_OPEN_START);											//提前改掉锁的状态标志位，防止主程序自动拉低霍尔

		Pad_Config(BAT_EN_HAL1_POW, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);			
		Pad_Config(PAIR_HAL1, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);//开启HAL1输入
		
		background_msg_set_led(BACKGROUND_MSG_LED_SUBTYPE_MATCH_SUCCESS);				
		background_msg_set_beep(150, 3);
		os_delay(1200);
		driver_motor_control(EM_MOTOR_CTRL_ON, 2000);
		os_delay(20);

		GPIO_INTConfig(GPIO_GetPin(PAIR_HAL1), ENABLE); 
		GPIO_MaskINTConfig(GPIO_GetPin(PAIR_HAL1), DISABLE);
		
		if(uMatchId == 0 || uMatchId == 1)
			bAdminFpFlag = true;
    }
	else
    {    	
    	background_msg_set_led(BACKGROUND_MSG_LED_SUBTYPE_MATCH_FAIL);
		background_msg_set_beep(50, 4);
		menu_set_press_status(false);
		while(MLAPI_QueryFingerPresent());
    }
 			
    return;
}


//设置指纹操作(subtype T_FP_SUB_MSG_TYPE)
void background_msg_set_fp(unsigned char subtype)
{
	T_MENU_MSG menMsg = {0};
	menMsg.type = MENU_MSG_TYPE_FINGERPRINT;
	menMsg.subtype = subtype;
	menu_task_msg_send(&menMsg);
}

uint32_t fingerprint_handle_msg(T_MENU_MSG *fp_msg)
{
		
	if(fp_msg == NULL)
	{
		FINGERPRINT_PRINT_INFO0("fingerprint_handle_msg input param error\n");
		return 0;
	}

	FINGERPRINT_PRINT_INFO1("fp_msg->subtyp is 0x%x\n", fp_msg->subtype);
	switch(fp_msg->subtype)
	{
		case FP_MSG_SUBTYPE_MATH:			
			fp_match(fp_msg);
			break;

		case FP_MSG_SUBTYPE_BT_ENROLL:
			tuya_ble_add_fp(0, NULL, 0);
			break;

		case FP_MSG_SUBTYPE_ENROLL:
			suspend_task();
			fp_enroll();
			resume_task(false);
			break;
			
		default:
			FINGERPRINT_PRINT_INFO0("bt_msg->subtype error\n");
			break;
	}

	

	return 0;
}




