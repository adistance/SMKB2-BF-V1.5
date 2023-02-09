#include "system_setting.h"
#include "driver_delay.h"
#include "trace.h"
#include "driver_led.h"

#if 1
#define SYSTEM_PRINT_INFO0   APP_PRINT_TRACE0
#define SYSTEM_PRINT_INFO1   APP_PRINT_TRACE1
#define SYSTEM_PRINT_INFO2   APP_PRINT_TRACE2
#define SYSTEM_PRINT_INFO3   APP_PRINT_TRACE3
#define SYSTEM_PRINT_INFO4   APP_PRINT_TRACE4
#else
#define SYSTEM_PRINT_INFO0(...)
#define SYSTEM_PRINT_INFO1(...)
#define SYSTEM_PRINT_INFO2(...)
#define SYSTEM_PRINT_INFO3(...)
#define SYSTEM_PRINT_INFO4(...)
#endif

#define SETTING_STORAGE_OK				(0)
#define SETTING_STORAGE_CHK_ERR 		(1)
#define SETTING_STORAGE_NULL			(2)

SYS_SETTING g_sysSetting;

/*****************************************************************************
 �� �� ��  : SYSSET_GetChksum
 ��������  : У����� 
 �������  : uint8_t *p    
             uint32_t num  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2016��5��13��
    ��    ��   : jack
    �޸�����   : �����ɺ���

*****************************************************************************/
uint8_t SYSSET_GetChksum(uint8_t *p, uint32_t num)
{
    uint32_t i;
    uint8_t sum = 0;
    
    for (i = 0; i < num; i++)
    {
        sum += p[i];
    }

    return sum;
}

/*****************************************************************************
 �� �� ��  : SYSSET_SyncToMaster
 ��������  : ͬ�����ݵ�flash�е�����
 �������  : void  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2017��1��13��
    ��    ��   : li dong
    �޸�����   : �����ɺ���

*****************************************************************************/
void SYSSET_SyncToMaster(void)
{
    uint8_t chksum;
    
    chksum = SYSSET_GetChksum((uint8_t *)&g_sysSetting, sizeof(SYS_SETTING) - 1);
    g_sysSetting.chksum = 0 - chksum;

    //ͬ��������
    //ftl_save((uint8_t*)&g_sysSetting, SYS_INIT_PARA_ADDRESS, sizeof(SYS_SETTING));
    flashWriteBuffer((uint8_t*)&g_sysSetting, SYS_INIT_PARA_ADDRESS, sizeof(SYS_SETTING), EM_FLASH_CTRL_FTL);
}

/*****************************************************************************
 �� �� ��  : SYSSET_SyncToSlave
 ��������  : ͬ����flash�еı�����
 �������  : void  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2017��1��13��
    ��    ��   : li dong
    �޸�����   : �����ɺ���

*****************************************************************************/
void SYSSET_SyncToSlave(void)
{
    uint8_t chksum;
    
    SYSTEM_PRINT_INFO0("SYSSET_SyncToSlave");
    
    chksum = SYSSET_GetChksum((uint8_t *)&g_sysSetting, sizeof(SYS_SETTING) - 1);
    g_sysSetting.chksum = 0 - chksum;

    //ͬ����������
    //ftl_save((uint8_t*)&g_sysSetting, SYS_INIT_PARA_BACK_ADDRESS, sizeof(SYS_SETTING));
    flashWriteBuffer((uint8_t*)&g_sysSetting, SYS_INIT_PARA_BACK_ADDRESS, sizeof(SYS_SETTING), EM_FLASH_CTRL_FTL);
}

