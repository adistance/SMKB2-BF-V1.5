#include <os_msg.h>
#include <os_task.h>
#include "os_timer.h"
#include "tuya_ble_stdlib.h"
#include "tuya_ble_type.h"
#include "tuya_ble_heap.h"
#include "tuya_ble_mem.h"
#include "tuya_ble_api.h"
#include "tuya_ble_port.h"
#include "tuya_ble_main.h"
#include "tuya_ble_secure.h"
#include "tuya_ble_data_handler.h"
#include "tuya_ble_storage.h"
#include "tuya_ble_sdk_version.h"
#include "tuya_ble_utils.h"
#include "tuya_ble_unix_time.h"
#include "tuya_ble_event.h"
#include "tuya_ble_app_demo.h"
#include "tuya_ble_log.h"
#include "tuya_ble_app_ota.h"
#include "tuya_ble_demo_version.h"
#include "custom_app_product_test.h"
#include "tuya_ble_sdk_test.h"
#include "filesys.h"
#include "menu_manage.h"
#include "driver_led.h"
#include "driver_motor.h"
#include "driver_rtc.h"
#include "record.h"
#include "menu_sleep.h"
#include "os_sched.h"
#include "system_setting.h"
#include "board.h"
#include "rtl876x_gpio.h"
#include "driver_hal.h"

#if ( TUYA_BLE_FEATURE_WEATHER_ENABLE != 0)
#include "tuya_ble_feature_weather.h"
#endif 

#if ( TUYA_BLE_FEATURE_IOT_CHANNEL_ENABLE != 0 )
#include "tuya_ble_feature_iot_channel.h"
#if ( TUYA_BLE_FEATURE_SCENE_ENABLE != 0)
#include "tuya_ble_feature_scene.h"
#endif
#endif 
#include <trace.h>
#include "fingerprint.h"

#if 1
#define TUYA_DEMO_PRINT_INFO0   APP_PRINT_TRACE0
#define TUYA_DEMO_PRINT_INFO1   APP_PRINT_TRACE1
#define TUYA_DEMO_PRINT_INFO2   APP_PRINT_TRACE2
#define TUYA_DEMO_PRINT_INFO3   APP_PRINT_TRACE3
#define TUYA_DEMO_PRINT_INFO4   APP_PRINT_TRACE4
#define TUYA_DEMO_PRINT_INFO7   APP_PRINT_TRACE7
#else
#define TUYA_DEMO_PRINT_INFO0(...)
#define TUYA_DEMO_PRINT_INFO1(...)
#define TUYA_DEMO_PRINT_INFO2(...)
#define TUYA_DEMO_PRINT_INFO3(...)
#define TUYA_DEMO_PRINT_INFO4(...)
#define TUYA_DEMO_PRINT_INFO7(...)

#endif


/*============================================================================*
 *                              Macros
 *============================================================================*/
#define APP_CUSTOM_TASK_PRIORITY             1         //!< Task priorities
#define APP_CUSTOM_TASK_STACK_SIZE           256 * 6   //!<  Task stack size
#define MAX_NUMBER_OF_TUYA_CUSTOM_MESSAGE    0x20      //!<  GAP message queue size
/*============================================================================*
 *                              Variables
 *============================================================================*/
void *app_custom_task_handle = NULL;
void *tuya_custom_queue_handle = NULL;

tuya_ble_device_param_t device_param = {0};
static const char device_local_name[] = "TY";

static const char auth_key_test[] 	= "Gv9lNhontqvObZdsQEK9nXtDqyp6XMaA";//"xGPGdIT7NAQcJ2oAK4kwB52npmBOf1Wn";Gv9lNhontqvObZdsQEK9nXtDqyp6XMaA
static const char device_id_test[] 	= "jx014ffe78d6e4a0";//"tuya658091be1e07";jx014ffe78d6e4a0
static const uint8_t mac_test[]     = "DC234D410A54";//"DC234DCA5D6E";DC234D410A54

//static const char auth_key_test[] =	{0x66,0x32,0x71,0x50,0x47,0x4D,0x4B,0x46,0x36,0x4A,0x45,0x61,0x7A,0x4C,0x59,0x46,0x6A,0x71,0x79,0x73,0x31,0x76,0x52,0x41,0x42,0x6C,0x71,0x70,0x5A,0x4C,0x75,0x45};

//static const char device_id_test[] 	= {0x64,0x64,0x35,0x62,0x63,0x31,0x38,0x65,0x37,0x30,0x34,0x65,0x37,0x63,0x39,0x31};
//static const uint8_t mac_test[]   = {0x42,0x32,0x44,0x39,0x45,0x32,0x34,0x44,0x32,0x33,0x44,0x43};

//static uint8_t s_u8RandNum[8] = {0}; //8字节随机数(用于校验蓝牙开锁)

uint8_t u8AddData[64];
eAPP_for_open  app_open = no_open_from_tuya;
extern uint8_t g_QuitAtOnce;


#define APP_CUSTOM_EVENT_1  1
#define APP_CUSTOM_EVENT_2  2
#define APP_CUSTOM_EVENT_3  3
#define APP_CUSTOM_EVENT_4  4
#define APP_CUSTOM_EVENT_5  5

typedef struct {
    uint8_t data[50];
} custom_data_type_t;


eAPP_for_open Get_App_for_open_flag(void)
{
	return app_open;
}

void Set_App_for_open_flag(eAPP_for_open data)
{
	app_open = data;
}


