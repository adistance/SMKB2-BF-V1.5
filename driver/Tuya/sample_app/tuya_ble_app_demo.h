#ifndef TUYA_BLE_APP_DEMO_H_
#define TUYA_BLE_APP_DEMO_H_


#ifdef __cplusplus
extern "C" {
#endif


#define APP_PRODUCT_ID      "msn3f6wd"//"2ufgepff"//"xxxxxxxx"//"fqdyjmce"//"aaquyyzd"

#define APP_BUILD_FIRMNAME  "tuya_ble_sdk_2.0_demo_rtl8762d"

// firmware version 
#define TY_APP_VER_NUM       0x0101   //ota升级会判断这个，每次发版本都要修改这里
#define TY_APP_VER_STR	     "1.1" 	  //ota升级会判断这个，每次发版本都要修改这里

// hardware version 
#define TY_HARD_VER_NUM      0x0100
#define TY_HARD_VER_STR	     "1.0" 	

//涂鸦平台的DP功能点PID
#define DP_ID_ADD_FP		1	//添加开锁方式
#define DP_ID_DEL_FP		2	//删除开锁方式
#define DP_ID_BATTERY		8	//剩余电量
#define DP_ID_FP_OPEN		12	//指纹开锁
#define DP_ID_BLE_RECORD	19  //蓝牙开锁记录
#define DP_ID_RECORD		20	//关锁记录
#define DP_ID_ALARM			21	//门锁警告
#define DP_ID_OPEN_MODE		33  //常开模式设置
#define DP_ID_RTC			44	//本地rtc时钟
#define DP_ID_DOOR_STATUS	47	//锁开合状态
#define DP_ID_SYNC			54	//同步开锁方式
#define DP_ID_REMOTE_OPEN   61  //远程开锁
#define DP_ID_REMOTE_RECORD 62	//远程手机开锁记录
#define DP_ID_GET_RECORD	69	//获取开锁记录
#define	DP_ID_SET_CONFIG	70	//配置蓝牙开锁
#define DP_ID_BLE_OPEN		71	//蓝牙开锁
#define DP_ID_REMOTE_CONFIG 73  //远程开锁配置

//注册阶段
#define	ENROLL_START	0x00	//注册开始
#define ENROLL_ING		0xFC	//注册中
#define ENROLL_FAIL		0xFD	//注册失败
#define	ENROLL_CANCEL	0xFE	//注册取消
#define	ENROLL_OK		0xFF	//注册完成

//类型
typedef enum
{
	E_DEL_NUM 	= 0x00,	//删除成员
	E_PASSWORD	= 0x01,	//密码
	E_DOORCARD	= 0x02, //门卡
	E_FINGER	= 0x03, //指纹
	E_FACE		= 0x04, //人脸
}E_OPEN_TYPE;

//录入失败的原因
typedef enum
{
	E_ENROLL_TIMEOUT = 0, 	//录入超时
	E_ENROLL_FAIL,			//录入失败
	E_ENROLL_REPEAT,		//录入重复
	E_ENROLL_FULL,			//硬件ID分配完(指纹已满)
	E_PS_DATA_ERR,			//添加密码错误，密码不是数字
	E_PS_LEN_ERR,			//添加密码错误，长度错误
	E_ADD_TYPE_ERR,			//不支持添加的开锁方式类型
	E_ENROLL_FP_ING,		//当前正在录入指纹
	E_ENROLL_DR_ING,		//当前正在绑定门卡
	E_ENROLL_FACE_ING,		//当前正在绑定人脸
	E_PS_EASY,				//密码过于简单
	E_HARD_ID_ERR,			//错误硬件ID
}E_ENROLL_FAIL_TYPE;

//报警原因
typedef enum
{
	E_FP_ERROR = 0,		//指纹试错
	E_PS_ERROR,			//密码试错
	E_DC_ERROR,			//门卡试错
	E_FACE_ERR0R,		//人脸试错
	E_FALSE_LOCK,		//假锁
	E_HIGH_TEMP,		//高温
	E_TIMEOUT,			//超时未关门
	E_LOKC_TONGUE_ERR,	//锁舌未弹出
	E_ANTI_PRYING,		//防撬
	E_KEY_INSIDE,		//钥匙插入
	E_LOW_POWER = 0x0A,	//低点
	E_POWER_EMPTY,		//电量耗尽
	E_VIBRATE,			//振动
	E_BUFANG,			//布防警告
}E_ALARM_TYPE;

// ble adv interval, unit: ms
#define TUYA_BLE_APP_DEFAULT_ADV_INTERVAL   ( 200 )
    
// ble conn interval, unit:ms. range: 7.5~4000ms
#define TUYA_BLE_APP_CONN_INTERVAL_MIN  	( 80 )      
#define TUYA_BLE_APP_CONN_INTERVAL_MAX  	( 100 )    

//蓝牙dp点的类型
typedef enum
{
	DP_TYPE_RAW		= 0,	//RAW类型
	DP_TYPE_BOOL 	= 1,	//布尔类型
	DP_TYPE_VALUE	= 2,	//数值类型
	DP_TYPE_STRING	= 3,	//字符串类型
	DP_TYPE_ENUM	= 4,	//枚举类型
	DP_TYPE_BINTMAP	= 5,	//位映射类型
}BLE_DP_TYPE;

//蓝牙开锁标志
typedef enum
{
	open_from_tuya		= 0,	//涂鸦开锁
	no_open_from_tuya 	= 1,	//涂鸦没有开锁
}eAPP_for_open;
	
eAPP_for_open Get_App_for_open_flag(void);
void Set_App_for_open_flag(eAPP_for_open data);

void tuya_ble_app_task_create(void);

void tuya_ble_app_init(void);
extern void app_custom_task_check(void);
void tuya_ble_time_show(uint32_t u32Time);
uint8_t tuya_ble_get_record(uint32_t u32Time, uint32_t u32FpId);
uint8_t tuya_ble_alarm_report(E_ALARM_TYPE eType);
uint8_t tuya_ble_add_fp(uint32_t u32Sn, uint8_t * pu8In, uint32_t u32InLen);
uint8_t tuya_ble_report_door_status(bool bStatus);
uint8_t tuya_ble_get_battery(void);

#ifdef __cplusplus
}
#endif

#endif // 

