#include "command_SY.h"
#include "os_sync.h"
#include "os_sched.h"
#include "trace.h"

#include "command.h"
#include "system_setting.h"
#include "filesys.h"
#include "fpsapi.h"
#include "fpsapierrors.h"
#include "action.h"
#include "driver_sensor.h"
#include "fpc_sensor_spi.h"
#include "menu_manage.h"
#include "driver_led.h"

#if 0
#define SY_COMMAND_PRINT_INFO0   APP_PRINT_TRACE0
#define SY_COMMAND_PRINT_INFO1   APP_PRINT_TRACE1
#define SY_COMMAND_PRINT_INFO2   APP_PRINT_TRACE2
#define SY_COMMAND_PRINT_INFO3   APP_PRINT_TRACE3
#define SY_COMMAND_PRINT_INFO4   APP_PRINT_TRACE4
#else
#define SY_COMMAND_PRINT_INFO0(...)
#define SY_COMMAND_PRINT_INFO1(...)
#define SY_COMMAND_PRINT_INFO2(...)
#define SY_COMMAND_PRINT_INFO3(...)
#define SY_COMMAND_PRINT_INFO4(...)
#endif

extern uint8_t g_cmdReqData[258];
extern uint8_t g_cmdRespData[260];
uint8_t g_RespCheckSum[2] = {0};

__align(4) uint8_t SY_TmpBuf[0x2000] = {0};

static SY_CMD_REQ_DATA  g_SY_stMsgReq;
static SY_CMD_RESP_DATA g_stSYCmdResp;
static PSY_CMD_RESP_DATA g_pstSYCmdResp;
static SY_CMD_REQ_DATA g_SY_stReq = {0};      //用于系统主动发送消息
static unsigned int g_stPacketSize = 128;

static uint8_t g_sy_uploadflag = 0;
static uint32_t g_sy_uploadmainlen = 0;
static uint32_t g_sy_uploadoffset = 0;

extern uint8_t g_SYframeHead[2];

/***************task**********************/
static uint32_t g_sy_FtrRecievedLen = 0;
uint32_t g_DownloadFTRLen = 0;
uint16_t g_DownloadFTRIndex = 0;
uint32_t g_UploadFTRLen = 0;
uint16_t g_UploadFTRIndex = 0;


extern uint8_t g_QuitAtOnce;
extern uint8_t g_RunningFlag;  //命令运行标志 
extern void *command_req_sem_handle;
extern void *command_resp_sem_handle;

extern void USART_SendData(uint8_t *data, uint16_t vCount);

void UTRNS_SendSYFrame(PSY_CMD_RESP_DATA pFrame)
{
    unsigned int i;
    unsigned short len;
    unsigned char data;

    //发送头信息
    for (i = 0; i < sizeof(g_SYframeHead); i++)
    {
        USART_SendData(&g_SYframeHead[i], 1);
    }

    for (i = 0; i < SY_CMD_RESP_HEAD_LEN; i++)
    {
        data = 0;
        SY_COMMAND_TransGetRespData(pFrame, &data, i);
        USART_SendData(&data, 1);
    }

    len = pFrame->len - 1;
    for (i = 0; i < len ; i++)
    {
        data = pFrame->resp[i];
        USART_SendData(&data, 1);
    }
	
}

static void SY_COMMAND_CalcChksum(PSY_CMD_RESP_DATA pResp)
{
    unsigned short usSrcChkSum = 0;
    unsigned int ii;
    
    usSrcChkSum = pResp->flag;                                      //包标识
    usSrcChkSum += (pResp->len >> 8) & 0xFF;
    usSrcChkSum += pResp->len & 0xFF;                               //包长度
    usSrcChkSum += pResp->comp_code;                                //确认码

    for (ii = 0; ii < pResp->len - 3; ii++) //除去 1 BYTE完成码和2 BYTE 校验和长度
    {
        usSrcChkSum += pResp->resp[ii];
    }

    //填充校验和(高位在前，低位在后)
    pResp->resp[ii] = (usSrcChkSum & 0xFF00)>> 8;
    pResp->resp[ii+1] = usSrcChkSum & 0x00FF;

    return;
}

static unsigned int SY_COMMAND_RespCmd(PSY_CMD_REQ_DATA pReq, PSY_CMD_RESP_DATA *ppResp, unsigned char comp_code, unsigned char *pdata, unsigned int len , unsigned char flag)
{
    if(NULL == pdata)
    {
        pdata = &g_RespCheckSum[0];
    }

	g_stSYCmdResp.address = g_sysSetting.chip_address;
    g_stSYCmdResp.flag = flag;
    g_stSYCmdResp.len = len + 3;
    g_stSYCmdResp.comp_code = comp_code;
    g_stSYCmdResp.resp = pdata;

    SY_COMMAND_CalcChksum(&g_stSYCmdResp);

    *ppResp = &g_stSYCmdResp;

    return 0;
}

static unsigned int SY_COMMAND_IsChksumOk(PSY_CMD_REQ_DATA pReq)
{
    unsigned short ii = 0;
    unsigned short usSrcChkSum = 0x0000;
    unsigned short usCalChkSum = 0x0000;

    usCalChkSum = pReq->flag;                                       //包标识
    usCalChkSum += (pReq->len >> 8) & 0xFF;                         //数据长度
    usCalChkSum += pReq->len & 0xFF;
    usCalChkSum += pReq->cmd;                                       //命令字

    if(pReq->len > sizeof(g_cmdReqData))
    {
        for (ii = 0; ii < pReq->len-3; ii++)
        {
            usCalChkSum += g_cmdRespData[ii];
        }

        usSrcChkSum = ((g_cmdRespData[ii] * 0x100) + g_cmdRespData[ii+1]);
    }
    else
    {
        for (ii = 0; ii < pReq->len-3; ii++)
        {
            usCalChkSum += g_cmdReqData[ii];
        }

        usSrcChkSum = ((g_cmdReqData[ii] * 0x100) + g_cmdReqData[ii+1]);
    }

    if (usCalChkSum == usSrcChkSum)
    {
        return 0;
    }
    else
    {
        SY_COMMAND_PRINT_INFO2("usSrcChkSum: 0x%x, usCalChkSum: 0x%x\r\n", usSrcChkSum, usCalChkSum);
        return 1;
    }
}

void SY_COMMAND_TransSetReqData(PSY_CMD_REQ_DATA pReq, unsigned char data, unsigned int index)
{    
    switch(index)
    {
    	case 0:
		pReq->address = 0;
		case 1:
		case 2:
		case 3:
		pReq->address |= (data << ((3-index) * 8));
		break;
		
        case 4:
        pReq->flag = data;
        break;

        case 5:
        pReq->len = 0;
        case 6:
        pReq->len |= (data << ((6-index) * 8));
        break;

        case 7:
        pReq->cmd = data;
        break;
            
        default:
        break;
    }
}


unsigned char SY_COMMAND_TransGetRespData(PSY_CMD_RESP_DATA pResp, unsigned char *pdata, unsigned int index)
{
    if((PSY_CMD_RESP_DATA)0 == pResp)
    {
        return 1;
    }

    switch(index)
    {
    	case 0:
		case 1:
		case 2:
		case 3:
		*pdata =((pResp->address >> ((3-index) * 8)) & 0xff); 
		break;

        case 4:
        *pdata = pResp->flag;
        break;

        case 5:
        case 6:
        *pdata =((pResp->len >> ((6-index) * 8)) & 0xff); 
        break;

        case 7:
        *pdata =pResp->comp_code; 
        break;

        default:
        break;
    }
    
    return 0;
}

/*****************************************************************************
 函 数 名  : SY_COMMAND_Post
 功能描述  : 发送请求消息到处理任务
 输入参数  : PCMD_REQ_DATA pReq  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2015年12月24日
    作    者   : jack
    修改内容   : 新生成函数

*****************************************************************************/
uint32_t SY_COMMAND_Post(PSY_CMD_REQ_DATA pReq)
{
    if(g_RunningFlag == false)
    {
        if(os_sem_take(command_resp_sem_handle, 0) == false)
        {
            SY_COMMAND_PRINT_INFO0("resp sem take fail");
            return COMP_CODE_CMD_NOT_FINISHED;
        }
    }
    else
    {
        SY_COMMAND_PRINT_INFO0("g_RunningFlag state is busy");
        return COMP_CODE_CMD_NOT_FINISHED;
    }
     
    T_MENU_MSG io_uart_main_msg;
    memcpy((void *)&g_SY_stMsgReq, (void *)pReq, sizeof(SY_CMD_REQ_DATA));
    io_uart_main_msg.type = MENU_MSG_TYPE_TASK;
    io_uart_main_msg.subtype = IO_MSG_UART_RX;
    if(false == menu_task_msg_send(&io_uart_main_msg))
    {
        APP_PRINT_INFO0("io_uart_background_task_deal send msg to main task fail");
    }
    
    os_sem_give(command_req_sem_handle);
//    
//    T_IO_MSG io_uart_main_msg;
//    io_uart_main_msg.type = IO_MSG_TYPE_UART;
//    io_uart_main_msg.subtype = IO_MSG_UART_RX;
//    if(false == app_send_msg_to_apptask(&io_uart_main_msg))
//    {
//        APP_PRINT_INFO0("io_uart_background_task_deal send msg to main task fail");
//    }
//    
//    //设置请求数据
//    memcpy((void *)&g_SY_stMsgReq, (void *)pReq, sizeof(SY_CMD_REQ_DATA));
//    os_sem_give(command_req_sem_handle);
//    
//    return COMP_CODE_OK;
//    

//    //当有命令正在执行情况下，则需要等待命令执行完成
//    if (false == g_RunningFlag)
//    {
// 
//        OSSemPend(&sem_task_sync, 0, OS_OPT_PEND_NON_BLOCKING, (CPU_TS *)NULL, &err);
//        if (OS_ERR_PEND_WOULD_BLOCK == err)
//        {
//            //返回busy
//            return SY_COMP_CODE_DATA_ERROR;
//        }
//        
//        if (OS_ERR_NONE != err)
//        {
//            //返回未知错误
//            return SY_COMP_CODE_DATA_ERROR;
//        }
//    }
//    else
//    {
//        return SY_COMP_CODE_DATA_ERROR;
//    }



    //返回OK
    return SY_COMP_CODE_OK;
}