void app_custom_task_check(void)
{
	tuya_ble_cb_evt_param_t event;
	
	if (os_msg_recv(tuya_custom_queue_handle, &event, 0xFFFFFFFF) == true)
    {
        int16_t result = 0;
        switch (event.evt) {
            case TUYA_BLE_CB_EVT_CONNECTE_STATUS:
                TUYA_APP_LOG_INFO("received tuya ble conncet status update event,current connect status = %d",event.connect_status);
                
                #if TUYA_BLE_SDK_TEST_ENABLE
                tuya_ble_sdk_test_send(TY_UARTV_CMD_GET_DEVICE_STATE, (uint8_t *)&event.connect_status, sizeof(uint8_t));
                #endif
                break;

            case TUYA_BLE_CB_EVT_DP_DATA_RECEIVED:
                TUYA_APP_LOG_DEBUG("received dp write sn = %d", event.dp_received_data.sn);    
                TUYA_APP_LOG_HEXDUMP_DEBUG("received dp write data :", event.dp_received_data.p_data, event.dp_received_data.data_len);
			
                #if TUYA_BLE_SDK_TEST_ENABLE
                tuya_ble_sdk_test_send(TY_UARTV_CMD_DP_WRITE, event.dp_received_data.p_data, event.dp_received_data.data_len);
                #endif
                break;

            case TUYA_BLE_CB_EVT_DP_DATA_SEND_RESPONSE:
                TUYA_APP_LOG_INFO("received dp data send response sn = %d , type = 0x%02x, flag_mode = %d , ack = %d, result code =%d", \
                            event.dp_send_response_data.sn, \
                            event.dp_send_response_data.type, \
                            event.dp_send_response_data.mode, \
                            event.dp_send_response_data.ack, \
                            event.dp_send_response_data.status);

                if (event.dp_send_response_data.mode == DP_SEND_FOR_CLOUD_PANEL) {
                    if (event.dp_send_response_data.type == DP_SEND_TYPE_PASSIVE) {
                        
                    } else {
                        
                    } 
                } else if(event.dp_send_response_data.mode == DP_SEND_FOR_CLOUD) {
                    
                } else if(event.dp_send_response_data.mode == DP_SEND_FOR_PANEL) {
                   
                } else if(event.dp_send_response_data.mode == DP_SEND_FOR_NONE) {
                   
                } else {
                    
                }
                
                #if TUYA_BLE_SDK_TEST_ENABLE
                tuya_ble_sdk_test_dp_report_handler();
                tuya_ble_sdk_test_send(TY_UARTV_CMD_DP_RSP, (uint8_t *)&event.dp_send_response_data.status, sizeof(uint8_t));
                #endif
                break;

            case TUYA_BLE_CB_EVT_DP_DATA_WITH_TIME_SEND_RESPONSE:
                TUYA_APP_LOG_INFO("received dp data with time send response sn = %d, type = 0x%02x, flag_mode = %d , need_ack = %d, result code =%d", \
                            event.dp_with_time_send_response_data.sn,   \
                            event.dp_with_time_send_response_data.type, \
                            event.dp_with_time_send_response_data.mode, \
                            event.dp_with_time_send_response_data.ack,  \
                            event.dp_with_time_send_response_data.status);

                if (event.dp_with_time_send_response_data.mode == DP_SEND_FOR_CLOUD_PANEL) {
                    
                } else if(event.dp_with_time_send_response_data.mode == DP_SEND_FOR_CLOUD) {
                    
                } else if(event.dp_with_time_send_response_data.mode == DP_SEND_FOR_PANEL) {
                    
                } else if(event.dp_with_time_send_response_data.mode == DP_SEND_FOR_NONE) {
                   
                }
                
                #if TUYA_BLE_SDK_TEST_ENABLE
                tuya_ble_sdk_test_dp_report_handler();
                tuya_ble_sdk_test_send(TY_UARTV_CMD_DP_RSP, (uint8_t *)&event.dp_with_time_send_response_data.status, sizeof(uint8_t));
                #endif
                break;        
             
            case TUYA_BLE_CB_EVT_BULK_DATA: 
                TUYA_APP_LOG_INFO("received bulk data response");
                //tuya_ble_bulk_data_demo_handler(&event.bulk_req_data);
                break;
            
        #if (TUYA_BLE_FEATURE_WEATHER_ENABLE != 0)
        	case TUYA_BLE_CB_EVT_WEATHER_DATA_REQ_RESPONSE:
        		TUYA_APP_LOG_INFO("received weather data request response result code =%d", event.weather_req_response_data.status);
                
                #if TUYA_BLE_SDK_TEST_ENABLE
                uint8_t  rsp_data[2] = {1};
                uint32_t rsp_len = 2;
                rsp_data[1] = event.weather_req_response_data.status;
                tuya_ble_sdk_test_send(TY_UARTV_CMD_GET_WEATHER, rsp_data, rsp_len);
                #endif
        		break;
        	
        	case TUYA_BLE_CB_EVT_WEATHER_DATA_RECEIVED:
        	{
        		TUYA_APP_LOG_DEBUG("received weather data, location = [%d] object_count = [%d]", event.weather_received_data.location, event.weather_received_data.object_count);    
                
        		tuya_ble_wd_object_t *object;
        		uint16_t object_len = 0;

        		for (;;)
        		{
        			object = (tuya_ble_wd_object_t *)(event.weather_received_data.p_data + object_len);
        			
        			TUYA_APP_LOG_DEBUG("weather data, n_day=[%d] key=[0x%08x] val_type=[%d] val_len=[%d]", \
        							object->n_day, object->key_type, object->val_type, object->value_len); 
        			TUYA_APP_LOG_HEXDUMP_DEBUG("vaule :", (uint8_t *)object->vaule, object->value_len);	
                    
                    // TODO .. YOUR JOBS 
                    
        			object_len += (sizeof(tuya_ble_wd_object_t) + object->value_len);
        			if (object_len >= event.weather_received_data.data_len)
        				break;
        		}
                
                #if TUYA_BLE_SDK_TEST_ENABLE
                tuya_ble_sdk_test_send(TY_UARTV_CMD_GET_WEATHER, event.weather_received_data.p_data, event.weather_received_data.data_len);
                #endif 
        	}
        		break;
        #endif 	        
	
		#if ( TUYA_BLE_FEATURE_IOT_CHANNEL_ENABLE != 0 )
			#if ( TUYA_BLE_FEATURE_SCENE_ENABLE != 0 )
			case TUYA_BLE_CB_EVT_SCENE_REQ_RESPONSE:
				TUYA_APP_LOG_INFO("received scene request response result code=[%d], err_code=[%d] sub_cmd=[%d]", \
									event.scene_req_response_data.status, \
									event.scene_req_response_data.err_code, \
									event.scene_req_response_data.scene_cmd);
				break;
			
			case TUYA_BLE_CB_EVT_SCENE_DATA_RECEIVED:
				TUYA_APP_LOG_INFO("received scene data result, status=[%d] err_code=[%d] need_update=[%d] check_code=[0x%X]", \
									event.scene_data_received_data.status, \
									event.scene_data_received_data.err_code, \
									event.scene_data_received_data.need_update, \
									event.scene_data_received_data.check_code);

				if (event.scene_data_received_data.status == 0 && event.scene_data_received_data.need_update)
				{
					// TODO .. please storage update scene check code.
					
					if (event.scene_data_received_data.data_len != 0)
					{
						uint8_t *iot_scene_object;
						uint16_t scene_id_len, scene_name_len;
						uint16_t object_len = 0;

						for (;;) 
						{
							iot_scene_object = (uint8_t *)(event.scene_data_received_data.p_data + object_len);
							
							scene_id_len = iot_scene_object[0];
							scene_name_len = (iot_scene_object[1+scene_id_len] << 8) + iot_scene_object[2+scene_id_len];

							TUYA_APP_LOG_HEXDUMP_DEBUG("scene id :", &iot_scene_object[1], scene_id_len);	
							TUYA_APP_LOG_HEXDUMP_DEBUG("scene name unicode :", &iot_scene_object[3+scene_id_len], scene_name_len);	

							object_len += (3 + scene_id_len + scene_name_len);
							if (object_len >= event.scene_data_received_data.data_len)
								break;
						}
					}
				}
				break;
				
			case TUYA_BLE_CB_EVT_SCENE_CTRL_RESULT_RECEIVED:
				TUYA_APP_LOG_INFO("received scene ctrl result, status=[%d] err_code=[%d]", \
								event.scene_ctrl_received_data.status, \
								event.scene_ctrl_received_data.err_code);
				TUYA_APP_LOG_HEXDUMP_DEBUG("scene ctrl id :", event.scene_ctrl_received_data.p_scene_id, event.scene_ctrl_received_data.scene_id_len);	
				break;
			#endif 
		#endif 

            case TUYA_BLE_CB_EVT_DP_QUERY:
                if (event.dp_query_data.data_len > 0)
                {
                    TUYA_APP_LOG_HEXDUMP_DEBUG("received TUYA_BLE_CB_EVT_DP_QUERY event, query dp id :",event.dp_query_data.p_data,event.dp_query_data.data_len);
                } 
                else 
                {
                    TUYA_APP_LOG_INFO("received TUYA_BLE_CB_EVT_DP_QUERY event, query all dp");
                }
                break;
                
            case TUYA_BLE_CB_EVT_OTA_DATA:
                TUYA_APP_LOG_INFO("received TUYA_BLE_CB_EVT_OTA_DATA event");
				tuya_ota_proc(event.ota_data.type, event.ota_data.p_data, event.ota_data.data_len);
                break;
                
            case TUYA_BLE_CB_EVT_NETWORK_INFO:
                TUYA_APP_LOG_INFO("received net info : %s", event.network_data.p_data);
                tuya_ble_net_config_response(result);
                break;
                
            case TUYA_BLE_CB_EVT_WIFI_SSID:
                break;
                
            case TUYA_BLE_CB_EVT_TIME_STAMP:
                TUYA_APP_LOG_INFO("received unix timestamp : %s ,time_zone : %d",event.timestamp_data.timestamp_string,event.timestamp_data.time_zone);
    			uint64_t time_stamp_ms;
    			uint32_t time_stamp;

    			time_stamp_ms = atoll((char *)event.timestamp_data.timestamp_string);
    			time_stamp = (time_stamp_ms / 1000);
    			if ((time_stamp_ms % 1000) >= 500)
    				time_stamp += 1;
               
                #if TUYA_BLE_SDK_TEST_ENABLE 
				//tuya_ble_bulk_data_generation(60, NULL, 0);
				
                tuya_ble_time_struct_data_t data;
                tuya_ble_time_noraml_data_t normal_data;
                    
                tuya_ble_utc_sec_2_mytime(time_stamp, &data, false);
                memcpy(&normal_data, &data, sizeof(tuya_ble_time_struct_data_t));
                normal_data.time_zone = event.timestamp_data.time_zone;
                
                tuya_ble_sdk_test_get_time_rsp(&normal_data);
                #endif
                break;
                
            case TUYA_BLE_CB_EVT_TIME_NORMAL:
            case TUYA_BLE_CB_EVT_APP_LOCAL_TIME_NORMAL:
        		TUYA_APP_LOG_HEXDUMP_DEBUG("received normal time", (uint8_t *)&(event.time_normal_data), sizeof(tuya_ble_time_noraml_data_t));
        		TUYA_APP_LOG_INFO(" timezone = [%d]", event.time_normal_data.time_zone);
                
                #if TUYA_BLE_SDK_TEST_ENABLE
                tuya_ble_sdk_test_get_time_rsp(&event.time_normal_data);
                #endif
        		break;
    
            case TUYA_BLE_CB_EVT_DATA_PASSTHROUGH:
                TUYA_APP_LOG_HEXDUMP_DEBUG("received ble passthrough data :", event.ble_passthrough_data.p_data,event.ble_passthrough_data.data_len);
               
                #if TUYA_BLE_SDK_TEST_ENABLE
                tuya_ble_sdk_test_send(TY_UARTV_CMD_PASSTHROUGH_WRITE, event.ble_passthrough_data.p_data, event.ble_passthrough_data.data_len);
                #endif
                break;
            
            case TUYA_BLE_CB_EVT_UPDATE_LOGIN_KEY_VID:
                TUYA_APP_LOG_HEXDUMP_DEBUG("binding success -> login key :",event.device_login_key_vid_data.login_key,event.device_login_key_vid_data.login_key_len);
                TUYA_APP_LOG_HEXDUMP_DEBUG(" -> beacon key :",event.device_login_key_vid_data.beacon_key,BEACON_KEY_LEN);
                TUYA_APP_LOG_HEXDUMP_DEBUG(" -> vid :",event.device_login_key_vid_data.vid,event.device_login_key_vid_data.vid_len);
                break; 
            
            case TUYA_BLE_CB_EVT_UNBOUND:
                TUYA_APP_LOG_INFO("received unbound req\n");
                
                #if TUYA_BLE_SDK_TEST_ENABLE
                tuya_ble_sdk_test_unbind_mode_rsp(0);
                #endif
                break;
                
            case TUYA_BLE_CB_EVT_ANOMALY_UNBOUND:
				
				
                TUYA_APP_LOG_INFO("received anomaly unbound req\n");
                #if TUYA_BLE_SDK_TEST_ENABLE
                tuya_ble_sdk_test_unbind_mode_rsp(1);
                #endif
                break;
                
            case TUYA_BLE_CB_EVT_DEVICE_RESET:
                TUYA_APP_LOG_INFO("received device reset req\n");
                
                #if TUYA_BLE_SDK_TEST_ENABLE
                tuya_ble_sdk_test_unbind_mode_rsp(2);
                #endif
                break;
                
            case TUYA_BLE_CB_EVT_UNBIND_RESET_RESPONSE:
                if (event.reset_response_data.type == RESET_TYPE_UNBIND)
                {
                    if (event.reset_response_data.status==0)
                    {
                        TUYA_APP_LOG_INFO("device unbind succeed.");
                        
                        #if TUYA_BLE_SDK_TEST_ENABLE
                        uint8_t device_reset_mode = 3;    
                        tuya_ble_sdk_test_send(TY_UARTV_CMD_UNBIND_MODE, &device_reset_mode, 1);
                        #endif
                    }
                    else
                    {
                        TUYA_APP_LOG_INFO("device unbind failed.");
                    }
                }
                else if (event.reset_response_data.type == RESET_TYPE_FACTORY_RESET)
                {
                    if (event.reset_response_data.status==0)
                    {
                        TUYA_APP_LOG_INFO("device factory reset succeed.");
                        
                        #if TUYA_BLE_SDK_TEST_ENABLE
                        uint8_t device_reset_mode = 4;    
                        tuya_ble_sdk_test_send(TY_UARTV_CMD_UNBIND_MODE, &device_reset_mode, 1);
                        #endif
                    }
                    else
                    {
                        TUYA_APP_LOG_INFO("device factory reset failed.");
                    }
                }
                break;
    
            default:
                TUYA_APP_LOG_WARNING("app_tuya_cb_queue msg: unknown event type 0x%04x", event.evt);
                break;
        }

        tuya_ble_event_response(&event);
    }
}