/*****************************************************************************
 �� �� ��  : SYSSET_SaveToFlash
 ��������  : ���������flash
 �������  : void  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2016��5��13��
    ��    ��   : jack
    �޸�����   : �����ɺ���

*****************************************************************************/
void SYSSET_SyncToFlash(void)
{
    uint8_t chksum;
    
    SYSTEM_PRINT_INFO0("SYSSET_Sync To Master and Slave");
    
    chksum = SYSSET_GetChksum((uint8_t *)&g_sysSetting, sizeof(SYS_SETTING) - 1);
    g_sysSetting.chksum = 0 - chksum;
    SYSTEM_PRINT_INFO2("len %d, data is %b", sizeof(g_sysSetting), TRACE_BINARY(sizeof(g_sysSetting), &g_sysSetting));
	//ftl_save((uint8_t*)&g_sysSetting, SYS_INIT_PARA_ADDRESS, sizeof(SYS_SETTING));
	//ftl_save((uint8_t*)&g_sysSetting, SYS_INIT_PARA_BACK_ADDRESS, sizeof(SYS_SETTING));
    flashWriteBuffer((uint8_t*)&g_sysSetting, SYS_INIT_PARA_ADDRESS, sizeof(SYS_SETTING), EM_FLASH_CTRL_FTL);
    flashWriteBuffer((uint8_t*)&g_sysSetting, SYS_INIT_PARA_BACK_ADDRESS, sizeof(SYS_SETTING), EM_FLASH_CTRL_FTL);
}

/*****************************************************************************
 �� �� ��  : SYSSET_SetEnrollNum
 ��������  : �����㷨ע�����
 �������  : uint32_t unEnrollNum  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2017��11��7��
    ��    ��   : li dong
    �޸�����   : �����ɺ���

*****************************************************************************/
uint8_t SYSSET_SetEnrollNum(uint32_t unEnrollNum)
{
    if (g_sysSetting.alg_enroll_num != unEnrollNum)
    {
        g_sysSetting.alg_enroll_num = unEnrollNum;
        g_sysSetting.alg_para_update = CONTENT_USE_UPDATE;
        
        SYSSET_SyncToFlash();
    }

    return 0;
}

/*****************************************************************************
 �� �� ��  : SYSSET_GetEnrollNum
 ��������  : ��ȡϵͳ�����е���ע�����
 �������  : �� 
 �������  : ��
 �� �� ֵ  :g_sysSetting.alg_enroll_num 
 ���ú���  : 
 ��������  : 

*****************************************************************************/
uint8_t SYSSET_GetEnrollNum(void)
{
	return g_sysSetting.alg_enroll_num;
}


/*****************************************************************************
 �� �� ��  : SYSSET_SetBoardId
 ��������  : ���ð����������ã�������ӿ���ר��дSN�ŵ�
 �������  : uint8_t *p    
             uint32_t len  
 �������  : uint8_t
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2016��5��13��
    ��    ��   : jack
    �޸�����   : �����ɺ���

*****************************************************************************/
uint8_t SYSSET_SetBoardId(uint8_t *p, uint32_t len)
{
    len = (len > 16) ? 16 : len;

    memcpy(&g_sysSetting.board_id[16], p, len);
	
    g_sysSetting.board_id_update = CONTENT_USE_UPDATE;

    SYSSET_SyncToFlash();
    
    return 0;
}

/*****************************************************************************
 �� �� ��  : SYSSET_SetSystemPolicy
 ��������  : ����ϵͳ����
 �������  : uint32_t policy  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2017��4��5��
    ��    ��   : li dong
    �޸�����   : �����ɺ���

*****************************************************************************/
uint8_t SYSSET_SetSystemPolicy(uint32_t policy)
{
	if(policy & 0x40)
	{
		ledAutoCtrl(AUTO_LED_BACKLIGHRT);
	}
	else
	{
		ledAutoCtrl(AUTO_LED_CLOSE);
	}

    if (g_sysSetting.sys_policy != policy)
    {
    	g_sysSetting.sys_policy = policy;       
        g_sysSetting.sys_policy_update = CONTENT_USE_UPDATE;
        
        SYSSET_SyncToFlash();
		
    }
	SYSTEM_PRINT_INFO1("g_sysSetting.sys_policy is 0x%x", g_sysSetting.sys_policy );
	
    return 0;
}