/*****************************************************************************
 函 数 名  : PostDelayRebootCommand
 功能描述  : 延时复位命令
 输入参数  : void  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2017年6月5日
    作    者   : li dong
    修改内容   : 新生成函数

*****************************************************************************/
uint32_t SY_PostDelayRebootCommand(void)
{
    memset(&g_SY_stReq, 0, sizeof(SY_CMD_REQ_DATA));

    //构造复位命令发送到命令处理任务
    g_SY_stReq.cmd = SY_CMD_FP_DELAY_REBOOT;
    
    return SY_COMMAND_Post(&g_SY_stReq);
}

uint32_t SY_PostUploadFTRCommand(void)
{
    memset(&g_SY_stReq, 0, sizeof(SY_CMD_REQ_DATA));

    //构造匹配命令发送到命令处理任务
    g_SY_stReq.cmd = SY_CMD_UPLOAD_SRAM_FTR;
    g_SY_stReq.flag = SY_FLAG_HOST_SENT;

    //设置请求数据
    memcpy((void *)&g_SY_stMsgReq, (void *)&g_SY_stReq, sizeof(SY_CMD_REQ_DATA));
    
    T_MENU_MSG io_uart_main_msg;
    io_uart_main_msg.type = MENU_MSG_TYPE_TASK;
    io_uart_main_msg.subtype = IO_MSG_UART_RX;
    if(false == menu_task_msg_send(&io_uart_main_msg))
    {
        APP_PRINT_INFO0("io_uart_background_task_deal send msg to main task fail");
    }
    
    os_sem_give(command_req_sem_handle);

    return 0;
}




static unsigned int SY_COMMAND_ReadSYSN(PSY_CMD_REQ_DATA pReq)
{
	memcpy(g_cmdRespData, &g_sysSetting.board_id[0], 32);

	SY_COMMAND_RespCmd(pReq, &g_pstSYCmdResp , SY_COMP_CODE_OK, g_cmdRespData , 32 ,SY_RESP_FLAG_OK);
    return 0;
}


static unsigned int SY_COMMAND_setBaud(PSY_CMD_REQ_DATA pReq)
{
    unsigned int baud;
    unsigned int comp_code = SY_COMP_CODE_OK;
    int ret;

    baud = (g_cmdReqData[0] << 24)
         | (g_cmdReqData[1] << 16)
         | (g_cmdReqData[2] << 8)
         | (g_cmdReqData[3]);

    ret = SYSSET_SetUartBaudRate(baud);         //暂不支持
    if (0 == ret)
    {
        comp_code = SY_COMP_CODE_OK;
    }
    else
    {
        comp_code = SY_COMP_CODE_DATA_ERROR;
    }

    SY_COMMAND_RespCmd(pReq, &g_pstSYCmdResp , comp_code, g_cmdRespData , 0 ,SY_RESP_FLAG_OK);
    return 0;
}

static unsigned int SY_COMMAND_megerFTR(PSY_CMD_REQ_DATA pReq)
{
    uint8_t comp_code = SY_COMP_CODE_OK;
    
    if(g_stEnrollPara.progress < 16)
    {
        comp_code = SY_COMP_CODE_MERGE_FTR_ERROR;
    }
    
    SY_COMMAND_PRINT_INFO1("SY_COMMAND_megerFTR :%d \r\n", comp_code);
    
    SY_COMMAND_RespCmd(pReq, &g_pstSYCmdResp, comp_code, NULL, 0, SY_RESP_FLAG_OK);
    return 0;
}

static unsigned int SY_COMMAND_readFtrCount(PSY_CMD_REQ_DATA pReq)
{
    int num = 0;
    num = fileSys_getStoreFtrNum();

    g_cmdRespData[0] = ((num & 0xff00) >> 8);
    g_cmdRespData[1] = (num & 0xff);

    SY_COMMAND_RespCmd(pReq, &g_pstSYCmdResp , SY_COMP_CODE_OK, g_cmdRespData , 2 ,SY_RESP_FLAG_OK);
    return 0;
}

static unsigned int SY_COMMAND_setChipAddress(PSY_CMD_REQ_DATA pReq)
{
	unsigned int chip_address;
	
	chip_address = ((g_cmdReqData[0] << 24) | (g_cmdReqData[1] << 16) | (g_cmdReqData[2] << 8) | (g_cmdReqData[3]));

	if(chip_address == g_sysSetting.chip_address)
	{
		SY_COMMAND_RespCmd(pReq, &g_pstSYCmdResp , SY_COMP_CODE_OK, g_cmdRespData , 0 ,RESP_FLAG_OK);
    	return 0;
	}

	SYSSET_SetChipAddress(chip_address);

	SY_COMMAND_RespCmd(pReq, &g_pstSYCmdResp , SY_COMP_CODE_OK, g_cmdRespData , 0 ,RESP_FLAG_OK);
    return 0;
}

static unsigned int SY_COMMAND_readFPIndexTable(PSY_CMD_REQ_DATA pReq)
{
    unsigned char idBuff[32] = {0};
    
    memset(g_cmdRespData, 0 , sizeof(g_cmdRespData));
    
    fileSys_getIdDistribute(idBuff);

    memcpy(&g_cmdRespData[0] , idBuff , 32);

    SY_COMMAND_RespCmd(pReq, &g_pstSYCmdResp ,SY_COMP_CODE_OK, g_cmdRespData , 32 , SY_RESP_FLAG_OK);

	return 0;
}

static unsigned int SY_COMMAND_CancelRegister(PSY_CMD_REQ_DATA pReq)
{
    g_cmdRespData[0] = 0;
    g_cmdRespData[1] = 0;

    MLAPI_AbortCommand();
    
    SY_COMMAND_RespCmd(pReq, &g_pstSYCmdResp , SY_COMP_CODE_OK, g_cmdRespData, 0,  SY_RESP_FLAG_OK);
    
    return 0;

}

static unsigned int SY_COMMAND_ReadNoteBook(PSY_CMD_REQ_DATA pReq)
{
    int ret;
    unsigned char comp_code = SY_COMP_CODE_OK;
    unsigned char sec;

    memset(g_cmdRespData, 0 , sizeof(g_cmdRespData));
    
    sec = g_cmdReqData[0];

    if (sec >= SY_MAX_NOTE)
    {
        comp_code = SY_COMP_CODE_NOTEBOOK_PAGE_NUM_ERROR;
        SY_COMMAND_RespCmd(pReq, &g_pstSYCmdResp , comp_code, g_cmdRespData, 0,  SY_RESP_FLAG_OK);
        return 0;
    }
    
    ret = fileSys_ReadSYNoteBook(sec, g_cmdRespData, SY_NOTE_LEN);
    if(0 != ret)
    {
        comp_code = SY_COMP_CODE_DATA_ERROR;
        SY_COMMAND_RespCmd(pReq, &g_pstSYCmdResp , comp_code, g_cmdRespData, 0,  SY_RESP_FLAG_OK);
        return 0;
    }

    SY_COMMAND_RespCmd(pReq, &g_pstSYCmdResp , comp_code, g_cmdRespData, SY_NOTE_LEN,  SY_RESP_FLAG_OK);
    return 0;
}

static unsigned int SY_COMMAND_WriterRegister(PSY_CMD_REQ_DATA pReq)
{
    uint32_t baudrate_set = 57600;
    
    switch(g_cmdReqData[0])
    {
        case 4:
            baudrate_set = 9600 * g_cmdReqData[1];
            SYSSET_SetUartBaudRate(baudrate_set);
        break;

        case 5:
            //匹配分数门限不处理
        break;

        default:
        break;
    }

    SY_COMMAND_RespCmd(pReq, &g_pstSYCmdResp , SY_COMP_CODE_OK, g_cmdRespData, 0,  SY_RESP_FLAG_OK);
    return 0;

}

static unsigned int SY_COMMAND_readRegiest(PSY_CMD_REQ_DATA pReq)
{
    memset(&g_cmdRespData[0], 0, 16);
    
	//SSR寄存器
	g_cmdRespData[0] = 0x00;
	g_cmdRespData[1] = 0x00;
	//传感器类型
	g_cmdRespData[2] = 0x00;
	g_cmdRespData[3] = 0x11;
	//指纹库容量
	g_cmdRespData[4] = ((STORE_MAX_FTR & 0xff00) >> 8);
    g_cmdRespData[5] = (STORE_MAX_FTR & 0xff);
	//分数等级
	g_cmdRespData[6] = 0x00;
    g_cmdRespData[7] = 0x03;
	//设备地址
    g_cmdRespData[8] = ((g_sysSetting.chip_address & 0xff000000) >> 24);
    g_cmdRespData[9] = ((g_sysSetting.chip_address & 0xff0000) >> 16);
    g_cmdRespData[10] = ((g_sysSetting.chip_address & 0xff00) >> 8);
    g_cmdRespData[11] = (g_sysSetting.chip_address & 0xff);
	//数据包大小
	g_cmdRespData[12] = 0x00;
    switch(g_stPacketSize)
    {
        case 32:
            g_cmdRespData[13] = 0;
            break;
            
        case 64:
            g_cmdRespData[13] = 1;
            break;
            
        case 128:
            g_cmdRespData[13] = 2;
            break;
            
        case 256:
            g_cmdRespData[13] = 3;
            break;

        default:
            g_stPacketSize = 128;
            g_cmdRespData[13] = 2;
            break;
    }

	//波特率9600 * N
	g_cmdRespData[14] = 0x00;
	g_cmdRespData[15] = (g_sysSetting.uart_baudrate / 9600);

    SY_COMMAND_RespCmd(pReq, &g_pstSYCmdResp , SY_COMP_CODE_OK, g_cmdRespData , 16 ,SY_RESP_FLAG_OK);
    return 0;
}

