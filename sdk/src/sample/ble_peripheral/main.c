/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
   * @file      main.c
   * @brief     Source file for BLE peripheral project, mainly used for initialize modules
   * @author    jane
   * @date      2017-06-12
   * @version   v1.0
   **************************************************************************************
   * @attention
   * <h2><center>&copy; COPYRIGHT 2017 Realtek Semiconductor Corporation</center></h2>
   **************************************************************************************
  */

/*============================================================================*
 *                              Header Files
 *============================================================================*/
#include <os_sched.h>
#include <string.h>
#include <stdlib.h>
#include <trace.h>
#include <gap.h>
#include <gap_adv.h>
#include <gap_bond_le.h>
#include <profile_server.h>
#include <gap_msg.h>
#include <simple_ble_service.h>
#include <bas.h>
#include <app_task.h>
#include <peripheral_app.h>
#include "ota_service.h"
#include "rtl876x_lib_platform.h"

#if F_BT_ANCS_CLIENT_SUPPORT
#include <profile_client.h>
#include <ancs.h>
#endif
#if F_BT_DLPS_EN
#include <dlps.h>
#include <rtl876x_io_dlps.h>
#endif

#include "otp_config.h"
#if (AON_WDG_ENABLE == 1)
#include "rtl876x_aon_wdg.h"
#endif

#include "driver_led.h"
#include "driver_button.h"
#include "driver_spi.h"
#include "driver_sensor.h"
#include "driver_uart.h"
#include "driver_led.h"
#include "driver_rtc.h"
#include "base64.h"
#include "test_mode.h"
#include "system_setting.h"
#include "menu_sleep.h"
#include "driver_motor.h"
#include "driver_wdg.h"
#include "driver_button.h"
#include "fpc_sensor_spi.h"
#include "gap_vendor.h"
#include "tuya_ble_service_rtl8762d.h"
#include "app_task.h"
#include "driver_hal.h"

/** @defgroup  PERIPH_DEMO_MAIN Peripheral Main
    * @brief Main file to initialize hardware and BT stack and start task scheduling
    * @{
    */

/*============================================================================*
 *                              Constants
 *============================================================================*/
/** @brief  Default minimum advertising interval when device is discoverable (units of 625us, 160=100ms) */
#define DEFAULT_ADVERTISING_INTERVAL_MIN           1120//800//320
/** @brief  Default maximum advertising interval */
#define DEFAULT_ADVERTISING_INTERVAL_MAX           1120//800//320

bool s_u8DlpsFlag = false;

extern void app_ble_init(void);

/*============================================================================*
 *                              Variables
 *============================================================================*/

/** @brief  GAP - scan response data (max size = 31 bytes) */
static const uint8_t scan_rsp_data[] =
{
    //0x03,                             /* length */
   // GAP_ADTYPE_APPEARANCE,            /* type="Appearance" */
    
    //LO_WORD(GAP_GATT_APPEARANCE_GENERIC_BLOOD_PRESSURE),
    //HI_WORD(GAP_GATT_APPEARANCE_GENERIC_BLOOD_PRESSURE),
    
	0x11,
	GAP_ADTYPE_128BIT_MORE,
	0x01, 0x19, 0x00, 0x00, 0xf0, 0xb5, 0x87, 0xb0, 0x01, 0x27, 0x68, 0x46, 0x07, 0x81, 0x00, 0x24,

	0x04,
	GAP_ADTYPE_SERVICE_DATA,
#if BLE_DMZ
	0x01, 0xC1, 0x80,
#elif BLE_ML
	0x01, 0xC5, 0x80,
#else
	0x01, 0x13, 0x80,
#endif

};

/** @brief  GAP - Advertisement data (max size = 31 bytes, best kept short to conserve power) */
static uint8_t adv_data[] =
{
    /* Flags */
    0x02,             /* length */
    GAP_ADTYPE_FLAGS, /* type="Flags" */
    GAP_ADTYPE_FLAGS_LIMITED | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,
    
	/* Local name */
	0x0F,             /* length */
    GAP_ADTYPE_LOCAL_NAME_COMPLETE,
//    'B', 'L', 'E', '_', 'P', 'E', 'R', 'I', 'P', 'H', 'E', 'R', 'A', 'L',
//    'M', 'L', '_', 'R', 'E', 'A', 'T', 'E', 'K', '8', '7', '6', '2', 'D',
      'M',  'L','B', 'L', 'E', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
      
    /* Service */
    0x03,             /* length */
    GAP_ADTYPE_16BIT_COMPLETE,
    0x0d, 0x18,
    
	0x02,             /* length */
	GAP_ADTYPE_POWER_LEVEL,
	0x0a, 0x00,
};

/*============================================================================*
 *                              Functions
 *============================================================================*/
