#include "sensor.h"
#include "driver_spi.h"
#include "trace.h"
#include "system_setting.h"
#include "string.h"
#include "filesys.h"
//#include "Command.h"
//#include "Flash.h"
#include "fpsapi.h"
#include "fpsapierrors.h"
//#include "mlinit.h"
#include "stdlib.h"
#include "mlapi.h"
#include "SensorAdjust.h"
#include "driver_sensor.h"
//#include "LedCtrl.h"
//#include "iwdg.h"
#include "menu_sleep.h"
#include "app_task.h"
#include "rtl876x_rcc.h"
#include "rtl876x_wdg.h"
#include "rtl876x_gpio.h"

extern volatile uint8_t g_bWork;
#if 0
#define SENSOR_DEBUG(x) printf x
#else
#define SENSOR_DEBUG(x)
#endif


#define QUERY_FINGER_CYCLE                  (20)


#define FLASH_FTRUPD_SPEED_1                (32850)
#define FLASH_FTRUPD_SPEED_2                (43543)
#define FLASH_FTRUPD_SPEED_3                (80000)


extern uint8_t g_ImgBuf[SENSOR_IMG_BUFFER_MAX];  //80*64+32,最小值需与特征缓冲区一致, 缓冲区大小为0x3000字节
extern  uint8_t g_FtrBufTmp[SENSOR_FTR_BUFFER_TMP];
extern  uint8_t g_EnrollFtrBuf[SENSOR_FTR_BUFFER_MAX];
extern uint32_t g_FtrLenTmp;

ST_ROI_CTRL g_stRoiCtrl;
ST_ROI g_stRoiNormal;
ST_ROI_CTRL g_stRoiDect;

//采图增益值
uint8_t g_capture_shift = DEFAULT_FPSEN_SHIFT;
uint8_t g_capture_gain = DEFAULT_FPSEN_GAIN;
uint8_t g_capture_pxlctrl = DEFAULT_FPSEN_PXLCTRL;

extern uint8_t g_QuitAtOnce;

uint8_t g_SensorHardErr_flag = 0;

fpsensor_adc_t adc_fixed;
uint8_t print_id = true;
uint8_t g_RecentFingerIndex[(STORE_MAX_FTR+4)];		//排序最近使用的指纹ID

extern uint32_t g_TotalTime;
//extern OS_SEM sem_spi_sync;        //防止SPI重复进入出现异常

#if TEST_SAVE_PIC
__align(4) uint8_t g_ftrbuffTemp[128] = {0};
uint8_t g_TESTPIC_update_flag = false;
uint8_t* g_FtrBuf = NULL;
uint32_t* g_FtrLen = 0;

void SENSOR_GetFTRInfo(uint8_t ** pImg, uint32_t *pLen)
{
    *pImg = g_FtrBuf;
    *pLen = *g_FtrLen;
}
#endif

/*****************************************************************************
 函 数 名  : SENSOR_GetSpiImgInfo
 功能描述  : 获取指纹图片储存位置，对外接口
 输入参数  : uint8_t ** pImg  
             uint32_t *pLen   
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2015年12月24日
    作    者   : jack
    修改内容   : 新生成函数

*****************************************************************************/
void SENSOR_GetSpiImgInfo(uint8_t ** pImg, uint32_t *pLen)
{
	*pImg = SPI_IMAGE_BUFFER;
	*pLen = FPSENSOR_IMAGE_HEIGHT*FPSENSOR_IMAGE_WIDTH;

}

void dumpMsg(u8 *pBuff, u32 len)
{
    u32 i;

    for (i = 0; i < len; ++i)
    {
        APP_PRINT_INFO1("%02x-", pBuff[i]);
        if(i%16 == 15)
        {
        	delay_ms(1);
			APP_PRINT_INFO0("\r\n");
		}
            
    }
    APP_PRINT_INFO0("\r\n");
}


/*****************************************************************************
 函 数 名  : SENSOR_RoiCapInit
 功能描述  : 初始化采图ROI
 输入参数  : void  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2019年12月18日
    作    者   : Alex
    修改内容   : 新生成函数

*****************************************************************************/
void SENSOR_RoiCapInit(void)
{
    int i;
    memset((uint8_t *)&g_stRoiCtrl, 0, sizeof(ST_ROI_CTRL));
    
    g_stRoiNormal.x = 0;
    g_stRoiNormal.y = 0;
    g_stRoiNormal.width = FPSENSOR_IMAGE_WIDTH;
    g_stRoiNormal.hight = FPSENSOR_IMAGE_HEIGHT;

    for (i = 0; i < 12; i++)
    {
        g_stRoiDect.stRoi[i].x = ((i & 0x3) + 1) << 5; //*32
        g_stRoiDect.stRoi[i].y = ((i >> 2) + 1) * 40;
        g_stRoiDect.stRoi[i].width = 8;
        g_stRoiDect.stRoi[i].hight = 8;
    }
    
    g_stRoiDect.area = (8 * 8 + 4) * 12 + 6;
    g_stRoiDect.roi_num = 12;

    //g_stRoiCtrl
    g_stRoiCtrl.stRoi[0].width = 64;
    g_stRoiCtrl.stRoi[0].hight = 16;
    g_stRoiCtrl.stRoi[0].x = (FPSENSOR_IMAGE_WIDTH - g_stRoiCtrl.stRoi[0].width) / 2; 
    g_stRoiCtrl.stRoi[0].y = (FPSENSOR_IMAGE_HEIGHT - g_stRoiCtrl.stRoi[0].hight) / 2; 

    g_stRoiCtrl.roi_num = 6;

    return;

}

#if 0
/*****************************************************************************
 函 数 名  : Sensor_SetAdcGain
 功能描述  : 设置ICN7000增益
 输入参数  : uint8_t shift  
             uint8_t gain   
             uint8_t pxl    
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2015年12月24日
    作    者   : jack
    修改内容   : 新生成函数

*****************************************************************************/
uint8_t SENSOR_SetAdcGain(uint8_t shift, uint8_t gain, uint8_t pxl)
{
    if(shift > 31)
    {
        SENSOR_DEBUG(("shift should be [0~31].\r\n"));
        return 1;
    }

    if(gain > 15)
    {
        SENSOR_DEBUG(("gain should be [0~15].\r\n"));
        return 1;
    }
        
    if((pxl != 0)&&(pxl != 4)&&(pxl != 16)&&(pxl != 20))
    {
        SENSOR_DEBUG(("pxl should be [0/4/16/20].\r\n"));
        return 1;
    }
    

	SYSSET_SetFpSensorGain(shift, gain, pxl);
	
    SENSOR_DEBUG(("set detect shift[0x%x] detect gain[0x%x] detect pxl[0x%x] ok.", 
        g_sysSetting.fp_detect_shift, g_sysSetting.fp_detect_gain, g_sysSetting.fp_detect_pxlctrl));
    
    return 0;
}

/*****************************************************************************
 函 数 名  : Sensor_GetAdcGain
 功能描述  : 获取ICN7000增益
 输入参数  : void  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2015年12月24日
    作    者   : jack
    修改内容   : 新生成函数

*****************************************************************************/
void SENSOR_GetAdcGain(uint8_t *pShift, uint8_t *pGain, uint8_t *pPxlCtrl)
{
    *pShift   = g_sysSetting.fp_shift;
    *pGain    = g_sysSetting.fp_gain;
    *pPxlCtrl = g_sysSetting.fp_pxlctrl & 0x14;
}
#endif


/*****************************************************************************
 函 数 名  : SENSOR_SpiTransceive
 功能描述  : 
 输入参数  : void  
 输出参数  : 无
 返 回 值  : uint8_t
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年9月14日
    作    者   : Alex
    修改内容   : 新生成函数

*****************************************************************************/
#if 0
uint8_t SENSOR_SpiTransceive(uint8_t *tx, uint32_t len , bool IsNeedDelay)
{
	int ret=0;
	uint8_t rx[16] = {0};
    
	if(tx[0]==FPSENSOR_CMD_READ_IMAGE)
	{
		SPI_TransmitDataEx(tx, 2,tx+2, len-2);
        spiSensorTransfer
		memcpy(tx,tx+2,len-2);
	}
	else
	{
		if(len >16)
			return 1;
		
		if (true == IsNeedDelay)
		{
			delay_ms(1);
		}
		SPI_TransmitData(tx, len,rx, len);
	
		memcpy(tx,rx,len);
		return ret;
	}
	return ret;
}
#endif

uint8_t SENSOR_SpiTransceive(uint8_t *tx, uint32_t len , unsigned char IsNeedDelay)
{
	int ret = 0;
    uint8_t spi_rx[16] = {0};
	
	if (tx[0] == FPSENSOR_CMD_READ_IMAGE)
	{
		spiSensorTransfer(tx, tx+2, len-2, false);
		memcpy(tx, tx+2, len-2);
	}
	else
	{
		if(len >16)
			return 1;
		
		if (true == IsNeedDelay)
		{
			delay_ms(1);	
		}
        
		spiSensorTransfer(tx, spi_rx, len, false);
	
		memcpy(tx, spi_rx, len);
		return ret;
	}
	return ret;
}