static unsigned int SY_COMMAND_readFlashInfoStart(PSY_CMD_REQ_DATA pReq)
{
    memset(&g_EnrollFtrBuf[0], 0xff, 512);
            
	//SSR寄存器
	g_EnrollFtrBuf[0] = 0x00;
	g_EnrollFtrBuf[1] = 0x00;
	//传感器类型
	g_EnrollFtrBuf[2] = 0x00;
	g_EnrollFtrBuf[3] = 0x00;

	//指纹库容量
	g_EnrollFtrBuf[4] = ((STORE_MAX_FTR & 0xff00) >> 8);
    g_EnrollFtrBuf[5] = (STORE_MAX_FTR & 0xff);
	//分数等级
	g_EnrollFtrBuf[6] = 0x00;
    g_EnrollFtrBuf[7] = 0x03;

	//设备地址
    g_EnrollFtrBuf[8] = ((g_sysSetting.chip_address & 0xff000000) >> 24);
    g_EnrollFtrBuf[9] = ((g_sysSetting.chip_address & 0xff0000) >> 16);
    g_EnrollFtrBuf[10] = ((g_sysSetting.chip_address & 0xff00) >> 8);
    g_EnrollFtrBuf[11] = (g_sysSetting.chip_address & 0xff);
	//数据包大小
	g_EnrollFtrBuf[12] = 0x00;
    
    switch(g_stPacketSize)
    {
        case 32:
            g_EnrollFtrBuf[13] = 0;
            break;
            
        case 64:
            g_EnrollFtrBuf[13] = 1;
            break;
            
        case 128:
            g_EnrollFtrBuf[13] = 2;
            break;
            
        case 256:
            g_EnrollFtrBuf[13] = 3;
            break;

        default:
            g_stPacketSize = 128;
            g_EnrollFtrBuf[13] = 2;
            break;
    }

	//波特率9600 * N
	g_EnrollFtrBuf[14] = 0x00;
	g_EnrollFtrBuf[15] = (g_sysSetting.uart_baudrate / 9600);

    //填0
    memset(&g_EnrollFtrBuf[16], 0x00, 12);
    
    //指纹模组型号
    memcpy(&g_EnrollFtrBuf[28], &g_sysSetting.board_id[3], 6);
    
    //填0
    memset(&g_EnrollFtrBuf[34], 0x00, 2);

    //填固件版本
	g_EnrollFtrBuf[36] = 'M';
	g_EnrollFtrBuf[37] = 'L';
    memcpy(&g_EnrollFtrBuf[38], &g_sysSetting.board_id[10], 6);

    memcpy(&g_EnrollFtrBuf[44],"ML_Techn",8);
    memcpy(&g_EnrollFtrBuf[52],"1180",4);


    g_sy_uploadmainlen = 512;
    g_sy_uploadflag = SY_RESP_FLAG_DATA;
    g_sy_uploadoffset = 0;

    SY_PostUploadFTRCommand();

    return 0;
}

void SY_COMMAND_SetLEDControlInfo(PSY_CMD_REQ_DATA pReq)
{
	unsigned char color = 0;
	unsigned short count = 0;
	EM_LED_CTRL mode = EM_LED_CTRL_OFF;
	
	//APP_PRINT_INFO2("SY_SetLEDInfo1 len:%d, data:%b", pReq->len, TRACE_BINARY(pReq->len-1, pReq->req));

	if(pReq->req[1]  == 0) //起始颜色
	{
		if(pReq->req[2]  == 0) //结束颜色
			color = pReq->req[1]; 
		else 
			color = pReq->req[2];
	}	
	else
	{
		color = pReq->req[1];
	}	
		
	 //循环次数
	count = pReq->req[3];

	switch(color)
	{
		case 0:
			color = EM_LED_NONE;
			break;
		
		case 1:
			color = EM_LED_BLUE;
			break;
			
		case 2:
			color = EM_LED_GREEN;
			break;
			
		case 3:
			color = EM_LED_CYAN;
			break;
			
		case 4:
			color = EM_LED_RED;
			break;
			
		case 5:
			color = EM_LED_PINK;
			break;
			
		case 6:
			color = EM_LED_YELLOW;
			break;
			
		case 7:
			color = EM_LED_WHITE;
			break;
		default:
			APP_PRINT_INFO0("SY_COMMAND_SetLEDControlInfo switch color error");
			break;
	}

	switch(pReq->req[0]) //模式
	{
		case SY_LED_TYPE_BREATH:   //呼吸灯
			if(count == 0) //当为0时，默认无限次
				count = 0xFFFF; 
			else
				count *= 2; //乘2是因为下面亮灭都进行了减一操作
			mode = EM_LED_CTRL_PWM_SY;
			break;
			
		case SY_LED_TYPE_BLINK:   //闪烁灯
			if(count == 0)
				count = 0xFFFF;
			mode = EM_LED_CTRL_BLINK;
			break;
		
		case SY_LED_TYPE_ON:  //常开灯		
			mode = EM_LED_CTRL_ON;
			break;
			
		case SY_LED_TYPE_OFF:   //常闭灯
			mode = EM_LED_CTRL_OFF;
			break;
			
		case SY_LED_TYPE_BRE_ON:   //渐开灯
			count = 1;
			mode = EM_LED_CTRL_PWM_SY;
			count = 2;
			break;

		#if 0
		case SY_LED_TYPE_BRE_OFF:  //渐闭灯
			count = 0;
			if(color == EM_LED_NONE) //加入渐关灯是无色控制，就是用渐开灯的颜色
			{
				color = pre_color;
			}
			mode = EM_LED_CTRL_PWM_SY;
			break;
		#endif
		default:
			APP_PRINT_INFO0("SY_COMMAND_SetLEDControlInfo switch mode error");
			break;
	}

		
	ledModeSet(mode, (EM_LED_COLOR)color, LED_FREQ_MS(200), count);
	
}

void SY_COMMAND_ReqProc(PSY_CMD_REQ_DATA pReq)
{
    unsigned char cmd;
    unsigned char flag;

    flag = pReq->flag;

    if((pReq->address != g_sysSetting.chip_address) || (0 != ( SY_COMMAND_IsChksumOk(pReq))))
    {
    	APP_PRINT_INFO0("SY check sum or chip_address error!\n");
		return;
	}

    cmd = pReq->cmd;
    flag = pReq->flag;

    if(SY_RESP_FLAG_DATA == flag || SY_RESP_FLAG_DATA_END == flag)
    {
        g_QuitAtOnce = 0;
        SY_COMMAND_Post(pReq);
        return;
    }
    
    SY_COMMAND_PRINT_INFO1("Start SY 0x%x \r\n", cmd);  
    
    switch(cmd)
    {
        case SY_CMD_GET_IMAGE:
		case SY_CMD_ENROLL_CAPTURE_IMAGE:
		case SY_CMD_GENERATE_FEATURE:           
        case SY_CMD_SEARCH_FTR:
        case SY_CMD_STORAGE_FTR:   
        case SY_CMD_DELETE_FTR:
        case SY_CMD_CLEAR_FTR:
        case SY_CMD_AUTO_ENROLL:
        case SY_CMD_AUTO_MATCH:
        case SY_CMD_AUTO_SEARCH:
        case SY_CMD_AUTO_IDENTIFY:
        case SY_CMD_AUTO_LOGIN:
        case SY_CMD_WRITE_NOTEBOOK:
        case SY_CMD_MEGER_FTR:
        case SY_CMD_READ_FLASH_FTR_TO_SRAM:
        case SY_CMD_UPLOAD_SRAM_FTR_START:
        case SY_CMD_UPLOAD_SRAM_FTR:
        case SY_CMD_DOWNLOAD_FTR:
    		g_QuitAtOnce = 0;
            SY_COMMAND_Post(pReq);
        	break;

        case SY_CMD_SLEEP_MODE:   
		
            g_QuitAtOnce = 1;
            if (0 != SY_COMMAND_Post(pReq))
            {
                SY_COMMAND_RespCmd(pReq, &g_pstSYCmdResp , SY_COMP_CODE_DATA_ERROR, g_cmdRespData, 0,  SY_RESP_FLAG_OK);
                UTRNS_SendSYFrame(g_pstSYCmdResp); 
            }
        break;

        case SY_CMD_GET_CHIP_SN:
            SY_COMMAND_ReadSYSN(pReq);
            UTRNS_SendSYFrame(g_pstSYCmdResp); 
            break;
            
        case SY_CMD_SET_BAUDRATE:
            SY_COMMAND_setBaud(pReq);
            SY_PostDelayRebootCommand();
            UTRNS_SendSYFrame(g_pstSYCmdResp); 
            break;

        case SY_CMD_FP_ADMIN_SLEEP: 
            SY_COMMAND_megerFTR(pReq);
            UTRNS_SendSYFrame(g_pstSYCmdResp); 
            break;

        case SY_CMD_MATCH_FTR:
            g_cmdRespData[0] = 0x2f;
            g_cmdRespData[1] = 0x07;
            SY_COMMAND_RespCmd(pReq, &g_pstSYCmdResp , SY_COMP_CODE_OK, g_cmdRespData , 2 ,SY_RESP_FLAG_OK);
            UTRNS_SendSYFrame(g_pstSYCmdResp);
            break;

        case SY_CMD_GET_FTR_COUNT:
            SY_COMMAND_readFtrCount(pReq);
            UTRNS_SendSYFrame(g_pstSYCmdResp);
            break;
            
        case SY_CMD_SET_LED_CTRL_INFO:
			SY_COMMAND_SetLEDControlInfo(pReq);
			
            SY_COMMAND_RespCmd(pReq, &g_pstSYCmdResp , SY_COMP_CODE_OK, g_cmdRespData, 0,  SY_RESP_FLAG_OK);
            UTRNS_SendSYFrame(g_pstSYCmdResp); 
        break;    


        case SY_CMD_OF_LED:
#if HARDWARE_POLICY_LED        
            SY_COMMAND_SetOFLEDControlInfo(pReq);
#else
            SY_COMMAND_RespCmd(pReq, &g_pstSYCmdResp , SY_COMP_CODE_OK, g_cmdRespData, 0,  SY_RESP_FLAG_OK);
#endif
            UTRNS_SendSYFrame(g_pstSYCmdResp); 
        break;  



		case SY_CMD_SET_CHIP_ADDRESS:
			SY_COMMAND_setChipAddress(pReq);
			UTRNS_SendSYFrame(g_pstSYCmdResp); 
			break;

        case SY_CMD_READ_FP_INDEX_TABLE:
            SY_COMMAND_readFPIndexTable(pReq);
            UTRNS_SendSYFrame(g_pstSYCmdResp); 
        break;

        case SY_CMD_CANCLE:  
            SY_COMMAND_CancelRegister(pReq);
            UTRNS_SendSYFrame(g_pstSYCmdResp); 
        break;

        case SY_CMD_READ_NOTEBOOK:  
            SY_COMMAND_ReadNoteBook(pReq);
            UTRNS_SendSYFrame(g_pstSYCmdResp); 
        break;
        
        case SY_CMD_HANDSHAKE:  
        case SY_CMD_VERIFY_HANDSHARK_PASSWORD:  
            SY_COMMAND_RespCmd(pReq, &g_pstSYCmdResp , SY_COMP_CODE_OK, g_cmdRespData, 0,  SY_RESP_FLAG_OK);
            UTRNS_SendSYFrame(g_pstSYCmdResp); 
        break;

        case SY_CMD_WRITE_REGIEST:
            SY_COMMAND_WriterRegister(pReq);
            UTRNS_SendSYFrame(g_pstSYCmdResp); 
        break;

        case SY_CMD_READ_SYSTEM_PARA:
            SY_COMMAND_readRegiest(pReq);
            UTRNS_SendSYFrame(g_pstSYCmdResp); 
            break;

        case SY_CMD_READ_INFO_PAGE:
            SY_COMMAND_RespCmd(pReq, &g_pstSYCmdResp , SY_COMP_CODE_OK, g_cmdRespData, 0,  SY_RESP_FLAG_OK);
            UTRNS_SendSYFrame(g_pstSYCmdResp);
            SY_COMMAND_readFlashInfoStart(pReq);
            break;
            
        default:
            SY_COMMAND_RespCmd(pReq, &g_pstSYCmdResp , SY_COMP_CODE_DATA_ERROR, g_cmdRespData , 0 ,SY_RESP_FLAG_OK);
            UTRNS_SendSYFrame(g_pstSYCmdResp); 
            break;
    }
}