uint8_t SYSSET_GetSystemPolicy(void)
{
	return g_sysSetting.sys_policy;
}

/*****************************************************************************
 �� �� ��  : SYSSET_SetUartBaudRate
 ��������  : ���ô���ͨ�Ų�����
 �������  : uint32_t baudrate  
 �������  : uint8_t
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2016��5��13��
    ��    ��   : jack
    �޸�����   : �����ɺ���

*****************************************************************************/
uint8_t SYSSET_SetUartBaudRate(uint32_t baudrate)
{
    if((baudrate != UART_BAUD_RATE_9600)
        &&(baudrate != UART_BAUD_RATE_19200)
        &&(baudrate != UART_BAUD_RATE_38400)
        &&(baudrate != UART_BAUD_RATE_57600)
        &&(baudrate != UART_BAUD_RATE_115200))
    {
        return 1;
    }

    g_sysSetting.uart_baudrate = baudrate;
    g_sysSetting.uart_baudrate_update = CONTENT_USE_UPDATE;

    SYSSET_SyncToFlash();
    
    return 0;
}

unsigned char SYSSET_SetChipAddress(const unsigned int address)
{
    g_sysSetting.chip_address = address;
    
    SYSSET_SyncToFlash();
    
    return 0;
}

/*****************************************************************************
 �� �� ��  : SYSSET_SetProtoPwd
 ��������  : ����ͨ��Э��������
 �������  : uint32_t pwd  
 �������  : uint8_t
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2016��5��13��
    ��    ��   : jack
    �޸�����   : �����ɺ���

*****************************************************************************/
uint8_t SYSSET_SetProtoPwd(uint32_t pwd)
{
    g_sysSetting.pwd = pwd;
    g_sysSetting.pwd_update = CONTENT_USE_UPDATE;
    
    SYSSET_SyncToFlash();
    
    return 0;
}


/*****************************************************************************
 �� �� ��  : SYSSET_SetBoardSerialNumber
 ��������  : ���ð�Ψһ���кţ������ã�
 �������  : uint8_t *p    
             uint32_t len  
 �������  : uint8_t
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2019��3��
    ��    ��   : phager
    �޸�����   : �����ɺ���

*****************************************************************************/
uint8_t SYSSET_SetBoardSerialNumber(uint8_t *p, uint32_t len)
{
    len = (len > 24) ? 24 : len;

    memcpy(&g_sysSetting.board_serial_number[0], p, len);

    SYSSET_SyncToFlash();
    
    return 0;
}

/*****************************************************************************
 �� �� ��  : SYSSET_GetBoardSerialNumber
 ��������  : ��ȡboard_serial_unmber�е�product_id��roorkey
 �������  : uint8_t *out  			�����ȡ������    
           eSerialType type   	��ȡ�ĸ�����
 �������  : 

*****************************************************************************/
uint8_t SYSSET_GetBoardSerialNumber(uint8_t *out, eSerialType type)
{
	if(out == NULL || type >= E_MAX)
	{
		SYSTEM_PRINT_INFO0("SYSSET_GetBoardSerialNumber input param error");
		return 0;
	}

	if(type == E_PRODUCT_id)
	{
		memcpy(out, &g_sysSetting.board_serial_number[2], 6);
	}
	else if(type == E_ROOTKEY)
	{
		memcpy(out, &g_sysSetting.board_serial_number[8], 16);
	}	

	return 0;
}


