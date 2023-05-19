#ifndef MENU_MANAGE_H
#define MENU_MANAGE_H
#include "menu_sleep.h"

#include "stdio.h"
#include "stdbool.h"
#include "string.h"

#define LOCAL_LOGIC_EN          0               //是否有本地锁的逻辑

typedef enum
{
    MENU_MSG_TYPE_MAIN              = 1, /*主菜单类型*/
    MENU_MSG_TYPE_FINGERPRINT       = 2, /*指纹类型*/
    MENU_MSG_TYPE_NFC               = 3, /*NFC类型*/
    MENU_MSG_TYPE_TOUCH             = 4, /*触控类型*/
    MENU_MSG_TYPE_BUTTON            = 5, /*物理按钮类型*/
    MENU_MSG_TYPE_BLUE              = 6, /*蓝牙操作类型*/     //当前工程使用 IO_MSG_TYPE_FINGERPRINT
    MENU_MSG_TYPE_TASK              = 7, /*模组命令处理类型*/
     MENU_MSG_TYPE_HAL				=8,/*霍尔开关类型*/
    
} T_MENU_MSG_TYPE;

typedef enum
{
	BT_MSG_RECV_DECODE 	= 1, //解码接受的数据
	BT_MSG_DATA_PROC 	= 2, //处理解码后的数据
	BT_MSG_SEND_DATA 	= 3, //回复数据到小程序
} T_BT_SUB_MSG_TYPE;

typedef enum
{
    BACKGROUND_MSG_TYPE_VOICE       = 7, /*语音类型*/
    BACKGROUND_MSG_TYPE_LED         = 8, /*灯光类型*/
    BACKGROUND_MSG_TYPE_MOTOR       = 9, /*电机类型*/
    BACKGROUND_MSG_TYPE_UART        = 10,/*串口数据*/
    BACKGROUND_MSG_TYPE_BEEP		= 11,/*蜂鸣器*/ 
    BACKGROUND_MSG_TYPE_BT			= 12, //蓝牙小程序
    BACKGROUND_WAKEUP, 
    BACKGROUND_SUSPEND,
    BACKGROUND_TEST,
} T_BACKGROUND_MSG_TYPE;

typedef enum
{
    MENU_MSG_MAIN_SUBTYPE_OPENDOOR      = 1,
} T_MENU_MSG_MAIN_SUBTYPE;

typedef enum
{
    MENU_MSG_FINGERPRINT_SUBTYPE_CHECK          = 0,
    MENU_MSG_FINGERPRINT_SUBTYPE_MATCH          = 1,
    MENU_MSG_FINGERPRINT_SUBTYPE_ENROLL         = 2,
    MENU_MSG_FINGERPRINT_SUBTYPE_DELETE         = 3,
    
} T_MENU_MSG_FINGERPRINT_SUBTYPE;

