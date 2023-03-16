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

bool bAdminFpFlag = false; //����Աָ�Ʊ�־

int fp_enroll()
{
    uint32_t ML_Error;
    uint8_t progress = 0, uEnrollNum = 1 ,uPressErrorNum = 0;
	int FtrNum = 0;
    uint16_t index = 0xFFFF;		//�����ID��	
	
	progress = 0;
	uEnrollNum = 1;	
		
    while(progress < 100)
    {
		background_msg_set_led(BACKGROUND_MSG_LED_SUBTYPE_BLUE_ON);

		menu_sleep_event_control(MENU_SLEEP_EVENT_WAKEUP_BIT, true);
    	ML_Error = MLAPI_Enrollment(uEnrollNum, &index, &progress, 2500);//progress :ע�����
		
        if(COMP_CODE_OK != ML_Error) 
        {
        	FINGERPRINT_PRINT_INFO1("ML_Error:%d\r\n",ML_Error);
            if(COMP_CODE_NO_FINGER_DECTECT == ML_Error)
            {
        	    FINGERPRINT_PRINT_INFO1("DEMO:Registration failed,errorCode:%d\r\n", ML_Error);//���ܱ����������ѻ������С�ȣ��賢�Ա����ٴ�ע�ᣬ���ﲻ������
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
            if(progress != 100)//�������һ��ע���ָ�ƺ����ƣ�ֱ����һ������
            {
				background_msg_set_led(BACKGROUND_MSG_LED_SUBTYPE_PRESS_SUC);
				background_msg_set_beep(50, 1);
                uEnrollNum ++;   					
            }			
		}
		if(progress != 100)	//�������һ��ע���ָ�ƺ󲻵ȴ���ָ�뿪��ֱ����һ������
        	while(MLAPI_QueryFingerPresent());      //�ȴ���ָ�뿪
    }
	
	
    ML_Error = MLAPI_StorageFtr(index, 1);//����ָ������
    if(COMP_CODE_OK == ML_Error)
    {
        FINGERPRINT_PRINT_INFO1("DEMO:Save success., index is %d\r\n", index);
		background_msg_set_led(BACKGROUND_MSG_LED_SUBTYPE_ENROLL_SUCCESS);
		background_msg_set_beep(100, 2);

		FtrNum = fileSys_getStoreFtrNum();//ע����ɼ���ע��
		if(STORE_MAX_FTR == FtrNum)                    //ָ����������
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
	while(MLAPI_QueryFingerPresent());      //�ȴ���ָ�뿪

    return 0;
}

/*****************************************************************************
 �� �� ��: fp_admin_match
 ��������  : У�����Աָ��(ÿһ��У����3�λ���)
 
 �������  : uint8_t u8OkCnt
            ��Ҫ��֤�Ĵ���
 �������  : 
			
 �� �� ֵ: �Ƿ�У�����Ա��trueУ�飬false��У��

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

		//��ʱû��⵽��ָ��ֱ���˳�
		if(COMP_CODE_NO_FINGER_DECTECT == ret)
		{
			background_msg_set_led(BACKGROUND_MSG_LED_SUBTYPE_STOP_ALL);
			return false;
		}			

		// uResult = ��ȡƥ���� 1:�ɹ�;  0��ʧ��  			
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

		//ǰ��3�μ���һ�ζ�ûͨ����ֱ���˳�
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
    if(0 == FtrNum && tuya_ble_current_para.sys_settings.bound_flag == 0)		//�ж��豸�Ƿ񱻰�
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
	
	if(uResult)             // uResult = ��ȡƥ���� 1:�ɹ�;  0��ʧ��
    {    			
    	
		if(door_open_status() == E_OPEN_NONE || door_open_status() == E_OPEN_SUC)
		set_door_open_status(E_OPEN_START);											//��ǰ�ĵ�����״̬��־λ����ֹ�������Զ����ͻ���

		Pad_Config(BAT_EN_HAL1_POW, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);			
		Pad_Config(PAIR_HAL1, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);//����HAL1����
		
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


//����ָ�Ʋ���(subtype T_FP_SUB_MSG_TYPE)
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




