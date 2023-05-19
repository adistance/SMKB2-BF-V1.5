#ifndef MENU_MANAGE_H
#define MENU_MANAGE_H
#include "menu_sleep.h"

#include "stdio.h"
#include "stdbool.h"
#include "string.h"

#define LOCAL_LOGIC_EN          0               //�Ƿ��б��������߼�

typedef enum
{
    MENU_MSG_TYPE_MAIN              = 1, /*���˵�����*/
    MENU_MSG_TYPE_FINGERPRINT       = 2, /*ָ������*/
    MENU_MSG_TYPE_NFC               = 3, /*NFC����*/
    MENU_MSG_TYPE_TOUCH             = 4, /*��������*/
    MENU_MSG_TYPE_BUTTON            = 5, /*����ť����*/
    MENU_MSG_TYPE_BLUE              = 6, /*������������*/     //��ǰ����ʹ�� IO_MSG_TYPE_FINGERPRINT
    MENU_MSG_TYPE_TASK              = 7, /*ģ�����������*/
     MENU_MSG_TYPE_HAL				=8,/*������������*/
    
} T_MENU_MSG_TYPE;

typedef enum
{
	BT_MSG_RECV_DECODE 	= 1, //������ܵ�����
	BT_MSG_DATA_PROC 	= 2, //�������������
	BT_MSG_SEND_DATA 	= 3, //�ظ����ݵ�С����
} T_BT_SUB_MSG_TYPE;