/**
  * @brief  Initialize peripheral and gap bond manager related parameters
  * @return void
  */
void app_le_gap_init(void)
{
    /* Device name and device appearance */
    uint8_t  device_name[GAP_DEVICE_NAME_LEN] = "smkb2-9318-0101";//"ML_REATEK8762D";       //name
    uint16_t appearance = GAP_GATT_APPEARANCE_GENERIC_BLOOD_PRESSURE;//GAP_GATT_APPEARANCE_UNKNOWN;
    uint8_t  slave_init_mtu_req = false;


    /* Advertising parameters */
    uint8_t  adv_evt_type = GAP_ADTYPE_ADV_IND;
    uint8_t  adv_direct_type = GAP_REMOTE_ADDR_LE_PUBLIC;
    uint8_t  adv_direct_addr[GAP_BD_ADDR_LEN] = {0};
    uint8_t  adv_chann_map = GAP_ADVCHAN_ALL;
    uint8_t  adv_filter_policy = GAP_ADV_FILTER_ANY;
    uint16_t adv_int_min = DEFAULT_ADVERTISING_INTERVAL_MIN;
    uint16_t adv_int_max = DEFAULT_ADVERTISING_INTERVAL_MAX;

    /* GAP Bond Manager parameters */
    uint8_t  auth_pair_mode = GAP_PAIRING_MODE_PAIRABLE;    //决定设备是否处于可配对模式,GAP_PAIRING_MODE_PAIRABLE:可配对
    uint16_t auth_flags = GAP_AUTHEN_BIT_BONDING_FLAG;      //表示要求的security属性的位域
    uint8_t  auth_io_cap = GAP_IO_CAP_NO_INPUT_NO_OUTPUT;   //表示设备的输入输出能力
    uint8_t  auth_oob = false;                              //表示是否使能Out of Band (OOB)。
    uint8_t  auth_use_fix_passkey = false;                  //表示当配对方法为passkey entry且本地设备需要生成passkey时，是使用随机生成的passkey还是固定的passkey。
    uint32_t auth_fix_passkey = 0;                          //配对时使用的固定passkey的值，当auth_use_fix_passkey 参数为true时auth_fix_passkey参数是有效的
#if F_BT_ANCS_CLIENT_SUPPORT
    uint8_t  auth_sec_req_enable = true;
#else
    uint8_t  auth_sec_req_enable = false;                   //决定在建立connection之后，是否发起配对流程
#endif
    uint16_t auth_sec_req_flags = GAP_AUTHEN_BIT_BONDING_FLAG;

    /* Set device name and device appearance */
    le_set_gap_param(GAP_PARAM_DEVICE_NAME, GAP_DEVICE_NAME_LEN, device_name);      //name
    le_set_gap_param(GAP_PARAM_APPEARANCE, sizeof(appearance), &appearance);
    le_set_gap_param(GAP_PARAM_SLAVE_INIT_GATT_MTU_REQ, sizeof(slave_init_mtu_req),
                     &slave_init_mtu_req);

    /* Set advertising parameters */
    le_adv_set_param(GAP_PARAM_ADV_EVENT_TYPE, sizeof(adv_evt_type), &adv_evt_type);
    le_adv_set_param(GAP_PARAM_ADV_DIRECT_ADDR_TYPE, sizeof(adv_direct_type), &adv_direct_type);
    le_adv_set_param(GAP_PARAM_ADV_DIRECT_ADDR, sizeof(adv_direct_addr), adv_direct_addr);
    le_adv_set_param(GAP_PARAM_ADV_CHANNEL_MAP, sizeof(adv_chann_map), &adv_chann_map);
    le_adv_set_param(GAP_PARAM_ADV_FILTER_POLICY, sizeof(adv_filter_policy), &adv_filter_policy);
    le_adv_set_param(GAP_PARAM_ADV_INTERVAL_MIN, sizeof(adv_int_min), &adv_int_min);
    le_adv_set_param(GAP_PARAM_ADV_INTERVAL_MAX, sizeof(adv_int_max), &adv_int_max);
    le_adv_set_param(GAP_PARAM_ADV_DATA, sizeof(adv_data), (void *)adv_data);
    le_adv_set_param(GAP_PARAM_SCAN_RSP_DATA, sizeof(scan_rsp_data), (void *)scan_rsp_data);

    /* Setup the GAP Bond Manager */
    gap_set_param(GAP_PARAM_BOND_PAIRING_MODE, sizeof(auth_pair_mode), &auth_pair_mode);
    gap_set_param(GAP_PARAM_BOND_AUTHEN_REQUIREMENTS_FLAGS, sizeof(auth_flags), &auth_flags);
    gap_set_param(GAP_PARAM_BOND_IO_CAPABILITIES, sizeof(auth_io_cap), &auth_io_cap);
    gap_set_param(GAP_PARAM_BOND_OOB_ENABLED, sizeof(auth_oob), &auth_oob);
    le_bond_set_param(GAP_PARAM_BOND_FIXED_PASSKEY, sizeof(auth_fix_passkey), &auth_fix_passkey);
    le_bond_set_param(GAP_PARAM_BOND_FIXED_PASSKEY_ENABLE, sizeof(auth_use_fix_passkey),
                      &auth_use_fix_passkey);
    le_bond_set_param(GAP_PARAM_BOND_SEC_REQ_ENABLE, sizeof(auth_sec_req_enable), &auth_sec_req_enable);
    le_bond_set_param(GAP_PARAM_BOND_SEC_REQ_REQUIREMENT, sizeof(auth_sec_req_flags),
                      &auth_sec_req_flags);

    /* register gap message callback */
    le_register_app_cb(app_gap_callback);
}


