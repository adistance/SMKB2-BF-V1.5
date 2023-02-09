#ifndef SY_COMMAND_H
#define SY_COMMAND_H




//SY命令字定义
#define SY_CMD_GET_IMAGE         			           (0x01) //验证用获取图像
#define SY_CMD_ENROLL_CAPTURE_IMAGE                    (0x29) //注册用获取图像
#define SY_CMD_GENERATE_FEATURE                        (0x02) //根据原始图像省出指纹特征存在模板缓冲区中
#define SY_CMD_MATCH_FTR                               (0x03) //精准匹配缓冲区中的特征文件
#define SY_CMD_SEARCH_FTR                              (0x04) //以模板缓冲区中的特征文件搜索整个或部分指纹库
#define SY_CMD_MEGER_FTR                               (0x05) //将特征文件合并生成模板存于模板缓冲区
#define SY_CMD_STORAGE_FTR                             (0x06) //将模板缓冲区中的文件储存到flash指纹库中
#define SY_CMD_READ_FLASH_FTR_TO_SRAM                  (0x07) //从flash指纹库中读取一个模板到模板缓冲区
#define SY_CMD_UPLOAD_SRAM_FTR_START                   (0x08) //将模板缓冲区中的文件上传给上位机
#define SY_CMD_DOWNLOAD_FTR                            (0x09) //从上位机下载一个特征文件到模板缓冲区
#define SY_CMD_UPLOAD_IMAGE                            (0x0A) //上传原始图像
#define SY_CMD_DOWNLOAD_IMAGE                          (0x0B) //下载原始图像
#define SY_CMD_DELETE_FTR                              (0x0C) //删除flash指纹库中的一个特征文件
#define SY_CMD_CLEAR_FTR                               (0x0D) //清空flash指纹库
#define SY_CMD_WRITE_REGIEST                           (0x0E) //写SOC系统寄存器
#define SY_CMD_READ_SYSTEM_PARA                        (0x0F) //读系统基本参数
#define SY_CMD_SET_HANDSHARK_PASSWORD                  (0x12) //设置设备握手口令
#define SY_CMD_VERIFY_HANDSHARK_PASSWORD               (0x13) //验证设备握手口令
#define SY_CMD_GET_RADOM_NUMBER                        (0x14) //采样随机数
#define SY_CMD_SET_CHIP_ADDRESS                        (0x15) //设置芯片地址
#define SY_CMD_READ_INFO_PAGE                          (0x16) //读取FLASH Information Page内容
#define SY_CMD_PORT_CONTROL                            (0x17) //通讯端口（UART/USB）开关控制
#define SY_CMD_WRITE_NOTEBOOK                          (0x18) //写记事本
#define SY_CMD_READ_NOTEBOOK                           (0x19) //读记事本
#define SY_CMD_BURN_CODE                               (0x1A) //烧写片内FLASH
#define SY_CMD_HIGH_SPEED_SEARCH                       (0x1B) //高速搜索FLASH
#define SY_CMD_GENERATE_BINARY_IMAGE                   (0x1C) //生成二值化指纹图像
#define SY_CMD_GET_FTR_COUNT                           (0x1D) //读有效模板个数
#define SY_CMD_GPIO_CONTROL                            (0x1E) //用户GPIO控制命令
#define SY_CMD_READ_FP_INDEX_TABLE                     (0x1F) //读索引表
#define SY_CMD_CANCLE                                  (0x30) //取消指令
#define SY_CMD_AUTO_ENROLL                             (0x31) //自动注册模块指令
#define SY_CMD_AUTO_IDENTIFY                           (0x32) //自动验证指纹指令
#define SY_CMD_SLEEP_MODE                              (0x33) //休眠指令
#define SY_CMD_GET_CHIP_SN                             (0x34) //获取芯片唯一序列号
#define SY_CMD_HANDSHAKE                               (0x35) //握手指令
#define SY_CMD_CHECK_SENSOR                            (0x36) //校验传感器
#define SY_CMD_OF_LED                                  (0x40) //欧菲点灯协议
#define SY_CMD_GET_ECHO								   (0x53)
#define SY_CMD_AUTO_LOGIN                              (0x54) 
#define SY_CMD_AUTO_MATCH                              (0x55)
#define SY_CMD_AUTO_SEARCH                             (0x58)
#define SY_CMD_BAUDTATE_TEST				  		   (0xd0) //波特率测试
#define SY_CMD_SET_FREQUENCY_CAILBRATION			   (0xd1) //主频校准参数
#define SY_CMD_GET_FREQUENCY_CAILBRATION			   (0xd2) //获取主频校准参数
#define SY_CMD_GET_BG                                  (0xe0)
#define SY_CMD_SET_BG                                  (0xe1)
#define SY_CMD_HOST_GET_BG                             (0xe3)
#define SY_CMD_GET_VERSION                             (0xf0) //读版本号
#define SY_CMD_SET_ML_SN                               (0xf1) //设置SN号
#define SY_CMD_GET_ML_SN                               (0xf2) //获取SN号
#define SY_CMD_SET_BAUDRATE                            (0xf3) //设置波特率
#define SY_CMD_TEST_CAPTURE                            (0xf4) //测试用采图命令
#define SY_CMD_TEST_UPLOAD_IMAGE                       (0xf5) //测试用上传图像
#define SY_CMD_UPLOAD_SRAM_FTR                         (0xf6) //上传FTR
#define SY_CMD_UPLOAD_FLASH_INFO                       (0xf7) //上传Flash_Info
#define SY_CMD_FP_DOWNLOAD_START                       (0xFE)
#define SY_CMD_FP_DELAY_REBOOT                         (0xFF)
#define SY_CMD_FP_ADMIN_SLEEP                          (0xAA)
#define SY_CMD_SET_LED_CTRL_INFO                       (0x3C)



