#ifndef SY_COMMAND_H
#define SY_COMMAND_H




//SY�����ֶ���
#define SY_CMD_GET_IMAGE         			           (0x01) //��֤�û�ȡͼ��
#define SY_CMD_ENROLL_CAPTURE_IMAGE                    (0x29) //ע���û�ȡͼ��
#define SY_CMD_GENERATE_FEATURE                        (0x02) //����ԭʼͼ��ʡ��ָ����������ģ�建������
#define SY_CMD_MATCH_FTR                               (0x03) //��׼ƥ�仺�����е������ļ�
#define SY_CMD_SEARCH_FTR                              (0x04) //��ģ�建�����е������ļ����������򲿷�ָ�ƿ�
#define SY_CMD_MEGER_FTR                               (0x05) //�������ļ��ϲ�����ģ�����ģ�建����
#define SY_CMD_STORAGE_FTR                             (0x06) //��ģ�建�����е��ļ����浽flashָ�ƿ���
#define SY_CMD_READ_FLASH_FTR_TO_SRAM                  (0x07) //��flashָ�ƿ��ж�ȡһ��ģ�嵽ģ�建����
#define SY_CMD_UPLOAD_SRAM_FTR_START                   (0x08) //��ģ�建�����е��ļ��ϴ�����λ��
#define SY_CMD_DOWNLOAD_FTR                            (0x09) //����λ������һ�������ļ���ģ�建����
#define SY_CMD_UPLOAD_IMAGE                            (0x0A) //�ϴ�ԭʼͼ��
#define SY_CMD_DOWNLOAD_IMAGE                          (0x0B) //����ԭʼͼ��
#define SY_CMD_DELETE_FTR                              (0x0C) //ɾ��flashָ�ƿ��е�һ�������ļ�
#define SY_CMD_CLEAR_FTR                               (0x0D) //���flashָ�ƿ�
#define SY_CMD_WRITE_REGIEST                           (0x0E) //дSOCϵͳ�Ĵ���
#define SY_CMD_READ_SYSTEM_PARA                        (0x0F) //��ϵͳ��������
#define SY_CMD_SET_HANDSHARK_PASSWORD                  (0x12) //�����豸���ֿ���
#define SY_CMD_VERIFY_HANDSHARK_PASSWORD               (0x13) //��֤�豸���ֿ���
#define SY_CMD_GET_RADOM_NUMBER                        (0x14) //���������
#define SY_CMD_SET_CHIP_ADDRESS                        (0x15) //����оƬ��ַ
#define SY_CMD_READ_INFO_PAGE                          (0x16) //��ȡFLASH Information Page����
#define SY_CMD_PORT_CONTROL                            (0x17) //ͨѶ�˿ڣ�UART/USB�����ؿ���
#define SY_CMD_WRITE_NOTEBOOK                          (0x18) //д���±�
#define SY_CMD_READ_NOTEBOOK                           (0x19) //�����±�
#define SY_CMD_BURN_CODE                               (0x1A) //��дƬ��FLASH
#define SY_CMD_HIGH_SPEED_SEARCH                       (0x1B) //��������FLASH
#define SY_CMD_GENERATE_BINARY_IMAGE                   (0x1C) //���ɶ�ֵ��ָ��ͼ��
#define SY_CMD_GET_FTR_COUNT                           (0x1D) //����Чģ�����
#define SY_CMD_GPIO_CONTROL                            (0x1E) //�û�GPIO��������
#define SY_CMD_READ_FP_INDEX_TABLE                     (0x1F) //��������
#define SY_CMD_CANCLE                                  (0x30) //ȡ��ָ��
#define SY_CMD_AUTO_ENROLL                             (0x31) //�Զ�ע��ģ��ָ��
#define SY_CMD_AUTO_IDENTIFY                           (0x32) //�Զ���ָ֤��ָ��
#define SY_CMD_SLEEP_MODE                              (0x33) //����ָ��
#define SY_CMD_GET_CHIP_SN                             (0x34) //��ȡоƬΨһ���к�
#define SY_CMD_HANDSHAKE                               (0x35) //����ָ��
#define SY_CMD_CHECK_SENSOR                            (0x36) //У�鴫����
#define SY_CMD_OF_LED                                  (0x40) //ŷ�Ƶ��Э��
#define SY_CMD_GET_ECHO								   (0x53)
#define SY_CMD_AUTO_LOGIN                              (0x54) 
#define SY_CMD_AUTO_MATCH                              (0x55)
#define SY_CMD_AUTO_SEARCH                             (0x58)
#define SY_CMD_BAUDTATE_TEST				  		   (0xd0) //�����ʲ���
#define SY_CMD_SET_FREQUENCY_CAILBRATION			   (0xd1) //��ƵУ׼����
#define SY_CMD_GET_FREQUENCY_CAILBRATION			   (0xd2) //��ȡ��ƵУ׼����
#define SY_CMD_GET_BG                                  (0xe0)
#define SY_CMD_SET_BG                                  (0xe1)
#define SY_CMD_HOST_GET_BG                             (0xe3)
#define SY_CMD_GET_VERSION                             (0xf0) //���汾��
#define SY_CMD_SET_ML_SN                               (0xf1) //����SN��
#define SY_CMD_GET_ML_SN                               (0xf2) //��ȡSN��
#define SY_CMD_SET_BAUDRATE                            (0xf3) //���ò�����
#define SY_CMD_TEST_CAPTURE                            (0xf4) //�����ò�ͼ����
#define SY_CMD_TEST_UPLOAD_IMAGE                       (0xf5) //�������ϴ�ͼ��
#define SY_CMD_UPLOAD_SRAM_FTR                         (0xf6) //�ϴ�FTR
#define SY_CMD_UPLOAD_FLASH_INFO                       (0xf7) //�ϴ�Flash_Info
#define SY_CMD_FP_DOWNLOAD_START                       (0xFE)
#define SY_CMD_FP_DELAY_REBOOT                         (0xFF)
#define SY_CMD_FP_ADMIN_SLEEP                          (0xAA)
#define SY_CMD_SET_LED_CTRL_INFO                       (0x3C)



