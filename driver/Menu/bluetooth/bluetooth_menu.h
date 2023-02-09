#ifndef BLUETOOTH_MENU_H
#define BLUETOOTH_MENU_H

#include "menu_manage.h"
#include "app_msg.h"


typedef enum
{
	E_AUTH_RESP = 0x00,	 		//认证
	E_SET_PASSWORD = 0x09,  		//设置密码 app -> lock
	E_RESP_SET_PASSWORD = 0x27, 	//返回设置密码结果 lock -> app
	E_ADD_PASSWORD = 0x0A,   //增加密码 app -> lock
	E_RESP_ADD_PASSWORD = 0x0B,  //返回增加密码结果 lock -> app
	E_DEL_PASSWORD = 0x0C,       //删除密码 app -> lock
	E_RESP_DEL_PASSWORD = 0x0D,    //返回删除结果 lock -> app
	E_GET_PW_LIST = 0x0E,          //获取密码列表 app -> lock
	E_RESP_GET_PW_LIST = 0x0F,       //返回密码列表结果 lock -> app
	E_CHANGE_PW_LABLE = 0x10,     //修改密码名字 app -> lock
	E_START_ADD_FP = 0x13,      //添加指纹 app -> lock
	E_RESP_START_ADD_FP = 0x14,        //返回添加指纹结果 lock -> app
	E_DEL_ONE_FP = 0x15,        //删除指纹 app -> lock
	E_RESP_DEL_ONE_FP = 0x16,     //返回删除指针结果 lock -> app
	E_ADD_CARD = 0x17,             // app -> lock
	E_RESP_ADD_CARD = 0x18,               // lock -> app
	E_DEL_CARD = 0x19,               // app -> lock
	E_RESP_DEL_CARD = 0x1A,            // lock -> app
	// 0x1B
	E_OPEN_DOOR = 0x1C,               //临时密钥开锁 app -> lock
	E_RESP_OPEN_DOOR = 0x1D,            //返回临时密钥开锁结果 lock -> app
	E_GET_FP_LIST = 0x1E,     //获取指纹列表 app -> lock
	E_RESP_GET_FP_LIST = 0x1F,  //返回指纹列表 lock -> app
	E_GET_CARD_LIST = 0x20,            // app -> lock
	E_RESP_GET_CARD_LIST = 0x21,         // lock -> app
	E_GET_CONFIGINFO = 0x22,          //获取配置信息 app -> lock
	E_RESP_GET_CONFIGINFO = 0x23,       //返回配置信息 lock -> app
	E_BLE_OPEN_DOOR = 0x24,       //管理员蓝牙开锁 app -> lock
	E_RESP_BLE_OPEN_DOOR = 0x25,    //返回管理员蓝牙开锁结果 lock -> app
	// 0x26, 0x27
	E_CHANGE_LOCK_NAME = 0x28,  //修改锁的名字 app -> lock
	// 0x29, 0x2A, 0x2B
	E_CHANGE_VOLUME = 0x2C,  //修改音量 app -> lock
	// 0x2D
	E_CHANGE_AUTH_MODE = 0x2E,  // app -> lock
	// 0x2F
	E_CHANGE_FP_NAME = 0x30,  //修改指纹名 app -> lock
	// 0x31
	E_CHANGE_CARD_NAME = 0x32,  // app -> lock
	// 0x33
	E_CHANGE_LANGUAGE = 0x34,  // app -> lock
	// 0x35	
	E_EXIT_CONFIG_MODE = 0x36,        //退出设置 app -> lock
	E_RESP_EXIT_CONFIG_MODE = 0x37,  //锁要求退出设置 lock -> app
	// 0x38, 0x39
	E_VERIFY_PW = 0x3A,         //密码校验 app -> lock
	E_RESP_VERIFY_PW = 0x3B,      //返回密码校验结果 lock -> app
	E_ADD_TEMP_FP = 0x3C,     //添加临时指纹 app -> lock
	E_RESP_ADD_TEMP_FP = 0x3D,  //返回临时指纹结果 lock -> app
	E_GET_PRODUCT_INFO = 0x3E,         //获取产品信息 app -> lock
	E_RESP_GET_PRODUCT_INFO = 0x3F,      //返回产品信息 lock -> app
	E_ERASE_SERVER_INFO = 0x40,        //服务器擦除 app -> lock
	E_GET_SERVER_PSK = 0x41,           //获取PSK app -> lock
	E_RESP_GET_SERVER_PSK = 0x42,        //返回PSK lock -> app
	E_RESET_LOCK = 0x43,         //锁初始化 app -> lock
	E_RESP_RESET_LOCK = 0x44,      //返回锁初始化结果 lock -> app
	E_REDUNDANT_PW = 0x45,      //虚位密码 app -> lock
	// 0x46, 0x47, 0x48
	E_GET_OPEN_DOOR_RECORD = 0x49,     //获取开锁记录 app -> lock
	E_RESP_GET_OPEN_DOOR_RECORD = 0x4A,  //返回开锁记录 lock -> app
	E_LOCK_DOOR_TIME = 0x4B,          //关门时间 app -> lock
	E_AUTO_LOCK_DOOR = 0x4C,          // app -> lock
	E_DOOR_OPEN_DIRECTION = 0x4D,     //开门方向 app -> lock
	E_MOTOR_OR_SION = 0x4E,          // app -> lock
	E_LATCH_BOLT_REVERT_TIME = 0x4F,    //斜舌反转时间 app -> lock

	E_SET_OPEN_DOOR_STATE      = 0x50,  //设置常开模式
	E_RESP_SET_OPEN_DOOR     = 0x83

}bt_cmd_list_e;


typedef __packed struct _ConfigInfo {
  unsigned char batteryInfo;
  unsigned char openDoorAuthMode;
  unsigned char volume;
  unsigned char language;
  unsigned char vitualPassWord;
  unsigned char cardCount;
  unsigned char fingerprintCount;
  unsigned char passwordCount;
  unsigned char passwordMinLength;
  unsigned char passwordMaxLength;
  unsigned char lockType;
  unsigned char closeDoorTime;
  unsigned char autoLockMode;
  unsigned char openDoorDirection;
  unsigned char motorTorSion;
  unsigned char latchBoltRevertTime;
  unsigned char passwordExist;
  unsigned char cardExist;
  unsigned char fingerprintExist;
  unsigned char tempoFingerprintExist;
  unsigned char tempoCardExist;
  unsigned char tempoPasswordExist;
} sConfigInfo __attribute__((aligned (1)));


void bluetooth_init(void);
int bluetooth_handle_msg(T_MENU_MSG *bt_msg);
int bluetooth_tx_data_encode(uint8_t *data, uint16_t len);
bool bluebooth_send_msg_to_menu_task(T_BT_SUB_MSG_TYPE subtype, unsigned char *data, unsigned short len);
int bluetooth_enroll(void);

#endif