#if 0
/*****************************************************************************
 函 数 名  : SENSOR_AfisInit
 功能描述  : 指纹算法初始化
 输入参数  : int nEnrollMaxNum  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年10月25日
    作    者   : li dong
    修改内容   : 新生成函数

*****************************************************************************/
void SENSOR_AfisInit(int nEnrollMaxNum, uint8_t nMaxSetNum)
{
	AfisParams_t stPara; 
	
	GetDefaultAfisParams(&stPara);
	g_sysSetting.fp_score = stPara.nMatchScoreThres;
	
	stPara.nEnrollMaxNum = nEnrollMaxNum;
	stPara.nMaxSetNum = nMaxSetNum;
	stPara.nMaxFtrSize = CalcMaxFtrSize(nMaxSetNum);

    if (g_flashErase_Num < FLASH_FTRUPD_SPEED_1)
    {
        //printf("g_flashErase_Num = %d < 32850\r\n",g_flashErase_Num);
    }
    else if (g_flashErase_Num < FLASH_FTRUPD_SPEED_2)
    {
        //printf("g_flashErase_Num = %d > 32850\r\n",g_flashErase_Num);
        stPara.nFadingSpeed = 1;
    }    
    else if (g_flashErase_Num < FLASH_FTRUPD_SPEED_3)
    {
        //printf("g_flashErase_Num = %d > 43543\r\n",g_flashErase_Num);
        stPara.nFadingSpeed = 2;
    }    
    else
    {
        //printf("g_flashErase_Num %d > 80000\r\n",g_flashErase_Num);
        stPara.nSelfLearningStrategy = 7;
    }   

    //注册图像等级控制
    if (1 == g_sysSetting.alg_enroll_level)
    {
        stPara.nMoistnessGoodFingerThr = 20;
    }
    else if (2 == g_sysSetting.alg_enroll_level)
    {
        stPara.nMoistnessGoodFingerThr = 30;
    }
    else
    {
        stPara.nMoistnessGoodFingerThr = 40;
    }
    printf("level = %d\r\n", stPara.nMoistnessGoodFingerThr);
    //printf("stPara.nEnrollMaxNum:%d; stPara.nMaxSetNum:%d stPara.nMaxFtrSize:%d\r\n", 
    //        stPara.nEnrollMaxNum, stPara.nMaxSetNum, stPara.nMaxFtrSize);

	SetAfisParams(&stPara);

#if 0
	AfisParams_t stPara;
	
	GetDefaultAfisParams(&stPara);
	g_sysSetting.fp_score = stPara.nMatchScoreThres;
	
	stPara.nEnrollMaxNum = nEnrollMaxNum;
    
	if (6 == nMaxSetNum)
	{
		stPara.nMaxSetNum = nMaxSetNum;
		stPara.nMaxFtrSize = nMaxSetNum*988+64;
	}

	SetAfisParams(&stPara);
	SENSOR_DEBUG(("MatchScoreThres :%d, MaxFtrSize:%d, EnrollMaxNum:%d, MaxSetNum:%d\r\n", 
		stPara.nMatchScoreThres, stPara.nMaxFtrSize, stPara.nEnrollMaxNum, stPara.nMaxSetNum));
#endif
}

void SENSOR_AfisInitEx(int nEnrollMaxNum, uint8_t nMaxSetNum)
{
	AfisParams_t stPara;

	GetDefaultAfisParams(&stPara);
    
	g_sysSetting.fp_score = stPara.nMatchScoreThres;
    
	stPara.nEnrollMaxNum = nEnrollMaxNum;
	stPara.nMaxSetNum = nMaxSetNum;
    stPara.nMaxFtrSize = CalcMaxFtrSize(nMaxSetNum);

    //关闭自学习
	stPara.nLearnScoreThres = 9999;
    
    g_FtrUpdateFlag = false;
    g_FtrInfoUpdateFlag = false;
    
	SetAfisParams(&stPara);
}

/*****************************************************************************
 函 数 名  : SENSOR_ImgJudgment
 功能描述  : 
 输入参数  : uint8_t *Imgbuff  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2015年12月24日
    作    者   : jack
    修改内容   : 新生成函数

*****************************************************************************/
int32_t SENSOR_ImgJudgment(uint8_t *Imgbuff)
{
    RawImageInfo_t stRawImgInfo;
    int32_t rst = 0;

    stRawImgInfo.pData= Imgbuff;
    stRawImgInfo.nHeight = FPSENSOR_IMAGE_HEIGHT;
    stRawImgInfo.nWidth = FPSENSOR_IMAGE_WIDTH;
    stRawImgInfo.nBpp = 0;
    stRawImgInfo.nReserved = 0;

    rst = IsAvailableImage(&stRawImgInfo);

    return rst;
}

/*****************************************************************************
 函 数 名  : SENSOR_Extract
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
int32_t SENSOR_Extract(void)
{
    RawImageInfo_t stRawImgInfo;
    int32_t rst = 0;
    uint8_t *pFtrBuf;
    uint32_t *pLen;
	OS_ERR err;
	uint32_t TimeTemp;
    
    TimeTemp = OSTimeGet(&err);
#if TEST_SAVE_PIC
	g_FtrBuf = g_FtrBufTmp;
	g_FtrLen = &g_FtrLenTmp;
#endif

    pFtrBuf = g_FtrBufTmp;
    pLen = &g_FtrLenTmp;
    
    stRawImgInfo.pData= SPI_IMAGE_BUFFER;
    stRawImgInfo.nHeight = FPSENSOR_IMAGE_HEIGHT;
    stRawImgInfo.nWidth = FPSENSOR_IMAGE_WIDTH;
    stRawImgInfo.nBpp = 0;
    stRawImgInfo.nReserved = 0;

    rst = ExtractFeature(&stRawImgInfo, pFtrBuf, pLen);
	g_FtrLenTmp = *pLen;
    TimeTemp =  OSTimeGet(&err) - TimeTemp;
    TimeTemp = TimeTemp*10;
	SENSOR_DEBUG(("e : %3d , ", TimeTemp));

    return rst;
}


/*****************************************************************************
 函 数 名  : SENSOR_Match
 功能描述  :  指纹特征匹配
 输入参数  : void  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2015年12月24日
    作    者   : jack
    修改内容   : 新生成函数

*****************************************************************************/
int32_t SENSOR_Match(uint16_t *Fp_Index)
{
    int32_t rst, timeCount = 0, timeall = 0;
    uint32_t index[2], i;
    uint32_t len, uTimeTick = 0;
    uint8_t *p_ftrTpl;
    uint32_t score;
	uint8_t FtrIndexBuf[STORE_MAX_FTR];
	uint32_t FtrIndexBufLen;
	OS_ERR err;
	uint32_t TimeTemp;
    //uint8_t flag = 1;
           
    UNUSED_PARA(timeCount);
    UNUSED_PARA(timeall);
    UNUSED_PARA(uTimeTick);

    p_ftrTpl = &g_EnrollFtrBuf[0];
    if(NULL == p_ftrTpl)
    {
        return -1;
    }
    
    TimeTemp = OSTimeGet(&err);

    index[0] = fileSys_getStoreFtrNum();

    if(index[0] == 0)
    {
        return -1;
    }

	fileSys_getUseIndex(FtrIndexBuf , &FtrIndexBufLen);
  	fileSys_rankFtrHead(FtrIndexBuf , FtrIndexBufLen);
    SENSOR_AfisInit(g_sysSetting.alg_enroll_num, FPS_SETNUM_MAX);
    WDT_IdleTaskFunc();
    
    g_pUpdateFTRData = NULL;
    g_UpdateFTRLen = 0;
    g_UpdateFTRIndex = 0xFFFF;
    g_FtrUpdateFlag = false;

    score = 0;
    for (i = 0; i < FtrIndexBufLen; i++)
    {	
        if (DRV_SUCCESS == fileSys_readFtr(FtrIndexBuf[i], p_ftrTpl, &len))
        {
            #if 0
			if (FtrIndexBuf[i] > (FILE_SYSTEM_STORE_FTR_NUM_8K-1))
			{
			    if (1 == flag)
                {
                    SENSOR_AfisInit(g_sysSetting.alg_enroll_num, FPS_SETNUM_MID);
                    flag = 0;
                }         
			}
            else 
            {
                if (0 == flag)
                {
                    SENSOR_AfisInit(g_sysSetting.alg_enroll_num, FPS_SETNUM_MAX);
                    flag = 1;
                }
            }
            #endif
            rst = MatchFeature(p_ftrTpl,  &len, g_FtrBufTmp, g_FtrLenTmp, &score);
            if (PR_DATA_POINTER_NOT_4_ALIGN == rst)
            {
                printf("Match err :not 4 len\r\n");
            }

			if(score >= g_sysSetting.fp_score)
			{
                *Fp_Index = fileSys_indexToId(FtrIndexBuf[i]);

                if (PR_MATCH_FTR_UPDATED == rst) //需要更新特征
                {
                    //2021.1.12 判断特征是否合法，特征合法才启动更新
                    if (1 == IsValidFeature(p_ftrTpl,len))
                    {
                        g_pUpdateFTRData = p_ftrTpl;
                        g_UpdateFTRLen = len;
                        g_UpdateFTRIndex = (*Fp_Index);
                        g_FtrInfoUpdateFlag = false;
                        g_FtrUpdateFlag = true;
#if TEST_SAVE_PIC
                        g_TESTPIC_update_flag = true;
#endif	
                    }
                    else
                    {
                		//数据错误，打印错误信息
                		printf("Ftr invalid update data. [%02x][%02x][%02x]\r\n", p_ftrTpl[0], p_ftrTpl[1], p_ftrTpl[2]);
                    }									
                }
                else if(PR_MATCH_FTR_HEAD_UPDATED == rst)
                {
                    //2021.1.12 判断特征是否合法，特征合法才启动更新
                    if (1 == IsValidFeature(p_ftrTpl,len))
                    {
                        g_pUpdateFTRData = p_ftrTpl;
                        g_UpdateFTRLen = len;
                        g_UpdateFTRIndex = (*Fp_Index);
                        g_FtrUpdateFlag = false;
                        g_FtrInfoUpdateFlag = true;
#if TEST_SAVE_PIC
                        g_TESTPIC_update_flag = true;
#endif													
                    }
                    else
                    {
                		//数据错误，打印错误信息
                		printf("Ftr invalid update info. [%02x][%02x][%02x]\r\n", p_ftrTpl[0], p_ftrTpl[1], p_ftrTpl[2]);
                    }												
                }

                TimeTemp =  OSTimeGet(&err) - TimeTemp;
                TimeTemp = TimeTemp*10;
                SENSOR_DEBUG(("m : %4d \r\n", TimeTemp));
                Led_AutoCtrl(AUTO_LED_MATCH_OK);
                SENSOR_AfisInit(g_sysSetting.alg_enroll_num, FPS_SETNUM_MAX);
                return score;
	        }
        }
    }

    TimeTemp =  OSTimeGet(&err) - TimeTemp;
    TimeTemp = TimeTemp*10;
    //SENSOR_DEBUG(("SENSOR_Match fail\r\n"));
    Led_AutoCtrl(AUTO_LED_FAIL);
    SENSOR_AfisInit(g_sysSetting.alg_enroll_num, FPS_SETNUM_MAX);
    return score;
}