//添加指纹
uint8_t tuya_ble_add_fp(uint32_t u32Sn, uint8_t * pu8In, uint32_t u32InLen)
{

	uint8_t u8Send[64] = {0};
	uint32_t u32SendLen;
	tuya_ble_status_t ret;

	uint32_t ML_Error;
    uint8_t progress = 0, uEnrollNum = 1,uPressErrorNum = 0;
    uint16_t index = 0xFFFF;		//保存的ID号

	//head
	u8Send[0] = DP_ID_ADD_FP; //dp id
	u8Send[1] = DP_TYPE_RAW; //dp type
	u8Send[2] = 0x00;		//dp data len
	u8Send[3] = 0x07;
	//data
	u8Send[4] = 0x03; //类型
	u8Send[5] = ENROLL_START; //阶段
	u8Send[6] = u8AddData[6];//pu8In[6]; //管理员标志
	u8Send[7] = u8AddData[7];//pu8In[7]; //成员ID
	u8Send[8] = 0xFF; //硬件ID(也是指纹的ID)
	u8Send[9] = SYSSET_GetEnrollNum();	  //注册次数
	u8Send[10] = 0x00; //返回状态
	u32SendLen = 11;

	suspend_task();

	TUYA_DEMO_PRINT_INFO1("-----------fileSys_getUnuseSmallestIndex is %d", u8Send[7]);
	TUYA_DEMO_PRINT_INFO1("send is %b", TRACE_BINARY(u32SendLen, u8Send));
	ret = tuya_ble_dp_data_send(++u32Sn, DP_SEND_TYPE_ACTIVE, DP_SEND_FOR_CLOUD_PANEL, DP_SEND_WITHOUT_RESPONSE, u8Send, u32SendLen);

	TUYA_DEMO_PRINT_INFO1("tuya_ble_dp_data_send ret is 0x%x", ret);

	//指纹满了 直接返回
    if(fileSys_getUnuseSmallestIndex() >= STORE_MAX_FTR)
    {
    	u8Send[5] = ENROLL_FAIL; //阶段
		u8Send[9] = ENROLL_ING;	  //次数
		u8Send[10] = E_ENROLL_FULL; //返回状态
		TUYA_DEMO_PRINT_INFO1("send is %b", TRACE_BINARY(u32SendLen, u8Send));
		ret = tuya_ble_dp_data_send(++u32Sn, DP_SEND_TYPE_ACTIVE, DP_SEND_FOR_CLOUD_PANEL, DP_SEND_WITHOUT_RESPONSE, u8Send, u32SendLen);
		TUYA_DEMO_PRINT_INFO1("tuya_ble_dp_data_send ret is 0x%x", ret);

		goto out;
    }

	
	while(progress < 100)
    {
    	background_msg_set_led(BACKGROUND_MSG_LED_SUBTYPE_BLUE_ON);
		menu_sleep_event_control(MENU_SLEEP_EVENT_WAKEUP_BIT, true);
    	ML_Error = MLAPI_Enrollment(uEnrollNum, &index, &progress, 5000);

        if(COMP_CODE_OK != ML_Error) 
        {
        	u8Send[5] = ENROLL_FAIL; //阶段
			u8Send[9] = ENROLL_ING;	  //次数
			u8Send[10] = E_ENROLL_FAIL; //返回状态
			
        	background_msg_set_led(BACKGROUND_MSG_LED_SUBTYPE_ENROLL_FAIL);
        	TUYA_DEMO_PRINT_INFO1("ML_Error:%d\r\n",ML_Error);
            if(COMP_CODE_NO_FINGER_DECTECT == ML_Error)
            {
        	    TUYA_DEMO_PRINT_INFO1("DEMO:Registration failed,errorCode:%d\r\n", ML_Error);//可能本次质量不佳或面积过小等，需尝试本次再次注册，这里不做处理。				
				u8Send[10] = E_ENROLL_TIMEOUT; //返回状态
			}
            else if(COMP_CODE_STORAGE_IS_FULL == ML_Error)
            {
				u8Send[10] = E_ENROLL_FULL; //返回状态
            }
            else if(COMP_CODE_STORAGE_REPEAT_FINGERPRINT == ML_Error)
           {		
            	background_msg_set_led(BACKGROUND_MSG_LED_SUBTYPE_PRESS_FAIL);
				uPressErrorNum++;
              if(uPressErrorNum >= 3)
              {
                uPressErrorNum = 0;
              }
              else
              {
                while(MLAPI_QueryFingerPresent());
                continue;
              }
				u8Send[10] = E_ENROLL_REPEAT; //返回状态
            }
            else if(COMP_CODE_FORCE_QUIT == ML_Error)
            {
            	TUYA_DEMO_PRINT_INFO0("quit add fp suc");
				u8Send[5] = ENROLL_CANCEL; //阶段
                u8Send[10] = 0; //返回状态
            }
			else
			{
				continue;
			}
			TUYA_DEMO_PRINT_INFO1("send is %b", TRACE_BINARY(u32SendLen, u8Send));
			ret = tuya_ble_dp_data_send(++u32Sn, DP_SEND_TYPE_ACTIVE, DP_SEND_FOR_CLOUD_PANEL, DP_SEND_WITHOUT_RESPONSE, u8Send, u32SendLen);
			TUYA_DEMO_PRINT_INFO1("tuya_ble_dp_data_send ret is 0x%x", ret);

out:			
			background_msg_set_led(BACKGROUND_MSG_LED_SUBTYPE_ENROLL_FAIL);
		}
		else
		{
			TUYA_DEMO_PRINT_INFO1("DEMO:This press[%d] registration is successful.\r\n", uEnrollNum);
			background_msg_set_led(BACKGROUND_MSG_LED_SUBTYPE_PRESS_SUC);
			background_msg_set_beep(100, 1);
			u8Send[5] = ENROLL_ING; //阶段
			u8Send[8] = 0xFF; //硬件ID
			u8Send[9] = uEnrollNum;	  //次数
			u8Send[10] = 0x00; //返回状态
			TUYA_DEMO_PRINT_INFO1("send is %b", TRACE_BINARY(u32SendLen, u8Send));
			ret = tuya_ble_dp_data_send(++u32Sn, DP_SEND_TYPE_ACTIVE, DP_SEND_FOR_CLOUD_PANEL, DP_SEND_WITHOUT_RESPONSE, u8Send, u32SendLen);
			TUYA_DEMO_PRINT_INFO1("tuya_ble_dp_data_send ret is 0x%x", ret);
			
            if(progress != 100)//按下最后一次注册的指纹后不闪灯，直接下一步保存
            {         									
                uEnrollNum ++;   					
            }			
		}
		if(progress != 100)	//按下最后一次注册的指纹后不等待手指离开，直接下一步保存
        	while(MLAPI_QueryFingerPresent());      //等待手指离开
    }

	ML_Error = MLAPI_StorageFtr(index, u8Send[7]);
    if(COMP_CODE_OK == ML_Error)
    {
        TUYA_DEMO_PRINT_INFO1("DEMO:Save success., index is %d\r\n", index);
		background_msg_set_led(BACKGROUND_MSG_LED_SUBTYPE_ENROLL_SUCCESS);
		background_msg_set_beep(100, 2);
		u8Send[5] = ENROLL_OK; //阶段
		u8Send[8] = index; //硬件ID
		u8Send[9] = 0;	  //次数	
		u8Send[10] = 0x00; //返回状态
    }
	else
	{
		TUYA_DEMO_PRINT_INFO1("DEMO:Save failed[%d].\r\n", ML_Error);		
		background_msg_set_led(BACKGROUND_MSG_LED_SUBTYPE_ENROLL_FAIL);
		u8Send[5] = ENROLL_FAIL; //阶段
		u8Send[9] = ENROLL_ING;	  //次数	
		u8Send[10] = E_ENROLL_FAIL; //返回状态
	}

	TUYA_DEMO_PRINT_INFO1("send is %b", TRACE_BINARY(u32SendLen, u8Send));
	ret = tuya_ble_dp_data_send(++u32Sn, DP_SEND_TYPE_ACTIVE, DP_SEND_FOR_CLOUD_PANEL, DP_SEND_WITHOUT_RESPONSE, u8Send, u32SendLen);
	TUYA_DEMO_PRINT_INFO1("tuya_ble_dp_data_send ret is 0x%x", ret);	
	
	while(MLAPI_QueryFingerPresent());      //等待手指离开

	resume_task(false);
	
	return 0;
}