static unsigned int SY_COMMAND_downloadFTR(PSY_CMD_REQ_DATA pReq)
{  
    unsigned char flag;
    uint32_t unDataLen = 0;
    uint8_t comp_code = SY_COMP_CODE_OK;
    uint32_t unSrcCRC = 0;
    uint32_t unCRC = 0;
    
    unDataLen = pReq->len - 2;

    flag = pReq->flag;
      
    g_EnrollFtrBuf[g_sy_FtrRecievedLen] = pReq->cmd;
    memcpy(&g_EnrollFtrBuf[g_sy_FtrRecievedLen + 1], &g_cmdReqData[0], unDataLen - 1);
    memset(g_cmdReqData, 0, sizeof(g_cmdReqData));

    g_sy_FtrRecievedLen = g_sy_FtrRecievedLen + unDataLen;

    if (flag == SY_RESP_FLAG_DATA_END)
    {
        unSrcCRC = (g_EnrollFtrBuf[g_sy_FtrRecievedLen-1] << 24)
                      | (g_EnrollFtrBuf[g_sy_FtrRecievedLen-2] << 16)
                      | (g_EnrollFtrBuf[g_sy_FtrRecievedLen-3] << 8)
                      | (g_EnrollFtrBuf[g_sy_FtrRecievedLen-4]);
        
        unCRC = CRC32_calc(g_EnrollFtrBuf, g_sy_FtrRecievedLen-4);

        if (unCRC == unSrcCRC)
        {
            g_stEnrollPara.length = g_sy_FtrRecievedLen - 4;
            g_sy_FtrRecievedLen = 0;
            comp_code = SY_COMP_CODE_OK;
        }
        else
        {
            comp_code = SY_COMP_CODE_DATA_ERROR;
        }

        SY_COMMAND_RespCmd(pReq, &g_pstSYCmdResp , comp_code, g_cmdRespData , 0 ,SY_RESP_FLAG_DATA);
        UTRNS_SendSYFrame(g_pstSYCmdResp); 
    }

    return 0;
}

static unsigned int SY_COMMAND_getImage(PSY_CMD_REQ_DATA pReq)
{
	int ret = 0;
    uint8_t comp_code = 0;
	
    ret = Sensor_WaitAndCapture(100);

    if(SENSOR_COMP_CODE_OK == ret)
    {
        comp_code = SY_COMP_CODE_OK;
    }
    else
    {
        comp_code = SY_COMP_CODE_NO_FINGER_DETECT;
    }
#if 0
    switch(ret)
    {
        case ICN7000_OK:
            comp_code = SY_COMP_CODE_OK;
            break;

        case ICN7000_ERROR_NO_FINGER:
            comp_code = SY_COMP_CODE_NO_FINGER_DETECT;
            break;
        
        case ICN7000_ERROR_ID:
            comp_code = SY_COMP_CODE_ICN7000_ERROR_ID;
        break;
        
        case ICN7000_ERROR_FORCE_QUIT:
            comp_code = SY_COMP_CODE_ICN7000_ERROR_FORCE_QUIT;
        break;
        
        case ICN7000_ERROR_UNQUALIFIED_IMAGE:
            comp_code = SY_COMP_CODE_ICN7000_ERROR_UNQUALIFIED_IMAGE;
        break;
        
        default:
            comp_code = SY_COMP_CODE_CAPTURE_ERROR;
            break;
    }
#endif
    SY_COMMAND_RespCmd(pReq, &g_pstSYCmdResp , comp_code, NULL, 0, SY_RESP_FLAG_OK);
    return 0;
}

/*****************************************************************************
 函 数 名  : action_Extract
 功能描述  : 指纹特征提取
 输入参数  : uint8_t mode  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2015年12月24日
    作    者   : jack
    修改内容   : 新生成函数

*****************************************************************************/
//int32_t action_ImgJudgment(uint8_t *Imgbuff)
//{
//    RawImageInfo_t stRawImgInfo;
//    int32_t rst = 0;

//    stRawImgInfo.pData= Imgbuff;
//    stRawImgInfo.nHeight = IMG_HEIGHT;
//    stRawImgInfo.nWidth = IMG_WIDTH;
//    stRawImgInfo.nBpp = 0;
//    stRawImgInfo.nReserved = 0;

//    rst = IsAvailableImage(&stRawImgInfo);

//    return rst;
//}

static unsigned int SY_COMMAND_getEnrollImage(PSY_CMD_REQ_DATA pReq)
{
	int ret = 0;
    uint8_t comp_code = SY_COMP_CODE_NO_FINGER_DETECT;
	
    ret = Sensor_WaitAndCapture(200);

    if(SENSOR_COMP_CODE_OK == ret)
    {
//        if(0 == action_ImgJudgment(SPI_IMAGE_BUFFER))
        {
            comp_code = SY_COMP_CODE_OK;
        }
        SY_COMMAND_PRINT_INFO0("SY_COMMAND_getEnrollImage finger down\r\n");
    }
    else 
    {
        SY_COMMAND_PRINT_INFO0("SY_COMMAND_getEnrollImage no finger\r\n");
    }

    SY_COMMAND_RespCmd(pReq, &g_pstSYCmdResp , comp_code, NULL, 0, SY_RESP_FLAG_OK);
    return 0;
}

static int SY_COMMAND_getChar(PSY_CMD_REQ_DATA pReq)
{
    int32_t ret;
    uint32_t comp_code = SY_COMP_CODE_OK;
    uint8_t progress = 0;
    uint16_t index;

	
	index = fileSys_getUnuseSmallestIndex();

    if((g_cmdReqData[0] < 0x01) || (g_cmdReqData[0] > 0x06))
    {
        comp_code = SY_COMP_CODE_DATA_ERROR;
        goto out;
    }

    //第一次注册
    if (1 == g_cmdReqData[0])
    {
        action_AfisInit_FirstEnroll(index);
    }
    SY_COMMAND_PRINT_INFO1("enroll index:%d\r\n", g_cmdReqData[0]);

    /************** 特征提取 **************/
    if(set_system_clock(SYSTEM_90MHZ) == 0)
    {
        DBG_DIRECT("set system clock 90M Fail");
    }
    
    ret = action_Extract(SENSOR_IMAGE_BUFFER);
    if (ret != 0)
    {
        comp_code = SY_COMP_CODE_CAPTURE_ERROR;
        SY_COMMAND_PRINT_INFO1("action_Extract error ret:0x%x\r\n", ret);
        goto out;
    }

    /************* 特征拼接 **************/
    ret = action_Enrollment(g_cmdReqData[0], &progress, index);
    switch (ret)
    {
        case PR_OK:
        case PR_ENROLL_OK:
        case PR_ENROLL_COMPLETE:
        case PR_ENROLL_ALMOST_NOMOVE:
            comp_code = SY_COMP_CODE_OK;
            break;

        default:
            comp_code = SY_COMP_CODE_CAPTURE_ERROR;
            SY_COMMAND_PRINT_INFO1("action_Enrollment error ret:0x%x\r\n", ret);
            break;
    }
    
    if(90000000 == get_cpu_clock())
    {
        if(set_system_clock(SYSTEM_40MHZ) == 0)
            DBG_DIRECT("set system clock 40M Fail");
    }

out:
    SY_COMMAND_PRINT_INFO1("action_Extract and enroll ret:%d\r\n", comp_code);
    
    SY_COMMAND_RespCmd(pReq, &g_pstSYCmdResp , comp_code, NULL, 0, SY_RESP_FLAG_OK);
    return 0;
}

static int SY_COMMAND_searchFTR(PSY_CMD_REQ_DATA pReq)
{
	int ret;
    uint16_t id = 0;
    unsigned char comp_code = 0;

    g_cmdRespData[0] = 0;
    g_cmdRespData[1] = 0;
    g_cmdRespData[2] = 0;
    g_cmdRespData[3] = 0;
    
//    memcpy(g_ImgBuf, g_stEnrollPara.pFeature, g_stEnrollPara.length); 
    memcpy(SY_TmpBuf, g_stEnrollPara.pFeature, g_stEnrollPara.length);
    
    ret = action_Match(&id);
    
    if(FILE_SYSTEM_READ_FTR_ERROR == ret)
    {
        comp_code = SY_COMP_CODE_READ_FTR_ERROR;
        goto out;
    }
    
    if (ret < g_algMatchScoreThres)
    {
        SY_COMMAND_PRINT_INFO0("Not Match!\r\n");
        comp_code = SY_COMP_CODE_SEARCH_FTR_NOT_MATCH;
        goto out;
    }
    else
    {
        SY_COMMAND_PRINT_INFO1("Match!! id = %d\r\n", id);
        g_cmdRespData[0] = ((id & 0xFF00) >> 8);
        g_cmdRespData[1] = (id & 0x00FF);
    	g_cmdRespData[2] = (ret & 0xFF00) >> 8;
    	g_cmdRespData[3] = (ret & 0x00FF);
        comp_code = SY_COMP_CODE_OK;
    }
#if HARDWARE_POLICY_TOUCH
    action_UpdateFtr();
#endif

out:
#if HARDWARE_POLICY_TOUCH
//    memcpy(g_stEnrollPara.pFeature, g_ImgBuf, g_stEnrollPara.length);
    memcpy(g_stEnrollPara.pFeature, SY_TmpBuf, g_stEnrollPara.length);
#endif
    SY_COMMAND_RespCmd(pReq, &g_pstSYCmdResp , comp_code, g_cmdRespData , 4 ,SY_RESP_FLAG_OK);
    return 0;
}

