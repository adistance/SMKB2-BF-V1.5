#ifndef BLUETOOTH_MENU_H
#define BLUETOOTH_MENU_H

#include "menu_manage.h"
#include "app_msg.h"


typedef enum
{
	E_AUTH_RESP = 0x00,	 		//��֤
	E_SET_PASSWORD = 0x09,  		//�������� app -> lock
	E_RESP_SET_PASSWORD = 0x27, 	//�������������� lock -> app
	E_ADD_PASSWORD = 0x0A,   //�������� app -> lock
	E_RESP_ADD_PASSWORD = 0x0B,  //�������������� lock -> app
	E_DEL_PASSWORD = 0x0C,       //ɾ������ app -> lock
	E_RESP_DEL_PASSWORD = 0x0D,    //����ɾ����� lock -> app
	E_GET_PW_LIST = 0x0E,          //��ȡ�����б� app -> lock
	E_RESP_GET_PW_LIST = 0x0F,       //���������б��� lock -> app
	E_CHANGE_PW_LABLE = 0x10,     //�޸��������� app -> lock
	E_START_ADD_FP = 0x13,      //���ָ�� app -> lock
	E_RESP_START_ADD_FP = 0x14,        //�������ָ�ƽ�� lock -> app
	E_DEL_ONE_FP = 0x15,        //ɾ��ָ�� app -> lock
	E_RESP_DEL_ONE_FP = 0x16,     //����ɾ��ָ���� lock -> app
	E_ADD_CARD = 0x17,             // app -> lock
	E_RESP_ADD_CARD = 0x18,               // lock -> app
	E_DEL_CARD = 0x19,               // app -> lock
	E_RESP_DEL_CARD = 0x1A,            // lock -> app
	// 0x1B
	E_OPEN_DOOR = 0x1C,               //��ʱ��Կ���� app -> lock
	E_RESP_OPEN_DOOR = 0x1D,            //������ʱ��Կ������� lock -> app
	E_GET_FP_LIST = 0x1E,     //��ȡָ���б� app -> lock
	E_RESP_GET_FP_LIST = 0x1F,  //����ָ���б� lock -> app
	E_GET_CARD_LIST = 0x20,            // app -> lock
	E_RESP_GET_CARD_LIST = 0x21,         // lock -> app
	E_GET_CONFIGINFO = 0x22,          //��ȡ������Ϣ app -> lock
	E_RESP_GET_CONFIGINFO = 0x23,       //����������Ϣ lock -> app
	E_BLE_OPEN_DOOR = 0x24,       //����Ա�������� app -> lock
	E_RESP_BLE_OPEN_DOOR = 0x25,    //���ع���Ա����������� lock -> app
	// 0x26, 0x27
	E_CHANGE_LOCK_NAME = 0x28,  //�޸��������� app -> lock
	// 0x29, 0x2A, 0x2B
	E_CHANGE_VOLUME = 0x2C,  //�޸����� app -> lock
	// 0x2D
	E_CHANGE_AUTH_MODE = 0x2E,  // app -> lock
	// 0x2F
	E_CHANGE_FP_NAME = 0x30,  //�޸�ָ���� app -> lock
	// 0x31
	E_CHANGE_CARD_NAME = 0x32,  // app -> lock
	// 0x33
	E_CHANGE_LANGUAGE = 0x34,  // app -> lock
	// 0x35	
	E_EXIT_CONFIG_MODE = 0x36,        //�˳����� app -> lock
	E_RESP_EXIT_CONFIG_MODE = 0x37,  //��Ҫ���˳����� lock -> app
	// 0x38, 0x39
	E_VERIFY_PW = 0x3A,         //����У�� app -> lock
	E_RESP_VERIFY_PW = 0x3B,      //��������У���� lock -> app
	E_ADD_TEMP_FP = 0x3C,     //�����ʱָ�� app -> lock
	E_RESP_ADD_TEMP_FP = 0x3D,  //������ʱָ�ƽ�� lock -> app
	E_GET_PRODUCT_INFO = 0x3E,         //��ȡ��Ʒ��Ϣ app -> lock
	E_RESP_GET_PRODUCT_INFO = 0x3F,      //���ز�Ʒ��Ϣ lock -> app
	E_ERASE_SERVER_INFO = 0x40,        //���������� app -> lock
	E_GET_SERVER_PSK = 0x41,           //��ȡPSK app -> lock
	E_RESP_GET_SERVER_PSK = 0x42,        //����PSK lock -> app
	E_RESET_LOCK = 0x43,         //����ʼ�� app -> lock
	E_RESP_RESET_LOCK = 0x44,      //��������ʼ����� lock -> app
	E_REDUNDANT_PW = 0x45,      //��λ���� app -> lock
	// 0x46, 0x47, 0x48
	E_GET_OPEN_DOOR_RECORD = 0x49,     //��ȡ������¼ app -> lock
	E_RESP_GET_OPEN_DOOR_RECORD = 0x4A,  //���ؿ�����¼ lock -> app
	E_LOCK_DOOR_TIME = 0x4B,          //����ʱ�� app -> lock
	E_AUTO_LOCK_DOOR = 0x4C,          // app -> lock
	E_DOOR_OPEN_DIRECTION = 0x4D,     //���ŷ��� app -> lock
	E_MOTOR_OR_SION = 0x4E,          // app -> lock
	E_LATCH_BOLT_REVERT_TIME = 0x4F,    //б�෴תʱ�� app -> lock

	E_SET_OPEN_DOOR_STATE      = 0x50,  //���ó���ģʽ
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