//删除指纹
uint8_t tuya_ble_del_fp(uint32_t u32Sn, uint8_t * pu8In, uint32_t u32InLen)
{
	if(pu8In == NULL)
	{
		TUYA_DEMO_PRINT_INFO0("input param error");
		return 0;
	}

	uint8_t u8Send[64] = {0};
	uint32_t u32SendLen;
	tuya_ble_status_t ret;
	uint16_t u16NumId = pu8In[7];  //成员ID
	uint16_t u16HardId = pu8In[8]; //硬件ID，就是指纹ID
	E_OPEN_TYPE eType = (E_OPEN_TYPE)pu8In[4]; //删除的类型

	u32SendLen = u32InLen + 1;
	memcpy(u8Send, pu8In, u32SendLen);
	
	u8Send[3] = 7; //长度
	
	if(eType == E_DEL_NUM)  //删除成员下面的所有指纹
	{	
		if(COMP_CODE_OK == MLAPI_DeleteFTR(ERASE_NUMBER_FINGER, 0, &u16NumId))
		{
			u8Send[u32SendLen - 1] = 0xFF; //删除成功
		}
		else
		{
			u8Send[u32SendLen - 1] = 0x00; //删除失败
		}	
	}
	else if(eType == E_FINGER) //删除指定的指纹ID
	{
		if(COMP_CODE_OK == MLAPI_DeleteFTR(ERASE_SINGLE_FINGER, 0, &u16HardId))
		{
			u8Send[u32SendLen - 1] = 0xFF; //删除成功
		}
		else
		{
			u8Send[u32SendLen - 1] = 0x00; //删除失败
		}
	}
		
	TUYA_DEMO_PRINT_INFO1("send is %b", TRACE_BINARY(u32SendLen, u8Send));
	ret = tuya_ble_dp_data_send(u32Sn + 1, DP_SEND_TYPE_ACTIVE, DP_SEND_FOR_CLOUD_PANEL, DP_SEND_WITHOUT_RESPONSE, u8Send, u32SendLen);

	TUYA_DEMO_PRINT_INFO1("tuya_ble_dp_data_send ret is 0x%x", ret);

	return 0;
}

//同步开锁信息
uint8_t tuya_ble_info_sync(uint32_t u32Sn, uint8_t * pu8In, uint32_t u32InLen)
{
	if(pu8In == NULL)
	{
		TUYA_DEMO_PRINT_INFO0("input param error");
		return 0;
	}

	uint8_t i, j = 0;
	uint8_t u8Temp[STORE_MAX_FTR];
	uint8_t u8Send[64] = {0};
	uint32_t u32SendLen;
	tuya_ble_status_t ret;
	int result;
	int nFtrNum = fileSys_getStoreFtrNum();

	TUYA_DEMO_PRINT_INFO1("------nFtrNum is %d------", nFtrNum);
	u8Send[0] = DP_ID_SYNC;
	u8Send[1] = DP_TYPE_RAW;
	u8Send[2] = 0;
	u8Send[3] = nFtrNum*4+2;
	
	u8Send[4] = 0; //阶段，同步中
	u8Send[5] = 0; //数据包序号

	for (i = 0; i < STORE_MAX_FTR; ++i)
	{
		result = fileSys_IdToIndex(i);
		if (result != -1)
		{
			u8Temp[j] = result;//i+1;
			j++;
		}
	}
	
	//同步数据（硬件ID+硬件类型+成员ID+冻结状态）
	for(i = 0; i < nFtrNum; i++)
	{
		u8Send[6+4*i] = u8Temp[i];
		u8Send[7+4*i] = E_FINGER;
		u8Send[8+4*i] = fileSys_GetNumId(u8Temp[i]);	//成员ID
		u8Send[9+4*i] = 1; //冻结状态 1是未冻结  0是冻结
	}

	u32SendLen = 6 + nFtrNum * 4;

	TUYA_DEMO_PRINT_INFO1("send is %b", TRACE_BINARY(u32SendLen, u8Send));
	ret = tuya_ble_dp_data_send(++u32Sn, DP_SEND_TYPE_ACTIVE, DP_SEND_FOR_CLOUD_PANEL, DP_SEND_WITHOUT_RESPONSE, u8Send, u32SendLen);

	TUYA_DEMO_PRINT_INFO1("tuya_ble_dp_data_send ret is 0x%x", ret);

	u8Send[3] = 2;
	u8Send[4] = 1; //阶段，同步结束
	u8Send[5] = 1; //总包数
	u32SendLen = 6;

	TUYA_DEMO_PRINT_INFO1("send is %b", TRACE_BINARY(u32SendLen, u8Send));
	ret = tuya_ble_dp_data_send(++u32Sn, DP_SEND_TYPE_ACTIVE, DP_SEND_FOR_CLOUD_PANEL, DP_SEND_WITHOUT_RESPONSE, u8Send, u32SendLen);

	TUYA_DEMO_PRINT_INFO1("tuya_ble_dp_data_send ret is 0x%x", ret);

	return 0;
}

//上报开锁记录
uint8_t tuya_ble_get_record(uint32_t u32Time, uint32_t u32FpId)
{
	uint8_t u8Send[64] = {0};
	uint32_t u32SendLen;
	uint8_t u8Time[4] = {0};
	tuya_ble_status_t ret;

	TUYA_DEMO_PRINT_INFO1("time1 is 0x%x", u32Time);
	tuya_ble_time_struct_data_t data;
	
	tuya_ble_utc_sec_2_mytime(u32Time, &data, false);
	
	TUYA_DEMO_PRINT_INFO7("---tuya_ble_get_record is %d:%d:%d--%d:%d:%d--%d", data.nYear, data.nMonth, data.nDay, data.nHour, data.nMin, data.nSec, data.DayIndex);

	memcpy(u8Time, &u32Time, sizeof(u8Time));
	
	TUYA_DEMO_PRINT_INFO1("time2 is %b", TRACE_BINARY(4, u8Time));

	u8Time[0] = (u32Time >> 24) & 0xFF; 
	u8Time[1] = (u32Time >> 16) & 0xFF; 
	u8Time[2] = (u32Time >> 8) & 0xFF;
	u8Time[3] = u32Time & 0xFF;

	TUYA_DEMO_PRINT_INFO1("time3 is %b", TRACE_BINARY(4, u8Time));

	u8Send[0] = DP_ID_FP_OPEN;
	u8Send[1] = DP_TYPE_VALUE;
	u8Send[2] = 0;
	u8Send[3] = 4;
	
	u8Send[4] = (u32FpId >> 24) & 0xFF; 
	u8Send[5] = (u32FpId >> 16) & 0xFF; 
	u8Send[6] = (u32FpId >> 8) & 0xFF;
	u8Send[7] = u32FpId & 0xFF;

	u32SendLen = 8;

	TUYA_DEMO_PRINT_INFO1("send is %b", TRACE_BINARY(u32SendLen, u8Send));
	ret = tuya_ble_dp_data_with_time_send(DP_ID_FP_OPEN, DP_SEND_FOR_CLOUD_PANEL, \
								DP_TIME_TYPE_UNIX_TIMESTAMP, u8Time, u8Send, u32SendLen);

	TUYA_DEMO_PRINT_INFO1("tuya_ble_dp_data_send ret is 0x%x", ret);

	return 0;
}

//上报电池电量
uint8_t tuya_ble_get_battery(void)
{
	uint8_t u8Send[64] = {0};
	uint32_t u32SendLen;
	tuya_ble_status_t ret;

	u8Send[0] = DP_ID_BATTERY;
	u8Send[1] = DP_TYPE_VALUE;
	u8Send[2] = 0;
	u8Send[3] = 4;
	
	u8Send[4] = 0; 
	u8Send[5] = 0; 
	u8Send[6] = 0;
	u8Send[7] = get_battery_data();

	u32SendLen = 8;

	TUYA_DEMO_PRINT_INFO1("send is %b", TRACE_BINARY(u32SendLen, u8Send));
	ret = tuya_ble_dp_data_send(DP_ID_BATTERY, DP_SEND_TYPE_ACTIVE, DP_SEND_FOR_CLOUD_PANEL, DP_SEND_WITHOUT_RESPONSE, u8Send, u32SendLen);

	TUYA_DEMO_PRINT_INFO1("tuya_ble_dp_data_send ret is 0x%x", ret);


	return 0;
}

/*
	功能：上报锁开合状态
	参数：true 锁打开，false 锁关闭
*/
uint8_t tuya_ble_report_door_status(bool bStatus)
{
	uint8_t u8Send[64] = {0};
	uint32_t u32SendLen;

	u8Send[0] = DP_ID_DOOR_STATUS;
	u8Send[1] = DP_TYPE_BOOL;
	u8Send[2] = 0;	//len
	u8Send[3] = 1;  //len

	//data	
	u8Send[4] = bStatus;  

	u32SendLen = 5;

	TUYA_DEMO_PRINT_INFO1("send is %b", TRACE_BINARY(u32SendLen, u8Send));
	tuya_ble_dp_data_send(DP_ID_BATTERY, DP_SEND_TYPE_ACTIVE, DP_SEND_FOR_CLOUD_PANEL, DP_SEND_WITHOUT_RESPONSE, u8Send, u32SendLen);

	return 0;
}