typedef enum
{
    BACKGROUND_MSG_TYPE_VOICE       = 7, /*��������*/
    BACKGROUND_MSG_TYPE_LED         = 8, /*�ƹ�����*/
    BACKGROUND_MSG_TYPE_MOTOR       = 9, /*�������*/
    BACKGROUND_MSG_TYPE_UART        = 10,/*��������*/
    BACKGROUND_MSG_TYPE_BEEP		= 11,/*������*/ 
    BACKGROUND_MSG_TYPE_BT			= 12, //����С����
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
    BACKGROUND_MSG_LED_SUBTYPE_MATCH_SUCCESS        , //ƥ��ɹ�0
    BACKGROUND_MSG_LED_SUBTYPE_MATCH_FAIL        	,	//ƥ��ʧ��1
    BACKGROUND_MSG_LED_SUBTYPE_ENROLL_STATE         ,	//ע��ָ����2
    BACKGROUND_MSG_LED_SUBTYPE_ENROLL_SUCCESS       ,	//ע��ɹ�3
    BACKGROUND_MSG_LED_SUBTYPE_ENROLL_FAIL          ,	//ע��ʧ��4
    BACKGROUND_MSG_LED_SUBTYPE_PRESS_SUC			,   //��ѹ�ɹ�һ��5
    BACKGROUND_MSG_LED_SUBTYPE_PRESS_FAIL			,   //��ѹʧ��һ��6
    BACKGROUND_MSG_LED_SUBTYPE_RESET_INIT			,	//�ָ���������7
    BACKGROUND_MSG_LED_SUBTYPE_POWER_LOW			, //�͵�������8
    BACKGROUND_MSG_LED_SUBTYPE_POWER_LOW_1			, //�͵�������9
    BACKGROUND_MSG_LED_SUBTYPE_MOTOR_BACK			,  //�����ת
    BACKGROUND_MSG_LED_SUBTYPE_CLEAN_MODE_SUC		,  //�������ģʽ��ȷƥ��	
    BACKGROUND_MSG_LED_SUBTYPE_PRESS_ALARM			, //��������
    BACKGROUND_MSG_LED_SUBTYPE_CLEAN_PW_SUC			, //�ɹ��������
    BACKGROUND_MSG_LED_SUBTYPE_CLEAN_PW_ERR			, //�������ʧ��
    BACKGROUND_MSG_LED_SUBTYPE_CHARGE				, //���
    BACKGROUND_MSG_LED_SUBTYPE_CHARGE_FULL			, //������
    BACKGROUND_MSG_LED_SUBTYPE_CHARGE_STOP			, //ֹͣ���
    BACKGROUND_MSG_LED_SUBTYPE_STOP_ALL				, //�ص����е�
    BACKGROUND_MSG_LED_SUBTYPE_CLEAN_FP    			,//���ָ��
    BACKGROUND_MSG_LED_SUBTYPE_RED_ON				,//��Ƴ���
    BACKGROUND_MSG_LED_SUBTYPE_GREEN_ON				,//�̵Ƴ���
    BACKGROUND_MSG_LED_SUBTYPE_BLUE_ON				,//���Ƴ���
    BACKGROUND_MSG_LED_SUBTYPE_INT_START			,//ɾ��״̬
    BACKGROUND_MSG_LED_SUBTYPE_INT_SUCCESS			,//ָ��ɾ���ɹ�
    BACKGROUND_MSG_LED_SUBTYPE_INT_FAIL				,//ָ��ɾ��ʧ��
    BACKGROUND_MSG_LED_SUBTYPE_USBKEY_SUCCESS		,//USBKEY���°󶨳ɹ�
    BACKGROUND_MSG_LED_SUBTYPE_USBKEY_FAIL			,//USBKEY���°�ʧ��
    BACKGROUND_MSG_LED_SUBTYPE_UART_MODE			, //����ģʽ
    BACKGROUND_MSG_LED_SUBTYPE_ENROLL_TIMEOUT		, //ע�ᳬʱ
    BACKGROUND_MSG_LED_SUBTYPE_BLE_CONNECT_TIPS		, //����������ʾ
    BACKGROUND_MSG_LED_SUBTYPE_BLE_DISCONNECT_TIPS	, //�����Ͽ���ʾ
     BACKGROUND_MSG_LED_SUBTYPE_INIT_TIPS			,//�ָ�������ʾ
    BACKGROUND_MSG_LED_SUBTYPE_ENROLL_TIPS			,//ע����ʾ��
    BACKGROUND_MSG_LED_SUBTYPE_CHECK_ADMIN 			, //��֤����Ա
    BACKGROUND_MSG_LED_SUBTYPE_CHECK_ADMIN_SUC		, //��֤����Ա�ɹ�
    BACKGROUND_MSG_LED_SUBTYPE_CHECK_ADMIN_FAIL		, //��֤����Աʧ��
    BACKGROUND_MSG_LED_SUBTYPE_REPEAT   			, //�ظ�ָ����ʾ
    BACKGROUND_MSG_LED_SUBTYPE_EXIT_ENROLL			, //�˳�ע��
} T_BACKGROUND_MSG_LED_SUBTYPE;

//���������
typedef enum
{
	BACKGROUND_MSG_MOTOR_SUBTYPE_LEFT  	=  0,		//��ת
	BACKGROUND_MSG_MOTOR_SUBTYPE_RIGHT 	=  1,		//��ת
	BACKGROUND_MSG_MOTOR_SUBTYPE_OFF	=  2,		//ֹͣ
	BACKGROUND_MSG_MOTOR_SUBTYPE_RST    = 3,		//������ת
} T_BACKGROUND_MSG_MOTOR_SUBTYPE;

//ָ����������
typedef enum
{
	FP_MSG_SUBTYPE_MATH 		= 1,	//ָ��ƥ��
	FP_MSG_SUBTYPE_BT_ENROLL	= 2, 	//����ָ��ע��
	FP_MSG_SUBTYPE_ENROLL       = 3,	//����ָ��ע��
	FP_MSG_SUBTYPE_DELETE       = 4,	//����ɾ��ָ��
	FP_MSG_SUBTYPE_KEY_ENROLL,          //�̰�ע��
}T_FP_SUB_MSG_TYPE;
	
typedef enum
{
	MENU_MSG_BUTTON_SUBTYPE_RESET,    //�ظ���������
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

//�˵�״̬������������Ҫ�����ڸ�ϸ�Ļ���(������Ƶ�)
typedef enum
{
	menu_status_idle,		//����
	menu_status_working,	//����
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