static unsigned int SY_COMMAND_saveFTR(PSY_CMD_REQ_DATA pReq)
{
    int ret;
    unsigned short id = 0;
    unsigned char comp_code = SY_COMP_CODE_OK;

    id = ((g_cmdReqData[1] * 0x100) + g_cmdReqData[2]);
    
    g_stEnrollPara.storage_addr = id;

    if(id  > 0xff)
    {
        comp_code = SY_COMP_CODE_STORAGE_IS_FULL;	
        goto out;
    }

    if(0 == fileSys_checkIDExist(id))
    {
        if(STORE_MAX_FTR <= fileSys_getStoreFtrNum())
        {
            comp_code = SY_COMP_CODE_STORAGE_IS_FULL;	
            goto out;
        }
    }

    ret = action_StoreFtr(id, 0);
    switch (ret)
    {
        case FILE_SYSTEM_OK:
        comp_code = SY_COMP_CODE_OK;
        break;
 
        default:
        comp_code = SY_COMP_CODE_FLASH_WRITE_FAIL;
        break;
    }
    
out:
    SY_COMMAND_PRINT_INFO2("SY_COMMAND_saveFTR id:%d, comp_code:%d\r\n", id, comp_code);
    
    SY_COMMAND_RespCmd(pReq, &g_pstSYCmdResp , comp_code, NULL, 0, SY_RESP_FLAG_OK);
    return 0;
}

static unsigned int SY_COMMAND_deleteFTR(PSY_CMD_REQ_DATA pReq)
{
    unsigned short id = 0;
    unsigned int comp_code = SY_COMP_CODE_OK;
    int ret = 0;
    int ii;
    uint16_t usFPNum = 0;
    uint16_t ausFPIdx[STORE_MAX_FTR] = {0};

    id = (g_cmdReqData[0] << 8) + g_cmdReqData[1];
    usFPNum = (g_cmdReqData[2] << 8) + g_cmdReqData[3];

    if(usFPNum >= STORE_MAX_FTR)
    {
        usFPNum = STORE_MAX_FTR;
    }
    
    for (ii = 0; ii < usFPNum; ii++)
    {
        ausFPIdx[ii] = id + ii;
    }

    ret = fileSys_deleteBatchFtr(ausFPIdx, usFPNum);
    
    if(0 != ret)
	{
	    comp_code = SY_COMP_CODE_DELETE_FTR_ERROR;
	}
    
    SY_COMMAND_RespCmd(pReq, &g_pstSYCmdResp , comp_code, g_cmdRespData , 0 ,SY_RESP_FLAG_OK);
    return 0;
}

static unsigned int SY_COMMAND_clearFTR(PSY_CMD_REQ_DATA pReq)
{
    int ret = 0;
    unsigned int comp_code = SY_COMP_CODE_OK;
    
        
    ret = fileSys_deleteAllFtr();
    if(0 != ret)
    {
        SY_COMMAND_PRINT_INFO0("deleteAllFtr error\r\n");
        comp_code = SY_COMP_CODE_CLEAR_FTR_ERROR;
    }

    SY_COMMAND_PRINT_INFO0("deleteAllFtr\r\n");
    

    SY_COMMAND_RespCmd(pReq, &g_pstSYCmdResp , comp_code, g_cmdRespData , 0 ,SY_RESP_FLAG_OK);
    return 0;
}

static uint32_t SY_COMMAND_autoEnroll(PSY_CMD_REQ_DATA pReq)
{
    uint8_t comp_code = SY_COMP_CODE_OK;
    uint8_t progress = 0;
    uint16_t storage_index = 0xFFFF;
    uint16_t Repeat_Fp_Index = 0xFFFF;
    uint16_t para = 0;
    uint32_t unCaptureTimes = 0;
    uint32_t unEnrollTimes = 0;
    uint32_t ii = 0;
    int ret;

    para = ((g_cmdReqData[3] * 0x100) + g_cmdReqData[4]);

    /****************************指令合法性回复***************************************/
    g_cmdRespData[0] = 0;
    g_cmdRespData[1] = 0;

    //注册次数
    unCaptureTimes = g_cmdReqData[2];
    if ((unCaptureTimes < 0x01) || (unCaptureTimes > 0x06))
    {
        comp_code = SY_COMP_CODE_ENROLL_NUM_ERROR;
        goto out;
    }
    
	g_sysSetting.alg_enroll_num = unCaptureTimes;
    
    //指纹ID
    storage_index = ((g_cmdReqData[0] * 0x100) + g_cmdReqData[1]);

    if(storage_index >= STORE_MAX_FTR)
    {
        comp_code = SY_COMP_CODE_FP_INDEX_OVERFLOW;
        goto out;
    }

    if(0 == (para & AUTO_CMD_COVER_ID))
    {
        if(1 == fileSys_checkIDExist(storage_index))
        {
            comp_code = SY_COMP_CODE_FP_BUFFER_NOT_EMPTY;
            goto out;
        }
    }

    if(fileSys_getStoreFtrNum() >= STORE_MAX_FTR)
    {
        comp_code = SY_COMP_CODE_STORAGE_IS_FULL;
        goto out;
    }
    
    if(0 == (para & AUTO_CMD_ACK_FLAG))
    {
        SY_COMMAND_RespCmd(pReq, &g_pstSYCmdResp ,SY_COMP_CODE_OK, g_cmdRespData , 2 ,SY_RESP_FLAG_OK);
        UTRNS_SendSYFrame(g_pstSYCmdResp);
    }
    /*********************************************************************************/

    /**************************************初始化**************************************/
    //每次注册时重新初始化算法
    action_AfisInit_FirstEnroll(storage_index);
    /*********************************************************************************/

    for (ii = 1; ii <= unCaptureTimes; ii++)
    {
		g_cmdRespData[1] = unEnrollTimes + 1;

        /**************** 采图 **************/
        g_cmdRespData[0] = 0x01;
        
        ret = Sensor_WaitAndCapture(8000);

        switch(ret)
        {
            case SENSOR_COMP_CODE_OK:
                comp_code = SY_COMP_CODE_OK;
//                Led_AutoCtrl(AUTO_LED_CAPT_OK);
            break;

            case SENSOR_COMP_CODE_NO_FINGER:
                comp_code = SY_COMP_CODE_NO_FINGER_DETECT;
            break;

            default:
                comp_code = SY_COMP_CODE_CAPTURE_ERROR;
            break;
        }
        
		if(SENSOR_COMP_CODE_QUIT == ret)
		{
			return 1;
		}
        
        if(0 == (para & AUTO_CMD_ACK_FLAG))
        {
            SY_COMMAND_RespCmd(pReq, &g_pstSYCmdResp ,SY_COMP_CODE_OK, g_cmdRespData , 2 ,SY_RESP_FLAG_OK);
            UTRNS_SendSYFrame(g_pstSYCmdResp);
        }

        
        if (SY_COMP_CODE_OK != ret)
        {
            ii--;
            goto CheckFingerLeave;
        }
        
        /************** 特征提取 **************/ 
        g_cmdRespData[0] = 0x02;
        
        ret = action_Extract(SENSOR_IMAGE_BUFFER);

        if(0 != ret)
        {
            comp_code = SY_COMP_CODE_LACK_FTR_GENERATE_FTR_ERROR;
        }
        
        if(0 == (para & AUTO_CMD_ACK_FLAG))
        {
            SY_COMMAND_RespCmd(pReq, &g_pstSYCmdResp ,SY_COMP_CODE_OK, g_cmdRespData , 2 ,SY_RESP_FLAG_OK);
            UTRNS_SendSYFrame(g_pstSYCmdResp);
        }


        if (SY_COMP_CODE_OK != ret)
        {
            ii--;
            goto CheckFingerLeave;
        }

        /*************** 特征拼接 ***********/
        g_cmdRespData[0] = 0x03;
        
        ret = action_Enrollment(unEnrollTimes+1, &progress, storage_index);
        
        switch (ret)
        {
            case PR_OK:
            case PR_ENROLL_OK:
            case PR_ENROLL_COMPLETE:
            case PR_ENROLL_ALMOST_NOMOVE:
                unEnrollTimes++;
                comp_code = SY_COMP_CODE_OK;			
                break;
        
            case PR_ENROLL_LOW_IMAGE_QUALITY:
            case PR_ENROLL_LOW_MOISTNESS:
                comp_code = SY_COMP_CODE_LACK_FTR_GENERATE_FTR_ERROR;
                break;
        
            default:
                comp_code = SY_COMP_CODE_MERGE_FTR_ERROR;
                break;
        }
#if 0  
        /**判断重复指纹**晟元协议手册不需要在每次按压查重，可添加此段代码来优化体验**/     
        if (0 != (g_sysSetting.sys_policy & SYS_POLICY_REPEAT_CHECK))
        {  
            if(0 != (para & AUTO_CMD_REPEAT_FLAG))
            {
                memcpy(g_ImgBuf, g_stEnrollPara.pFeature, g_stEnrollPara.length);
                ret = action_MatchRepeatCheck(&Repeat_Fp_Index);
                memcpy(g_stEnrollPara.pFeature, g_ImgBuf, g_stEnrollPara.length);

                if (ret < g_sysSetting.fp_score)
                {
                    SY_COMMAND_PRINT(("Not Match,new one\r\n"));
                }
                else
                {
                    SY_COMMAND_PRINT(("Match,same as FP%d\r\n", Repeat_Fp_Index));
                    comp_code = SY_COMP_CODE_FP_EXIST;
                }
            }
        }
        /***************************************************************************/
#endif
        os_delay(100);
        
    
        if(0 == (para & AUTO_CMD_ACK_FLAG))
        {
            SY_COMMAND_RespCmd(pReq, &g_pstSYCmdResp ,SY_COMP_CODE_OK, g_cmdRespData , 2 ,SY_RESP_FLAG_OK);
            UTRNS_SendSYFrame(g_pstSYCmdResp);
        }

        
        if (SY_COMP_CODE_OK != ret)
        {
            ii--;
            goto CheckFingerLeave;
        }
        /*********************************************************************************/

CheckFingerLeave:
		if(progress == 100)
		{
			break;
		}
		else if(0 == (para & AUTO_CMD_FP_LEAVE_FLAG))
		{
			while(1)
			{
				ret = Sensor_WaitAndCapture(100);

				if(SENSOR_COMP_CODE_QUIT == ret)
				{
					return 1;
				}
				else if(SENSOR_COMP_CODE_NO_FINGER == ret)
				{
					break;
				}
				else
				{
					continue;
				}
			}
		}
    }
    /*********************************************************************************/


    /******************************合并模板直接回复***********************************/
    g_cmdRespData[0] = 0x04;
    g_cmdRespData[1] = 0xf0;

	comp_code = SY_COMP_CODE_OK;

    if(0 == (para & AUTO_CMD_ACK_FLAG))
    {
        SY_COMMAND_RespCmd(pReq, &g_pstSYCmdResp ,SY_COMP_CODE_OK, g_cmdRespData , 2 ,SY_RESP_FLAG_OK);
        UTRNS_SendSYFrame(g_pstSYCmdResp);
    }	
    /*********************************************************************************/

    /***************************判断重复指纹******************************************/
    g_cmdRespData[0] = 0x05;
    g_cmdRespData[1] = 0xf1;
    if (0 != (g_sysSetting.sys_policy & SYS_POLICY_REPEAT_CHECK))
    {  
        if(0 != (para & AUTO_CMD_REPEAT_FLAG))
        {
            memcpy(g_ImgBuf, g_stEnrollPara.pFeature, g_stEnrollPara.length);
            ret = action_MatchEx(&Repeat_Fp_Index);
            memcpy(g_stEnrollPara.pFeature, g_ImgBuf, g_stEnrollPara.length);

            if (ret < g_algMatchScoreThres)
            {
                SY_COMMAND_PRINT_INFO0("Not Match,new one\r\n");
            }
            else
            {
                SY_COMMAND_PRINT_INFO1("Match,same as FP%d\r\n", Repeat_Fp_Index);
                comp_code = SY_COMP_CODE_FP_EXIST;
            }
        }
    }

    if(SY_COMP_CODE_OK != comp_code)
    {
        goto out;
    }
    
    if(0 == (para & AUTO_CMD_ACK_FLAG))
    {
        SY_COMMAND_RespCmd(pReq, &g_pstSYCmdResp ,SY_COMP_CODE_OK, g_cmdRespData , 2 ,SY_RESP_FLAG_OK);
        UTRNS_SendSYFrame(g_pstSYCmdResp);
    }
    /*********************************************************************************/

    /**************************保存模板***********************************************/
    g_cmdRespData[0] = 0x06;
    g_cmdRespData[1] = 0xf2;
    
    ret = action_StoreFtr(storage_index, 0);
    switch (ret)
    {
        case FILE_SYSTEM_OK:
        comp_code = SY_COMP_CODE_OK;
        break;
    
        default:
        comp_code = SY_COMP_CODE_READ_WRITE_FLASH_ERROR;
        break;
    }
    /*********************************************************************************/
	
out:
    SY_COMMAND_RespCmd(pReq, &g_pstSYCmdResp , comp_code, g_cmdRespData , 2 , SY_RESP_FLAG_OK);
    UTRNS_SendSYFrame(g_pstSYCmdResp); 
    return 0;
}

