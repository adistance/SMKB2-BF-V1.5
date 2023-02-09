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
 函 数 名  : SYSSET_GetChksum
 功能描述  : 校验计算 
 输入参数  : uint8_t *p    
             uint32_t num  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年5月13日
    作    者   : jack
    修改内容   : 新生成函数

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
 函 数 名  : SYSSET_SyncToMaster
 功能描述  : 同步数据到flash中的主区
 输入参数  : void  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2017年1月13日
    作    者   : li dong
    修改内容   : 新生成函数

*****************************************************************************/
void SYSSET_SyncToMaster(void)
{
    uint8_t chksum;
    
    chksum = SYSSET_GetChksum((uint8_t *)&g_sysSetting, sizeof(SYS_SETTING) - 1);
    g_sysSetting.chksum = 0 - chksum;

    //同步到主区
    //ftl_save((uint8_t*)&g_sysSetting, SYS_INIT_PARA_ADDRESS, sizeof(SYS_SETTING));
    flashWriteBuffer((uint8_t*)&g_sysSetting, SYS_INIT_PARA_ADDRESS, sizeof(SYS_SETTING), EM_FLASH_CTRL_FTL);
}

/*****************************************************************************
 函 数 名  : SYSSET_SyncToSlave
 功能描述  : 同步到flash中的备份区
 输入参数  : void  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2017年1月13日
    作    者   : li dong
    修改内容   : 新生成函数

*****************************************************************************/
void SYSSET_SyncToSlave(void)
{
    uint8_t chksum;
    
    SYSTEM_PRINT_INFO0("SYSSET_SyncToSlave");
    
    chksum = SYSSET_GetChksum((uint8_t *)&g_sysSetting, sizeof(SYS_SETTING) - 1);
    g_sysSetting.chksum = 0 - chksum;

    //同步到备份区
    //ftl_save((uint8_t*)&g_sysSetting, SYS_INIT_PARA_BACK_ADDRESS, sizeof(SYS_SETTING));
    flashWriteBuffer((uint8_t*)&g_sysSetting, SYS_INIT_PARA_BACK_ADDRESS, sizeof(SYS_SETTING), EM_FLASH_CTRL_FTL);
}

/*****************************************************************************
 函 数 名  : SYSSET_SaveToFlash
 功能描述  : 保存参数到flash
 输入参数  : void  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年5月13日
    作    者   : jack
    修改内容   : 新生成函数

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
 函 数 名  : SYSSET_SetEnrollNum
 功能描述  : 设置算法注册次数
 输入参数  : uint32_t unEnrollNum  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2017年11月7日
    作    者   : li dong
    修改内容   : 新生成函数

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
 函 数 名  : SYSSET_GetEnrollNum
 功能描述  : 获取系统参数中的自注册次数
 输入参数  : 无 
 输出参数  : 无
 返 回 值  :g_sysSetting.alg_enroll_num 
 调用函数  : 
 被调函数  : 

*****************************************************************************/
uint8_t SYSSET_GetEnrollNum(void)
{
	return g_sysSetting.alg_enroll_num;
}


/*****************************************************************************
 函 数 名  : SYSSET_SetBoardId
 功能描述  : 设置板名（生产用），这个接口是专门写SN号的
 输入参数  : uint8_t *p    
             uint32_t len  
 输出参数  : uint8_t
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年5月13日
    作    者   : jack
    修改内容   : 新生成函数

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
 函 数 名  : SYSSET_SetSystemPolicy
 功能描述  : 设置系统策略
 输入参数  : uint32_t policy  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2017年4月5日
    作    者   : li dong
    修改内容   : 新生成函数

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
 函 数 名  : SYSSET_SetUartBaudRate
 功能描述  : 设置串口通信波特率
 输入参数  : uint32_t baudrate  
 输出参数  : uint8_t
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年5月13日
    作    者   : jack
    修改内容   : 新生成函数

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
 函 数 名  : SYSSET_SetProtoPwd
 功能描述  : 设置通信协议密码字
 输入参数  : uint32_t pwd  
 输出参数  : uint8_t
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年5月13日
    作    者   : jack
    修改内容   : 新生成函数

*****************************************************************************/
uint8_t SYSSET_SetProtoPwd(uint32_t pwd)
{
    g_sysSetting.pwd = pwd;
    g_sysSetting.pwd_update = CONTENT_USE_UPDATE;
    
    SYSSET_SyncToFlash();
    
    return 0;
}