//SYȷ���붨��
#define SY_COMP_CODE_OK                                (0x00) // 00H����ʾָ��ִ����ϻ�OK�� 
#define SY_COMP_CODE_DATA_ERROR                        (0x01) // 01H����ʾ���ݰ����մ��� 
#define SY_COMP_CODE_NO_FINGER_DETECT                  (0x02) // 02H����ʾ��������û����ָ�� 
#define SY_COMP_CODE_CAPTURE_ERROR                     (0x03) // 03H����ʾ¼��ָ��ͼ��ʧ�ܣ� 
#define SY_COMP_CODE_FINGER_DRY_GENERATE_FTR_ERROR     (0x04) // 04H����ʾָ��ͼ��̫�ɡ�̫���������������� 
#define SY_COMP_CODE_FINGER_WET_GENERATE_FTR_ERROR     (0x05) // 05H����ʾָ��ͼ��̫ʪ��̫���������������� 
#define SY_COMP_CODE_UNQUALIFIED_IMAGE_ERROR           (0x06) // 06H����ʾָ��ͼ��̫�Ҷ������������� 
#define SY_COMP_CODE_LACK_FTR_GENERATE_FTR_ERROR       (0x07) // 07H����ʾָ��ͼ����������������̫�٣������̫С���������������� 
#define SY_COMP_CODE_FTR_MATCH_ERROR                   (0x08) // 08H����ʾָ�Ʋ�ƥ�䣻 
#define SY_COMP_CODE_SEARCH_FTR_NOT_MATCH              (0x09) // 09H����ʾû������ָ�ƣ� 
#define SY_COMP_CODE_MERGE_FTR_ERROR                   (0x0A) // 0aH����ʾ�����ϲ�ʧ�ܣ�
#define SY_COMP_CODE_FP_INDEX_OVERFLOW                 (0x0B) // 0bH����ʾ����ָ�ƿ�ʱ��ַ��ų���ָ�ƿⷶΧ�� 
#define SY_COMP_CODE_READ_FTR_ERROR                    (0x0C) // 0cH����ʾ��ָ�ƿ��ģ��������Ч�� 
#define SY_COMP_CODE_UPLOAD_FTR_ERROR                  (0x0D) // 0dH����ʾ�ϴ�����ʧ�ܣ� 
#define SY_COMP_CODE_RECIEVE_LATER_PACKAGE_ERROR       (0x0E) // 0eH����ʾģ�鲻�ܽ��պ������ݰ��� 
#define SY_COMP_CODE_UPLOAD_IMAGE_ERROR                (0x0F) // 0fH����ʾ�ϴ�ͼ��ʧ�ܣ� 
#define SY_COMP_CODE_DELETE_FTR_ERROR                  (0x10) // 10H����ʾɾ��ģ��ʧ�ܣ� 
#define SY_COMP_CODE_CLEAR_FTR_ERROR                   (0x11) // 11H����ʾ���ָ�ƿ�ʧ�ܣ�
#define SY_COMP_CODE_ENTER_SLEEP_MODE_ERROR            (0x12) // 12H����ʾ���ܽ���͹���״̬��
#define SY_COMP_CODE_PASSWORD_INVALID                  (0x13) // 13H����ʾ�����ȷ�� 
#define SY_COMP_CODE_REBOOT_ERROR                      (0x14) // 14H����ʾϵͳ��λʧ�ܣ� 
#define SY_COMP_CODE_NO_IMAGE                          (0x15) // 15H����ʾ��������û����Чԭʼͼ��������ͼ�� 
#define SY_COMP_CODE_ONLINE_UPDATE_ERROR               (0x16) // 16H����ʾ��������ʧ�ܣ� 
#define SY_COMP_CODE_DO_NOT_MOVE_FINGER                (0x17) // 17H����ʾ����ָ�ƻ����βɼ�֮����ָû���ƶ����� 
#define SY_COMP_CODE_READ_WRITE_FLASH_ERROR            (0x18) // 18H����ʾ��дFLASH���� 
#define SY_COMP_CODE_GENERATE_RANDOM_ERROR             (0x19) // 19H�����������ʧ�ܣ� 
#define SY_COMP_CODE_INVALID_REG_ADDR                  (0x1A) // 1aH����Ч�Ĵ����ţ� 
#define SY_COMP_CODE_REG_VALUE_ERRPR                   (0x1B) // 1bH���Ĵ����趨���ݴ���ţ� 
#define SY_COMP_CODE_NOTEBOOK_PAGE_NUM_ERROR           (0x1C) // 1cH�����±�ҳ��ָ������ 
#define SY_COMP_CODE_PORT_OPRATION_ERROR               (0x1D) // 1dH���˿ڲ���ʧ�ܣ� 
#define SY_COMP_CODE_AUTO_ENROLL_ERROR                 (0x1E) // 1eH���Զ�ע�ᣨenroll��ʧ�ܣ� 
#define SY_COMP_CODE_STORAGE_IS_FULL                   (0x1F) // 1fH��ָ�ƿ����� 
#define SY_COMP_CODE_DEVICE_ADDR_ERROR                 (0x20) // 20H���豸��ַ���� 
#define SY_COMP_CODE_PASSWORD_ERROR                    (0x21) // 21H���������� 
#define SY_COMP_CODE_FP_BUFFER_NOT_EMPTY               (0x22) // 22H��ָ��ģ��ǿգ� 
#define SY_COMP_CODE_FP_BUFFER_EMPTY                   (0x23) // 23H��ָ��ģ��Ϊ�գ�
#define SY_COMP_CODE_STORAGE_SPACE_EMPTY               (0x24) // 24H��ָ�ƿ�Ϊ�գ�
#define SY_COMP_CODE_FP_EXIST_AUTO                     (0x24) // 24H��ָ���Ѵ���(autoLogin��)��

#define SY_COMP_CODE_ENROLL_NUM_ERROR                  (0x25) // 25H��¼��������ô��� 
#define SY_COMP_CODE_TIMEOUT                           (0X26) // 26H����ʱ�� 
#define SY_COMP_CODE_FP_EXIST                          (0x27) // 27H��ָ���Ѵ��ڣ� 
#define SY_COMP_CODE_FP_ASSOCIAT                       (0x28) // 28H��ָ��ģ���й����� 
#define SY_COMP_CODE_SENSOR_INITIAL_ERROR              (0x29) // 29H����������ʼ��ʧ�ܣ�
#define SY_COMP_CODE_SN_VALID                          (0x2A) // 2AH��ģ��Ψһ���кŷǿգ�
#define SY_COMP_CODE_SN_INVALID                        (0x2B) // 2BH��ģ��Ψһ���к�Ϊ�գ� 
#define SY_COMP_CODE_OTP_ERROR                         (0x2C) // 2CH��OTP����ʧ��;
#define SY_COMP_CODE_ECHO							   (0x55)
#define SY_COMP_CODE_FIRST_ENROLL_OK                   (0x56) // 56H:    �Զ�ע���һ�βɼ��ɹ�;
#define SY_COMP_CODE_SECOND_ENROLL_OK                  (0x57) // 57H:    �Զ�ע��ڶ��βɼ��ɹ�;
#define SY_COMP_CODE_THIRD_ENROLL_OK                   (0x58) // 58H:    �Զ�ע������βɼ��ɹ�;
#define SY_COMP_CODE_LATER_PACKAGE_RECIEVE_OK          (0xF0) // f0H���к������ݰ���ָ���ȷ���պ���0xf0Ӧ�� 
#define SY_COMP_CODE_LATER_CMD_PACKAGE_OK              (0xF1) // f1H���к������ݰ���ָ��������0xf1Ӧ�� 
#define SY_COMP_CODE_FLASH_CHECKSUM_ERROR              (0xF2) // f2H����ʾ��д�ڲ�FLASHʱ��У��ʹ��� 
#define SY_COMP_CODE_FLASH_PACKAGE_FLAG_ERROR          (0xF3) // f3H����ʾ��д�ڲ�FLASHʱ������ʶ���� 
#define SY_COMP_CODE_FLASH_PACKAGE_LEN_ERROR           (0xF4) // f4H����ʾ��д�ڲ�FLASHʱ�������ȴ��� 
#define SY_COMP_CODE_FLASH_CODE_SIZE_ERROR             (0xF5) // f5H����ʾ��д�ڲ�FLASHʱ�����볤��̫���� 
#define SY_COMP_CODE_FLASH_WRITE_FAIL                  (0xF6) // f6H����ʾ��д�ڲ�FLASHʱ����дFLASHʧ�ܣ�
#define SY_COMP_CODE_OTHER_ERROR                       (0xff)
#define SY_COMP_CODE_ICN7000_ERROR_ID                  (0xF7)
#define SY_COMP_CODE_ICN7000_ERROR_FORCE_QUIT          (0xF8)
#define SY_COMP_CODE_ICN7000_ERROR_UNQUALIFIED_IMAGE   (0xF9)