static unsigned int SY_COMMAND_autoLogin(PSY_CMD_REQ_DATA pReq)
{
    uint8_t progress = 0;
    int i, ret = 0;
    unsigned int comp_code = SY_COMP_CODE_OK;
    unsigned int pre_comp_code = SY_COMP_CODE_OK;
    unsigned char enrollNum = 0;
    unsigned char repeatCheckFlag = 0;
    unsigned short id;
    unsigned short Fp_Index;

    enrollNum = (g_cmdReqData[1]) & 0x0f;
    if ((enrollNum < 0x01) || (enrollNum > 0x06))
    {
        comp_code = SY_COMP_CODE_ENROLL_NUM_ERROR;
        goto out;
    }
    
    g_sysSetting.alg_enroll_num = enrollNum;
    
    SY_COMMAND_PRINT_INFO1("enrollNum =%d\r\n", enrollNum);

    id = ((g_cmdReqData[2] * 0x100) + g_cmdReqData[3]);
    repeatCheckFlag = g_cmdReqData[4];

    for(i = 1; i < enrollNum+1 ;i++)
    { 
        SY_COMMAND_PRINT_INFO1(" the %d enrollNum\r\n", i);
        //第一次注册
        if (1 == i)
        {
            //注册时容量判断
            if(id  > 0xff)
            {
                id = fileSys_getUnuseSmallestIndex();
                //fileSys_GetUnuseSmallestId(0, &id);
            }
            
            //每次注册时重新初始化算法
            action_AfisInit_FirstEnroll(id);            
        }
          
        /*********** 采集指纹图片 *************/     
        ret = Sensor_WaitAndCapture(10000);
        switch(ret)
        {
            case SENSOR_COMP_CODE_OK:
                comp_code = SY_COMP_CODE_OK;
                break;
        
            case SENSOR_COMP_CODE_NO_FINGER:
                comp_code = SY_COMP_CODE_NO_FINGER_DETECT;
                goto out;
        
            default:
                comp_code = SY_COMP_CODE_UNQUALIFIED_IMAGE_ERROR;
                break;
        }
        
        /*用于判断第一次注册是否正常  */
        if(SY_COMP_CODE_OK != pre_comp_code)
        {
            comp_code = pre_comp_code;
            goto out;
        }
        
        if(SY_COMP_CODE_OK != comp_code)
        {
            switch(i)
            {
                case 1:
                    pre_comp_code = comp_code;
                    goto next;
                default:
                    goto out;
            }
        }

        /************** 特征提取 **************/
        ret = action_Extract(SENSOR_IMAGE_BUFFER);
        if(0 != ret)
        {
            SY_COMMAND_PRINT_INFO1("extration fail %d\r\n",ret);
            comp_code = SY_COMP_CODE_UNQUALIFIED_IMAGE_ERROR;
            switch(i)
            {
                case 1:
                    pre_comp_code = comp_code;
                    goto next;
                default:
                    goto out;
            } 
        }
        SY_COMMAND_PRINT_INFO0("extration OK!\r\n");


            /************** 判断重复指纹 *************/
        if (0 != (g_sysSetting.sys_policy & SYS_POLICY_REPEAT_CHECK))
        {   
            SY_COMMAND_PRINT_INFO0("SYS_POLICY_REPEAT_CHECK!\r\n");
            if(0 == repeatCheckFlag)
            {
                SY_COMMAND_PRINT_INFO0("repeatCheckFlag\r\n");
                
                memcpy(g_ImgBuf, g_stEnrollPara.pFeature, g_stEnrollPara.length);
                ret = action_MatchEx(&Fp_Index);
                memcpy(g_stEnrollPara.pFeature, g_ImgBuf, g_stEnrollPara.length);

                if (ret < g_algMatchScoreThres)
                {
                    SY_COMMAND_PRINT_INFO0("Not Match,new one\r\n");
                }
                else
                {
                    SY_COMMAND_PRINT_INFO1("Match,same as FP%d\r\n", Fp_Index);
                    comp_code = SY_COMP_CODE_FP_EXIST;
                }
            }
        }
    
        /************ 特征拼接 **************/
        ret = action_Enrollment(pReq->cmd, &progress, id);
        switch (ret)
        {
            case PR_OK:
            case PR_ENROLL_OK:
            case PR_ENROLL_COMPLETE:
            case PR_ENROLL_ALMOST_NOMOVE:
                comp_code = SY_COMP_CODE_OK;
                break;
        
            case PR_ENROLL_LOW_IMAGE_QUALITY:
            case PR_ENROLL_LOW_MOISTNESS:
                comp_code = SY_COMP_CODE_UNQUALIFIED_IMAGE_ERROR;
                break;
        
            default:
                comp_code = SY_COMP_CODE_MERGE_FTR_ERROR;
                break;
        }
        
        if(SY_COMP_CODE_OK != comp_code)
        {
            switch(i)
            {
                case 1:
                    pre_comp_code = comp_code;
                    goto next;
                default:
                    goto out;
            }
        } 
        
        if(100 == progress)
        {
            break;
        }
    	
next:
        comp_code = SY_COMP_CODE_FIRST_ENROLL_OK + i - 1; 
        SY_COMMAND_RespCmd(pReq, &g_pstSYCmdResp , comp_code, NULL , 0 ,SY_RESP_FLAG_OK);
        UTRNS_SendSYFrame(g_pstSYCmdResp); 
   
        /************ 等待手指离开 **************/
        #if 1
        do
        {
            if(g_QuitAtOnce)
            {
                return 1;
            }
            os_delay(50);
        }while(Sensor_FingerCheck());
        #endif
    }

    if(0 == fileSys_checkIDExist(id))
    {
        if(STORE_MAX_FTR <= fileSys_getStoreFtrNum())
        {
            comp_code = SY_COMP_CODE_STORAGE_IS_FULL; 
            goto out;
        }
    }

    ret = action_StoreFtr(id, 0);
    switch (ret)
    {
        case FILE_SYSTEM_OK:
        comp_code = SY_COMP_CODE_OK;
        break;
    
        default:
        comp_code = SY_COMP_CODE_READ_WRITE_FLASH_ERROR;
        break;
    }
    
    g_cmdRespData[0] = (id & 0xFF00) >> 8;    //index:h
    g_cmdRespData[1] = (id & 0x00FF);         //index:l

out:
    SY_COMMAND_RespCmd(pReq, &g_pstSYCmdResp , comp_code, g_cmdRespData, 2, SY_RESP_FLAG_OK);
    return 0;

}