/*****************************************************************************
 函 数 名  : SENSOR_MatchEx
 功能描述  : 快速匹配
 输入参数  : uint16_t *Fp_Index  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2019年12月19日
    作    者   : Alex
    修改内容   : 新生成函数

*****************************************************************************/
int32_t SENSOR_MatchEx(uint16_t *Fp_Index)
{
    uint32_t index[2], i;
    uint32_t len;
    uint8_t *p_ftrTpl;
    uint32_t score;
	uint8_t FtrIndexBuf[STORE_MAX_FTR];
	uint32_t FtrIndexBufLen;

    SENSOR_AfisInitEx(g_sysSetting.alg_enroll_num, FPS_SETNUM_MAX);

    p_ftrTpl = &g_EnrollFtrBuf[0];
    if(NULL == p_ftrTpl)
    {
        SENSOR_AfisInit(g_sysSetting.alg_enroll_num, FPS_SETNUM_MAX);
        return -1;
    }

    index[0] = fileSys_getStoreFtrNum();
    if(index[0] == 0)
    {
        SENSOR_AfisInit(g_sysSetting.alg_enroll_num, FPS_SETNUM_MAX);
        return -1;
    }

	fileSys_getUseIndex(FtrIndexBuf , &FtrIndexBufLen);

    g_pUpdateFTRData = NULL;
    g_UpdateFTRLen = 0;
    g_UpdateFTRIndex = 0xFFFF;
    g_FtrUpdateFlag = false;

    score = 0;

	for(i = 0; i < FtrIndexBufLen; i++)
	{		
		if (DRV_SUCCESS == fileSys_readFtr(FtrIndexBuf[i], p_ftrTpl, &len))
		{        
			
			MatchFeature(p_ftrTpl, &len, g_FtrBufTmp, g_FtrLenTmp, &score);
			if(score >= g_sysSetting.fp_score)
			{
		        *Fp_Index = fileSys_indexToId(FtrIndexBuf[i]);
                SENSOR_AfisInit(g_sysSetting.alg_enroll_num, FPS_SETNUM_MAX);
                return score;
			}
		}
	}

    SENSOR_AfisInit(g_sysSetting.alg_enroll_num, FPS_SETNUM_MAX);

    return score;
}

/*****************************************************************************
 函 数 名  : SENSOR_Enrollment
 功能描述  : 指纹注册
 输入参数  : uint8_t num             
             uint8_t *pProgress      
             uint32_t storage_index  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2019年12月20日
    作    者   : Alex
    修改内容   : 新生成函数

*****************************************************************************/
int32_t SENSOR_Enrollment(uint8_t num, uint8_t *pProgress, uint32_t storage_index)
{
    int32_t ret;
    uint32_t progress;

#if TEST_SAVE_PIC
	g_FtrBuf = g_stEnrollPara.pFeature;
    g_FtrLen = &g_stEnrollPara.length;
#endif

    progress = g_stEnrollPara.progress; 

	ret = EnrollFeature(g_FtrBufTmp, &g_stEnrollPara.pFeature, &g_stEnrollPara.length, &g_stEnrollPara.progress);
    SENSOR_DEBUG(("len = %d, progress = %d, ret = 0x%x .\r\n", g_stEnrollPara.length, g_stEnrollPara.progress, ret));
    *pProgress = g_stEnrollPara.progress;
    
    if ((g_stEnrollPara.progress > progress)||(100 == g_stEnrollPara.progress))
    {        
        progress = g_stEnrollPara.progress;
        ret = 0;
    }
    else
    {
        ret = PR_ENROLL_LOW_IMAGE_QUALITY;
    }
    
    return ret;
}

/*****************************************************************************
 函 数 名  : SENSOR_StorageFeature
 功能描述  : 指纹特征保存
 输入参数  : uint32_t storage_index  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年10月25日
    作    者   : li dong
    修改内容   : 新生成函数

*****************************************************************************/
int32_t SENSOR_StorageFeature(uint32_t storage_index)
{

    int32_t nErrorcode = SENSOR_OK;
    int32_t nRet = SENSOR_OK;

	nRet = fileSys_storeFtrAndUpdateFtrHead(storage_index , g_stEnrollPara.pFeature, g_stEnrollPara.length);
    switch(nRet)
    {
        case FILE_SYSTEM_OK:
            nErrorcode = SENSOR_OK;
            break;
                        
        case FILE_SYSTEM_STORAGE_FULL:
            nErrorcode = SENSOR_ERROR_STORAGE_FULL;
            break;

        default:
            nErrorcode = SENSOR_ERROR_FAILURE;
            break;
    }

    return nErrorcode;
}
#endif

/*****************************************************************************
 函 数 名  : SENSOR_Sleep
 功能描述  : Sensor休眠
 输入参数  : void  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2019年12月20日
    作    者   : Alex
    修改内容   : 新生成函数

*****************************************************************************/
int32_t SENSOR_Sleep(void)
{
	return SENSOR_ActiveSleepMode_1();
}

/*****************************************************************************
 函 数 名  : SENSOR_DeepSleep
 功能描述  : Sensor深度休眠
 输入参数  : void  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年10月25日
    作    者   : li dong
    修改内容   : 新生成函数

*****************************************************************************/
int32_t SENSOR_DeepSleep(void)
{
	uint8_t buffer[1] = {FPSENSOR_CMD_ACTIVATE_DEEP_SLEEP_MODE};
	return SENSOR_SpiTransceive(buffer, sizeof(buffer) , false);
}

int32_t SENSOR_DeepSleep_1(void)
{
	uint8_t status = 0;
	uint8_t buffer[11];

	status |= (fpsensor_reg_fngr_det_cntr());
	status |= (fpsensor_unknow_cmd());
	status |= (fpsensor_finger_drive_config());
	status |= (SENSOR_SetCaptureCrop(buffer,FPSENSOR_IMAGE_WIDTH*FPSENSOR_IMAGE_HEIGHT,0,80,0,64));
	
	

	buffer[0] = FPSENSOR_REG_PXL_CTRL;
	buffer[1] = 0x0f;
	buffer[2] = 0x07; 
	status |= SENSOR_SpiTransceive(buffer, 3 , false);

	buffer[0] = FPSENSOR_REG_ADC_SHIFT_GAIN;
	buffer[1] = 0x0d;
	buffer[2] = 0x04;
	status |= SENSOR_SpiTransceive(buffer, 3 , false);

	status |= (fpsensor_image_rd(0x40));
	status |= (SENSOR_SetInverse_1(buffer, FPSENSOR_IMAGE_WIDTH*FPSENSOR_IMAGE_HEIGHT , 0x69));
	status |= (fpsensor_smpl_setup());
	status |= (fpsensor_reg_ana_cfg1());
	status |= (fpsensor_reg_ana_cfg2());
	
	buffer[0] = FPSENSOR_CMD_ACTIVATE_DEEP_SLEEP_MODE;
	status |= SENSOR_SpiTransceive(buffer, 1 , false);

	return status;
}

uint8_t SENSOR_SetInverse(uint8_t *buffer, uint32_t length, fpsensor_invert_color_t color)
{
    uint8_t status = 0;
	
	if(buffer == NULL || length < 2)
	{
		return FPSENSOR_BUFFER_ERROR;
	}
	
    buffer[0] = FPSENSOR_REG_IMAGE_SETUP;
    buffer[1] = (uint8_t)color;
    status = SENSOR_SpiTransceive(buffer, 2 , false);

    return status;
}