//SY确认码定义
#define SY_COMP_CODE_OK                                (0x00) // 00H：表示指令执行完毕或OK； 
#define SY_COMP_CODE_DATA_ERROR                        (0x01) // 01H：表示数据包接收错误； 
#define SY_COMP_CODE_NO_FINGER_DETECT                  (0x02) // 02H：表示传感器上没有手指； 
#define SY_COMP_CODE_CAPTURE_ERROR                     (0x03) // 03H：表示录入指纹图像失败； 
#define SY_COMP_CODE_FINGER_DRY_GENERATE_FTR_ERROR     (0x04) // 04H：表示指纹图像太干、太淡而生不成特征； 
#define SY_COMP_CODE_FINGER_WET_GENERATE_FTR_ERROR     (0x05) // 05H：表示指纹图像太湿、太糊而生不成特征； 
#define SY_COMP_CODE_UNQUALIFIED_IMAGE_ERROR           (0x06) // 06H：表示指纹图像太乱而生不成特征； 
#define SY_COMP_CODE_LACK_FTR_GENERATE_FTR_ERROR       (0x07) // 07H：表示指纹图像正常，但特征点太少（或面积太小）而生不成特征； 
#define SY_COMP_CODE_FTR_MATCH_ERROR                   (0x08) // 08H：表示指纹不匹配； 
#define SY_COMP_CODE_SEARCH_FTR_NOT_MATCH              (0x09) // 09H：表示没搜索到指纹； 
#define SY_COMP_CODE_MERGE_FTR_ERROR                   (0x0A) // 0aH：表示特征合并失败；
#define SY_COMP_CODE_FP_INDEX_OVERFLOW                 (0x0B) // 0bH：表示访问指纹库时地址序号超出指纹库范围； 
#define SY_COMP_CODE_READ_FTR_ERROR                    (0x0C) // 0cH：表示从指纹库读模板出错或无效； 
#define SY_COMP_CODE_UPLOAD_FTR_ERROR                  (0x0D) // 0dH：表示上传特征失败； 
#define SY_COMP_CODE_RECIEVE_LATER_PACKAGE_ERROR       (0x0E) // 0eH：表示模块不能接收后续数据包； 
#define SY_COMP_CODE_UPLOAD_IMAGE_ERROR                (0x0F) // 0fH：表示上传图像失败； 
#define SY_COMP_CODE_DELETE_FTR_ERROR                  (0x10) // 10H：表示删除模板失败； 
#define SY_COMP_CODE_CLEAR_FTR_ERROR                   (0x11) // 11H：表示清空指纹库失败；
#define SY_COMP_CODE_ENTER_SLEEP_MODE_ERROR            (0x12) // 12H：表示不能进入低功耗状态；
#define SY_COMP_CODE_PASSWORD_INVALID                  (0x13) // 13H：表示口令不正确； 
#define SY_COMP_CODE_REBOOT_ERROR                      (0x14) // 14H：表示系统复位失败； 
#define SY_COMP_CODE_NO_IMAGE                          (0x15) // 15H：表示缓冲区内没有有效原始图而生不成图像； 
#define SY_COMP_CODE_ONLINE_UPDATE_ERROR               (0x16) // 16H：表示在线升级失败； 
#define SY_COMP_CODE_DO_NOT_MOVE_FINGER                (0x17) // 17H：表示残留指纹或两次采集之间手指没有移动过； 
#define SY_COMP_CODE_READ_WRITE_FLASH_ERROR            (0x18) // 18H：表示读写FLASH出错； 
#define SY_COMP_CODE_GENERATE_RANDOM_ERROR             (0x19) // 19H：随机数生成失败； 
#define SY_COMP_CODE_INVALID_REG_ADDR                  (0x1A) // 1aH：无效寄存器号； 
#define SY_COMP_CODE_REG_VALUE_ERRPR                   (0x1B) // 1bH：寄存器设定内容错误号； 
#define SY_COMP_CODE_NOTEBOOK_PAGE_NUM_ERROR           (0x1C) // 1cH：记事本页码指定错误； 
#define SY_COMP_CODE_PORT_OPRATION_ERROR               (0x1D) // 1dH：端口操作失败； 
#define SY_COMP_CODE_AUTO_ENROLL_ERROR                 (0x1E) // 1eH：自动注册（enroll）失败； 
#define SY_COMP_CODE_STORAGE_IS_FULL                   (0x1F) // 1fH：指纹库满； 
#define SY_COMP_CODE_DEVICE_ADDR_ERROR                 (0x20) // 20H：设备地址错误； 
#define SY_COMP_CODE_PASSWORD_ERROR                    (0x21) // 21H：密码有误； 
#define SY_COMP_CODE_FP_BUFFER_NOT_EMPTY               (0x22) // 22H：指纹模板非空； 
#define SY_COMP_CODE_FP_BUFFER_EMPTY                   (0x23) // 23H：指纹模板为空；
#define SY_COMP_CODE_STORAGE_SPACE_EMPTY               (0x24) // 24H：指纹库为空；
#define SY_COMP_CODE_FP_EXIST_AUTO                     (0x24) // 24H：指纹已存在(autoLogin用)；