/**
 * @brief  Add GATT services and register callbacks
 * @return void
 */
void app_le_profile_init(void)
{
    server_init(1); //service attribute table的总数目
  	tuya_srv_id  = kns_add_service(app_profile_callback);  	
    //simp_srv_id = simp_ble_service_add_service(app_profile_callback);
    //g_ota_service_id = ota_add_service(app_profile_callback);
    
    server_register_app_cb(app_profile_callback);
#if F_BT_ANCS_CLIENT_SUPPORT
    client_init(1);
    ancs_init(APP_MAX_LINKS);
#endif
}


//设置蓝牙广播间隔
void app_le_set_adv_int(uint16_t interval)
{
	uint16_t adv_int_data = interval;
	le_adv_set_param(GAP_PARAM_ADV_INTERVAL_MIN, sizeof(adv_int_data), &adv_int_data);
    le_adv_set_param(GAP_PARAM_ADV_INTERVAL_MAX, sizeof(adv_int_data), &adv_int_data);
}

/**
 * @brief    Contains the initialization of pinmux settings and pad settings
 * @note     All the pinmux settings and pad settings shall be initiated in this function,
 *           but if legacy driver is used, the initialization of pinmux setting and pad setting
 *           should be peformed with the IO initializing.
 * @return   void
 */
void board_init(void)
{
    gpio_spi_pinmux_config();
    gpio_spi_pad_config();
    
    gpio_sensor_pinmux_config();
    gpio_sensor_pad_config();
    
    board_uart_init();
}

/**
 * @brief    Contains the initialization of peripherals
 * @note     Both new architecture driver and legacy driver initialization method can be used
 * @return   void
 */