uint8_t SENSOR_SetInverse_1(uint8_t *buffer, uint32_t length, uint8_t color)
{
    uint8_t status = 0;
	
	if(buffer == NULL || length < 2)
	{
		return FPSENSOR_BUFFER_ERROR;
	}
	
    buffer[0] = FPSENSOR_REG_IMAGE_SETUP;
    buffer[1] = color;
    status = SENSOR_SpiTransceive(buffer, 2 , false);

    return status;
}

uint8_t SENSOR_SetCaptureCrop(uint8_t *buffer,
											   uint32_t length,
							                   uint32_t rowStart,
							                   uint32_t rowCount,
											   uint32_t colStart,
							                   uint32_t colGroup)
{
	if(buffer == NULL || length < 5)
	{
		return FPSENSOR_BUFFER_ERROR;
	}
	
    buffer[0] = FPSENSOR_REG_IMG_CAPT_SIZE;
    buffer[1] = rowStart;
    buffer[2] = rowCount;
    buffer[3] = colStart;
    buffer[4] = colGroup;

    return SENSOR_SpiTransceive(buffer, 5 , false);
}

uint8_t SENSOR_CaptureImageHW(void)
{
	uint8_t buffer[1] = {FPSENSOR_CMD_CAPTURE_IMAGE};
	
 	return SENSOR_SpiTransceive(buffer, 1 , false);
}

uint8_t SENSOR_GetImgData(uint8_t *buffer, uint32_t length)
{
	if(buffer == NULL || length < 2)
	{
		return FPSENSOR_BUFFER_ERROR;
	}
	memset(buffer, 0, length);
	buffer[0] = FPSENSOR_CMD_READ_IMAGE;
    buffer[1] = 0x00;
    return SENSOR_SpiTransceive(buffer, length , false);
}

/*****************************************************************************
 函 数 名  : SENSOR_SetAdc
 功能描述  : 设置sensor ADC
 输入参数  : void
 输出参数  : 无
 返 回 值  : uint8_t
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2019年12月19日
    作    者   : Alex
    修改内容   : 新生成函数

*****************************************************************************/
uint8_t SENSOR_SetAdc(fpsensor_adc_t *adc)
{	
	uint8_t status = 0;
	uint8_t buffer[4] = { 0 };

	if(adc == NULL)
	{
		return FPSENSOR_BUFFER_ERROR;
	}
	buffer[0] = FPSENSOR_REG_FNGR_DET_THRES;
	buffer[1] = (FINGER_STATUS_THRES>>8)&0xFF;
	buffer[2] = FINGER_STATUS_THRES&0xFF;
	status |= SENSOR_SpiTransceive(buffer, 3 , false);


	buffer[0] = FPSENSOR_REG_PXL_CTRL;
	buffer[1] = 0x0f;
	buffer[2] = adc->pixel;
	status |= SENSOR_SpiTransceive(buffer, 3 , false);

	buffer[0] = FPSENSOR_REG_ADC_SHIFT_GAIN;
	buffer[1] = adc->shift;
	buffer[2] = adc->gain;
	status |= SENSOR_SpiTransceive(buffer, 3 , false);

	if (FPSENSOR_OK != status)
	{
		APP_PRINT_INFO0("Fpsensor config adc failed.\r\n");
		return FPSENSOR_SPI_ERROR;
	}

	return FPSENSOR_OK;

}

/*****************************************************************************
 函 数 名  : SENSOR_AdcSetup
 功能描述  : 
 输入参数  : void
 输出参数  : 无
 返 回 值  : uint8_t
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2019年12月19日
    作    者   : Alex
    修改内容   : 新生成函数

*****************************************************************************/
void SENSOR_AdcSetup(uint8_t shift, uint8_t gain, uint8_t pxl)
{
	fpsensor_adc_t AdcTemp;
	AdcTemp.gain = gain;
	AdcTemp.pixel = pxl;
	AdcTemp.shift = shift;
	SENSOR_SetAdc(&AdcTemp);

}


/*****************************************************************************
 函 数 名  : SENSOR_ReadIrqNoClear
 功能描述  : 
 输入参数  : uint8_t *buffer, uint32_t length
 输出参数  : 无
 返 回 值  : uint8_t
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2019年12月19日
    作    者   : Alex
    修改内容   : 新生成函数

*****************************************************************************/
uint8_t SENSOR_ReadIrqNoClear(uint8_t *buffer, uint32_t length)
{
	if(buffer == NULL || length < 2)
	{
		return FPSENSOR_BUFFER_ERROR;
	}
	
	buffer[0] = FPSENSOR_REG_READ_INTERRUPT;
	buffer[1] = 0x00;
	return SENSOR_SpiTransceive(buffer, 2 , false);
}

/*****************************************************************************
 函 数 名  : SENSOR_FingerPresentStatus
 功能描述  : 
 输入参数  : uint8_t *buffer, uint32_t length
 输出参数  : 无
 返 回 值  : uint8_t
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2019年12月19日
    作    者   : Alex
    修改内容   : 新生成函数

*****************************************************************************/
uint8_t SENSOR_FingerPresentStatus(uint8_t *buffer, uint32_t length)
{
	uint8_t status = 0;
	
	if(buffer == NULL || length < 3)
	{
		return FPSENSOR_BUFFER_ERROR;
	}
	
    buffer[0] = FPSENSOR_REG_FNGR_DET_THRES;
	buffer[1] = (uint8_t)(FINGER_STATUS_THRES >> 8);
	buffer[2] = (uint8_t)(FINGER_STATUS_THRES &0X00FF);
    status |= SENSOR_SpiTransceive(buffer, 3 , false);
    buffer[0] = FPSENSOR_CMD_FINGER_PRESENT_QUERY;
    status |= SENSOR_SpiTransceive(buffer, 1 , false);

    buffer[0] = FPSENSOR_REG_FINGER_PRESENT_STATUS;
    buffer[1] = 0x00;
    buffer[2] = 0x00;
    status |= SENSOR_SpiTransceive(buffer, 3 , true);

    return status;
}


/*****************************************************************************
 函 数 名  : SENSOR_FingerStatusStatistics
 功能描述  : 检测手指在位设置
 输入参数  : void
 输出参数  : 无
 返 回 值  : uint8_t
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2019年12月19日
    作    者   : Alex
    修改内容   : 新生成函数

*****************************************************************************/
uint8_t SENSOR_FingerStatusStatistics(void)
{
	uint8_t status = 0;
	uint8_t sum    = 0;
	uint8_t i      = 0;
	uint8_t buffer[10] = {0};
	
	status |= SENSOR_SetAdc(&adc_fixed);		
	
	status |= SENSOR_ReadIrqNoClear(buffer,sizeof(buffer));
	
	status |= SENSOR_FingerPresentStatus(buffer,sizeof(buffer));
	if(FPSENSOR_OK != status)
	{
		return 0;
	}

	buffer[1] &= 0x0f;
	
	i = 0;
	while(i<4)
	{
		sum += buffer[1] >> i++ & 1;
	}
	i = 0;
	while(i<8)
	{
		sum += buffer[2] >> i++ & 1;
	}
	
	return sum;
}

uint8_t SENSOR_FingerStatusStatistics_1(void)
{
	uint8_t status = 0;
	uint8_t sum    = 0;
	uint8_t i      = 0;
	uint8_t buffer[10] = {0};
	//status |= (fpsensor_unknow_cmd());
	//status |= (fpsensor_finger_drive_config());
	//status |= (SENSOR_SetCaptureCrop(buffer,FPSENSOR_IMAGE_WIDTH*FPSENSOR_IMAGE_HEIGHT,0,80,0,64));
	status |= SENSOR_SetAdc(&adc_fixed);	
	//fpsensor_image_rd(0X40);
	//SENSOR_SetInverse_1(buffer, FPSENSOR_IMAGE_WIDTH*FPSENSOR_IMAGE_HEIGHT , 0x69);
	//fpsensor_smpl_setup();
	//fpsensor_reg_ana_cfg1();
	//fpsensor_reg_ana_cfg2();
	status |= SENSOR_ReadIrqNoClear(buffer,sizeof(buffer));
	status |= SENSOR_FingerPresentStatus(buffer,sizeof(buffer));
	if(FPSENSOR_OK != status)
	{
		return 0;
	}

	buffer[1] &= 0x0f;

	i = 0;

	while(i<4)
	{		
		sum += buffer[1] >> i++ & 1;
	}

	i = 0;
	while(i<8)
	{
		sum += buffer[2] >> i++ & 1;
	}
	return sum;
}


/*****************************************************************************
 函 数 名  : SENSOR_FpsensorFingerDown
 功能描述  : 检测手指在位
 输入参数  : void
 输出参数  : 无
 返 回 值  : uint8_t
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2019年12月19日
    作    者   : Alex
    修改内容   : 新生成函数

*****************************************************************************/
uint8_t SENSOR_FpsensorFingerDown(void)
{
	uint8_t sum = SENSOR_FingerStatusStatistics_1();
//    printf("sum:%d\r\n",sum);
	return sum > 11? 1 : 0;
	//return 0;
}

