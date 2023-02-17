#ifndef TUYA_BLE_APP_DEMO_H_
#define TUYA_BLE_APP_DEMO_H_


#ifdef __cplusplus
extern "C" {
#endif


#define APP_PRODUCT_ID      "msn3f6wd"//"2ufgepff"//"xxxxxxxx"//"fqdyjmce"//"aaquyyzd"

#define APP_BUILD_FIRMNAME  "tuya_ble_sdk_2.0_demo_rtl8762d"

// firmware version 
#define TY_APP_VER_NUM       0x0101   //ota�������ж������ÿ�η��汾��Ҫ�޸�����
#define TY_APP_VER_STR	     "1.1" 	  //ota�������ж������ÿ�η��汾��Ҫ�޸�����

// hardware version 
#define TY_HARD_VER_NUM      0x0100
#define TY_HARD_VER_STR	     "1.0" 	

//Ϳѻƽ̨��DP���ܵ�PID
#define DP_ID_ADD_FP		1	//��ӿ�����ʽ
#define DP_ID_DEL_FP		2	//ɾ��������ʽ
#define DP_ID_BATTERY		8	//ʣ�����
#define DP_ID_FP_OPEN		12	//ָ�ƿ���
#define DP_ID_BLE_RECORD	19  //����������¼
#define DP_ID_RECORD		20	//������¼
#define DP_ID_ALARM			21	//��������
#define DP_ID_OPEN_MODE		33  //����ģʽ����
#define DP_ID_RTC			44	//����rtcʱ��
#define DP_ID_DOOR_STATUS	47	//������״̬
#define DP_ID_SYNC			54	//ͬ��������ʽ
#define DP_ID_REMOTE_OPEN   61  //Զ�̿���
#define DP_ID_REMOTE_RECORD 62	//Զ���ֻ�������¼
#define DP_ID_GET_RECORD	69	//��ȡ������¼
#define	DP_ID_SET_CONFIG	70	//������������
#define DP_ID_BLE_OPEN		71	//��������
#define DP_ID_REMOTE_CONFIG 73  //Զ�̿�������

//ע��׶�
#define	ENROLL_START	0x00	//ע�Ὺʼ
#define ENROLL_ING		0xFC	//ע����
#define ENROLL_FAIL		0xFD	//ע��ʧ��
#define	ENROLL_CANCEL	0xFE	//ע��ȡ��
#define	ENROLL_OK		0xFF	//ע�����

//����
typedef enum
{
	E_DEL_NUM 	= 0x00,	//ɾ����Ա
	E_PASSWORD	= 0x01,	//����
	E_DOORCARD	= 0x02, //�ſ�
	E_FINGER	= 0x03, //ָ��
	E_FACE		= 0x04, //����
}E_OPEN_TYPE;

//¼��ʧ�ܵ�ԭ��
typedef enum
{
	E_ENROLL_TIMEOUT = 0, 	//¼�볬ʱ
	E_ENROLL_FAIL,			//¼��ʧ��
	E_ENROLL_REPEAT,		//¼���ظ�
	E_ENROLL_FULL,			//Ӳ��ID������(ָ������)
	E_PS_DATA_ERR,			//�������������벻������
	E_PS_LEN_ERR,			//���������󣬳��ȴ���
	E_ADD_TYPE_ERR,			//��֧����ӵĿ�����ʽ����
	E_ENROLL_FP_ING,		//��ǰ����¼��ָ��
	E_ENROLL_DR_ING,		//��ǰ���ڰ��ſ�
	E_ENROLL_FACE_ING,		//��ǰ���ڰ�����
	E_PS_EASY,				//������ڼ�
	E_HARD_ID_ERR,			//����Ӳ��ID
}E_ENROLL_FAIL_TYPE;

//����ԭ��
typedef enum
{
	E_FP_ERROR = 0,		//ָ���Դ�
	E_PS_ERROR,			//�����Դ�
	E_DC_ERROR,			//�ſ��Դ�
	E_FACE_ERR0R,		//�����Դ�
	E_FALSE_LOCK,		//����
	E_HIGH_TEMP,		//����
	E_TIMEOUT,			//��ʱδ����
	E_LOKC_TONGUE_ERR,	//����δ����
	E_ANTI_PRYING,		//����
	E_KEY_INSIDE,		//Կ�ײ���
	E_LOW_POWER = 0x0A,	//�͵�
	E_POWER_EMPTY,		//�����ľ�
	E_VIBRATE,			//��
	E_BUFANG,			//��������
}E_ALARM_TYPE;

// ble adv interval, unit: ms
#define TUYA_BLE_APP_DEFAULT_ADV_INTERVAL   ( 200 )
    
// ble conn interval, unit:ms. range: 7.5~4000ms
#define TUYA_BLE_APP_CONN_INTERVAL_MIN  	( 80 )      
#define TUYA_BLE_APP_CONN_INTERVAL_MAX  	( 100 )    

//����dp�������
typedef enum
{
	DP_TYPE_RAW		= 0,	//RAW����
	DP_TYPE_BOOL 	= 1,	//��������
	DP_TYPE_VALUE	= 2,	//��ֵ����
	DP_TYPE_STRING	= 3,	//�ַ�������
	DP_TYPE_ENUM	= 4,	//ö������
	DP_TYPE_BINTMAP	= 5,	//λӳ������
}BLE_DP_TYPE;

//����������־
typedef enum
{
	open_from_tuya		= 0,	//Ϳѻ����
	no_open_from_tuya 	= 1,	//Ϳѻû�п���
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