uint8_t SYSSET_UpdateSystemPara(unsigned char *p)
{
    uint8_t pre_sysSetting[24] = {0};
    uint8_t pre_board_id[16] = {0};
    
    memcpy(pre_sysSetting, g_sysSetting.board_serial_number , 24);//protect 24Byte NXP SN
    memcpy(pre_board_id, &g_sysSetting.board_id[16], 16);//protect 16Byte Board SN 
    
    
    memcpy(&g_sysSetting , p , sizeof(SYS_SETTING));

	g_sysSetting.frequency_cailbration_value = 0;   //((RCU_CTL >> 3) & 0x1f);
    memcpy(g_sysSetting.board_serial_number, pre_sysSetting , 24); 
    memcpy(&g_sysSetting.board_id[16], pre_board_id, 16);
    
    SYSSET_SyncToFlash();

	return 0;
}

/*****************************************************************************
 �� �� ��  : SYSSET_DefaultValue
 ��������  : ��ʼ��ȫ�ֲ���
 �������  : PSYS_SETTING pSys  
             uint8_t stoStat    
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2016��5��13��
    ��    ��   : jack
    �޸�����   : �����ɺ���

*****************************************************************************/
void SYSSET_DefaultValue(PSYS_SETTING pSys, uint8_t stoStat)
{
    uint8_t *p = (uint8_t *)pSys;
    uint8_t chksum;

    if (stoStat != SETTING_STORAGE_OK)
    {
        pSys->head[0] = 'f';
        pSys->head[1] = 'p';
        pSys->head[2] = 'm';
        pSys->head[3] = ' ';

        pSys->ver[0] = 1;
        pSys->ver[1] = 0;
		pSys->ver[2] = 0;
        pSys->ver[3] = 0;
    }

    if (SETTING_USE_DEFAULT(stoStat, pSys->board_id_update))
    {
        pSys->board_id_update = CONTENT_USE_DEFAULT;
        memcpy(&pSys->board_id[0], DEFAULT_BOARD_ID, strlen(DEFAULT_BOARD_ID));
        memcpy(&pSys->board_id[16], DEFAULT_BOARD_ID, strlen(DEFAULT_BOARD_ID));
    }

    if (SETTING_USE_DEFAULT(stoStat, pSys->pwd_update))
    {
        pSys->pwd_update = CONTENT_USE_DEFAULT;
        pSys->pwd = DEFAULT_PROTO_PWD;
    }

    if (SETTING_USE_DEFAULT(stoStat, pSys->uart_baudrate_update))
    {
        pSys->uart_baudrate_update = CONTENT_USE_DEFAULT;
        pSys->uart_baudrate = DEFAULT_BAUD_RATE;
        pSys->loader_uart_baudrate = DEFAULT_LOADER_BAUD_RATE;
    }

    if (SETTING_USE_DEFAULT(stoStat, pSys->fp_score_update))
    {
        pSys->fp_score_update = CONTENT_USE_DEFAULT;
        pSys->fp_score = DEFAULT_MATCH_THRESHOLD;
        pSys->fp_IsSingleScore = DEFAULT_MERGE_THRESHOLD;
    }

    if (SETTING_USE_DEFAULT(stoStat, pSys->fp_gain_update))
    {
        pSys->fp_gain_update = CONTENT_USE_DEFAULT;
        pSys->fp_shift = DEFAULT_FPSEN_SHIFT;
        pSys->fp_gain = DEFAULT_FPSEN_GAIN;
        pSys->fp_pxlctrl = DEFAULT_FPSEN_PXLCTRL;

        pSys->fp_detect_shift = ICN7000_DETECT_FP_ADC_SHIFT;
        pSys->fp_detect_gain = ICN7000_DETECT_FP_ADC_GAIN;
        pSys->fp_detect_pxlctrl = ICN7000_DETECT_FP_ADC_PXLCTRL;
    }

    if (SETTING_USE_DEFAULT(stoStat, pSys->alg_para_update))
    {
        pSys->alg_para_update = CONTENT_USE_DEFAULT;
        pSys->alg_coverage_rate = DEFAULT_ALG_COVERAGE_RATE;
        pSys->alg_quality_level = DEFAULT_ALG_QULITY_LEVEL;
        pSys->alg_enroll_num = DEFAULT_ALG_ENROLL_NUM;
    }

    if (SETTING_USE_DEFAULT(stoStat, pSys->sys_policy_update))
    {
        pSys->sys_policy_update = CONTENT_USE_DEFAULT;
        pSys->sys_policy = DEFAULT_SYSTEM_POLICY;
    }

	if (SETTING_USE_DEFAULT(stoStat, pSys->chip_address_update))
	{
		pSys->chip_address_update = CONTENT_USE_DEFAULT;
		pSys->chip_address = DEFAULT_SY_CHIP_ADDRESS;
	}

	if (SETTING_USE_DEFAULT(stoStat, pSys->frequency_cailbration_value_update))
	{
		pSys->frequency_cailbration_value_update = CONTENT_USE_DEFAULT;
		pSys->frequency_cailbration_value = DEFAULT_FREQUENCY_CAILBRATION_VALUE;
	}

	if (SETTING_USE_DEFAULT(stoStat, pSys->pin_value_update))
	{
		pSys->pin_value_update = CONTENT_USE_DEFAULT;
		pSys->pin_value_h = DEFAULT_PIN_VALUE;
		pSys->pin_value_l = DEFAULT_PIN_VALUE;
		pSys->pin_error_times = DEFAULT_PIN_ERROR_TIMES;
	}
    
	if (SETTING_USE_DEFAULT(stoStat, pSys->board_serial_number_update))
	{
		pSys->board_serial_number_update = CONTENT_USE_DEFAULT; 

		memset(pSys->board_serial_number, 0, sizeof(pSys->board_serial_number));
	}

	if (SETTING_USE_DEFAULT(stoStat, pSys->tz_password_update))
	{
		pSys->tz_password_update = CONTENT_USE_DEFAULT;
        
		pSys->tz_password = 0x00;
	}
    
	if (SETTING_USE_DEFAULT(stoStat, pSys->YD_rootkey_update))
	{
		pSys->YD_rootkey_update = CONTENT_USE_DEFAULT;
                
		pSys->YD_rootkey[0] = 0x00;
		pSys->YD_rootkey[1] = 0x00;
		pSys->YD_rootkey[2] = 0x00;
		pSys->YD_rootkey[3] = 0x00;
	}

	if (SETTING_USE_DEFAULT(stoStat, pSys->StartUpCheck_update))
	{
        pSys->StartUpCheck[0] = 'S';
        pSys->StartUpCheck[1] = 'T';
        pSys->StartUpCheck[2] = 'A';
        pSys->StartUpCheck[3] = 'R';
	}

    chksum = SYSSET_GetChksum(p, sizeof(SYS_SETTING) - 1);
    pSys->chksum = 0 - chksum;
}