uint8_t SENSOR_FpCaptureImageEx(uint8_t* buffer, uint32_t length, uint32_t timeout_seconds)
{
	uint8_t status = 0;
	uint8_t soft_irq = 0;
	int i =0;
	
	if(buffer == NULL || length < FPSENSOR_IMAGE_HEIGHT*FPSENSOR_IMAGE_WIDTH + 2)
	{
		return FPSENSOR_BUFFER_ERROR;
	}
	
	print_id = false;
	print_id = true;
	
	SENSOR_SetInverse_1(buffer, length, DISPLAY_FORWORD);
	status |= SENSOR_SetCaptureCrop(buffer, length, 0, FPSENSOR_IMAGE_HEIGHT, 0, FPSENSOR_IMAGE_WIDTH);
	status |= SENSOR_CaptureImageHW();

	while(1)
  	{
		status |= SENSOR_ReadIrqNoClear(buffer, length);
		soft_irq = buffer[1] & FPSENSOR_IRQ_REG_BIT_FIFO_NEW_DATA;

		if (soft_irq)
		{
			break;
		}
		
		if (FPSENSOR_OK != status)
		{

		}
		i++;
		delay_ms(1);
		if(i>10)
			break;
  	}

	status |= SENSOR_GetImgData(buffer, FPSENSOR_IMAGE_HEIGHT*FPSENSOR_IMAGE_WIDTH + 2);
	return status;
}

int SENSOR_ImageClassification(unsigned char *pdata1, unsigned char *pdata2, int size)
{
    int ret;
	int i = 0;
	int nDifNum = 0;
	const int nDifThe = 22;
	const int nDifNumThe = 270;
	int val = 0;

	for (i = 0; i < size; ++i)
	{
		val = (int)pdata1[i] - (int)pdata2[i];
		if (abs(val) > nDifThe)
		{
			++nDifNum;
		}
	}
    ret = (nDifNum > nDifNumThe ? 1 : 0);
	return ret;
}

void SENSOR_ImageProgerss(unsigned char *pIn1, unsigned char *pIn2, unsigned char *pIn3,
	unsigned char *pIn4, unsigned char *pIn5, int nSize, unsigned char *pOut)
{
	int i = 0;
	unsigned char nOrtA, nOrtB, nOrtC, nOrtD, nOrtE;
    int nTmp;
	for (i = 0; i < nSize; ++i)
	{
		nOrtA = pIn1[i];
		nOrtB = pIn2[i];
		nOrtC = pIn3[i];
		nOrtD = pIn4[i];
		nOrtE = pIn5[i];
		
		//six times in comparison to find the median.
		if (nOrtA < nOrtB)
		{
			nTmp = nOrtA;
			nOrtA = nOrtB;
			nOrtB = nTmp;
		}
		if (nOrtC < nOrtD)
		{
			nTmp = nOrtC;
			nOrtC = nOrtD;
			nOrtD = nTmp;
		}
		if (nOrtA < nOrtC)
		{
			nTmp = nOrtA;
			nOrtA = nOrtC;
			nOrtC = nTmp;
			nTmp = nOrtB;
			nOrtB = nOrtD;
			nOrtD = nTmp;
		}
		if (nOrtB < nOrtE)
		{
			nTmp = nOrtB;
			nOrtB = nOrtE;
			nOrtE = nTmp;
		}
		if (nOrtB < nOrtC)
		{
			nTmp = nOrtB;
			nOrtB = nOrtC;
			nOrtC = nTmp;
			nTmp = nOrtE;
			nOrtE = nOrtD;
			nOrtD = nTmp;
		}
		if (nOrtE < nOrtC)
			pOut[i] = nOrtC;
		else
			pOut[i] = nOrtE;
	}
}


void SENSOR_ReadImage(uint8_t *image)
{

	//Adjust_read_image_Wave_filtering(image);
#if 0
	uint8_t *p_ImgBuf1;
    uint8_t *p_ImgBuf2;
    uint8_t *p_ImgBuf3;
    uint8_t *p_ImgBuf4;
    uint32_t pLen;
	
	if(image == NULL)
	{
		return;
	}

	 if(0 == GetAfisMemory(&p_ImgBuf1, &pLen))   //调用获取内存接口，返回72KB内存地址
    {
        if(pLen >= 4 * SENSOR_IMG_BUFFER_MAX)
        {
            p_ImgBuf2 = p_ImgBuf1 + SENSOR_IMG_BUFFER_MAX;
            p_ImgBuf3 = p_ImgBuf2 + SENSOR_IMG_BUFFER_MAX;
            p_ImgBuf4 = p_ImgBuf3 + SENSOR_IMG_BUFFER_MAX;

            p_ImgBuf1 += 8;
            p_ImgBuf2 += 8;
            p_ImgBuf3 += 8;
            p_ImgBuf4 += 8;
        }
        else
        {
            SENSOR_DEBUG(("GetAfis Memory  len err\r\n"));
            return;
        }
    }
    else
    {
        SENSOR_DEBUG(("GetAfis Memory err\r\n"));
		return;
    }

	SENSOR_FpCaptureImageEx(image, FPSENSOR_IMAGE_HEIGHT*FPSENSOR_IMAGE_WIDTH+2, 10);
 	SENSOR_FpCaptureImageEx(p_ImgBuf1, FPSENSOR_IMAGE_HEIGHT*FPSENSOR_IMAGE_WIDTH+2, 10);

	if (SENSOR_ImageClassification(image, p_ImgBuf1, FPSENSOR_IMAGE_HEIGHT*FPSENSOR_IMAGE_WIDTH+2 ))
    {
        SENSOR_FpCaptureImageEx(p_ImgBuf2, FPSENSOR_IMAGE_HEIGHT*FPSENSOR_IMAGE_WIDTH+2, 10);
        SENSOR_FpCaptureImageEx(p_ImgBuf3, FPSENSOR_IMAGE_HEIGHT*FPSENSOR_IMAGE_WIDTH+2, 10);
        SENSOR_FpCaptureImageEx(p_ImgBuf4, FPSENSOR_IMAGE_HEIGHT*FPSENSOR_IMAGE_WIDTH+2, 10);
        
        SENSOR_ImageProgerss(image, p_ImgBuf1, p_ImgBuf2, p_ImgBuf3, p_ImgBuf4, FPSENSOR_IMAGE_HEIGHT*FPSENSOR_IMAGE_WIDTH+2, image);
    }
    
    AfisMemInit();      //释放内存
#endif
}

uint8_t SENSOR_ReadImage_1(uint8_t *image)
{
    uint8_t unCompCode = 0;

    if(image == NULL)
    {
        APP_PRINT_INFO0("p_image err\r\n");
        return 1;
    }
#if FILTERING_ALGO_ON
    unCompCode = Adjust_read_image_Wave_filtering(SPI_IMAGE_BUFFER);
#else   
    unCompCode  = SENSOR_FpCaptureImage_1(image, 0, 0, FPSENSOR_IMAGE_WIDTH, FPSENSOR_IMAGE_HEIGHT, 200);
#endif
    return unCompCode;
}


/*****************************************************************************
 函 数 名  : SENSOR_HardReset
 功能描述  :sensor hard reset
 输入参数  : void  
 输出参数  : 无
 返 回 值  : uint8_t
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年9月14日
    作    者   : Alex
    修改内容   : 新生成函数

*****************************************************************************/
uint8_t SENSOR_HardReset(void)
{
	uint8_t buffer[3] = {0};
	
	buffer[0] = 0xf8;
    buffer[1] = 0x00;
	
    return SENSOR_SpiTransceive(buffer, 2 , false);
}

/*****************************************************************************
 函 数 名  : SENSOR_ReadIrqWithClear
 功能描述  :sensor 读并清除标志
 输入参数  : void  
 输出参数  : 无
 返 回 值  : uint8_t
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年9月14日
    作    者   : Alex
    修改内容   : 新生成函数

*****************************************************************************/
uint8_t SENSOR_ReadIrqWithClear(uint8_t *buffer, uint32_t length)
{
	if(buffer == NULL || length < 2)
	{
		return FPSENSOR_BUFFER_ERROR;
	}
	
	buffer[0] = FPSENSOR_REG_READ_INTERRUPT_WITH_CLEAR;
	buffer[1] = 0x00;
	
	return SENSOR_SpiTransceive(buffer, 2 , false);
}

/*****************************************************************************
 函 数 名  : SENSOR_FpCaptureImage
 功能描述  : 检查图像数据是否可读
 输入参数  : void  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2015年12月24日
    作    者   : jack
    修改内容   : 新生成函数

*****************************************************************************/
uint8_t SENSOR_FpCaptureImage(uint8_t* buffer, uint32_t x , uint32_t y , uint32_t ImageWidth , uint32_t ImageHeight , uint32_t timeout_seconds)
{
	uint8_t status = 0;
	uint8_t soft_irq = 0;
	int i =0;
	
	if(buffer == NULL )
	{
		return FPSENSOR_BUFFER_ERROR;
	}
	
	SENSOR_SetInverse_1(buffer, ImageWidth*ImageHeight , DISPLAY_FORWORD);
	status |= SENSOR_SetCaptureCrop(buffer, ImageWidth*ImageHeight, y, ImageHeight, x, ImageWidth);
	status |= SENSOR_CaptureImageHW();

	while(1)
  	{
		status |= SENSOR_ReadIrqNoClear(buffer, ImageWidth*ImageHeight);
		soft_irq = buffer[1] & FPSENSOR_IRQ_REG_BIT_FIFO_NEW_DATA;
		if (soft_irq)
		{
			break;
		}
		
		if (FPSENSOR_OK != status)
		{
	
		}
		i++;
		delay_ms(1);
		if(i>10)
			break;
  	}

	status |= SENSOR_GetImgData(buffer, ImageWidth*ImageHeight + 2);

	return status;
}