typedef enum
{
    BACKGROUND_MSG_LED_SUBTYPE_MATCH_SUCCESS        , //匹配成功0
    BACKGROUND_MSG_LED_SUBTYPE_MATCH_FAIL        	,	//匹配失败1
    BACKGROUND_MSG_LED_SUBTYPE_ENROLL_STATE         ,	//注册指纹中2
    BACKGROUND_MSG_LED_SUBTYPE_ENROLL_SUCCESS       ,	//注册成功3
    BACKGROUND_MSG_LED_SUBTYPE_ENROLL_FAIL          ,	//注册失败4
    BACKGROUND_MSG_LED_SUBTYPE_PRESS_SUC			,   //按压成功一次5
    BACKGROUND_MSG_LED_SUBTYPE_PRESS_FAIL			,   //按压失败一次6
    BACKGROUND_MSG_LED_SUBTYPE_RESET_INIT			,	//恢复出厂设置7
    BACKGROUND_MSG_LED_SUBTYPE_POWER_LOW			, //低电量报警8
    BACKGROUND_MSG_LED_SUBTYPE_POWER_LOW_1			, //低电量报警9
    BACKGROUND_MSG_LED_SUBTYPE_MOTOR_BACK			,  //电机回转
    BACKGROUND_MSG_LED_SUBTYPE_CLEAN_MODE_SUC		,  //清除密码模式正确匹配	
    BACKGROUND_MSG_LED_SUBTYPE_PRESS_ALARM			, //长按警告
    BACKGROUND_MSG_LED_SUBTYPE_CLEAN_PW_SUC			, //成功清除密码
    BACKGROUND_MSG_LED_SUBTYPE_CLEAN_PW_ERR			, //清除密码失败
    BACKGROUND_MSG_LED_SUBTYPE_CHARGE				, //充电
    BACKGROUND_MSG_LED_SUBTYPE_CHARGE_FULL			, //充满电
    BACKGROUND_MSG_LED_SUBTYPE_CHARGE_STOP			, //停止充电
    BACKGROUND_MSG_LED_SUBTYPE_STOP_ALL				, //关掉所有灯
    BACKGROUND_MSG_LED_SUBTYPE_CLEAN_FP    			,//清楚指纹
    BACKGROUND_MSG_LED_SUBTYPE_RED_ON				,//红灯常亮
    BACKGROUND_MSG_LED_SUBTYPE_GREEN_ON				,//绿灯常亮
    BACKGROUND_MSG_LED_SUBTYPE_BLUE_ON				,//蓝灯常亮
    BACKGROUND_MSG_LED_SUBTYPE_INT_START			,//删除状态
    BACKGROUND_MSG_LED_SUBTYPE_INT_SUCCESS			,//指纹删除成功
    BACKGROUND_MSG_LED_SUBTYPE_INT_FAIL				,//指纹删除失败
    BACKGROUND_MSG_LED_SUBTYPE_USBKEY_SUCCESS		,//USBKEY重新绑定成功
    BACKGROUND_MSG_LED_SUBTYPE_USBKEY_FAIL			,//USBKEY重新绑定失败
    BACKGROUND_MSG_LED_SUBTYPE_UART_MODE			, //串口模式
    BACKGROUND_MSG_LED_SUBTYPE_ENROLL_TIMEOUT		, //注册超时
    BACKGROUND_MSG_LED_SUBTYPE_BLE_CONNECT_TIPS		, //蓝牙连接提示
    BACKGROUND_MSG_LED_SUBTYPE_BLE_DISCONNECT_TIPS	, //蓝牙断开提示
     BACKGROUND_MSG_LED_SUBTYPE_INIT_TIPS			,//恢复出厂提示
    BACKGROUND_MSG_LED_SUBTYPE_ENROLL_TIPS			,//注册提示灯
    BACKGROUND_MSG_LED_SUBTYPE_CHECK_ADMIN 			, //验证管理员
    BACKGROUND_MSG_LED_SUBTYPE_CHECK_ADMIN_SUC		, //验证管理员成功
    BACKGROUND_MSG_LED_SUBTYPE_CHECK_ADMIN_FAIL		, //验证管理员失败
    BACKGROUND_MSG_LED_SUBTYPE_REPEAT   			, //重复指纹提示
    BACKGROUND_MSG_LED_SUBTYPE_EXIT_ENROLL			, //退出注册
} T_BACKGROUND_MSG_LED_SUBTYPE;

//电机子类型
typedef enum
{
	BACKGROUND_MSG_MOTOR_SUBTYPE_LEFT  	=  0,		//左转
	BACKGROUND_MSG_MOTOR_SUBTYPE_RIGHT 	=  1,		//右转
	BACKGROUND_MSG_MOTOR_SUBTYPE_OFF	=  2,		//停止
	BACKGROUND_MSG_MOTOR_SUBTYPE_RST    = 3,		//补偿回转
} T_BACKGROUND_MSG_MOTOR_SUBTYPE;

//指纹类子类型
typedef enum
{
	FP_MSG_SUBTYPE_MATH 		= 1,	//指纹匹配
	FP_MSG_SUBTYPE_BT_ENROLL	= 2, 	//蓝牙指纹注册
	FP_MSG_SUBTYPE_ENROLL       = 3,	//长按指纹注册
	FP_MSG_SUBTYPE_DELETE       = 4,	//长按删除指纹
	FP_MSG_SUBTYPE_KEY_ENROLL,          //短按注册
}T_FP_SUB_MSG_TYPE;
	
typedef enum
{
	MENU_MSG_BUTTON_SUBTYPE_RESET,    //回复出厂设置
}T_MENU_MSG_BUTTON_SUBTYPE;


typedef struct
{
    unsigned char  deleteMode;
    unsigned char  deleteNum;
    unsigned short deleteIdx;
}__attribute__((packed)) T_MENU_MSG_FINGERPRINT_DELETE, *PT_MENU_MSG_FINGERPRINT_DELETE;


typedef struct
{
    unsigned char   type;
    unsigned char   subtype;
    unsigned short  len;
    union
    {
        unsigned int param;
        void         *buf;
    } u;
} T_MENU_MSG;

//菜单状态机，后面有需要可以在更细的划分(电机，灯等)
typedef enum
{
	menu_status_idle,		//空闲
	menu_status_working,	//工作
}e_menu_status;

void menu_task_init(void);
bool menu_task_msg_send(T_MENU_MSG *menu_msg);
void Set_u32Voltage_Value(unsigned int Voltage);
unsigned int Get_u32Voltage_Value(void);

bool background_task_msg_send(T_MENU_MSG *p_background_msg);
bool menu_task_msg_send_front(T_MENU_MSG *p_menu_msg);   
bool background_task_msg_send_front(T_MENU_MSG *p_menu_msg);

bool menu_fingerprint_sem_take(unsigned int wait_ms);
void menu_fingerprint_sem_give(void);
void menu_loop_timer_conctrol(bool flag);
void suspend_task(void);
void resume_task(bool bStartBLE);
void menu_set_press_status(bool data);
void menu_reset_bt_timeout(void);

#endif