/*****************************************************************************
 函 数 名  : SYSSET_SetBoardSerialNumber
 功能描述  : 设置板唯一序列号（生产用）
 输入参数  : uint8_t *p    
             uint32_t len  
 输出参数  : uint8_t
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2019年3月
    作    者   : phager
    修改内容   : 新生成函数

*****************************************************************************/
uint8_t SYSSET_SetBoardSerialNumber(uint8_t *p, uint32_t len)
{
    len = (len > 24) ? 24 : len;

    memcpy(&g_sysSetting.board_serial_number[0], p, len);

    SYSSET_SyncToFlash();
    
    return 0;
}

/*****************************************************************************
 函 数 名  : SYSSET_GetBoardSerialNumber
 功能描述  : 获取board_serial_unmber中的product_id和roorkey
 输入参数  : uint8_t *out  			保存获取额数据    
           eSerialType type   	获取哪个数据
 输出参数  : 

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
 函 数 名  : SYSSET_DefaultValue
 功能描述  : 初始化全局参数
 输入参数  : PSYS_SETTING pSys  
             uint8_t stoStat    
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年5月13日
    作    者   : jack
    修改内容   : 新生成函数

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
 函 数 名  : SYSSET_Init
 功能描述  : 系统参数配置初始化
 输入参数  : void  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年5月13日
    作    者   : jack
    修改内容   : 新生成函数

*****************************************************************************/
void SYSSET_Init(void)
{
    uint8_t *p = (uint8_t*)&g_sysSetting;
    SYS_SETTING stSysPara = {0};
    uint8_t chksum;
    uint8_t cnt;
    uint8_t stoStatMaster = SETTING_STORAGE_NULL;
    uint8_t stoStatSlave = SETTING_STORAGE_NULL;
    //读取主区，最多尝试做3次
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

    //读取备份区，最多尝试做3次
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
        //主区数据有效，备份区数据无效，将主区数据同步到备份区
        SYSSET_SyncToSlave();
    }
    else if ((SETTING_STORAGE_OK != stoStatMaster) && (SETTING_STORAGE_OK == stoStatSlave)) 
    {
        //主区数据无效，备份区数据有效，将备份区数据同步到主区
        memcpy(&g_sysSetting, &stSysPara, sizeof(SYS_SETTING));
        SYSSET_SyncToMaster();
    }
    else if ((SETTING_STORAGE_OK != stoStatMaster) && (SETTING_STORAGE_OK != stoStatSlave))
    {
        //主区和备份区都失效，采用默认参数
        SYSSET_DefaultValue(&g_sysSetting, stoStatMaster);
		SYSTEM_PRINT_INFO1("Default Enroll Num:%d \r\n" , g_sysSetting.alg_enroll_num);
        SYSSET_SyncToFlash();
    }
    else
    {
        //主区和备份区数据都有效
    }

	//当flash里面的版本号和DEFAULT_BOARD_ID不一样时，就把DEFAULT_BOARD_ID的更新到flash里面
	//if(memcmp(DEFAULT_BOARD_ID, g_sysSetting.board_id, sizeof(DEFAULT_BOARD_ID)-1))
	{
		memcpy(g_sysSetting.board_id, DEFAULT_BOARD_ID, 16);
	}

	g_sysSetting.alg_enroll_num = DEFAULT_ALG_ENROLL_NUM;
	
    return;
}