uint8_t SENSOR_FpCaptureImage_1(uint8_t* buffer, uint32_t x , uint32_t y , uint32_t ImageWidth , uint32_t ImageHeight , uint32_t timeout_seconds)
{
	uint8_t status = 0;
	uint8_t soft_irq = 0;
	int i =0;
	
	if (buffer == NULL)
	{
		return FPSENSOR_BUFFER_ERROR;
	}
	
	status |= SENSOR_SetCaptureCrop(buffer, ImageWidth*ImageHeight + 2, y, ImageHeight, x, ImageWidth);
	SENSOR_AdcSetup(g_capture_shift, g_capture_gain, g_capture_pxlctrl);
	fpsensor_image_rd(0x40);
	SENSOR_SetInverse_1(buffer, ImageWidth*ImageHeight , 0x69);

	//status |= fpsensor_smpl_setup();
	//status |= fpsensor_reg_ana_cfg1();
	//status |= fpsensor_reg_ana_cfg2();
	status |= fpsensor_image_rd(0x60);
	
	status |= SENSOR_ReadIrqNoClear(buffer,ImageWidth*ImageHeight);	
	status |= SENSOR_CaptureImageHW();

	while(!soft_irq)
  	{
		status |= SENSOR_ReadIrqNoClear(buffer, ImageWidth*ImageHeight);
		soft_irq = buffer[1] & FPSENSOR_IRQ_REG_BIT_FIFO_NEW_DATA;
		if (soft_irq)
		{
			//APP_PRINT_INFO0("----------------soft_irq\r\n");	
			break;
		}
		
		if (FPSENSOR_OK != status)
		{
	
		}
		i++;
		delay_ms(10);
		if(i>10)
		{
			//APP_PRINT_INFO0("--------------i>10\r\n");
			break;
		}
  	}

	status |= SENSOR_GetImgData(buffer, ImageWidth*ImageHeight + 2);

	return status;
}


/*****************************************************************************
 函 数 名  : SENSOR_CaptureImage
 功能描述  : 检查图像数据是否可读
 输入参数  : void  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2015年12月24日
    作    者   : jack
    修改内容   : 新生成函数

*****************************************************************************/
uint32_t SENSOR_CaptureImage(uint8_t *pImg, PST_ROI pRoi)
{
	SENSOR_FpCaptureImage_1(pImg , pRoi->x, pRoi->y, pRoi->width, pRoi->hight , 200);

    return 0;
}

uint8_t SENSOR_ActiveSleepMode(int32_t fngerDectPeriod)
{
	uint8_t status = 0;
	uint8_t period = 0;
	uint8_t buffer[4];

	if(fngerDectPeriod <= 0)
	{
		period = 0x00;
	}
	else if(fngerDectPeriod >= (64*255*1000/SLEEP_MODE_CLK)) 
	{
		period = 0xFF;
	}
	else{
		period = (fngerDectPeriod*SLEEP_MODE_CLK/1000)>>6;
	}
	status |= SENSOR_SetAdc(&adc_fixed);   //maybe you should change adc parameter

	status |= SENSOR_ReadIrqWithClear(buffer, sizeof(buffer));
	
	buffer[0] = FPSENSOR_REG_FNGR_DET_CNTR;
	buffer[1] = 0x00;
	buffer[2] = period; 
	status |= SENSOR_SpiTransceive(buffer, 3 , false);  
	
	buffer[0] = FPSENSOR_CMD_ACTIVATE_SLEEP_MODE;
	status |= SENSOR_SpiTransceive(buffer, 1 , false);

	return status;
}

uint8_t SENSOR_ActiveSleepMode_1(void)
{
	uint8_t status = 0;
	uint8_t buffer[11];

//	status |= (fpsensor_reg_fngr_det_cntr());
	//status |= (fpsensor_unknow_cmd());
	//status |= (fpsensor_finger_drive_config());
	//status |= (SENSOR_SetCaptureCrop(buffer,FPSENSOR_IMAGE_WIDTH*FPSENSOR_IMAGE_HEIGHT,0,80,0,64));

	/*buffer[0] = FPSENSOR_REG_PXL_CTRL;
	buffer[1] = 0x0f;
	buffer[2] = 0x07; 
	status |= SENSOR_SpiTransceive(buffer, 3 , false);

	buffer[0] = FPSENSOR_REG_ADC_SHIFT_GAIN;
	buffer[1] = 0x0d;
	buffer[2] = 0x04;
	status |= SENSOR_SpiTransceive(buffer, 3 , false);*/

	//status |= (fpsensor_image_rd(0x40));
	//status |= (SENSOR_SetInverse_1(buffer, FPSENSOR_IMAGE_WIDTH*FPSENSOR_IMAGE_HEIGHT , 0x69));
	//status |= (fpsensor_smpl_setup());
	//status |= (fpsensor_reg_ana_cfg1());
	//status |= (fpsensor_reg_ana_cfg2());

	/*buffer[0] = FPSENSOR_REG_ADC_SHIFT_GAIN;
	buffer[1] = 0x1F;
	buffer[2] = 0x0F;
	status |= SENSOR_SpiTransceive(buffer, 3 , false);

	buffer[0] = FPSENSOR_REG_PXL_CTRL;
	buffer[1] = 0x0f;
	buffer[2] = 0x07; 
	status |= SENSOR_SpiTransceive(buffer, 3 , false);*/
	status |= SENSOR_SetAdc(&adc_fixed);   //maybe you should change adc parameter
	status |= SENSOR_ReadIrqWithClear(buffer, sizeof(buffer));
	
//	buffer[0] = FPSENSOR_REG_FNGR_DET_CNTR;
//		buffer[1] = 0x00;
//		buffer[2] = (20*SLEEP_MODE_CLK/1000)>>6; 
//		status |= SENSOR_SpiTransceive(buffer, 3 , false);  
//		
//		buffer[0] = FPSENSOR_CMD_ACTIVATE_SLEEP_MODE;
//		status |= SENSOR_SpiTransceive(buffer, 1 , false);
//	
//		printf ("wait for finger down.....\n");
//		while(1){
//			buffer[0] = 0x18;
//			buffer[1] = 0x00;
//			status |= SENSOR_SpiTransceive(buffer, 2 , false);
//			printf ("buffer[1]=%x\n",buffer[1]);
//			if (buffer[1]&0x01) // finger down irq
//			{
//				break;
//			}
//			delay_ms(50);
//		}

		buffer[0] = FPSENSOR_REG_FNGR_DET_THRES;//
		buffer[1] = (uint8_t)(FINGER_STATUS_THRES >> 8);
		buffer[2] = (uint8_t)(FINGER_STATUS_THRES &0X00FF); 
		status |= SENSOR_SpiTransceive(buffer, 3 , false);
		
		buffer[0] = FPSENSOR_CMD_ACTIVATE_SLEEP_MODE;
		status |= SENSOR_SpiTransceive(buffer, 1 , false);
		
		return status;
}

void SENSOR_Common(void)
{
	uint8_t status = 0;
	uint8_t buffer[11];
    
	status |= (fpsensor_reg_fngr_det_cntr());
	status |= (fpsensor_unknow_cmd());
	status |= (fpsensor_finger_drive_config());
	status |= (SENSOR_SetCaptureCrop(buffer,FPSENSOR_IMAGE_WIDTH*FPSENSOR_IMAGE_HEIGHT,0,80,0,64));
	status |= (fpsensor_image_rd(0x40));
	status |= (SENSOR_SetInverse_1(buffer, FPSENSOR_IMAGE_WIDTH*FPSENSOR_IMAGE_HEIGHT , 0x69));
	status |= (fpsensor_smpl_setup());
	status |= (fpsensor_reg_ana_cfg1());
	status |= (fpsensor_reg_ana_cfg2());
}


/*****************************************************************************
 函 数 名  : SENSOR_HWID
 功能描述  : 获取sensor硬件ID
 输入参数  : void  
 输出参数  : 无
 返 回 值  : uint8_t
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年9月14日
    作    者   : Alex
    修改内容   : 新生成函数

*****************************************************************************/
uint8_t SENSOR_HWID(void)
{
	uint8_t buffer[3] = {0};
	
	buffer[0] = FPSENSOR_REG_HWID;
    buffer[1] = 0x00;
    buffer[2] = 0x00;
    SENSOR_SpiTransceive(buffer, 3 , true);
    
	if (0x73 == buffer[1] && 0x32 == buffer[2])
	{
		g_SensorHardErr_flag = 0;
		return FPSENSOR_OK;
	}
    
	APP_PRINT_INFO2("Fpsensor HWID = %02x %02x \r\n", buffer[1], buffer[2]);

	g_SensorHardErr_flag = 1;
	return FPSENSOR_SPI_ERROR;
}