void driver_init(void)
{
    //driver_led_init();
    Pad_Config(LED_RED_PIN,   PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    driver_spi_init();
    
    driver_sensor_gpio_init();
    
//    driver_flash_read_id(MF_DEVICE_ID);
    
    DBG_DIRECT("driver init");
}



void app_ble_init(void)
{
	//if(get_test_mode() == BOOT_STACK_MODE)
	{
		le_gap_init(APP_MAX_LINKS);         //初始化GAP并设置link数目
    	gap_lib_init();                     //初始化gap_utils.lib或gap_bt5.lib
    	app_le_gap_init();                  //GAP参数的初始化
    	app_le_profile_init();              //初始化基于GATT的Profiles
	}	
}


/**
 * @brief    Contains the power mode settings
 * @return   void
 */
void io_dlps_enter_cb(void)
{
	app_wdg_reset();
    //DBG_DIRECT("enter dlps cb");
    gpio_sensor_enter_dlps_config();
	gpio_spi_enter_dlps_config();
	
    if(!s_u8DlpsFlag)
    {
    	s_u8DlpsFlag = true;
	    gpio_led_enter_dlps_config();	    
		gpio_motor_enter_dlps_config();
		gpio_hal_enter_dlps_config();
		
	    io_uart_dlps_enter();
    }

}

void io_dlps_exit_init()
{
	//app_le_set_adv_int(320);	
	s_u8DlpsFlag = false;
	gpio_sensor_exit_dlps_config();
	gpio_led_exit_dlps_config();
	gpio_spi_exit_dlps_config();
	gpio_motor_exit_dlps_config();
	gpio_hal_exit_dlps_config();
	
	io_uart_dlps_exit();
	//gpio_button_exit_dlps_config();
}


void io_dlps_exit_cb(void)
{
	app_wdg_reset();
    //DBG_DIRECT("exit dlps cb");

    gpio_sensor_exit_dlps_config();
    gpio_spi_exit_dlps_config();
        
   // io_uart_dlps_exit();
}

bool check_dlps_cb(void)
{
    static uint8_t flag = 0;
    bool result = true;
    
    if(menu_sleep_event_state_get() != 0)
    {
        if(flag == 0)
            DBG_DIRECT("before enter dlps false");
        
        flag = 1;
        result = false;
    }
    else
    {
        if(flag == 1)
            DBG_DIRECT("before enter dlps succes");
        flag = 0;

		//mcu进入休眠之前再次判断sensor是否进入普通休眠
		//if(!fpc_sleep_check_and_set())
		//	result = false;
    }
//    DBG_DIRECT("before enter dlps check cb");

    return result;
}

void pwr_mgr_init(void)
{
#if F_BT_DLPS_EN
	//if(get_test_mode() == BOOT_STACK_MODE)
	{
	    DLPS_IORegUserDlpsEnterCb(io_dlps_enter_cb);
	    DLPS_IORegUserDlpsExitCb(io_dlps_exit_cb);
	    dlps_check_cb_reg(check_dlps_cb);
	    DLPS_IORegister();                          //进出DLPS的回调处理
	    gap_set_lps_bootup_active_time(5);  
#if DLPS_FLAG   
    	lps_mode_set(LPM_DLPS_MODE);
#else
    	lps_mode_set(LPM_ACTIVE_MODE);
#endif
	}
    
#endif
}

/**
 * @brief    Contains the initialization of all tasks
 * @note     There is only one task in BLE Peripheral APP, thus only one APP task is init here
 * @return   void
 */
void task_init(void)
{    
    app_task_init();
    menu_task_init();
}

void debug_level(void)
{
    for (uint8_t i = 0; i < MODULE_NUM; i++) 
    {
        log_module_trace_set((T_MODULE_ID)i, LEVEL_TRACE, false); 
        log_module_trace_set((T_MODULE_ID)i, LEVEL_INFO, false); 
    } 
    
    log_module_trace_set(MODULE_APP, LEVEL_INFO, true); 
    log_module_trace_set(MODULE_APP, LEVEL_TRACE, true); 
    log_module_trace_set(MODULE_SNOOP, LEVEL_ERROR, true);
    
    log_module_trace_set(MODULE_DFU, LEVEL_INFO, true); 
    log_module_trace_set(MODULE_DFU, LEVEL_TRACE, true);
}

/**
 * @brief    Entry of APP code
 * @return   int (To avoid compile warning)
 */
int main(void)
{
    extern uint32_t random_seed_value;
    srand(random_seed_value);
   // APP_PRINT_INFO1("system clock:%d",get_cpu_clock());
    board_init();                       //PINMUX和PAD初始化设置
    pwr_mgr_init();                     //电源管理相关初始化
    //swTimerInit();                    //软件定时器初始化
    debug_level();	
    task_init();        
    os_sched_start();

    return 0;
}
/** @} */ /* End of group PERIPH_DEMO_MAIN */

void wakeup_config(void)
{
	DBG_DIRECT("wakeup_config");

	io_dlps_exit_init();
	
	driver_spi_init(); 
    driver_sensor_gpio_init();
//	Sensor_Init();
//	driver_led_init();
	driver_uart_init();
	driver_motor_init();
	
	driver_wdg_init();

	T_MENU_MSG menMsg = {0};
	menMsg.type =  BACKGROUND_WAKEUP;
	background_task_msg_send(&menMsg);
}


void System_Handler(void)
{
    NVIC_DisableIRQ(System_IRQn);

	menu_sleep_event_control(MENU_SLEEP_EVENT_WAKEUP_BIT, true); 
	
    if (System_WakeUpInterruptValue(UART_RX_PIN) == SET)
    {
        APP_PRINT_INFO0("rx wakeUp !!!!!!!!!!");
        Pad_ClearWakeupINTPendingBit(UART_RX_PIN);
        System_WakeUpPinDisable(UART_RX_PIN);
    }
#if 1
   	if((System_WakeUpInterruptValue(SENSOR_INT_PIN) == SET) )
    {   
        DBG_DIRECT("sensor wakeUp !!!!!!!!!!");
        Pad_ClearWakeupINTPendingBit(SENSOR_INT_PIN);
        System_WakeUpPinDisable(SENSOR_INT_PIN);
        
    }
#endif	

	if(System_WakeUpInterruptValue(PAIR_HAL1) == RESET)
    {   
        DBG_DIRECT("hal1 wakeUp !!!!!!!!!!");
        Pad_ClearWakeupINTPendingBit(PAIR_HAL1);
        System_WakeUpPinDisable(PAIR_HAL1);
    }	

	if(System_WakeUpInterruptValue(PAIR_HAL2) == RESET)
    {   
        DBG_DIRECT("hal2 wakeUp !!!!!!!!!!");
        Pad_ClearWakeupINTPendingBit(PAIR_HAL2);
        System_WakeUpPinDisable(PAIR_HAL2);
    }

    Pad_ClearAllWakeupINT();
    
    NVIC_ClearPendingIRQ(System_IRQn);

	wakeup_config();
	
}