/*****************************************************************************
 �� �� ��  : SYSSET_Init
 ��������  : ϵͳ�������ó�ʼ��
 �������  : void  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2016��5��13��
    ��    ��   : jack
    �޸�����   : �����ɺ���

*****************************************************************************/
void SYSSET_Init(void)
{
    uint8_t *p = (uint8_t*)&g_sysSetting;
    SYS_SETTING stSysPara = {0};
    uint8_t chksum;
    uint8_t cnt;
    uint8_t stoStatMaster = SETTING_STORAGE_NULL;
    uint8_t stoStatSlave = SETTING_STORAGE_NULL;
    //��ȡ��������ೢ����3��
    for (cnt = 0; cnt < 3; cnt++)
    {
    	//ftl_load(p, SYS_INIT_PARA_ADDRESS, sizeof(SYS_SETTING));
        flashReadBuffer(p, SYS_INIT_PARA_ADDRESS, sizeof(SYS_SETTING), EM_FLASH_CTRL_FTL);            //read
        if (('f' == g_sysSetting.head[0])
            &&('p' == g_sysSetting.head[1])
            &&('m' == g_sysSetting.head[2])
            &&(' ' == g_sysSetting.head[3]))
        {
            chksum = SYSSET_GetChksum(p, sizeof(SYS_SETTING));
            if (0x00 == chksum)
            {
            	SYSTEM_PRINT_INFO0("chksum is OK1");
                stoStatMaster = SETTING_STORAGE_OK;
                break;
            }
            else
            {
                SYSTEM_PRINT_INFO1("MASTER SYSSET_Init chksum error, chksum = 0x%02x.\r\n", chksum);
                delay_ms(10);
                memset(p, 0xff, sizeof(SYS_SETTING));
                stoStatMaster = SETTING_STORAGE_CHK_ERR;
            }
        }
		else
		{
			delay_ms(100);
		}
    }

    //��ȡ����������ೢ����3��
    for (cnt = 0; cnt < 3; cnt++)
    {
    	//ftl_load((uint8_t*)&stSysPara, SYS_INIT_PARA_BACK_ADDRESS, sizeof(SYS_SETTING));
        flashReadBuffer((uint8_t*)&stSysPara, SYS_INIT_PARA_BACK_ADDRESS, sizeof(SYS_SETTING), EM_FLASH_CTRL_FTL);
        if (('f' == stSysPara.head[0])
            &&('p' == stSysPara.head[1])
            &&('m' == stSysPara.head[2])
            &&(' ' == stSysPara.head[3]))
        {
            chksum = SYSSET_GetChksum((uint8_t*)&stSysPara, sizeof(SYS_SETTING));
            if (0x00 == chksum)
            {
            	SYSTEM_PRINT_INFO0("chksum is OK2");
                stoStatSlave= SETTING_STORAGE_OK;
                break;
            }
            else
            {
                SYSTEM_PRINT_INFO1("SLAVE SYSSET_Init chksum error, chksum = 0x%02x.\r\n", chksum);
                delay_ms(100);
                memset((uint8_t*)&stSysPara, 0xff, sizeof(SYS_SETTING));
                stoStatSlave = SETTING_STORAGE_CHK_ERR;
            }
        }
		else
		{
			delay_ms(100);
		}
    }

    if ((SETTING_STORAGE_OK == stoStatMaster) && (SETTING_STORAGE_OK != stoStatSlave))
    {
        //����������Ч��������������Ч������������ͬ����������
        SYSSET_SyncToSlave();
    }
    else if ((SETTING_STORAGE_OK != stoStatMaster) && (SETTING_STORAGE_OK == stoStatSlave)) 
    {
        //����������Ч��������������Ч��������������ͬ��������
        memcpy(&g_sysSetting, &stSysPara, sizeof(SYS_SETTING));
        SYSSET_SyncToMaster();
    }
    else if ((SETTING_STORAGE_OK != stoStatMaster) && (SETTING_STORAGE_OK != stoStatSlave))
    {
        //�����ͱ�������ʧЧ������Ĭ�ϲ���
        SYSSET_DefaultValue(&g_sysSetting, stoStatMaster);
		SYSTEM_PRINT_INFO1("Default Enroll Num:%d \r\n" , g_sysSetting.alg_enroll_num);
        SYSSET_SyncToFlash();
    }
    else
    {
        //�����ͱ��������ݶ���Ч
    }

	//��flash����İ汾�ź�DEFAULT_BOARD_ID��һ��ʱ���Ͱ�DEFAULT_BOARD_ID�ĸ��µ�flash����
	//if(memcmp(DEFAULT_BOARD_ID, g_sysSetting.board_id, sizeof(DEFAULT_BOARD_ID)-1))
	{
		memcpy(g_sysSetting.board_id, DEFAULT_BOARD_ID, 16);
	}

	g_sysSetting.alg_enroll_num = DEFAULT_ALG_ENROLL_NUM;
	
    return;
}