/*****************************************************************************
 函 数 名  : SENSOR_VID
 功能描述  : 获取sensor VID
 输入参数  : void  
 输出参数  : 无
 返 回 值  : uint8_t
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年9月14日
    作    者   : Alex
    修改内容   : 新生成函数

*****************************************************************************/
uint8_t SENSOR_VID(void)
{
	uint8_t status    = 0;
	uint8_t buffer[4] = {0};
	
	buffer[0] = FPSENSOR_REG_VID;
    buffer[1] = 0x00;
    buffer[2] = 0x00;
    buffer[3] = 0x00;
    status = SENSOR_SpiTransceive(buffer, 4 , true);
	if(buffer[2] > 0x14)
	{
		buffer[0] = FPSENSOR_REG_OSC_TRIM;
		buffer[1] = 0x00;
		buffer[2] = 0x06;
		buffer[3] = 0x0a;
		
		status |= SENSOR_SpiTransceive(buffer, 4 , false);
	}

	return status;
}

uint8_t fpsensor_set_adc(fpsensor_adc_t *adc)
{	
	uint8_t status = 0;
	uint8_t buffer[4] = { 0 };

	if(adc == NULL)
	{
		return FPSENSOR_BUFFER_ERROR;
	}

    buffer[0] = FPSENSOR_REG_FNGR_DET_THRES;
    buffer[1] = (FINGER_STATUS_THRES>>8)&0xFF;
    buffer[2] = FINGER_STATUS_THRES&0xFF;
    status |= SENSOR_SpiTransceive(buffer, 3 , false);

    buffer[0] = FPSENSOR_REG_PXL_CTRL;
    buffer[1] = 0x0f;
    buffer[2] = adc->pixel;
    status |= SENSOR_SpiTransceive(buffer, 3 , false);

    buffer[0] = FPSENSOR_REG_ADC_SHIFT_GAIN;
    buffer[1] = adc->shift;
    buffer[2] = adc->gain;
    status |= SENSOR_SpiTransceive(buffer, 3 , false);

	if (FPSENSOR_OK != status)
	{
		APP_PRINT_INFO0("Fpsensor config adc failed.\r\n");
		return FPSENSOR_SPI_ERROR;
	}

	return FPSENSOR_OK;
}


/*****************************************************************************
 函 数 名  : SENSOR_BaseInit
 功能描述  : Sensor基础配置
 输入参数  : void  
 输出参数  : 无
 返 回 值  : uint8_t
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年9月14日
    作    者   : Alex
    修改内容   : 新生成函数

*****************************************************************************/
uint8_t SENSOR_BaseInit(void)
{
	uint8_t status = 0;
	uint8_t buffer[16] = { 0 };

	status |= SENSOR_HardReset();
	status |= SENSOR_ReadIrqWithClear(buffer, sizeof(buffer));
	if(print_id)
	{
		status |= SENSOR_HWID();
		status |= SENSOR_VID();
	}

	buffer[0] = FPSENSOR_REG_FINGER_DRIVE_CONF; 
	buffer[1] = 0x92;
	status |= SENSOR_SpiTransceive(buffer, 2 , false);

	buffer[0] = FPSENSOR_REG_IMAGE_SETUP;
	buffer[1] = (1<<5)/*invert*/ | (1 << 6);
	buffer[1] |= (0x1F & 9);
	status |= SENSOR_SpiTransceive(buffer, 2, false);

	buffer[0] = FPSENSOR_REG_ANA_CFG2;
	buffer[1] = 0xb6;
	buffer[2] = 0x17;
	buffer[3] = 0x20;
	buffer[4] = 0x00;
	buffer[5] = 0x00;
	buffer[6] = 0xff;
	buffer[7] = 0x0c;
	buffer[8] = 0xb7;
	buffer[9] = 0x00;
	buffer[10] = 0x0f;
	status |= SENSOR_SpiTransceive(buffer, 11, false);

	buffer[0] = FPSENSOR_REG_ANA_CFG1;
	buffer[1] = 0x30;
	buffer[2] = 0x31;
	buffer[3] = 0x01;
	buffer[4] = 0x85;
	buffer[5] = 0x62;
	buffer[6] = 0x80;
	buffer[7] = 0x00;
	buffer[8] = 0xff;
	status |= SENSOR_SpiTransceive(buffer, 9, false);

	buffer[0] = FPSENSOR_REG_DBG_CFG;
	buffer[1] = 0x00;
	buffer[2] = 0x04;
	buffer[3] = 0x32;
	buffer[4] = 0x10;
	buffer[5] = 0x00;
	buffer[6] = 0x01;	
	status |= SENSOR_SpiTransceive(buffer, 7, false);

	buffer[0] = FPSENSOR_REG_IMG_RD;
	buffer[1] = 0x00;
	buffer[2] = 0x00;
	buffer[3] = 0x40;
	status |= SENSOR_SpiTransceive(buffer, 4, false);
    
	status |= fpsensor_set_adc(&adc_fixed);
   	SENSOR_Common();
	if (FPSENSOR_OK != status)
	{
		return FPSENSOR_SPI_ERROR;
	}

	return FPSENSOR_OK;
}

uint8_t fpsensor_reg_fngr_det_cntr(void)
{
	uint8_t status = 0;
	uint8_t buffer[5];
	buffer[0] = FPSENSOR_REG_FNGR_DET_CNTR;//
	buffer[1] = 0x00;
	buffer[2] = 0x00;
	buffer[3] = 0xFF; 
	buffer[4] = 0x30; 
	status |= SENSOR_SpiTransceive(buffer, 5 , false);
	return status;
}

uint8_t fpsensor_unknow_cmd(void)
{
	uint8_t status = 0;
	uint8_t buffer[7];
	buffer[0] = 0x54;//
	buffer[1] = 0x00;
	buffer[2] = 0x04;
	buffer[3] = 0x32; 
	buffer[4] = 0x10; 
	buffer[5] = 0x00; 
	buffer[6] = 0x01; 
	status |= SENSOR_SpiTransceive(buffer, 7 , false); 
	return status;
}

uint8_t fpsensor_finger_drive_config(void)
{
	uint8_t status = 0;
	uint8_t buffer[2];
	buffer[0] = FPSENSOR_REG_FINGER_DRIVE_CONF; 
	buffer[1] = 0x82;
	status |= SENSOR_SpiTransceive(buffer, 2 , false);
	return status;

}
uint8_t fpsensor_image_rd(uint8_t data)
{
	uint8_t buffer[4] = {0};
	uint8_t status = 0;
	buffer[0] = FPSENSOR_REG_IMG_RD;
    buffer[1] = 0x00;
	buffer[2] = 0x00;
	buffer[3] = data;
    status = SENSOR_SpiTransceive(buffer, 4 , false);
	return status;
}
uint8_t fpsensor_smpl_setup(void)
{
	uint8_t buffer[4] = {0};
	uint8_t status = 0;
	buffer[0] = FPSENSOR_REG_IMG_SMPL_SETUP;
	buffer[1] = 0x00;
	buffer[2] = 0xFF;
	buffer[3] = 0x11;
	status |= SENSOR_SpiTransceive(buffer, 4 , false);
	return status;
}

uint8_t fpsensor_reg_ana_cfg1(void)
{
	uint8_t buffer[9] = {0};
	uint8_t status = 0;
	buffer[0] = FPSENSOR_REG_ANA_CFG1;
	buffer[1] = 0x30;
	buffer[2] = 0x31;
	buffer[3] = 0x01; 
	buffer[4] = 0x85;  
	buffer[5] = 0x62;
	buffer[6] = 0x80;
	buffer[7] = 0x00; 
	buffer[8] = 0xFF;  
	status |= SENSOR_SpiTransceive(buffer, 9 , false); 
	return status;
}
uint8_t fpsensor_reg_ana_cfg2(void)
{
	uint8_t buffer[11] = {0};
	uint8_t status = 0;
	buffer[0] = FPSENSOR_REG_ANA_CFG2;//
	buffer[1] = 0xB6;
	buffer[2] = 0x17;
	buffer[3] = 0x20; 
	buffer[4] = 0x00;  
	buffer[5] = 0x00;
	buffer[6] = 0xFF;
	buffer[7] = 0x0C; 
	buffer[8] = 0xB7;  
	buffer[9] = 0x00; 
	buffer[10] = 0x0F; 
	status |= SENSOR_SpiTransceive(buffer, 11 , false);
	return status;
}

/*****************************************************************************
 函 数 名  : SENSOR_AdcInit
 功能描述  : Sensor传感器adc初始化
 输入参数  : uint8_t shift, uint8_t gain, uint8_t pixel, uint8_t et1  
 输出参数  : 无
 返 回 值  : uint8_t
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年9月14日
    作    者   : Alex
    修改内容   : 新生成函数

*****************************************************************************/
uint8_t SENSOR_AdcInit(uint8_t shift, uint8_t gain, uint8_t pixel, uint8_t et1)
{
	//shift range 0-31, gain range 0-15, pixel range: 0 4 16 20
	adc_fixed.shift = shift > 31 ? 31 : shift;
	adc_fixed.gain  = gain > 15 ? 15 : gain;
	adc_fixed.pixel = pixel < 2 ? 0 : pixel < 10 ? 4 : pixel < 18 ? 16 : 20;
	adc_fixed.et1   = et1;

	return 0;
}