//addr flag len cmd
#define SY_CMD_REQ_HEAD_LEN                        (8)
//flag len comp_code
#define SY_CMD_RESP_HEAD_LEN                       (8)

#define RESP_FLAG_DATA                       (0x02)
#define RESP_FLAG_DATA_END                   (0x08)   



#define AUTO_CMD_AUTOLED_FLAG                   (0X0001)
#define AUTO_CMD_PRE_FLAG                       (0X0002)
#define AUTO_CMD_ACK_FLAG                       (0X0004)
#define AUTO_CMD_COVER_ID                       (0X0008)
#define AUTO_CMD_REPEAT_FLAG                    (0X0010)
#define AUTO_CMD_FP_LEAVE_FLAG                  (0X0020)


//��Ԫ�Ĵ�����
#define BAUDRATE_REG        (4)
#define MATCH_LEVEL_REG     (5)
#define PACKET_LENGTH_REG   (6)

typedef enum
{
	SY_LED_MODE_NONE = 0,
	SY_LED_TYPE_BREATH = 1,	 //������
	SY_LED_TYPE_BLINK = 2,	//��˸��
	SY_LED_TYPE_ON = 3,		//����
	SY_LED_TYPE_OFF = 4,	//����
	SY_LED_TYPE_BRE_ON = 5,	//������
	SY_LED_TYPE_BRE_OFF = 6, //���ص�
}SY_LED_MODE_E;

typedef struct{
	unsigned int address;
    unsigned char flag;
    unsigned short len;
    unsigned char cmd;
    unsigned char req[50];    //req����
}SY_CMD_REQ_DATA, *PSY_CMD_REQ_DATA;

typedef struct{
	unsigned int address;
    unsigned char flag;
    unsigned short len;
    unsigned char comp_code;
    unsigned char *resp;      //resp��������
}SY_CMD_RESP_DATA, *PSY_CMD_RESP_DATA;

void SY_COMMAND_TransSetReqData(PSY_CMD_REQ_DATA pReq, unsigned char data, unsigned int index);
unsigned char SY_COMMAND_TransGetRespData(PSY_CMD_RESP_DATA pResp, unsigned char *pdata, unsigned int index);
void SY_COMMAND_ReqProc(PSY_CMD_REQ_DATA pReq);

void SY_COMMAND_Task(void);

#endif