//配置蓝牙开锁信息
uint8_t tuya_ble_set_config(uint32_t u32Sn, uint8_t * pu8In, uint32_t u32InLen)
{
	if(pu8In == NULL)
	{
		TUYA_DEMO_PRINT_INFO0("input param error");
		return 0;
	}

	uint8_t u8Send[64] = {0};
	uint32_t u32SendLen;
	tuya_ble_status_t ret;

	u32SendLen = u32InLen - 8 + 1;
	memcpy(u8Send, pu8In, u32SendLen);

	//data len
	u8Send[3] = pu8In[3] - 8 +1;
	//调换一下主机ID和从机ID的位置
	u8Send[4] = pu8In[6];
	u8Send[5] = pu8In[7];

	u8Send[6] = pu8In[4];
	u8Send[7] = pu8In[5];

	memcpy(&u8Send[8], &pu8In[u32InLen-8], 8);

	u8Send[u32SendLen-1] = 0;

	TUYA_DEMO_PRINT_INFO1("send is %b", TRACE_BINARY(u32SendLen, u8Send));
	ret = tuya_ble_dp_data_send(u32Sn + 1, DP_SEND_TYPE_ACTIVE, DP_SEND_FOR_CLOUD_PANEL, DP_SEND_WITHOUT_RESPONSE, u8Send, u32SendLen);

	TUYA_DEMO_PRINT_INFO1("tuya_ble_dp_data_send ret is 0x%x", ret);

	return 0;
}


//开门模式上报
uint8_t tuya_ble_open_mode_report(void)
{
	uint8_t u8Send[20] = {0};
	uint32_t u32SendLen;
	tuya_ble_status_t ret;
	
	u8Send[0] = DP_ID_OPEN_MODE;  //dp pid
	u8Send[1] = DP_TYPE_BOOL;  //dp type
	u8Send[2] = 0;			//dp len
	u8Send[3] = 1;			//dp len
	u8Send[4] = 1;//SYSSET_GetOpenMode();		//dp data

	u32SendLen = 5;

	TUYA_DEMO_PRINT_INFO1("send is %b", TRACE_BINARY(u32SendLen, u8Send));
	ret = tuya_ble_dp_data_send(DP_ID_OPEN_MODE, DP_SEND_TYPE_ACTIVE, DP_SEND_FOR_CLOUD_PANEL, DP_SEND_WITHOUT_RESPONSE, u8Send, u32SendLen);

	TUYA_DEMO_PRINT_INFO1("tuya_ble_dp_data_send ret is 0x%x", ret);

	return 0;
}


//警告信息上报
uint8_t tuya_ble_alarm_report(E_ALARM_TYPE eType)
{
	if(eType > E_BUFANG)
	{
		TUYA_DEMO_PRINT_INFO0("input param error");
		return 0;
	}

	uint8_t u8Send[64] = {0};
	uint32_t u32SendLen;
	tuya_ble_status_t ret;

	u8Send[0] = DP_ID_ALARM;  //dp pid
	u8Send[1] = DP_TYPE_ENUM;  //dp type
	u8Send[2] = 0;			//dp len
	u8Send[3] = 1;			//dp len
	u8Send[4] = eType;		//dp len

	u32SendLen = 5;
	
	TUYA_DEMO_PRINT_INFO1("send is %b", TRACE_BINARY(u32SendLen, u8Send));
	ret = tuya_ble_dp_data_send(DP_ID_ALARM, DP_SEND_TYPE_ACTIVE, DP_SEND_FOR_CLOUD_PANEL, DP_SEND_WITHOUT_RESPONSE, u8Send, u32SendLen);

	TUYA_DEMO_PRINT_INFO1("tuya_ble_dp_data_send ret is 0x%x", ret);

	u8Send[0] = DP_ID_BATTERY;
	u8Send[1] = DP_TYPE_VALUE;
	u8Send[2] = 0;
	u8Send[3] = 4;
	
	u8Send[4] = 0; 
	u8Send[5] = 0; 
	u8Send[6] = 0;
	u8Send[7] = get_battery_data();

	u32SendLen = 8;

	TUYA_DEMO_PRINT_INFO1("send is %b", TRACE_BINARY(u32SendLen, u8Send));
	ret = tuya_ble_dp_data_send(DP_ID_BATTERY, DP_SEND_TYPE_ACTIVE, DP_SEND_FOR_CLOUD_PANEL, DP_SEND_WITHOUT_RESPONSE, u8Send, u32SendLen);

	TUYA_DEMO_PRINT_INFO1("tuya_ble_dp_data_send ret is 0x%x", ret);

	return 0;
}

//远程开锁配置
uint8_t tuya_ble_remote_config(uint32_t u32Sn, uint8_t * pu8In, uint32_t u32InLen)
{
	if(pu8In == NULL)
	{
		TUYA_DEMO_PRINT_INFO0("input param error");
		return 0;
	}

	uint8_t u8Send[64] = {0};
	uint32_t u32SendLen;
	tuya_ble_status_t ret;

	u32SendLen = u32InLen - 18 + 1;
	memcpy(u8Send, pu8In, u32SendLen);
	
	//data len
	u8Send[3] = pu8In[3] - 18 + 1;
	//调换一下主机ID和从机ID的位置
	u8Send[4] = pu8In[6];
	u8Send[5] = pu8In[7];

	u8Send[6] = pu8In[4];
	u8Send[7] = pu8In[5];

	u8Send[u32SendLen-1] = 0;

	TUYA_DEMO_PRINT_INFO1("send is %b", TRACE_BINARY(u32SendLen, u8Send));
	ret = tuya_ble_dp_data_send(++u32Sn, DP_SEND_TYPE_ACTIVE, DP_SEND_FOR_CLOUD_PANEL, DP_SEND_WITHOUT_RESPONSE, u8Send, u32SendLen);

	TUYA_DEMO_PRINT_INFO1("tuya_ble_dp_data_send ret is 0x%x", ret);

	return 0;
}

//远程开锁
uint8_t tuya_ble_remote_open(uint32_t u32Sn, uint8_t * pu8In, uint32_t u32InLen)
{
	if(pu8In == NULL)
	{
		TUYA_DEMO_PRINT_INFO0("input param error");
		return 0;
	}

	uint8_t u8Send[64] = {0};
	uint32_t u32SendLen;
	tuya_ble_status_t ret;

	u32SendLen = u32InLen - 10;
	memcpy(u8Send, pu8In, u32SendLen);
	
	//data len
	u8Send[3] = pu8In[3] - 10;

	u8Send[4] = 0;

	TUYA_DEMO_PRINT_INFO1("send is %b", TRACE_BINARY(u32SendLen, u8Send));
	ret = tuya_ble_dp_data_send(++u32Sn, DP_SEND_TYPE_ACTIVE, DP_SEND_FOR_CLOUD_PANEL, DP_SEND_WITHOUT_RESPONSE, u8Send, u32SendLen);

	TUYA_DEMO_PRINT_INFO1("tuya_ble_dp_data_send ret is 0x%x", ret);

	
	app_open = open_from_tuya;						//APP开锁标志位

	if(door_open_status() == E_OPEN_NONE || door_open_status() == E_OPEN_SUC)
		set_door_open_status(E_OPEN_START);										//提前改掉锁的状态标志位，防止主程序自动拉低霍尔

	Pad_Config(BAT_EN_HAL1_POW, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);			
	Pad_Config(PAIR_HAL1, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);//开启HAL1输入
	
	background_msg_set_led(BACKGROUND_MSG_LED_SUBTYPE_MATCH_SUCCESS);				
	background_msg_set_beep(150, 3);
	os_delay(1200);
	driver_motor_control(EM_MOTOR_CTRL_ON, 2000);
	os_delay(20);

	GPIO_INTConfig(GPIO_GetPin(PAIR_HAL1), ENABLE); 
	GPIO_MaskINTConfig(GPIO_GetPin(PAIR_HAL1), DISABLE);

	u8Send[0] = DP_ID_REMOTE_RECORD;
	u8Send[1] = DP_TYPE_VALUE;
	u8Send[2] = 0;
	u8Send[3] = 4;

	u8Send[4] = 0;
	u8Send[5] = 0;
	u8Send[6] = 0;
	u8Send[7] = 0;

	u32SendLen = 8;

	TUYA_DEMO_PRINT_INFO1("send is %b", TRACE_BINARY(u32SendLen, u8Send));
	ret = tuya_ble_dp_data_send(++u32Sn, DP_SEND_TYPE_ACTIVE, DP_SEND_FOR_CLOUD_PANEL, DP_SEND_WITHOUT_RESPONSE, u8Send, u32SendLen);

	TUYA_DEMO_PRINT_INFO1("tuya_ble_dp_data_send ret is 0x%x", ret);

	return 0;
}