#define SY_COMP_CODE_ENROLL_NUM_ERROR                  (0x25) // 25H：录入次数设置错误； 
#define SY_COMP_CODE_TIMEOUT                           (0X26) // 26H：超时； 
#define SY_COMP_CODE_FP_EXIST                          (0x27) // 27H：指纹已存在； 
#define SY_COMP_CODE_FP_ASSOCIAT                       (0x28) // 28H：指纹模板有关联； 
#define SY_COMP_CODE_SENSOR_INITIAL_ERROR              (0x29) // 29H：传感器初始化失败；
#define SY_COMP_CODE_SN_VALID                          (0x2A) // 2AH：模组唯一序列号非空；
#define SY_COMP_CODE_SN_INVALID                        (0x2B) // 2BH：模组唯一序列号为空； 
#define SY_COMP_CODE_OTP_ERROR                         (0x2C) // 2CH：OTP操作失败;
#define SY_COMP_CODE_ECHO							   (0x55)
#define SY_COMP_CODE_FIRST_ENROLL_OK                   (0x56) // 56H:    自动注册第一次采集成功;
#define SY_COMP_CODE_SECOND_ENROLL_OK                  (0x57) // 57H:    自动注册第二次采集成功;
#define SY_COMP_CODE_THIRD_ENROLL_OK                   (0x58) // 58H:    自动注册第三次采集成功;
#define SY_COMP_CODE_LATER_PACKAGE_RECIEVE_OK          (0xF0) // f0H：有后续数据包的指令，正确接收后用0xf0应答； 
#define SY_COMP_CODE_LATER_CMD_PACKAGE_OK              (0xF1) // f1H：有后续数据包的指令，命令包用0xf1应答； 
#define SY_COMP_CODE_FLASH_CHECKSUM_ERROR              (0xF2) // f2H：表示烧写内部FLASH时，校验和错误； 
#define SY_COMP_CODE_FLASH_PACKAGE_FLAG_ERROR          (0xF3) // f3H：表示烧写内部FLASH时，包标识错误； 
#define SY_COMP_CODE_FLASH_PACKAGE_LEN_ERROR           (0xF4) // f4H：表示烧写内部FLASH时，包长度错误； 
#define SY_COMP_CODE_FLASH_CODE_SIZE_ERROR             (0xF5) // f5H：表示烧写内部FLASH时，代码长度太长； 
#define SY_COMP_CODE_FLASH_WRITE_FAIL                  (0xF6) // f6H：表示烧写内部FLASH时，烧写FLASH失败；
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


//晟元寄存器号
#define BAUDRATE_REG        (4)
#define MATCH_LEVEL_REG     (5)
#define PACKET_LENGTH_REG   (6)

typedef enum
{
	SY_LED_MODE_NONE = 0,
	SY_LED_TYPE_BREATH = 1,	 //呼吸灯
	SY_LED_TYPE_BLINK = 2,	//闪烁灯
	SY_LED_TYPE_ON = 3,		//常亮
	SY_LED_TYPE_OFF = 4,	//常闭
	SY_LED_TYPE_BRE_ON = 5,	//渐开灯
	SY_LED_TYPE_BRE_OFF = 6, //渐关灯
}SY_LED_MODE_E;

typedef struct{
	unsigned int address;
    unsigned char flag;
    unsigned short len;
    unsigned char cmd;
    unsigned char req[50];    //req数据
}SY_CMD_REQ_DATA, *PSY_CMD_REQ_DATA;

typedef struct{
	unsigned int address;
    unsigned char flag;
    unsigned short len;
    unsigned char comp_code;
    unsigned char *resp;      //resp数据所在
}SY_CMD_RESP_DATA, *PSY_CMD_RESP_DATA;

void SY_COMMAND_TransSetReqData(PSY_CMD_REQ_DATA pReq, unsigned char data, unsigned int index);
unsigned char SY_COMMAND_TransGetRespData(PSY_CMD_RESP_DATA pResp, unsigned char *pdata, unsigned int index);
void SY_COMMAND_ReqProc(PSY_CMD_REQ_DATA pReq);

void SY_COMMAND_Task(void);

#endif


