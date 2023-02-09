#include "bluetooth_menu.h"
#include "protocol_default.h"
#include "bluetooth_default.h"
#include "trace.h"
#include "app_crypto.h"
#include "os_sync.h"
#include "trace.h"
#include "mlapi.h"
#include "password.h"
#include "app_task.h"
#include "driver_rtc.h"
#include "driver_motor.h"
#include "driver_led.h"
#include "system_setting.h"
#include <simple_ble_service.h>
#include "record.h"
#include "os_sched.h"
#include "menu_manage.h"
#include "driver_motor.h"

//机密库密钥(每个密钥对应一个product id)
static uint8_t rootKey[] = {
    // 0x25, 0x5E, 0x0D, 0x5B, 0x59, 0x49, 0xF8, 0x09,
    // 0x9B, 0x38, 0xF6, 0xCA, 0xBF, 0x0A, 0x07, 0x86 
     //0x41, 0x5C, 0x64, 0x23, 0xD4, 0x11, 0x7A, 0x73,  //这个对应的product id (0x35, 0x09, 0x7D, 0x9D, 0x97, 0xB9) 
    // 0xC0, 0x52, 0x68, 0xC3, 0x89, 0xE0, 0xD8, 0x83   
    //0x80, 0xCE, 0xE2, 0xA2, 0x42, 0xA7, 0xB6, 0xC7,
    //0x91, 0xCA, 0xB3, 0x78, 0x85, 0x67, 0xE2, 0x21
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
static uint8_t passWord[] = {1, 2, 3, 4, 5, 6};  //加密库初始密码

sBLECmdHdr tmp = {0};			//加密库的包头数据
uint8_t send_buf[280] = {0};	//发送buf
uint8_t tmp_buf[10][128] = {0};		//保存数据的一个缓冲区
static uint8_t index = 0;

void *bt_mutex_handle = NULL;   //互斥锁句柄
void *bt_msg_mutex_handle = NULL;   //互斥锁句柄
void *bt_sem_handle = NULL;  //信号量句柄

static sConfigInfo productInfo = {78, 1, 3, 0, 2, 0, 0, 0, 6, 16, 0,
                                  5,  2, 1, 3, 1, 1, 2, 2, 1, 1,  1};

#if BLE_DMZ
#define DEFAULT_BOARD_APP_ID 	"DMZ.D1888LY.021.021.210416.120.120"
#else
#define DEFAULT_BOARD_APP_ID	"Topstrong.SD1/SL1.021.021.191105.106.106"   //16Bytes
#endif

static char s_reset_flag = 0;		//回复出厂设置标志		
static bool s_bEnroll = false; 		//指纹注册的标志

extern uint8_t g_QuitAtOnce;		//马上结束当前指纹采图标志

void bluetooth_crypto_en_cb_handle(cyp_status_t status, uint8_t *buf, uint8_t len);

void bluetooth_open_door(void)
{
	
	T_MENU_MSG menMsg = {0};	
	
	menMsg.type =  BACKGROUND_MSG_TYPE_LED;
	menMsg.subtype = BACKGROUND_MSG_LED_SUBTYPE_MATCH_SUCCESS;
	background_task_msg_send(&menMsg);
	menMsg.type = BACKGROUND_MSG_TYPE_MOTOR;
	menMsg.subtype = BACKGROUND_MSG_MOTOR_SUBTYPE_LEFT;
	//menMsg.u.buf = NULL;
	background_task_msg_send(&menMsg);
}


unsigned char bluetooth_msg_emit(unsigned char mode, unsigned char comp, unsigned char*data, unsigned char len)
{
	int ret = 0;    
	tmp.data_length = 3;
	send_buf[sizeof(sBLECmdHdr)+1] = 0;
	send_buf[sizeof(sBLECmdHdr)+2] = 0;    
	if((COMP_CODE_NO_FINGER_DECTECT == comp)|| (COMP_CODE_STORAGE_WRITE_ERROR == comp))
	{
	    send_buf[sizeof(sBLECmdHdr)] = 10;
	}
	else if(COMP_CODE_STORAGE_IS_FULL == comp)
	{
	    send_buf[sizeof(sBLECmdHdr)] = 100;
	}
	else if(COMP_CODE_STORAGE_REPEAT_FINGERPRINT == comp)
	{
	    send_buf[sizeof(sBLECmdHdr)] = 200;
	}
	else 	
	{
		tmp.flow_port = E_RESP_START_ADD_FP;
	    tmp.data_length = len;
	    memcpy(send_buf+sizeof(sBLECmdHdr), data, len);
	}
	memcpy(send_buf, &tmp, sizeof(sBLECmdHdr));
	//APP_PRINT_INFO1("before encode %b", TRACE_BINARY(len, data));	
	int send_len = sizeof(sBLECmdHdr) + tmp.data_length;
	//APP_PRINT_INFO1("before encode22 %b", TRACE_BINARY(send_len, send_buf));	
	ret = app_crypto_msg_encode(send_buf, send_len, bluetooth_crypto_en_cb_handle);
	if(ret != 0)
		APP_PRINT_INFO2("app_crypto_msg_encode error %d!, len %d\n", ret, send_len);
   
    return 0;
}

//蓝牙添加指纹
int bluetooth_enroll(void)
{
    uint32_t ML_Error;
    uint8_t progress = 0, uEnrollNum = 1;
    uint16_t index;
    uint8_t buf[3];
	T_MENU_MSG ledMsg = {0};

	ledMsg.type = BACKGROUND_MSG_TYPE_LED;
	ledMsg.subtype = BACKGROUND_MSG_LED_SUBTYPE_ENROLL_STATE;
	background_task_msg_send(&ledMsg);	

    while(progress < 100)
    {
    	menu_sleep_event_control(MENU_SLEEP_EVENT_WAKEUP_BIT, true);
    	if(s_bEnroll)
    	{
    		ML_Error = MLAPI_Enrollment(uEnrollNum, &index, &progress, 5000);
    	}
        else 
        {
        	//蓝牙小程序上面暂时能修改的指纹只有管理员指纹，管理员指纹ID是0
        	index = 0;
        	ML_Error = MLAPI_EditFP(uEnrollNum, &index, &progress, 5000);
        }
				
        if(COMP_CODE_OK != ML_Error) 
        {
        	ledMsg.subtype = BACKGROUND_MSG_LED_SUBTYPE_ENROLL_FAIL;
			background_task_msg_send(&ledMsg);	
        	APP_PRINT_INFO1("ML_Error:%d\r\n",ML_Error);
            if(COMP_CODE_NO_FINGER_DECTECT == ML_Error)
            {
        	    APP_PRINT_INFO1("DEMO:Registration failed,errorCode:%d\r\n", ML_Error);//可能本次质量不佳或面积过小等，需尝试本次再次注册，这里不做处理。
                
                bluetooth_msg_emit(E_RESP_START_ADD_FP, COMP_CODE_NO_FINGER_DECTECT, NULL, 0);
        	    return 0;
            }
            else if(COMP_CODE_STORAGE_IS_FULL == ML_Error)
            {
                bluetooth_msg_emit(E_RESP_START_ADD_FP, COMP_CODE_STORAGE_IS_FULL, NULL, 0);
        	    return 0;
            }
            else if(COMP_CODE_STORAGE_REPEAT_FINGERPRINT == ML_Error)
            {
                bluetooth_msg_emit(E_RESP_START_ADD_FP, COMP_CODE_STORAGE_REPEAT_FINGERPRINT, NULL, 0);
                return 0;
            }
            else if(COMP_CODE_FORCE_QUIT == ML_Error)
            {
                bluetooth_msg_emit(E_RESP_EXIT_CONFIG_MODE, COMP_CODE_OK, NULL, 0);
                return 0;
            }
            else 
            {
            }
		}
		else
		{
			APP_PRINT_INFO1("DEMO:This press[%d] registration is successful.\r\n", uEnrollNum);

            if(progress != 100)
            {
                buf[0] = uEnrollNum;
                buf[1] = progress;   
                bluetooth_msg_emit(E_RESP_START_ADD_FP, COMP_CODE_OK, buf, 2);
                uEnrollNum ++;   
				ledMsg.subtype = BACKGROUND_MSG_LED_SUBTYPE_PRESS_SUC;
				background_task_msg_send(&ledMsg);	
            }			
		}
        while(MLAPI_QueryFingerPresent());      //等待手指离开
    }

    ML_Error = MLAPI_StorageFtr(index, 0);
    if(COMP_CODE_OK == ML_Error)
    {
        APP_PRINT_INFO0("DEMO:Save success.\r\n");

        buf[0] = uEnrollNum;
        buf[1] = progress;
        buf[2] = index+1;
        bluetooth_msg_emit(E_RESP_START_ADD_FP, COMP_CODE_OK, buf, 3);
		ledMsg.subtype = BACKGROUND_MSG_LED_SUBTYPE_ENROLL_SUCCESS;
		background_task_msg_send(&ledMsg);	
    }
	else
	{
		APP_PRINT_INFO1("DEMO:Save failed[%d].\r\n", ML_Error);
		ledMsg.subtype = BACKGROUND_MSG_LED_SUBTYPE_ENROLL_FAIL;
		background_task_msg_send(&ledMsg);	
        bluetooth_msg_emit(E_RESP_START_ADD_FP, COMP_CODE_STORAGE_WRITE_ERROR, NULL, 0);
	}
	while(MLAPI_QueryFingerPresent());      //等待手指离开
	
    return 0;
}


//将已处理好的蓝牙数据发送给主任务
bool bluebooth_send_msg_to_menu_task(T_BT_SUB_MSG_TYPE subtype, unsigned char *data, unsigned short len)
{
	if(data == NULL || len <= 0)
	{
	
		APP_PRINT_INFO0("bluebooth_send_msg_to_menu_task input param error\n");
		return false;
	}
	//os_mutex_take(bt_mutex_handle, 0);
	T_MENU_MSG send_data = {0};
	send_data.type = BACKGROUND_MSG_TYPE_BT;
	send_data.subtype = subtype;
	send_data.len = len;
	if(len > 0)
		send_data.u.buf = tmp_buf[index++];
	
	if(index == 10)
		index = 0;
	
	memcpy(send_data.u.buf, data, len);
	if(false == background_task_msg_send(&send_data))
    {
        APP_PRINT_INFO0("[ERROR][FINGERPRINT]:menu task msg send fail");
    }
	
	//os_mutex_give(bt_mutex_handle);
    return true;
}

//把消息发送到头部位置
bool bluebooth_send_msg_to_menu_task_front(T_BT_SUB_MSG_TYPE subtype, unsigned char *data, unsigned short len)
{
	if(data == NULL || len <= 0)
	{
	
		APP_PRINT_INFO0("bluebooth_send_msg_to_menu_task input param error\n");
		return false;
	}
	//os_mutex_take(bt_mutex_handle, 0);
	
	T_MENU_MSG send_data = {0};
	send_data.type = BACKGROUND_MSG_TYPE_BT;
	send_data.subtype = subtype;
	send_data.len = len;
	if(len > 0)
		send_data.u.buf = tmp_buf[index++];
	if(index == 10)
		index = 0;
	
	memcpy(send_data.u.buf, data, len);
	if(false == background_task_msg_send_front(&send_data))
    {
        APP_PRINT_INFO0("[ERROR][FINGERPRINT]:menu task msg send fail");
    }
	
	//os_mutex_give(bt_mutex_handle);
    return true;
}


void bluetooth_init()
{
	int ret = 0;

	SYSSET_GetBoardSerialNumber(rootKey, E_ROOTKEY);
	//APP_PRINT_INFO1("rootkey is %b", TRACE_BINARY(16, rootKey));
	//加密库初始化
	ret = app_crypto_init(IO_MSG_TYPE_ENCRYPT_COMP, 0, rootKey, passWord, sizeof(passWord), app_send_msg_to_apptask);
	if(ret == CYP_OK)
		APP_PRINT_INFO0("app_crypto_init success!");
	else 
		APP_PRINT_INFO1("app_crypto_init error(%d)!", ret);

	PswFileInit(); 			//密码初始化
	BleProductInfoInit();	//开锁记录初始化

	if(bt_mutex_handle == NULL)
	{
		if (os_mutex_create(&bt_mutex_handle) != true)
		{
			APP_PRINT_INFO0("bt_men os_mutex_create fail");
		}
	}

	if(bt_msg_mutex_handle == NULL)
	{
		if (os_mutex_create(&bt_msg_mutex_handle) != true)
		{
			APP_PRINT_INFO0("bt_men os_mutex_create fail");
		}
	}

	if(bt_sem_handle == NULL)
	{
		if(os_sem_create(&bt_sem_handle, 1, 1) != true)
	    {
	        APP_PRINT_INFO0("bt_men os_sem_create fail");
	    }
	}	 
}


//加密库认证回调函数
void bluetooth_auth_cb_handle(cyp_status_t status, uint8_t *buf, uint8_t len)
{
    if (status == CYP_OK) 
    {
		APP_PRINT_INFO0("bluetooth_auth_cb_handle suc!\n");
    } 
    else
    {
    	APP_PRINT_INFO1("bluetooth_auth_cb_handle error %d !\n", status);
    }
}


//加密库解码的回调函数
void bluetooth_crypto_dec_cb_handle(cyp_status_t status, uint8_t *buf, uint8_t len)
{
    if (status == CYP_OK) 
    {
    	APP_PRINT_INFO0("bluetooth_crypto_dec_cb_handle succ");
        T_MENU_MSG send_data = {0};
		send_data.type = BACKGROUND_MSG_TYPE_BT;
		send_data.subtype = BT_MSG_DATA_PROC;
		send_data.len = len;
		
		if(len > 0)
			send_data.u.buf = tmp_buf[index++];
		if(index == 10)
			index = 0;

		memcpy(send_data.u.buf, buf, len);
		if(false == background_task_msg_send(&send_data))
	    {
	        //do something here
	        APP_PRINT_INFO0("[ERROR][FINGERPRINT]:menu task msg send fail");
	    }
    } 
    else 
    {
        APP_PRINT_INFO1("P2PS_APP_Dec_Callback_Handle error %d !\n", status);
    }
}

//加密库密码校验回调函数(加密库的回调函数都需要快进快去，不适宜做过多的操作)
void bluetooth_pw_verify_cb_handle(cyp_status_t status, uint8_t *buf, uint8_t len)
{

	if (status == CYP_OK)
	{
		APP_PRINT_INFO0("bluetooth_pw_verify_cb_handle suc\n");
		if(tmp.flow_port == E_RESP_RESET_LOCK) //恢复出厂设置
		{
			APP_PRINT_INFO0("eRspInitializeLock suc\n");
			s_reset_flag = 1;
		}

		if(tmp.flow_port == E_RESP_BLE_OPEN_DOOR || tmp.flow_port == E_RESP_OPEN_DOOR)
		{
			if(door_open_status() == E_OPEN_NONE)
				bluetooth_open_door();
		}
		
		send_buf[sizeof(sBLECmdHdr)] = 1; //密码校验通过
	}		
	else 
	{
		APP_PRINT_INFO1("bluetooth_pw_verify_cb_handle error %d\n", status);
		
		send_buf[sizeof(sBLECmdHdr)] = 0; //密码错误
	}
		
}


void bluetooth_crypto_en_cb_handle(cyp_status_t status, uint8_t *buf, uint8_t len)
{
	
    if (status == CYP_OK) 
    {
		APP_PRINT_INFO0("bluetooth_crypto_en_cb_handle\n");
    	if(tmp.flow_port == E_RESP_START_ADD_FP) 
			bluetooth_tx_data_encode(buf, len);
		else
       		bluebooth_send_msg_to_menu_task(BT_MSG_SEND_DATA, buf, len);
    } 
    else 
    {
        APP_PRINT_INFO1("bluetooth_crypto_en_cb_handle error %d !\n", status);
    }
}


//机密库获取psk回调函数
void bluetooth_psk_cb_handle(cyp_status_t status, uint8_t *buf, uint8_t len)
{
    if (status == CYP_OK)
    {
    	APP_PRINT_INFO1("bluetooth_psk_cb_handle suc %b\n", TRACE_BINARY(len, buf));		
		send_buf[sizeof(tmp)] = 1;
		memcpy(send_buf + sizeof(tmp) + 1, buf, len);
    }	
	else 
		APP_PRINT_INFO0("bluetooth_psk_cb_handle error\n");
    
}

//对小程序过来的数据进行解密
int bluetooth_rx_data_decode(uint8_t *data, uint16_t len)
{
	if(data == NULL || len <= 0)
	{
		APP_PRINT_INFO0("bluetooth_rx_data_decode input param error\n");
		return 0;
	}
	os_mutex_take(bt_mutex_handle, 0);
	char ret = 0;
	sBLECmdHdr* tmp = (sBLECmdHdr *)data;

	//APP_PRINT_INFO1("********tmp->flow_ctrl is %d *********", tmp->flow_ctrl);
	//APP_PRINT_INFO2("len %d****data %b\n", len, TRACE_BINARY(len, data));
	if(tmp->flow_ctrl == CYP_AUTH_RESP) //认证数据不需要解密
	{
		bluebooth_send_msg_to_menu_task_front(BT_MSG_DATA_PROC, data, len);
	}
	else if(tmp->flow_ctrl == CYP_DATA_DOWN) //其它从小程序过来的都需要解密
	{
		ret = app_crypto_msg_decode(data, len, bluetooth_crypto_dec_cb_handle);
		os_delay(50);
		if(ret == 1) //当加密库在忙的时候再次进行解密
		{			
			ret = app_crypto_msg_decode(data, len, bluetooth_crypto_dec_cb_handle);
		}
		if(ret != CYP_OK)
			APP_PRINT_INFO1("bluetooth_rx_data_decode decoder error %d\n", ret);			
	}
	else
	{
		APP_PRINT_INFO0("bluetooth_rx_data_decode tmp->flow_ctrl error\n");
	}
	
	os_mutex_give(bt_mutex_handle);
	
	return 0;
}

//对解密后的数据进行处理
int bluetooth_data_process(uint8_t *data, uint16_t len)
{
	if(data == NULL || len <= 0)
	{
		APP_PRINT_INFO0("bluetooth_data_process input param error\n");
		return 0;
	}
	os_sem_take(bt_sem_handle, 0);
	
	uint16_t deleteId = 0;
	uint16_t ftrNum = 0;
	uint8_t cmd_type = data[6];  //指令类型
	uint16_t send_len = 0;
	int ret = 0, i = 0;
	T_MENU_MSG menMsg = {0};
	tmp.magic_num = 0xFE;
	tmp.flow_ctrl = CYP_DATA_UP;
	tmp.flow_cnt = 0x0100;
    tmp.flow_mic = 0xCDAB;
    tmp.flow_port = 0x00;
	
	APP_PRINT_INFO1("****cmd type 0x%x ****\n", cmd_type);
	switch(cmd_type)
	{
		case E_AUTH_RESP: //认证加密库
			if(app_crypto_auth_resp(data, len, bluetooth_auth_cb_handle) != 0)
				APP_PRINT_INFO0("app_crypto_auth_resp error!\n");
			rtc_set_time(data[sizeof(sBLECmdHdr)+7]<<24 | data[sizeof(sBLECmdHdr)+6]<<16 | data[sizeof(sBLECmdHdr)+5]<<8 | data[sizeof(sBLECmdHdr)+4]);
			goto no_resp;			
			//break;
			
		case E_GET_CONFIGINFO: //获取配置信息
			tmp.flow_port = E_RESP_GET_CONFIGINFO;
			tmp.data_length = sizeof(productInfo);
			MLAPI_GetFTRNum(&ftrNum);
			productInfo.fingerprintCount = ftrNum;
            productInfo.batteryInfo = get_battery_data();
            productInfo.passwordCount = GetStorePswNum();
			productInfo.passwordMinLength = 4;
			productInfo.passwordMaxLength = 16;
			productInfo.lockType = 9;//防止获取不成功，跳转到其他界面。
			memcpy(send_buf + sizeof(sBLECmdHdr), (void *)&productInfo, tmp.data_length);
			break;
			
		case E_GET_PRODUCT_INFO: //获取产品信息
			tmp.flow_port = E_RESP_GET_PRODUCT_INFO;
			tmp.data_length = sizeof(DEFAULT_BOARD_APP_ID);
			memcpy(send_buf + sizeof(sBLECmdHdr), DEFAULT_BOARD_APP_ID, sizeof(DEFAULT_BOARD_APP_ID));
			break;
			
		case E_VERIFY_PW:  //密码校验
			tmp.flow_port = E_RESP_VERIFY_PW;
			tmp.data_length = 3;	
			APP_PRINT_INFO2("GetStorePswNum is %d, password len is %d", GetStorePswNum(), data[7]);
			if(GetStorePswNum() == 0) //没有密码
			{
				send_buf[sizeof(sBLECmdHdr) + 0] = 0;
				send_buf[sizeof(sBLECmdHdr) + 1] = 0;
				send_buf[sizeof(sBLECmdHdr) + 2] = 1;
			}
			else if(data[7] < 4) //首次进入密码校验界面，还没开始输入密码
			{
				send_buf[sizeof(sBLECmdHdr) + 0] = 0;
				send_buf[sizeof(sBLECmdHdr) + 1] = 1;
				send_buf[sizeof(sBLECmdHdr) + 2] = 0;
			}
			else //传过来的密码数据
			{
				APP_PRINT_INFO1("recv password %b\n", TRACE_BINARY(data[7], &data[8]));
				app_crypto_pw_verify(&data[8], data[7], bluetooth_pw_verify_cb_handle);
				send_buf[sizeof(sBLECmdHdr) + 1] = 1;
				send_buf[sizeof(sBLECmdHdr) + 2] = 0;
			}
			break;

		case E_SET_PASSWORD: //设置密码
			tmp.flow_port = E_RESP_SET_PASSWORD;
			tmp.data_length = 2;
			send_buf[sizeof(sBLECmdHdr) + 0] = 1;  //通常返回1就是成功，0就是失败
			send_buf[sizeof(sBLECmdHdr) + 1] = 0;
			APP_PRINT_INFO1("set password %b\n", TRACE_BINARY(data[7], &data[8]));
			app_crypto_set_pw(&data[9], data[8]);
			StorePassword(1, &data[9], data[8]);
			break;

		case E_GET_SERVER_PSK: //获取psk
			tmp.flow_port = E_RESP_GET_SERVER_PSK;
			tmp.data_length = 5;
			app_crypto_get_psk(bluetooth_psk_cb_handle);	
			break;

		case E_GET_OPEN_DOOR_RECORD: //获取开锁记录
			APP_PRINT_INFO1("recv record data**** %b\n", TRACE_BINARY(data[7], &data[8]));
			bleRecordGet(send_buf+sizeof(sBLECmdHdr), &tmp.data_length, data[8]);
			//BleRecordGet(send_buf+sizeof(sBLECmdHdr), data[8]);
			tmp.flow_port = E_RESP_GET_OPEN_DOOR_RECORD;

			break;

		case E_RESET_LOCK: //锁初始化(恢复出厂设置)
			tmp.flow_port = E_RESP_RESET_LOCK;
			tmp.data_length = 1;
			//加密库的回调底层有点像开了另一个任务在运行，因为这个回调函数的打印还没出来，加载下方的打印都先出来了
		 	app_crypto_pw_verify(&data[8], data[7], bluetooth_pw_verify_cb_handle);
			//恢复设置的处理放在了下面
			break;

		case E_GET_PW_LIST: //获取密码列表
			tmp.flow_port = E_RESP_GET_PW_LIST;
			ret = GetStorePswNum();
			if (ret < 1 || ret >= STORE_MAX_PASSWORD_NUM)
			{
				tmp.data_length = 2;
				send_buf[sizeof(sBLECmdHdr)] = 0;
				send_buf[sizeof(sBLECmdHdr) + 1] = 0;
			}
			else
			{
				tmp.data_length = 2 + ret;
				send_buf[sizeof(sBLECmdHdr)] = 1;
				send_buf[sizeof(sBLECmdHdr) + 1] = ret;
				GetExistPswList(&send_buf[sizeof(sBLECmdHdr) + 2]);
			}
			break;

		case E_DEL_PASSWORD: //删除密码
			tmp.flow_port = E_RESP_DEL_PASSWORD;
			tmp.data_length = 1;
			//APP_PRINT_INFO1("del ps id is %d\n", data[8]);
			if(DeleteOnePassword(data[8]) == 0)
				send_buf[sizeof(sBLECmdHdr)] = 1; //成功
			else 
				send_buf[sizeof(sBLECmdHdr)] = 0; //失败
			break;

		case E_GET_FP_LIST: //获取指纹列表
			tmp.flow_port = E_RESP_GET_FP_LIST;
			GetFpList(&send_buf[sizeof(sBLECmdHdr)]);
			tmp.data_length = send_buf[sizeof(sBLECmdHdr) + 1]+2;
			APP_PRINT_INFO2("fp num is %d data %b\n",tmp.data_length,  TRACE_BINARY(tmp.data_length, send_buf));
			break;

		case E_START_ADD_FP: //添加指纹
			tmp.flow_port = E_RESP_START_ADD_FP;
			APP_PRINT_INFO1("cmd data %b\n", TRACE_BINARY(data[7], &data[8]));	
			if(data[8] == 0)	//添加指纹
				s_bEnroll = true;
			else if(data[8] == 1)  //修改指纹
				s_bEnroll = false;
				
			menMsg.type =  MENU_MSG_TYPE_FINGERPRINT;
			menMsg.subtype = FP_MSG_SUBTYPE_BT_ENROLL;
		   	menu_task_msg_send(&menMsg);			
			goto no_resp;

		case E_DEL_ONE_FP: //删除指纹
			tmp.flow_port = E_RESP_DEL_ONE_FP;
			tmp.data_length = 1;
			deleteId = data[8] - 1;
			//APP_PRINT_INFO1("---deleteId ps id is %d\n", data[8]);
            if(COMP_CODE_OK == MLAPI_DeleteFTR(ERASE_SINGLE_FINGER, 0, &deleteId))
            {
            	send_buf[sizeof(sBLECmdHdr)] = 1;
            }
			else
			{
				send_buf[sizeof(sBLECmdHdr)] = 0;
			}
			break;

		case E_BLE_OPEN_DOOR: //管理员蓝牙开锁
			tmp.flow_port = E_RESP_BLE_OPEN_DOOR;
			tmp.data_length = 1;
			ret = app_crypto_pw_verify(&data[8], data[7], bluetooth_pw_verify_cb_handle);
			APP_PRINT_INFO1("recv password123 %b", TRACE_BINARY(data[7], &data[8]));			
			break;

		case E_OPEN_DOOR:  //临时密钥开锁
			tmp.flow_port = E_RESP_OPEN_DOOR;
			tmp.data_length = 2;
			app_crypto_pw_verify(&data[8], data[7], bluetooth_pw_verify_cb_handle);
			send_buf[sizeof(sBLECmdHdr) + 1] = 0;
			APP_PRINT_INFO1("cmd data %b\n", TRACE_BINARY(data[7], &data[8]));
			break;

		case E_EXIT_CONFIG_MODE:  //退出配置
			APP_PRINT_INFO0("exit configure");
			g_QuitAtOnce = 1;
			tmp.flow_port = E_RESP_EXIT_CONFIG_MODE;
			tmp.data_length = 0;
			break;
			
		case E_SET_OPEN_DOOR_STATE: //设置开门模式		
		case E_ADD_PASSWORD: //增加密码						
		case E_CHANGE_LOCK_NAME: //修改锁名字
		case E_CHANGE_FP_NAME: //修改指纹名字
		case E_CHANGE_VOLUME: //设置音量					
			APP_PRINT_INFO1("cmd data %b\n", TRACE_BINARY(data[7], &data[8]));
			goto no_resp;
			//break;
	
		default:
			APP_PRINT_INFO1("***cmd type 0x%x no need respond***\n", cmd_type);
			goto no_resp;
			//break;
	}
	
	memcpy(send_buf, &tmp, sizeof(sBLECmdHdr));
	send_len = sizeof(sBLECmdHdr) + tmp.data_length;
		
	//当加密库正在使用时，等待三次
	for(i = 0; i < 3; i++)
	{
		ret = app_crypto_msg_encode(send_buf, send_len, bluetooth_crypto_en_cb_handle);
		if(ret == 1) 
		{
			os_delay(20);
		}
		else 
		{
			break;
		}
	}

	if(tmp.flow_port == E_RESP_RESET_LOCK)
	{
		if(s_reset_flag == 1)
		{
	
			T_MENU_MSG msg;
			msg.type = BACKGROUND_MSG_TYPE_LED;
			msg.subtype = BACKGROUND_MSG_LED_SUBTYPE_RESET_INIT;
			background_task_msg_send(&msg);
			s_reset_flag = 0;
			BleRecordInfoReset();
			DeleteAllPassword();
			MLAPI_DeleteFTR(ERASE_ALL_FINGER, 0, &deleteId);
		}
	}
			
no_resp:
	os_sem_give(bt_sem_handle);

	return 0;
}

//回复数据给到小程序
int bluetooth_tx_data_encode(uint8_t *data, uint16_t len)
{
	if(data == NULL || len <= 0)
	{
		APP_PRINT_INFO0("bluetooth_tx_data_encode input param error\n");
		return 0;
	}
	
	int i = 0, sLen = 0;
	int cnt = len / 20;
	cnt += ((len%20)>0) ? 1 : 0;
	
	for(i = 0; i < cnt; i++)
	{
		sLen = (len >= 20) ? 20 : len;
		simp_ble_service_send_v3_notify(0, 1, data+i*20, sLen);
		len -= sLen; 
	}

	return 0;
}


//蓝牙小程序消息处理接口
int bluetooth_handle_msg(T_MENU_MSG *bt_msg)
{
		
	if(bt_msg == NULL)
	{
		APP_PRINT_INFO0("bluetooth_handle_msg input param error\n");
		return 0;
	}

	//APP_PRINT_INFO1("bt_msg->subtyp is 0x%x\n", bt_msg->subtype);
	switch(bt_msg->subtype)
	{
		case BT_MSG_RECV_DECODE:	
			bluetooth_rx_data_decode(bt_msg->u.buf, bt_msg->len);
			break;
			
		case BT_MSG_DATA_PROC:
			bluetooth_data_process(bt_msg->u.buf, bt_msg->len);
			break;

		case BT_MSG_SEND_DATA:
			bluetooth_tx_data_encode(bt_msg->u.buf, bt_msg->len);
			break;
			
		default:
			APP_PRINT_INFO0("bt_msg->subtype error\n");
			break;
	}

	

	return 0;
}