//蓝牙开锁
uint8_t tuya_ble_open(uint32_t u32Sn, uint8_t * pu8In, uint32_t u32InLen)
{
	if(pu8In == NULL)
	{
		TUYA_DEMO_PRINT_INFO0("input param error");
		return 0;
	}

	uint8_t u8Send[64] = {0};
	uint32_t u32SendLen;
	tuya_ble_status_t ret;

	u32SendLen = u32InLen;
	memcpy(u8Send, pu8In, u32SendLen);
	

	//调换一下主机ID和从机ID的位置
	u8Send[4] = pu8In[6];
	u8Send[5] = pu8In[7];

	u8Send[6] = pu8In[4];
	u8Send[7] = pu8In[5];

	u8Send[u32SendLen-1] = 0;

	TUYA_DEMO_PRINT_INFO1("send is %b", TRACE_BINARY(u32SendLen, u8Send));
	ret = tuya_ble_dp_data_send(++u32Sn, DP_SEND_TYPE_ACTIVE, DP_SEND_FOR_CLOUD_PANEL, DP_SEND_WITHOUT_RESPONSE, u8Send, u32SendLen);

	TUYA_DEMO_PRINT_INFO1("tuya_ble_dp_data_send ret is 0x%x", ret);
	

	app_open = open_from_tuya;						//APP开锁标志位

	if(door_open_status() == E_OPEN_NONE || door_open_status() == E_OPEN_SUC)
		set_door_open_status(E_OPEN_START);										//提前改掉锁的状态标志位，防止主程序自动拉低霍尔

	Pad_Config(BAT_EN_HAL1_POW, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);			
	Pad_Config(PAIR_HAL1, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);//开启HAL1输入
	
	background_msg_set_led(BACKGROUND_MSG_LED_SUBTYPE_MATCH_SUCCESS);				
	background_msg_set_beep(150, 3);
	os_delay(1200);
	driver_motor_control(EM_MOTOR_CTRL_ON, 2000);
	os_delay(20);

	GPIO_INTConfig(GPIO_GetPin(PAIR_HAL1), ENABLE); 
	GPIO_MaskINTConfig(GPIO_GetPin(PAIR_HAL1), DISABLE);
	
	u8Send[0] = DP_ID_BLE_RECORD;
	u8Send[1] = DP_TYPE_VALUE;
	u8Send[2] = 0;
	u8Send[3] = 4;

	u8Send[4] = 0;
	u8Send[5] = 0;
	u8Send[6] = 0;
	u8Send[7] = pu8In[u32InLen - 1];

	u32SendLen = 8;

	TUYA_DEMO_PRINT_INFO1("send is %b", TRACE_BINARY(u32SendLen, u8Send));
	ret = tuya_ble_dp_data_send(++u32Sn, DP_SEND_TYPE_ACTIVE, DP_SEND_FOR_CLOUD_PANEL, DP_SEND_WITHOUT_RESPONSE, u8Send, u32SendLen);

	TUYA_DEMO_PRINT_INFO1("tuya_ble_dp_data_send ret is 0x%x", ret);

	return 0;
}

//常开模式设置
uint8_t tuya_ble_set_open_mode(uint32_t u32Sn, uint8_t * pu8In, uint32_t u32InLen)
{
	if(pu8In == NULL)
	{
		TUYA_DEMO_PRINT_INFO0("input param error");
		return 0;
	}

	uint8_t u8Send[64] = {0};
	uint32_t u32SendLen;
	tuya_ble_status_t ret;

	u32SendLen = u32InLen;
	memcpy(u8Send, pu8In, u32SendLen);
		
	TUYA_DEMO_PRINT_INFO1("send is %b", TRACE_BINARY(u32SendLen, u8Send));
	ret = tuya_ble_dp_data_send(++u32Sn, DP_SEND_TYPE_ACTIVE, DP_SEND_FOR_CLOUD_PANEL, DP_SEND_WITHOUT_RESPONSE, u8Send, u32SendLen);

	//SYSSET_SetOpenMode(pu8In[4]);

	TUYA_DEMO_PRINT_INFO1("tuya_ble_dp_data_send ret is 0x%x", ret);

	return 0;
}


//处理APP下发下来的的数据
uint8_t tuya_ble_data_delawith(tuya_ble_dp_data_received_data_t *in)
{
	if(in == NULL)
	{
		TUYA_DEMO_PRINT_INFO0("input param error");
		return 0;
	}

	uint32_t u32Sn = in->sn + 1;
	uint8_t u8DpId = in->p_data[0];
	BLE_DP_TYPE eDpType = (BLE_DP_TYPE)in->p_data[1];
	uint16_t u16DpLen = (in->p_data[2] << 8) | (in->p_data[3]);
	TUYA_DEMO_PRINT_INFO1("in->data_len %d", in->data_len);
	//tuya_ble_status_t ret = TUYA_BLE_SUCCESS;

	TUYA_DEMO_PRINT_INFO0("*********************************************************");
	TUYA_DEMO_PRINT_INFO4("u32Sn is %d, u8DpId is %d, eDpType is %d, u16DpLen is %d", u32Sn, u8DpId, eDpType, u16DpLen);
	TUYA_DEMO_PRINT_INFO1("DP data is %b", TRACE_BINARY(u16DpLen, &in->p_data[4]));

	switch(u8DpId)
	{
		case DP_ID_ADD_FP:
			if(in->p_data[5] == ENROLL_CANCEL)
			{
				g_QuitAtOnce = 1;
			}
			else
			{
				memset(u8AddData, 0, sizeof(u8AddData));
				memcpy(u8AddData, in->p_data, (in->data_len > sizeof(u8AddData) ? (sizeof(u8AddData)) : (in->data_len)));
				background_msg_set_fp(FP_MSG_SUBTYPE_BT_ENROLL);
				//tuya_ble_add_fp(u32Sn, in->p_data, (uint32_t)in->data_len);
			}
			break;

		case DP_ID_DEL_FP:
			tuya_ble_del_fp(u32Sn, in->p_data, (uint32_t)in->data_len);
			break;

		case DP_ID_SYNC:
			tuya_ble_info_sync(u32Sn, in->p_data, (uint32_t)in->data_len);
			break;

		case DP_ID_GET_RECORD:
			BleRecordSync(true);
			break;

		case DP_ID_SET_CONFIG:
			tuya_ble_set_config(u32Sn, in->p_data, (uint32_t)in->data_len);
			break;

		case DP_ID_BLE_OPEN:			
			
			Pad_Config(BAT_EN_HAL1_POW, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);

			tuya_ble_open(u32Sn, in->p_data, (uint32_t)in->data_len);
			break;

		case DP_ID_REMOTE_CONFIG:
			tuya_ble_remote_config(u32Sn, in->p_data, (uint32_t)in->data_len);
			break;
	
		case DP_ID_REMOTE_OPEN:
			tuya_ble_remote_open(u32Sn, in->p_data, (uint32_t)in->data_len);
			break;

		default:
			TUYA_DEMO_PRINT_INFO1("not found this DP ID(%d)", u8DpId);
			break;
	}

	
	//ret = tuya_ble_dp_data_send(u32Sn, DP_SEND_TYPE_ACTIVE, DP_SEND_FOR_CLOUD_PANEL, DP_SEND_WITHOUT_RESPONSE, u8Temp, u32Len);

	//TUYA_DEMO_PRINT_INFO1("tuya_ble_dp_data_send ret is 0x%x", ret);

	return 0;
}