uint8_t SENSOR_AdcInit_1(uint8_t shift, uint8_t gain, uint8_t pixel, uint8_t et1)
{
	adc_fixed.gain	= gain > 15 ? 15 : gain;
	//adc_fixed.pixel = pixel < 2 ? 0 : pixel < 10 ? 4 : pixel < 18 ? 16 : 20;
	adc_fixed.et1	= et1;
	adc_fixed.shift = shift > 63 ? 63 : shift;
	adc_fixed.pixel = pixel > 7 ? 7:pixel ;
	
	return 0;
}


/*****************************************************************************
 函 数 名  : SENSOR_AlgoInit
 功能描述  : Sensor传感器初始化
 输入参数  : void  
 输出参数  : 无
 返 回 值  : uint8_t
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年9月14日
    作    者   : Alex
    修改内容   : 新生成函数

*****************************************************************************/
uint8_t SENSOR_AlgoInit(void)
{	
	uint8_t status = 0;

	status = SENSOR_BaseInit();
	if (status == FPSENSOR_OK)
	{
		SENSOR_AdcInit_1(30, 9, 4, 3);//7332
	}
	
	return status;
}

/*****************************************************************************
 函 数 名  : SENSOR_RecentIndexInit
 功能描述  : 匹配排序初始化
 输入参数  : void  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2019年12月18日
    作    者   : Alex
    修改内容   : 新生成函数

*****************************************************************************/
void SENSOR_RecentIndexInit(void)
{
	uint32_t i = 0;
	uint32_t uwCRCValue = 0, uwCRCTmp = 0;

	uwCRCTmp = 	 g_RecentFingerIndex[STORE_MAX_FTR+0] << 24 
				|g_RecentFingerIndex[STORE_MAX_FTR+1] << 16
				|g_RecentFingerIndex[STORE_MAX_FTR+2] << 8 
				|g_RecentFingerIndex[STORE_MAX_FTR+3];
	uwCRCValue = CRC32_calc(g_RecentFingerIndex , STORE_MAX_FTR);
	
	if (uwCRCTmp != uwCRCValue)
	{
		for(i=0 ; i<STORE_MAX_FTR ; i++)
		{
			g_RecentFingerIndex[i] = i;
		}

		uwCRCValue = CRC32_calc(g_RecentFingerIndex , STORE_MAX_FTR);
		g_RecentFingerIndex[STORE_MAX_FTR+0] = (uwCRCValue&0xff000000)>>24;
		g_RecentFingerIndex[STORE_MAX_FTR+1] = (uwCRCValue&0x00ff0000)>>16;
		g_RecentFingerIndex[STORE_MAX_FTR+2] = (uwCRCValue&0x0000ff00)>>8;
		g_RecentFingerIndex[STORE_MAX_FTR+3] = (uwCRCValue&0x000000ff);
		
	}

	if (g_RecentFingerIndex[0] == g_RecentFingerIndex[1] || g_RecentFingerIndex[1] == g_RecentFingerIndex[2])
	{
		for(i=0 ; i<STORE_MAX_FTR ; i++)
		{
			g_RecentFingerIndex[i] = i;
		}
		APP_PRINT_INFO0("crc test error!\r\n");
	}

}

/*****************************************************************************
 函 数 名  : SENSOR_Init
 功能描述  : Sensor初始化
 输入参数  : uint8_t  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年9月14日
    作    者   : Alex
    修改内容   : 新生成函数

*****************************************************************************/
uint8_t SENSOR_Init(void)
{
	uint8_t state;
	//printf("SENSOR_Init.\r\n");

    SENSOR_RecentIndexInit();

	SENSOR_RoiCapInit();
	//SENSOR_SpiInit();
	//SENSOR_PinInit();
	//SENSOR_Reset();
       
    //AlgoGlobalInit();
    
	//算法相关初始化,默认注册时拼接6次
    //COMMAND_First_Enroll_AfisInit(g_sysSetting.alg_enroll_num, 0);
	
	state = SENSOR_AlgoInit();
	if (state != FPSENSOR_OK)
	{
		goto ERR;
	}

    APP_PRINT_INFO0("sensor SPI Init .............................................. [ok]\r\n");
    return SENSOR_OK;
    
ERR:

    APP_PRINT_INFO0("sensor SPI Init ............................................ [fail]\r\n");
    return SENSOR_INIT_ERROR;
}


sensor_comp_code Sensor_Init(void)
{
	uint8_t state;

    APP_PRINT_INFO0("SENSOR_Init.\r\n");

    Ucas_memInit();

    spi_sem_check();
    Sensor_Reset();
    spi_sem_give();

    SENSOR_RecentIndexInit();

	SENSOR_RoiCapInit();
	//SENSOR_SpiInit();
	//SENSOR_PinInit();
	//SENSOR_Reset();

    //AlgoGlobalInit();
    
	//算法相关初始化,默认注册时拼接6次
    //COMMAND_First_Enroll_AfisInit(g_sysSetting.alg_enroll_num, 0);
	
	state = SENSOR_AlgoInit();
	if (state != FPSENSOR_OK)
	{
		goto ERR;
	}

    APP_PRINT_INFO0("sensor SPI Init .............................................. [ok]\r\n");
    return SENSOR_COMP_CODE_OK;
    
ERR:

    APP_PRINT_INFO0("sensor SPI Init ............................................ [fail]\r\n");
    return SENSOR_COMP_CODE_HARDWARE;
}

sensor_comp_code Sensor_InitEx(void)
{
	uint8_t state;

    Ucas_memInit();

    spi_sem_check();
    Sensor_Reset();
    spi_sem_give();

	SENSOR_RoiCapInit();
	
	state = SENSOR_AlgoInit();
	if (state != FPSENSOR_OK)
	{
		goto ERR;
	}

    return SENSOR_COMP_CODE_OK;
    
ERR:
    APP_PRINT_INFO0("sensor SPI InitEx ............................................ [fail]\r\n");
    
    return SENSOR_COMP_CODE_HARDWARE;
}



uint8_t fpsensor_set_capture_crop(uint8_t *buffer,
											   uint32_t length,
							                   uint32_t rowStart,
							                   uint32_t rowCount,
											   uint32_t colStart,
							                   uint32_t colGroup)
{
	if(buffer == NULL || length < 5)
	{
		return FPSENSOR_BUFFER_ERROR;
	}
	
    buffer[0] = FPSENSOR_REG_IMG_CAPT_SIZE;
    buffer[1] = rowStart;
    buffer[2] = rowCount;
    buffer[3] = colStart;
    buffer[4] = colGroup;

    return SENSOR_SpiTransceive(buffer, 5 , false);
}

uint8_t fpsensor_capture_image()
{
	uint8_t buffer[1] = {FPSENSOR_CMD_CAPTURE_IMAGE};
 	return SENSOR_SpiTransceive(buffer, 1 , false);
}


sensor_comp_code Sensor_WaitAndCapture(uint32_t wait_time)
{
	uint32_t t1 = 0;
	uint8_t ret;

    Sensor_InitEx();

    while (t1 < (wait_time/10))
    {
    	if (g_QuitAtOnce)
        {
        	g_QuitAtOnce = 0;
            return SENSOR_COMP_CODE_QUIT;
        }

		if (MLAPI_QueryFingerPresent())
        {            
			SENSOR_AdjustGainStart(SPI_IMAGE_BUFFER);	//自动增益调整
			SENSOR_AdcSetup(g_capture_shift, g_capture_gain, g_capture_pxlctrl);
            
			ret = SENSOR_ReadImage_1(SPI_IMAGE_BUFFER);
			if (ret != 0)
            {
                 return SENSOR_COMP_CODE_UNQUALIFIED;
            }
			return SENSOR_COMP_CODE_OK;
        }
        else
        {
        	app_wdg_reset();
            delay_ms(10);
        }
		
		menu_sleep_event_control(MENU_SLEEP_EVENT_WAKEUP_BIT, true);
		t1++;
    }

	return SENSOR_COMP_CODE_NO_FINGER;
}

bool check_sensor_int(void)
{
 Pad_Config(SENSOR_INT_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE, PAD_OUT_LOW);
 Pinmux_Config(SENSOR_INT_PIN, DWGPIO);
 
    RCC_PeriphClockCmd(APBPeriph_GPIO, APBPeriph_GPIO_CLOCK, ENABLE);

   /// GPIO_InitTypeDef GPIO_InitStruct;
   // GPIO_StructInit(&GPIO_InitStruct);
   // GPIO_InitStruct.GPIO_Pin        = GPIO_GetPin(H_1);
   // GPIO_InitStruct.GPIO_Mode       = GPIO_Mode_IN;
   // GPIO_InitStruct.GPIO_ITCmd      = DISABLE;
   // GPIO_Init(&GPIO_InitStruct);

 if(GPIO_ReadInputDataBit(GPIO_GetPin(SENSOR_INT_PIN)))
 {
  WDG_SystemReset(RESET_ALL_EXCEPT_AON, RESET_REASON_HW);
  return true;
 }  
 else
 {
  Pad_Config(SENSOR_INT_PIN, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_DOWN, PAD_OUT_DISABLE, PAD_OUT_LOW);
  System_WakeUpPinEnable(SENSOR_INT_PIN, PAD_WAKEUP_POL_HIGH, PAD_WK_DEBOUNCE_DISABLE);
  return false;
 }  
}