static unsigned int SY_COMMAND_quiteMatch(PSY_CMD_REQ_DATA pReq)
{
    int32_t ret;
    uint32_t comp_code =SY_COMP_CODE_OK;
    uint16_t id;
    
    g_cmdRespData[0] = 0;
    g_cmdRespData[1] = 0;
    g_cmdRespData[2] = 0;
    g_cmdRespData[3] = 0;
        
    /*********** 采集指纹图片 *************/
     ret = Sensor_WaitAndCapture(10000);
     switch (ret)
     {
         case SENSOR_COMP_CODE_OK:
             comp_code = SY_COMP_CODE_OK;
             break;
         
         case SENSOR_COMP_CODE_NO_FINGER:
             comp_code = SY_COMP_CODE_NO_FINGER_DETECT;
             break;
         
         case SENSOR_COMP_CODE_ID_ERROR:
             comp_code = SY_COMP_CODE_ICN7000_ERROR_ID;
         break;
         
         case SENSOR_COMP_CODE_QUIT:
             comp_code = SY_COMP_CODE_ICN7000_ERROR_FORCE_QUIT;
         break;
         
         case SENSOR_COMP_CODE_UNQUALIFIED:
             comp_code = SY_COMP_CODE_ICN7000_ERROR_UNQUALIFIED_IMAGE;
         break;
         
         default:
             comp_code = SY_COMP_CODE_CAPTURE_ERROR;
             break;
     }
    
     if (ret != SENSOR_COMP_CODE_OK)
     {
         goto out;
     }

     /************** 特征提取 **************/
     ret = action_Extract(SENSOR_IMAGE_BUFFER);
     if (ret != 0)
     {
         comp_code = SY_COMP_CODE_UNQUALIFIED_IMAGE_ERROR;
         goto out;
     }
   
    ret = action_Match(&id);
    if(FILE_SYSTEM_READ_FTR_ERROR == ret)
    {
         comp_code = SY_COMP_CODE_READ_FTR_ERROR;
         goto out;
    }
     
    if (ret < g_algMatchScoreThres)
    {
        SY_COMMAND_PRINT_INFO0("Not Match!\r\n");
        comp_code = SY_COMP_CODE_SEARCH_FTR_NOT_MATCH;
        goto out;
    }
    else
    {        
        g_cmdRespData[0] = (id & 0xFF00) >> 8;
        g_cmdRespData[1] = (id & 0x00FF);
        g_cmdRespData[2] = (ret & 0xFF00) >> 8;
        g_cmdRespData[3] = (ret & 0x00FF);
    }
#if HARDWARE_POLICY_TOUCH
    action_UpdateFtr();
#endif
out:    
	ledAutoCtrl(AUTO_LED_FAIL);
    SY_COMMAND_RespCmd(pReq, &g_pstSYCmdResp , comp_code, g_cmdRespData , 4 ,SY_RESP_FLAG_OK);
    
    return 0;  
}

static unsigned int SY_COMMAND_autoIdentify(PSY_CMD_REQ_DATA pReq)
{
    int ret;
    unsigned short score = 0;
    unsigned short id = 0;
    unsigned int comp_code = SY_COMP_CODE_OK;
    uint16_t para = 0;
    
    para = ((g_cmdReqData[3] * 0x100) + g_cmdReqData[4]);
    
    g_cmdRespData[1] = g_cmdReqData[1];
    g_cmdRespData[2] = g_cmdReqData[2];
    g_cmdRespData[3] = 0;
    g_cmdRespData[4] = 0;

    /****************************指令合法性回复***************************************/
    g_cmdRespData[0] = 0;

    if(0 == (para & AUTO_CMD_ACK_FLAG))
    {
        SY_COMMAND_RespCmd(pReq, &g_pstSYCmdResp , comp_code, g_cmdRespData , 5, SY_RESP_FLAG_OK);
        UTRNS_SendSYFrame(g_pstSYCmdResp);
    }
    /********************************************************************************/

    /*********** 采集指纹图片 *************/
    g_cmdRespData[0] = 0x01;
    ret = Sensor_WaitAndCapture(10000);
    switch(ret)
    {
        case SENSOR_COMP_CODE_OK:
            comp_code = SY_COMP_CODE_OK;
        break;

        case SENSOR_COMP_CODE_NO_FINGER:
            comp_code = SY_COMP_CODE_NO_FINGER_DETECT;
        break;

        case SENSOR_COMP_CODE_UNQUALIFIED:
            comp_code = SY_COMP_CODE_UNQUALIFIED_IMAGE_ERROR;
        break;

        default:
            comp_code = SY_COMP_CODE_OTHER_ERROR;
        break;
    }
    
    if(0 == (para & AUTO_CMD_ACK_FLAG))
    {
        SY_COMMAND_RespCmd(pReq, &g_pstSYCmdResp , comp_code, g_cmdRespData , 5, SY_RESP_FLAG_OK);
        UTRNS_SendSYFrame(g_pstSYCmdResp);
    }
    
    if(ret != SENSOR_COMP_CODE_OK)
    {
        goto out;
    }

    /************** 特征提取 **************/
    ret = action_Extract(SENSOR_IMAGE_BUFFER);
  
    if (ret != 0)
    {
        comp_code = SY_COMP_CODE_UNQUALIFIED_IMAGE_ERROR;
        goto out;
    }

    /************** 搜索指纹 **************/
    g_cmdRespData[0] = 0x05;
    ret = action_Match(&id);
    
    if(FILE_SYSTEM_READ_FTR_ERROR == ret)
    {
        comp_code = SY_COMP_CODE_READ_FTR_ERROR;
        goto out;
    }
    
    if (ret < g_sysSetting.fp_score)
    {
        SY_COMMAND_PRINT_INFO0("Not Match!\r\n");
        comp_code = SY_COMP_CODE_SEARCH_FTR_NOT_MATCH;
        goto out;
    }
    g_cmdRespData[1] = (id & 0xFF00) >> 8;
    g_cmdRespData[2] = (id & 0x00FF);
    g_cmdRespData[3] = (score & 0xFF00) >> 8;
    g_cmdRespData[4] = (score & 0x00FF);
#if HARDWARE_POLICY_TOUCH
    action_UpdateFtr();
#endif
    
out:
    SY_COMMAND_RespCmd(pReq, &g_pstSYCmdResp , comp_code, g_cmdRespData , 5 , SY_RESP_FLAG_OK);
    return 0;
}

uint32_t SY_COMMAND_PowerSavingStart(PSY_CMD_REQ_DATA pReq)
{
#if !HARDWARE_POLICY_TOUCH
    int ret;
    
	g_QuitAtOnce = 1; 
	delay_ms(100);
    
    ret = Sensor_Sleep();
    if(ret == SENSOR_COMP_CODE_PRESS_FINGER)
    {
        SY_COMMAND_RespCmd(pReq, &g_pstSYCmdResp , SY_COMP_CODE_DATA_ERROR, g_cmdRespData, 0,SY_RESP_FLAG_OK);
    }
    //fpc_sensor_spi_read_irq();
    //delay_ms(200);
    //if (true == fpc_sensor_spi_read_irq() || (0 != ret))
    //{
    //    SY_COMMAND_RespCmd(pReq, &g_pstSYCmdResp, SY_COMP_CODE_DATA_ERROR, g_cmdRespData, 0,SY_RESP_FLAG_OK);
    //}
    //else
    //{
    //    SY_COMMAND_RespCmd(pReq, &g_pstSYCmdResp, SY_COMP_CODE_OK, g_cmdRespData, 0,SY_RESP_FLAG_OK);
    //}
	
#else
    SY_COMMAND_RespCmd(pReq, &g_pstSYCmdResp , SY_COMP_CODE_OK, g_cmdRespData , 0,SY_RESP_FLAG_OK);
#endif
    return 0;
}

static unsigned int SY_COMMAND_WriteNoteBook(PSY_CMD_REQ_DATA pReq)
{
    int ret;
    unsigned char comp_code = SY_COMP_CODE_OK;
    unsigned char sec;
    unsigned char sy_buff[SY_NOTE_LEN];

    sec = g_cmdReqData[0];
    
    if (sec >= SY_MAX_NOTE)
    {
        comp_code = SY_COMP_CODE_NOTEBOOK_PAGE_NUM_ERROR;
        goto out;
    }
    
    memcpy(sy_buff, &g_cmdReqData[1], SY_NOTE_LEN);

    ret = fileSys_WriteSYNoteBook(sec, sy_buff, SY_NOTE_LEN);
    if(0 != ret)
    {
        comp_code = SY_COMP_CODE_DATA_ERROR;
    }
    
out:
    SY_COMMAND_RespCmd(pReq, &g_pstSYCmdResp , comp_code, g_cmdRespData, 0,  SY_RESP_FLAG_OK);
    return 0;
}

static unsigned int SY_COMMAND_readFlashFTRToSRAM(PSY_CMD_REQ_DATA pReq)
{
    short index;
    unsigned int id = 0;
    unsigned int comp_code = 0;
    unsigned int length;

    if(2 != g_cmdReqData[0])
    {
        comp_code = SY_COMP_CODE_DATA_ERROR;
        goto out;
    }
    
    id = ((g_cmdReqData[1] * 0x100) + g_cmdReqData[2]);
    index = fileSys_IdToIndex(id);
    if(0 > index)
    {
        comp_code = SY_COMP_CODE_FP_INDEX_OVERFLOW;
        goto out;
    }
    
    if(FILE_SYSTEM_OK != fileSys_readFtr((unsigned short)index, g_EnrollFtrBuf, &length))
    {
        comp_code = SY_COMP_CODE_READ_FTR_ERROR;
    }   
    
    g_UploadFTRLen = length;

    comp_code = SY_COMP_CODE_OK;

out:
	SY_COMMAND_RespCmd(pReq, &g_pstSYCmdResp , comp_code, g_cmdRespData , 0 ,SY_RESP_FLAG_OK);
    return 0;
}