void app_custom_task(void *p_param)
{
    tuya_ble_cb_evt_param_t event;	
	uint16_t deleteId;
	uint16_t dele_Id = 0;
    while (true)
    {
        if (os_msg_recv(tuya_custom_queue_handle, &event, 0xFFFFFFFF) == true)
        {
        	menu_reset_bt_timeout();
        	menu_sleep_event_control(MENU_SLEEP_EVENT_WAKEUP_BIT, true);
        	TUYA_DEMO_PRINT_INFO1("recv event.evt is %d", event.evt);
            int16_t result = 0;
            switch (event.evt) {
                case TUYA_BLE_CB_EVT_CONNECTE_STATUS:
                    TUYA_APP_LOG_INFO("received tuya ble conncet status update event,current connect status = %d",event.connect_status);
                    
                    #if TUYA_BLE_SDK_TEST_ENABLE
                    tuya_ble_sdk_test_send(TY_UARTV_CMD_GET_DEVICE_STATE, (uint8_t *)&event.connect_status, sizeof(uint8_t));
                    #endif
                    break;

                case TUYA_BLE_CB_EVT_DP_DATA_RECEIVED:
                    TUYA_APP_LOG_DEBUG("received dp write sn = %d", event.dp_received_data.sn);    
                    TUYA_APP_LOG_HEXDUMP_DEBUG("received dp write data :", event.dp_received_data.p_data, event.dp_received_data.data_len);
					tuya_ble_data_delawith(&event.dp_received_data);
					
                    #if TUYA_BLE_SDK_TEST_ENABLE
                    tuya_ble_sdk_test_send(TY_UARTV_CMD_DP_WRITE, event.dp_received_data.p_data, event.dp_received_data.data_len);
                    #endif
                    break;

                case TUYA_BLE_CB_EVT_DP_DATA_SEND_RESPONSE:
                    TUYA_APP_LOG_INFO("received dp data send response sn = %d , type = 0x%02x, flag_mode = %d , ack = %d, result code =%d", \
                                event.dp_send_response_data.sn, \
                                event.dp_send_response_data.type, \
                                event.dp_send_response_data.mode, \
                                event.dp_send_response_data.ack, \
                                event.dp_send_response_data.status);

                    if (event.dp_send_response_data.mode == DP_SEND_FOR_CLOUD_PANEL) {
                        if (event.dp_send_response_data.type == DP_SEND_TYPE_PASSIVE) {
                            
                        } else {
                            
                        } 
                    } else if(event.dp_send_response_data.mode == DP_SEND_FOR_CLOUD) {
                        
                    } else if(event.dp_send_response_data.mode == DP_SEND_FOR_PANEL) {
                       
                    } else if(event.dp_send_response_data.mode == DP_SEND_FOR_NONE) {
                       
                    } else {
                        
                    }
                    
                    #if TUYA_BLE_SDK_TEST_ENABLE
                    tuya_ble_sdk_test_dp_report_handler();
                    tuya_ble_sdk_test_send(TY_UARTV_CMD_DP_RSP, (uint8_t *)&event.dp_send_response_data.status, sizeof(uint8_t));
                    #endif
                    break;
    
                case TUYA_BLE_CB_EVT_DP_DATA_WITH_TIME_SEND_RESPONSE:
                    TUYA_APP_LOG_INFO("received dp data with time send response sn = %d, type = 0x%02x, flag_mode = %d , need_ack = %d, result code =%d", \
                                event.dp_with_time_send_response_data.sn,   \
                                event.dp_with_time_send_response_data.type, \
                                event.dp_with_time_send_response_data.mode, \
                                event.dp_with_time_send_response_data.ack,  \
                                event.dp_with_time_send_response_data.status);

                    if (event.dp_with_time_send_response_data.mode == DP_SEND_FOR_CLOUD_PANEL) {
                        
                    } else if(event.dp_with_time_send_response_data.mode == DP_SEND_FOR_CLOUD) {
                        
                    } else if(event.dp_with_time_send_response_data.mode == DP_SEND_FOR_PANEL) {
                        
                    } else if(event.dp_with_time_send_response_data.mode == DP_SEND_FOR_NONE) {
                       
                    }
                    
                    #if TUYA_BLE_SDK_TEST_ENABLE
                    tuya_ble_sdk_test_dp_report_handler();
                    tuya_ble_sdk_test_send(TY_UARTV_CMD_DP_RSP, (uint8_t *)&event.dp_with_time_send_response_data.status, sizeof(uint8_t));
                    #endif
                    break;        
                 
                case TUYA_BLE_CB_EVT_BULK_DATA: 
                    TUYA_APP_LOG_INFO("received bulk data response");
                    //tuya_ble_bulk_data_demo_handler(&event.bulk_req_data);
                    break;
                
            #if (TUYA_BLE_FEATURE_WEATHER_ENABLE != 0)
            	case TUYA_BLE_CB_EVT_WEATHER_DATA_REQ_RESPONSE:
            		TUYA_APP_LOG_INFO("received weather data request response result code =%d", event.weather_req_response_data.status);
                    
                    #if TUYA_BLE_SDK_TEST_ENABLE
                    uint8_t  rsp_data[2] = {1};
                    uint32_t rsp_len = 2;
                    rsp_data[1] = event.weather_req_response_data.status;
                    tuya_ble_sdk_test_send(TY_UARTV_CMD_GET_WEATHER, rsp_data, rsp_len);
                    #endif
            		break;
            	
            	case TUYA_BLE_CB_EVT_WEATHER_DATA_RECEIVED:
            	{
            		TUYA_APP_LOG_DEBUG("received weather data, location = [%d] object_count = [%d]", event.weather_received_data.location, event.weather_received_data.object_count);    
                    
            		tuya_ble_wd_object_t *object;
            		uint16_t object_len = 0;

            		for (;;)
            		{
            			object = (tuya_ble_wd_object_t *)(event.weather_received_data.p_data + object_len);
            			
            			TUYA_APP_LOG_DEBUG("weather data, n_day=[%d] key=[0x%08x] val_type=[%d] val_len=[%d]", \
            							object->n_day, object->key_type, object->val_type, object->value_len); 
            			TUYA_APP_LOG_HEXDUMP_DEBUG("vaule :", (uint8_t *)object->vaule, object->value_len);	
                        
                        // TODO .. YOUR JOBS 
                        
            			object_len += (sizeof(tuya_ble_wd_object_t) + object->value_len);
            			if (object_len >= event.weather_received_data.data_len)
            				break;
            		}
                    
                    #if TUYA_BLE_SDK_TEST_ENABLE
                    tuya_ble_sdk_test_send(TY_UARTV_CMD_GET_WEATHER, event.weather_received_data.p_data, event.weather_received_data.data_len);
                    #endif 
            	}
            		break;
            #endif 	        
		
			#if ( TUYA_BLE_FEATURE_IOT_CHANNEL_ENABLE != 0 )
				#if ( TUYA_BLE_FEATURE_SCENE_ENABLE != 0 )
				case TUYA_BLE_CB_EVT_SCENE_REQ_RESPONSE:
					TUYA_APP_LOG_INFO("received scene request response result code=[%d], err_code=[%d] sub_cmd=[%d]", \
										event.scene_req_response_data.status, \
										event.scene_req_response_data.err_code, \
										event.scene_req_response_data.scene_cmd);
					break;
				
				case TUYA_BLE_CB_EVT_SCENE_DATA_RECEIVED:
					TUYA_APP_LOG_INFO("received scene data result, status=[%d] err_code=[%d] need_update=[%d] check_code=[0x%X]", \
										event.scene_data_received_data.status, \
										event.scene_data_received_data.err_code, \
										event.scene_data_received_data.need_update, \
										event.scene_data_received_data.check_code);

					if (event.scene_data_received_data.status == 0 && event.scene_data_received_data.need_update)
					{
						// TODO .. please storage update scene check code.
						
						if (event.scene_data_received_data.data_len != 0)
						{
							uint8_t *iot_scene_object;
							uint16_t scene_id_len, scene_name_len;
							uint16_t object_len = 0;

							for (;;) 
							{
								iot_scene_object = (uint8_t *)(event.scene_data_received_data.p_data + object_len);
								
								scene_id_len = iot_scene_object[0];
								scene_name_len = (iot_scene_object[1+scene_id_len] << 8) + iot_scene_object[2+scene_id_len];

								TUYA_APP_LOG_HEXDUMP_DEBUG("scene id :", &iot_scene_object[1], scene_id_len);	
								TUYA_APP_LOG_HEXDUMP_DEBUG("scene name unicode :", &iot_scene_object[3+scene_id_len], scene_name_len);	

								object_len += (3 + scene_id_len + scene_name_len);
								if (object_len >= event.scene_data_received_data.data_len)
									break;
							}
						}
					}
					break;
					
				case TUYA_BLE_CB_EVT_SCENE_CTRL_RESULT_RECEIVED:
					TUYA_APP_LOG_INFO("received scene ctrl result, status=[%d] err_code=[%d]", \
									event.scene_ctrl_received_data.status, \
									event.scene_ctrl_received_data.err_code);
					TUYA_APP_LOG_HEXDUMP_DEBUG("scene ctrl id :", event.scene_ctrl_received_data.p_scene_id, event.scene_ctrl_received_data.scene_id_len);	
					break;
				#endif 
			#endif 
	
                case TUYA_BLE_CB_EVT_DP_QUERY:
                    if (event.dp_query_data.data_len > 0)
                    {
                        TUYA_APP_LOG_HEXDUMP_DEBUG("received TUYA_BLE_CB_EVT_DP_QUERY event, query dp id :",event.dp_query_data.p_data,event.dp_query_data.data_len);
                    } 
                    else 
                    {
                        TUYA_APP_LOG_INFO("received TUYA_BLE_CB_EVT_DP_QUERY event, query all dp");
                    }
                    break;
                    
                case TUYA_BLE_CB_EVT_OTA_DATA:
                    TUYA_APP_LOG_INFO("received TUYA_BLE_CB_EVT_OTA_DATA event");
					tuya_ota_proc(event.ota_data.type, event.ota_data.p_data, event.ota_data.data_len);
                    break;
                    
                case TUYA_BLE_CB_EVT_NETWORK_INFO:
                    TUYA_APP_LOG_INFO("received net info : %s", event.network_data.p_data);
                    tuya_ble_net_config_response(result);
                    break;
                    
                case TUYA_BLE_CB_EVT_WIFI_SSID:
                    break;
                    
                case TUYA_BLE_CB_EVT_TIME_STAMP:
					TUYA_APP_LOG_INFO("received TUYA_BLE_CB_EVT_TIME_STAMP event");
                    //TUYA_APP_LOG_INFO("received unix timestamp : %s ,time_zone : %d",event.timestamp_data.timestamp_string,event.timestamp_data.time_zone);
					TUYA_DEMO_PRINT_INFO1("received unix timestamp is %b", TRACE_BINARY(14, event.timestamp_data.timestamp_string));
					uint64_t time_stamp_ms;
        			uint32_t time_stamp;

        			time_stamp_ms = atoll((char *)event.timestamp_data.timestamp_string);
        			time_stamp = (time_stamp_ms / 1000);
        			if ((time_stamp_ms % 1000) >= 500)
        				time_stamp += 1;
					TUYA_APP_LOG_INFO("-----time_stamp_ms is %d, time_stamp is 0x%x", time_stamp_ms, time_stamp);

					rtc_set_time(time_stamp);

					//tuya_ble_time_struct_data_t data;
					
					//tuya_ble_utc_sec_2_mytime(time_stamp, &data, false);

					//TUYA_DEMO_PRINT_INFO7("---time is %d:%d:%d--%d:%d:%d--%d", data.nYear, data.nMonth, data.nDay, data.nHour, data.nMin, data.nSec, data.DayIndex);
				
					BleRecordSync(true);

                    #if TUYA_BLE_SDK_TEST_ENABLE 
					//tuya_ble_bulk_data_generation(60, NULL, 0);
					
                    tuya_ble_time_struct_data_t data;
                    tuya_ble_time_noraml_data_t normal_data;
                        
                    tuya_ble_utc_sec_2_mytime(time_stamp, &data, false);
                    memcpy(&normal_data, &data, sizeof(tuya_ble_time_struct_data_t));
                    normal_data.time_zone = event.timestamp_data.time_zone;
                    
                    tuya_ble_sdk_test_get_time_rsp(&normal_data);
                    #endif
                    break;
                    
                case TUYA_BLE_CB_EVT_TIME_NORMAL:
                case TUYA_BLE_CB_EVT_APP_LOCAL_TIME_NORMAL:
            		TUYA_APP_LOG_HEXDUMP_DEBUG("received normal time", (uint8_t *)&(event.time_normal_data), sizeof(tuya_ble_time_noraml_data_t));
            		TUYA_APP_LOG_INFO(" timezone = [%d]", event.time_normal_data.time_zone);
                    
                    #if TUYA_BLE_SDK_TEST_ENABLE
                    tuya_ble_sdk_test_get_time_rsp(&event.time_normal_data);
                    #endif
            		break;
        
                case TUYA_BLE_CB_EVT_DATA_PASSTHROUGH:
                    TUYA_APP_LOG_HEXDUMP_DEBUG("received ble passthrough data :", event.ble_passthrough_data.p_data,event.ble_passthrough_data.data_len);
                   
                    #if TUYA_BLE_SDK_TEST_ENABLE
                    tuya_ble_sdk_test_send(TY_UARTV_CMD_PASSTHROUGH_WRITE, event.ble_passthrough_data.p_data, event.ble_passthrough_data.data_len);
                    #endif
                    break;
                
                case TUYA_BLE_CB_EVT_UPDATE_LOGIN_KEY_VID:
                    TUYA_APP_LOG_HEXDUMP_DEBUG("binding success -> login key :",event.device_login_key_vid_data.login_key,event.device_login_key_vid_data.login_key_len);
                    TUYA_APP_LOG_HEXDUMP_DEBUG(" -> beacon key :",event.device_login_key_vid_data.beacon_key,BEACON_KEY_LEN);
                    TUYA_APP_LOG_HEXDUMP_DEBUG(" -> vid :",event.device_login_key_vid_data.vid,event.device_login_key_vid_data.vid_len);
                    break; 
                
                case TUYA_BLE_CB_EVT_UNBOUND:
                    TUYA_APP_LOG_INFO("received unbound req\n");
                    
                    #if TUYA_BLE_SDK_TEST_ENABLE
                    tuya_ble_sdk_test_unbind_mode_rsp(0);
                    #endif
                    break;
                    
                case TUYA_BLE_CB_EVT_ANOMALY_UNBOUND:
					
					
					MLAPI_DeleteFTR(ERASE_ALL_FINGER, 0, &dele_Id);
                    TUYA_APP_LOG_INFO("received anomaly unbound req\n");
                    
                    #if TUYA_BLE_SDK_TEST_ENABLE
                    tuya_ble_sdk_test_unbind_mode_rsp(1);
                    #endif
                    break;
                    
                case TUYA_BLE_CB_EVT_DEVICE_RESET:
                    TUYA_APP_LOG_INFO("received device reset req\n");					
                    if(MLAPI_DeleteFTR(ERASE_ALL_FINGER, 0, &deleteId) != 0)
                    {
                    	os_delay(100);
						MLAPI_DeleteFTR(ERASE_ALL_FINGER, 0, &deleteId);
                    }
					BleRecordInfoReset();
					//SYSSET_SetOpenMode(1);
                    #if TUYA_BLE_SDK_TEST_ENABLE
                    tuya_ble_sdk_test_unbind_mode_rsp(2);
                    #endif
                    break;
                    
                case TUYA_BLE_CB_EVT_UNBIND_RESET_RESPONSE:
                    if (event.reset_response_data.type == RESET_TYPE_UNBIND)
                    {
                        if (event.reset_response_data.status==0)
                        {
                            TUYA_APP_LOG_INFO("device unbind succeed.");
                            
                            #if TUYA_BLE_SDK_TEST_ENABLE
                            uint8_t device_reset_mode = 3;    
                            tuya_ble_sdk_test_send(TY_UARTV_CMD_UNBIND_MODE, &device_reset_mode, 1);
                            #endif
                        }
                        else
                        {
                            TUYA_APP_LOG_INFO("device unbind failed.");
                        }
                    }
                    else if (event.reset_response_data.type == RESET_TYPE_FACTORY_RESET)
                    {
                        if (event.reset_response_data.status==0)
                        {
                            TUYA_APP_LOG_INFO("device factory reset succeed.");
                            
                            #if TUYA_BLE_SDK_TEST_ENABLE
                            uint8_t device_reset_mode = 4;    
                            tuya_ble_sdk_test_send(TY_UARTV_CMD_UNBIND_MODE, &device_reset_mode, 1);
                            #endif
                        }
                        else
                        {
                            TUYA_APP_LOG_INFO("device factory reset failed.");
                        }
                    }
                    break;
        
                default:
                    TUYA_APP_LOG_WARNING("app_tuya_cb_queue msg: unknown event type 0x%04x", event.evt);
                    break;
            }

            tuya_ble_event_response(&event);
        }
    }
}