static unsigned int SY_COMMAND_uploadSRAMFTRStart(PSY_CMD_REQ_DATA pReq)
{
    uint32_t unCRC = 0;
    unsigned char upload_mode = 1;
    
    upload_mode = g_cmdReqData[0];

    switch (upload_mode)
    {
        case 1:
            g_sy_uploadmainlen = g_stEnrollPara.length;
            
            unCRC = CRC32_calc(g_EnrollFtrBuf , g_sy_uploadmainlen);
            g_EnrollFtrBuf[g_sy_uploadmainlen] = (unCRC >> 0) & 0xFF;
            g_EnrollFtrBuf[g_sy_uploadmainlen+1] = (unCRC >> 8) & 0xFF;
            g_EnrollFtrBuf[g_sy_uploadmainlen+2] = (unCRC >> 16) & 0xFF;
            g_EnrollFtrBuf[g_sy_uploadmainlen+3] = (unCRC >> 24) & 0xFF;
            
            g_sy_uploadmainlen += 4;
            
            SY_COMMAND_RespCmd(pReq, &g_pstSYCmdResp , SY_COMP_CODE_OK, g_cmdRespData , 0 ,SY_RESP_FLAG_OK);
            UTRNS_SendSYFrame(g_pstSYCmdResp); 
        break;

        case 2:
            g_sy_uploadmainlen = g_UploadFTRLen + 4;
            
            SY_COMMAND_RespCmd(pReq, &g_pstSYCmdResp , SY_COMP_CODE_OK, g_cmdRespData , 0 ,SY_RESP_FLAG_OK);
            UTRNS_SendSYFrame(g_pstSYCmdResp); 
        break;

        default:
            SY_COMMAND_RespCmd(pReq, &g_pstSYCmdResp , SY_COMP_CODE_DATA_ERROR, g_cmdRespData , 0 ,SY_RESP_FLAG_DATA);
            UTRNS_SendSYFrame(g_pstSYCmdResp);
            return 0;
    }

    g_sy_uploadflag = SY_RESP_FLAG_DATA;
    g_sy_uploadoffset = 0;

    SY_PostUploadFTRCommand();

    return 0;
}

static unsigned int SY_COMMAND_uploadSRAMFTR(PSY_CMD_REQ_DATA pReq)
{
    unsigned int recvLen = 0;
    if(SY_RESP_FLAG_DATA == g_sy_uploadflag)
    {
        if((g_sy_uploadmainlen - g_sy_uploadoffset) > g_stPacketSize)
        {
            recvLen = g_stPacketSize - 1;
            memcpy(&g_cmdRespData[0] , &g_EnrollFtrBuf[g_sy_uploadoffset] , g_stPacketSize);
            SY_COMMAND_RespCmd(pReq, &g_pstSYCmdResp , g_cmdRespData[0], &g_cmdRespData[1] , recvLen , g_sy_uploadflag); 
            UTRNS_SendSYFrame(g_pstSYCmdResp);
            g_sy_uploadoffset += g_stPacketSize;
            g_sy_uploadflag = SY_RESP_FLAG_DATA;
        }
        else
        {
            g_sy_uploadflag = SY_RESP_FLAG_DATA_END; 
        }


        SY_PostUploadFTRCommand();
    }
    else if(SY_RESP_FLAG_DATA_END == g_sy_uploadflag)
    {
        recvLen = g_sy_uploadmainlen - g_sy_uploadoffset - 1;
        memcpy(&g_cmdRespData[0] , &g_EnrollFtrBuf[g_sy_uploadoffset] , g_stPacketSize);
        SY_COMMAND_RespCmd(pReq, &g_pstSYCmdResp , g_cmdRespData[0], &g_cmdRespData[1] , recvLen , g_sy_uploadflag); 
        UTRNS_SendSYFrame(g_pstSYCmdResp);
    }
    
    return 0;
}

static unsigned int SY_COMMAND_downloadStart(PSY_CMD_REQ_DATA pReq)
{     
    memset(g_EnrollFtrBuf , 0 , SENSOR_FTR_BUFFER_MAX);
    g_sy_FtrRecievedLen = 0;
    SY_COMMAND_RespCmd(pReq, &g_pstSYCmdResp , SY_COMP_CODE_OK, g_cmdRespData , 0 ,SY_RESP_FLAG_DATA);
    return 0;
}

static unsigned int SY_COMMAND_readFlashInfo(PSY_CMD_REQ_DATA pReq)
{
    unsigned int recvLen = 0;
    if(SY_RESP_FLAG_DATA == g_sy_uploadflag)
    {
        if((g_sy_uploadmainlen - g_sy_uploadoffset) > g_stPacketSize)
        {
            recvLen = g_stPacketSize - 1;
            memcpy(&g_cmdRespData[0] , &g_EnrollFtrBuf[g_sy_uploadoffset] , g_stPacketSize);
            SY_COMMAND_RespCmd(pReq, &g_pstSYCmdResp , g_cmdRespData[0], &g_cmdRespData[1] , recvLen , g_sy_uploadflag); 
            UTRNS_SendSYFrame(g_pstSYCmdResp);
            g_sy_uploadoffset += g_stPacketSize;
            g_sy_uploadflag = SY_RESP_FLAG_DATA;
        }
        else
        {
            g_sy_uploadflag = SY_RESP_FLAG_DATA_END; 
        }

        SY_PostUploadFTRCommand();
    }
    else if(SY_RESP_FLAG_DATA_END == g_sy_uploadflag)
    {
        recvLen = g_sy_uploadmainlen - g_sy_uploadoffset - 1;
        memcpy(&g_cmdRespData[0] , &g_EnrollFtrBuf[g_sy_uploadoffset] , g_stPacketSize);
        SY_COMMAND_RespCmd(pReq, &g_pstSYCmdResp , g_cmdRespData[0], &g_cmdRespData[1] , recvLen , g_sy_uploadflag); 
        UTRNS_SendSYFrame(g_pstSYCmdResp);
    }

    return 0;
}

/*****************************************************************************
 函 数 名  : COMMAND_Task
 功能描述  : 请求处理任务
 输入参数  : void * p_arg  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2015年12月24日
    作    者   : jack
    修改内容   : 新生成函数

*****************************************************************************/
void SY_COMMAND_Task(void)
{
    uint8_t cmd;
    PSY_CMD_REQ_DATA pReq;
    unsigned char flag;

    
    pReq = &g_SY_stMsgReq;
    cmd = pReq->cmd;
    flag = pReq->flag;

    if(SY_RESP_FLAG_DATA == flag || SY_RESP_FLAG_DATA_END == flag)
    {
        SY_COMMAND_downloadFTR(pReq);
        goto out;
    }
    
    switch(cmd)
    {
        case SY_CMD_MEGER_FTR: 
            SY_COMMAND_megerFTR(pReq);
            UTRNS_SendSYFrame(g_pstSYCmdResp); 
            break;
        
        case SY_CMD_GET_IMAGE:
            SY_COMMAND_getImage(pReq);
            UTRNS_SendSYFrame(g_pstSYCmdResp); 
            break;

        case SY_CMD_ENROLL_CAPTURE_IMAGE:
            SY_COMMAND_getEnrollImage(pReq);
            UTRNS_SendSYFrame(g_pstSYCmdResp); 
            break;

        case SY_CMD_GENERATE_FEATURE:
            SY_COMMAND_getChar(pReq);
            UTRNS_SendSYFrame(g_pstSYCmdResp); 
            break;
                       
        case SY_CMD_SEARCH_FTR:
            SY_COMMAND_searchFTR(pReq);
            UTRNS_SendSYFrame(g_pstSYCmdResp); 
#if !HARDWARE_POLICY_TOUCH
            MLAPI_UpdateFTR();
//            memcpy(g_stEnrollPara.pFeature, g_ImgBuf, g_stEnrollPara.length);
            memcpy(g_stEnrollPara.pFeature, SY_TmpBuf, g_stEnrollPara.length);         
#endif
            break;
            
        case SY_CMD_STORAGE_FTR:
            SY_COMMAND_saveFTR(pReq);
            UTRNS_SendSYFrame(g_pstSYCmdResp); 
            break;
                
        case SY_CMD_DELETE_FTR:
            SY_COMMAND_deleteFTR(pReq);
            UTRNS_SendSYFrame(g_pstSYCmdResp); 
            break;
        
        case SY_CMD_CLEAR_FTR:
            SY_COMMAND_clearFTR(pReq);
            UTRNS_SendSYFrame(g_pstSYCmdResp); 
            break;
        
        case SY_CMD_AUTO_ENROLL:
            SY_COMMAND_autoEnroll(pReq);
            break;
            
        case SY_CMD_AUTO_LOGIN:
            SY_COMMAND_autoLogin(pReq);
            UTRNS_SendSYFrame(g_pstSYCmdResp); 
            break;

            
        case SY_CMD_AUTO_MATCH:
        case SY_CMD_AUTO_SEARCH:
            SY_COMMAND_quiteMatch(pReq);
            UTRNS_SendSYFrame(g_pstSYCmdResp);
#if !HARDWARE_POLICY_TOUCH
            MLAPI_UpdateFTR();
#endif
            break;
            
        case SY_CMD_AUTO_IDENTIFY:
            SY_COMMAND_autoIdentify(pReq);
            UTRNS_SendSYFrame(g_pstSYCmdResp);
#if !HARDWARE_POLICY_TOUCH
            MLAPI_UpdateFTR();
#endif
            break;

        case SY_CMD_FP_DELAY_REBOOT:
//            CommWaitTXBufEmpty(USART_COMM);
            NVIC_SystemReset();
            break;
            
        case SY_CMD_SLEEP_MODE:
			SY_COMMAND_PowerSavingStart(pReq);
            UTRNS_SendSYFrame(g_pstSYCmdResp); 
            break; 

        case SY_CMD_WRITE_NOTEBOOK:
            SY_COMMAND_WriteNoteBook(pReq);
            UTRNS_SendSYFrame(g_pstSYCmdResp);
            break;

        case SY_CMD_READ_FLASH_FTR_TO_SRAM:
            SY_COMMAND_readFlashFTRToSRAM(pReq);
            UTRNS_SendSYFrame(g_pstSYCmdResp); 
            break;
        
        case SY_CMD_UPLOAD_SRAM_FTR_START:
            SY_COMMAND_uploadSRAMFTRStart(pReq);
            break;
        
        case SY_CMD_UPLOAD_SRAM_FTR:
            os_delay(10);
            SY_COMMAND_uploadSRAMFTR(pReq);
            break;
            
        case SY_CMD_DOWNLOAD_FTR:
            SY_COMMAND_downloadStart(pReq);
            UTRNS_SendSYFrame(g_pstSYCmdResp); 
            break;

        case SY_CMD_UPLOAD_FLASH_INFO:
            os_delay(10);
            SY_COMMAND_readFlashInfo(pReq);
            break;
                
        default:
        break;
    }

out:
    g_QuitAtOnce = 0;
//    g_CurrentCmd = 0;
}