/*void tuya_ble_app_task_create(void)
{
	if(os_task_create(&app_custom_task_handle, "app_custom", app_custom_task, 0, APP_CUSTOM_TASK_STACK_SIZE,APP_CUSTOM_TASK_PRIORITY))
	{
		TUYA_DEMO_PRINT_INFO0("app_custom_task create success");
	}
	else
	{
		TUYA_DEMO_PRINT_INFO0("app_custom_task create fail");
	}
}*/


void tuya_ble_time_show(uint32_t u32Time)
{
	TUYA_DEMO_PRINT_INFO0("**************************************");
	char test[20] = {0};
	TUYA_DEMO_PRINT_INFO1("u32Time is 0x%x", u32Time);
	tuya_ble_time_struct_data_t data;

	tuya_ble_utc_sec_2_mytime(u32Time, &data, false);	
	TUYA_DEMO_PRINT_INFO7("---time1 is %d:%d:%d--%d:%d:%d--%d", data.nYear, data.nMonth, data.nDay, data.nHour, data.nMin, data.nSec, data.DayIndex);

	TUYA_DEMO_PRINT_INFO1("utc time is 0x%x", tuya_ble_mytime_2_utc_sec(&data, false));

	tuya_ble_utc_sec_2_mytime_string(u32Time, false, test);

	TUYA_DEMO_PRINT_INFO1("string is %b", TRACE_BINARY(20, test));
	
}

void tuya_ble_app_init(void)
{
    os_task_create(&app_custom_task_handle, "app_custom", app_custom_task, 0, APP_CUSTOM_TASK_STACK_SIZE,APP_CUSTOM_TASK_PRIORITY);
    os_msg_queue_create(&tuya_custom_queue_handle, MAX_NUMBER_OF_TUYA_CUSTOM_MESSAGE, sizeof(tuya_ble_cb_evt_param_t));
    
    device_param.p_type = TUYA_BLE_PRODUCT_ID_TYPE_PID;
    device_param.product_id_len = strlen(APP_PRODUCT_ID);
    memcpy(device_param.product_id, APP_PRODUCT_ID, device_param.product_id_len);
    device_param.firmware_version = TY_APP_VER_NUM;
    device_param.hardware_version = TY_HARD_VER_NUM;
	device_param.adv_local_name_len = strlen(device_local_name);
	memcpy(device_param.adv_local_name,device_local_name,device_param.adv_local_name_len);
	
	// Noticed!!! if use the license stored by the SDK initialized to 0, Otherwise 1.
    device_param.use_ext_license_key = 0; 
    device_param.device_id_len = 16;  
    if (device_param.use_ext_license_key == 1)
    {
        memcpy(device_param.auth_key, (void *)auth_key_test, AUTH_KEY_LEN);
        memcpy(device_param.device_id, (void *)device_id_test, DEVICE_ID_LEN);
        memcpy(device_param.mac_addr_string, mac_test, 12);
        device_param.mac_addr.addr_type = TUYA_BLE_ADDRESS_TYPE_RANDOM;
    }
	
    tuya_ble_sdk_init(&device_param);
    tuya_ble_callback_queue_register(tuya_custom_queue_handle);
	
    tuya_ble_device_delay_ms(200);
    tuya_ota_init();
    
#if TUYA_BLE_SDK_TEST_ENABLE
    tuya_ble_sdk_test_init();
#endif

//	memset(tuya_ble_current_para.auth_settings.device_id, 0 , sizeof(tuya_ble_current_para.auth_settings.device_id));
//	memset(tuya_ble_current_para.auth_settings.mac_string, 0 , sizeof(tuya_ble_current_para.auth_settings.mac_string));
//	memset(tuya_ble_current_para.auth_settings.auth_key, 0 , sizeof(tuya_ble_current_para.auth_settings.auth_key));
//	tuya_ble_storage_save_auth_settings();

    TUYA_APP_LOG_INFO("tuya ble demo version : "TUYA_BLE_DEMO_VERSION_STR);
    TUYA_APP_LOG_INFO("app version : "TY_APP_VER_STR);

	TUYA_DEMO_PRINT_INFO1("auth settings mac %b", TRACE_BINARY(MAC_LEN, tuya_ble_current_para.auth_settings.mac));
	TUYA_DEMO_PRINT_INFO1("auth settings mac_string %b", TRACE_BINARY(MAC_LEN*2, tuya_ble_current_para.auth_settings.mac_string));
    TUYA_DEMO_PRINT_INFO1("product_id %b", TRACE_BINARY(tuya_ble_current_para.pid_len, tuya_ble_current_para.pid));
    TUYA_DEMO_PRINT_INFO1("device_uuid %b",TRACE_BINARY(DEVICE_ID_LEN, tuya_ble_current_para.auth_settings.device_id));
    TUYA_DEMO_PRINT_INFO1("device_authkey %b", TRACE_BINARY(AUTH_KEY_LEN, tuya_ble_current_para.auth_settings.auth_key));
    TUYA_DEMO_PRINT_INFO1("bond_flag = %d", tuya_ble_current_para.sys_settings.bound_flag);
	
}



