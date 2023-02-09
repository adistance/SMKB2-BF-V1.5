#include "stdio.h"
#include "string.h"
#include "math.h"
#include "stdlib.h"
#include "trace.h"

#include "mlapi.h"
#include "SensorAdjust.h"
#include "sensor.h"
#include "fpsapi.h"
//#include "delay.h"

#include "system_setting.h"

//采图增益值
extern int GetAfisMemory(uint8_t **ppMem, uint32_t *pLen);
//extern void Ucas_memInit(void);

extern uint8_t g_capture_shift;
extern uint8_t g_capture_gain;
extern uint8_t g_capture_pxlctrl;
extern uint8_t g_QuitAtOnce;
extern SYS_SETTING g_sysSetting;

extern void SENSOR_AdcSetup(uint8_t shift, uint8_t gain, uint8_t pxl);
extern uint32_t SENSOR_CaptureImage(uint8_t *pImg, PST_ROI pRoi);

typedef struct 
{
	int32_t nTimes;
	int32_t nLastIshift;
	int32_t bFixShift;
}GainSymbol_t;


GainSymbol_t g_GainSymbol = {0};
#define QUERY_FINGER_CYCLE                                          (10)


uint32_t AdjustGainSet(int8_t ishift, int8_t igain, int8_t ipxlctl) 
{
    int8_t shift0, gain0, pxlCtrl0;
    int8_t shift1, gain1, pxlCtrl1;
	int32_t oShift, oGain;

	oShift = ishift;
	oGain = igain;

	shift0	 = g_capture_shift;
	gain0	 = g_capture_gain;
	pxlCtrl0 = g_capture_pxlctrl;// & 0x14;	

	if(ishift > 7)
		ishift = 7;
	if(ishift < -7)
		ishift = -7;
	
    ishift    = shift0 + ishift;
    igain     = gain0 + igain;
    ipxlctl   = pxlCtrl0 + ipxlctl;
 
	ishift = MAX(0, ishift);
	shift1 = MIN(ishift, 63);

	igain = MAX(0, igain);
	gain1 = MIN(igain, 15);
    pxlCtrl1 = ipxlctl; 
//    if ((pxlCtrl1 != 0)&&(pxlCtrl1 != 4)&&(pxlCtrl1 != 16)&&(pxlCtrl1 != 20))
	if(pxlCtrl1 < 0 ||pxlCtrl1 > 7)
    {
        pxlCtrl1 = pxlCtrl0;
    }

    g_capture_shift = shift1;
    g_capture_gain = gain1;
    g_capture_pxlctrl = pxlCtrl1;

	if((0 == oGain || (g_capture_gain == 0) || (g_capture_gain == 15)) 
		&& ((g_GainSymbol.nLastIshift * oShift) < 0))
	{
		g_GainSymbol.nLastIshift = 0;
		return 0;
	}

	if(oShift != 0)
		g_GainSymbol.nLastIshift = oShift;
	
    if ((shift1 == shift0) && (gain1 == gain0) && (pxlCtrl1 == pxlCtrl0)) 
    {
		g_GainSymbol.nLastIshift = 0;
        return 0;
    } 
    else 
    {
        return 1;
    }
}


int Adjust_HistMean(unsigned int *pHist, int nSize)
{
	int i = 0;
	int nVal = 0;

	for (i = 0; i < 256; ++i)
	{
		nVal += i * pHist[i];
	}

	return (nVal / nSize);
}

int Adjust_ImgMean(unsigned char *pImgs, int nSize)
{
	int i = 0;
	int nVal = 0;

	for (i = 0; i < nSize; ++i)
	{
		nVal += pImgs[i];
	}

	return (nVal / nSize);
}


int Adjust_HistVar(unsigned int *pHist, int nMean, unsigned int nSize)
{
	int i = 0;
	int nEnd = 256 - nMean;
	int nVal = 0;
	unsigned int *pHistCur = pHist;
	for (i = -nMean, pHistCur = pHist; i < nEnd; ++i, ++pHistCur)
	{
		nVal += i * i * *pHistCur;
	}

	return nVal / nSize;
}

uint32_t Adjust_is_Image_white(unsigned char *buf, int size)
{
    unsigned int p[256];
    int nMean = 0;
    int nVar = 0;
	int i;
	unsigned char *pBuff;

	memset(p, 0, 256 * sizeof(unsigned int));
	pBuff = buf;
	for (i = 0; i < size; i ++, pBuff++)
	{
		p[*pBuff] += 1;
	}
   
    nMean = Adjust_HistMean(p, size);
    nVar = Adjust_HistVar(p, nMean, size);

    if(nVar < 100)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}


void Adjust_GetImageHistogram(unsigned char *pImgBuffIn, int nRows, int nCols, unsigned int *pHist)
{
	int i;
	int nSize;
	unsigned char *pBuff;

	memset(pHist, 0, 256 * sizeof(unsigned int));
	nSize = nRows * nCols;
	pBuff = pImgBuffIn;
	for (i = 0; i < nSize; i ++, pBuff ++)
	{
		pHist[*pBuff] += 1;
	}
}


uint32_t AdjustGain(uint8_t *pSrc, uint8_t nCols, uint8_t nRows)
{
    unsigned int p[256];
    unsigned int *pHist = p;
    int32_t nShiftOff = 0;
    int32_t nGainOff = 0;
    int32_t nPxlOff = 0;
	int32_t nSize = nCols * nRows;
	int32_t nMean;
	int32_t nVar;

    Adjust_GetImageHistogram(pSrc, nRows, nCols, pHist);
    nMean = Adjust_HistMean(pHist, nSize);
	nVar = Adjust_HistVar(pHist, nMean, nSize);

	if(nMean > 148 || nMean < 98)
	{      
	    nShiftOff = (nMean - 128) >> 3;
	    nShiftOff = -nShiftOff;
	}

	if(nVar < 1000 || nVar > 3500)
	{
		if(!g_GainSymbol.bFixShift)
		{
			nGainOff = (nVar - 2250) / 600;
			nGainOff = -nGainOff;
		}
	}
    else
    { 
        g_GainSymbol.bFixShift = 1;
    }

#if 0
	printf("nMean: %d, nVar = %d, nShiftOff = %d, nGainOff = %d\r\n", 
		nMean, nVar, nShiftOff, nGainOff);
		
	printf("g_capture_shift: %d, g_capture_gain = %d, g_capture_pxlctrl = %d\r\n", 
		g_capture_shift, g_capture_gain, g_capture_pxlctrl);
#endif


    return AdjustGainSet(nShiftOff, nGainOff, nPxlOff);
   
}


void AdjustRoiInit(ST_ROI_CTRL *pRoiCtrl)
{
	memset((unsigned char *)pRoiCtrl, 0, sizeof(ST_ROI_CTRL));

	pRoiCtrl->stRoi[0].width = 64;
	pRoiCtrl->stRoi[0].hight = 64;
	pRoiCtrl->stRoi[0].x = (FPSENSOR_IMAGE_WIDTH - pRoiCtrl->stRoi[0].width) / 2;//0
	pRoiCtrl->stRoi[0].y = (FPSENSOR_IMAGE_HEIGHT - pRoiCtrl->stRoi[0].hight) / 2;//8

	pRoiCtrl->stRoi[1].width = 32;
	pRoiCtrl->stRoi[1].hight = 32;
	pRoiCtrl->stRoi[1].x = pRoiCtrl->stRoi[0].x;//0
	pRoiCtrl->stRoi[1].y = pRoiCtrl->stRoi[0].y;//8

	pRoiCtrl->stRoi[2].width = 32;
	pRoiCtrl->stRoi[2].hight = 32;
	pRoiCtrl->stRoi[2].x = pRoiCtrl->stRoi[1].x + 32;//32
	pRoiCtrl->stRoi[2].y = pRoiCtrl->stRoi[1].y;//8

	pRoiCtrl->stRoi[3].width = 32;
	pRoiCtrl->stRoi[3].hight = 32;
	pRoiCtrl->stRoi[3].x = pRoiCtrl->stRoi[1].x;//0
	pRoiCtrl->stRoi[3].y = pRoiCtrl->stRoi[1].y + 32;//40

	pRoiCtrl->stRoi[4].width = 32;
	pRoiCtrl->stRoi[4].hight = 32;
	pRoiCtrl->stRoi[4].x = pRoiCtrl->stRoi[1].x + 32;//32
	pRoiCtrl->stRoi[4].y = pRoiCtrl->stRoi[1].y + 32;//40


	pRoiCtrl->roi_num = 5;
}

int GetRoiIdx(unsigned char *pImg, int nRows, int nCols, int nThe)
{
	int i = 0, j = 0;
	int pCnt[2][2] = {0};
	int *pCntCur = pCnt[0];
	int nRMid = nRows >> 1;
	int nCMid = nCols >> 1;
	unsigned char *pImgCur = pImg;
	int nMin1, nMin2;

	for (i = 0; i < nRows; ++i)
	{
		pCntCur = pCnt[i >= nRMid];
		for (j = 0; j < nCols; ++j)
		{
			if (*pImgCur < nThe)
				++pCntCur[j >= nCMid];

			++pImgCur;
		}
	}

	nMin1 = MAX(pCnt[0][0], pCnt[0][1]);
	nMin2 = MAX(pCnt[1][0], pCnt[1][1]);
	nMin1 = MAX(nMin1, nMin2);

	if (pCnt[0][0] == nMin1)
		return 1;
	else if (pCnt[0][1] == nMin1)
		return 2;
	else if (pCnt[1][0] == nMin1)
		return 3;
	else
		return 4;
}


void AdjustCropImg(unsigned char *pSrc, int nRows, int nCols, 
ST_ROI *pRoi, int dx, int dy, unsigned char *pDest)
{
	unsigned char *pImgBuffInCurr;
	unsigned char *pImgBuffOutCurr;
	int i;

	pImgBuffInCurr = pSrc + (pRoi->y - dy) * nCols + (pRoi->x - dx);
	pImgBuffOutCurr = pDest;

	for (i = 0; i < pRoi->hight; ++i)
	{
		memcpy(pImgBuffOutCurr, pImgBuffInCurr, pRoi->width);
		pImgBuffOutCurr += pRoi->width;
		pImgBuffInCurr += nCols;
	}
}

/*****************************************************************************
 函 数 名  : Sensor_FingerAdjustGain
 功能描述  : 自动增益调整
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年10月25日
    作    者   : li tian bao
    修改内容   : 新生成函数

*****************************************************************************/
uint32_t SENSOR_AdjustGainStart_1(uint8_t *pImgBuf)
{
	uint32_t ret = 0;
	uint32_t i;
	int idx;
	ST_ROI_CTRL stRoiCtrl;
	ST_ROI stRoi;
	int nMean;

	if (g_QuitAtOnce)
		 return SENSOR_ERROR_FORCE_QUIT;

	g_capture_shift = 22;	//g_sysSetting.fp_shift;
	g_capture_gain = 4;		//g_sysSetting.fp_gain;
	g_capture_pxlctrl = 7;	//g_sysSetting.fp_pxlctrl;

	g_GainSymbol.nTimes = 0;
	g_GainSymbol.nLastIshift = 0;
	g_GainSymbol.bFixShift = 0;

	AdjustRoiInit(&stRoiCtrl);
	SENSOR_AdcSetup(g_capture_shift, g_capture_gain, g_capture_pxlctrl);
	SENSOR_CaptureImage(pImgBuf, &stRoiCtrl.stRoi[0]);

	nMean = Adjust_ImgMean(pImgBuf, stRoiCtrl.stRoi[0].hight*stRoiCtrl.stRoi[0].width);

	idx = GetRoiIdx(pImgBuf, stRoiCtrl.stRoi[0].hight, stRoiCtrl.stRoi[0].width, nMean);
	stRoi = stRoiCtrl.stRoi[idx];

	AdjustCropImg(pImgBuf, stRoiCtrl.stRoi[0].hight, stRoiCtrl.stRoi[0].width, 
		&stRoi, 12, 24, pImgBuf);
	
	ret = AdjustGain(pImgBuf, stRoi.width, stRoi.hight);
	if (0 == ret)
		return SENSOR_OK;
	
    for(i = 1; i < QUERY_FINGER_CYCLE; i++)
    {   
        if (g_QuitAtOnce)
            return SENSOR_ERROR_FORCE_QUIT;

        SENSOR_AdcSetup(g_capture_shift, g_capture_gain, g_capture_pxlctrl);
        		
        SENSOR_CaptureImage(pImgBuf, &stRoi);

        ret = AdjustGain(pImgBuf, stRoi.width, stRoi.hight);  

		if (0 == ret) 
		{
			printf("g_capture_shift = %d, g_capture_gain = %d, g_capture_pxlctrl = %d\r\n",g_capture_shift, g_capture_gain, g_capture_pxlctrl);
			return SENSOR_OK;
		}
            
    }

    return ret;
}

/*****************************************************************************
 函 数 名  : Sensor_FingerAdjustGain
 功能描述  : 自动增益调整
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年10月25日
    作    者   : li tian bao
    修改内容   : 新生成函数

*****************************************************************************/
uint32_t SENSOR_AdjustGainStart(uint8_t *pImgBuf)
{
	uint32_t ret = 0;
	uint32_t i;
	int idx;
	ST_ROI_CTRL stRoiCtrl;
	ST_ROI stRoi;
	int nMean;

	if (g_QuitAtOnce)
		 return SENSOR_ERROR_FORCE_QUIT;

	g_capture_shift = g_sysSetting.fp_shift;
	g_capture_gain = g_sysSetting.fp_gain;
	g_capture_pxlctrl = g_sysSetting.fp_pxlctrl;

	g_GainSymbol.nTimes = 0;
	g_GainSymbol.nLastIshift = 0;
	g_GainSymbol.bFixShift = 0;

	AdjustRoiInit(&stRoiCtrl);
	SENSOR_AdcSetup(g_capture_shift, g_capture_gain, g_capture_pxlctrl);
	SENSOR_CaptureImage(pImgBuf, &stRoiCtrl.stRoi[0]);

	nMean = Adjust_ImgMean(pImgBuf, stRoiCtrl.stRoi[0].hight*stRoiCtrl.stRoi[0].width);

	idx = GetRoiIdx(pImgBuf, stRoiCtrl.stRoi[0].hight, stRoiCtrl.stRoi[0].width, nMean);
	stRoi = stRoiCtrl.stRoi[idx];

	AdjustCropImg(pImgBuf, stRoiCtrl.stRoi[0].hight, stRoiCtrl.stRoi[0].width, 
		&stRoi, 0, 8, pImgBuf);
	
	ret = AdjustGain(pImgBuf, stRoi.width, stRoi.hight);
	if (0 == ret)
		return SENSOR_OK;
	
    for(i = 1; i < QUERY_FINGER_CYCLE; i++)
    {   
        if (g_QuitAtOnce)
            return SENSOR_ERROR_FORCE_QUIT;

        SENSOR_AdcSetup(g_capture_shift, g_capture_gain, g_capture_pxlctrl);
        		
        SENSOR_CaptureImage(pImgBuf, &stRoi);

        ret = AdjustGain(pImgBuf, stRoi.width, stRoi.hight);  

		if (0 == ret) 
            return SENSOR_OK;
    }

    return ret;
}

void Adjust_ImageProgerss(unsigned char *pIn1, unsigned char *pIn2, unsigned char *pIn3,
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

int Adjust_ImageClassification(unsigned char *pdata1, unsigned char *pdata2, int size)
{
	int i = 0;
	int nDifNum = 0;
	const int nDifThe = 22;
	const int nDifNumThe = 270;
	const int nDifNumThe2 = 4000;
	int val = 0;

	for (i = 0; i < size; ++i)
	{
		val = (int)pdata1[i] - (int)pdata2[i];
		if (abs(val) > nDifThe)
		{
			++nDifNum;
		}
	}
	
	if (nDifNum > nDifNumThe && nDifNum < nDifNumThe2)
		return 1;
	else
		return 0;
}


uint8_t Adjust_read_image_Wave_filtering(uint8_t *image)
{
    uint8_t *p_ImgBuf1;
    uint8_t *p_ImgBuf2;
    uint8_t *p_ImgBuf3;
    uint8_t *p_ImgBuf4;
    uint8_t *p_ImgBuf5;
    uint32_t pLen;

    memset(image, 0xFF,FPSENSOR_IMAGE_HEIGHT*FPSENSOR_IMAGE_WIDTH+2);

    if(0 == GetAfisMemory(&p_ImgBuf1, &pLen))   // will retrun 72KB Ramspace
    {
        if(pLen >= 5 * FPSENSOR_IMAGE_SIZE)
        {
            p_ImgBuf2 = p_ImgBuf1 + FPSENSOR_IMAGE_SIZE;
            p_ImgBuf3 = p_ImgBuf2 + FPSENSOR_IMAGE_SIZE;
            p_ImgBuf4 = p_ImgBuf3 + FPSENSOR_IMAGE_SIZE;
            p_ImgBuf5 = p_ImgBuf4 + FPSENSOR_IMAGE_SIZE;
            
            p_ImgBuf1 += 8;
            p_ImgBuf2 += 8;
            p_ImgBuf3 += 8;
            p_ImgBuf4 += 8;
            p_ImgBuf5 += 8;
        }
        else
        {
            APP_PRINT_INFO0("GetAfis Memory  len err\r\n");
            Ucas_memInit();      //free Ramspace 
            return 1;
        }
    }
    else
    {
        APP_PRINT_INFO0("GetAfis Memory err\r\n");
        Ucas_memInit();      //free Ramspace 
        return 1;
    }

    /************** Take the 1st picture. If it is a white picture, return ********************/
    //SENSOR_FpCaptureImageEx(p_ImgBuf1, FPSENSOR_IMAGE_HEIGHT*FPSENSOR_IMAGE_WIDTH+2, 10);
	SENSOR_FpCaptureImage_1(p_ImgBuf1, 0, 0, FPSENSOR_IMAGE_WIDTH, FPSENSOR_IMAGE_HEIGHT, 200);
    /************** Take the 2th picture. If it is a white picture, go out ********************/
    //SENSOR_FpCaptureImageEx(p_ImgBuf2, FPSENSOR_IMAGE_HEIGHT*FPSENSOR_IMAGE_WIDTH+2, 10);
	SENSOR_FpCaptureImage_1(p_ImgBuf2, 0, 0, FPSENSOR_IMAGE_WIDTH, FPSENSOR_IMAGE_HEIGHT, 200);

    if (Adjust_is_Image_white(p_ImgBuf2 +(FPSENSOR_IMAGE_HEIGHT-2)* FPSENSOR_IMAGE_WIDTH, 2*FPSENSOR_IMAGE_WIDTH))
    {
        memcpy(image, p_ImgBuf1, FPSENSOR_IMAGE_HEIGHT*FPSENSOR_IMAGE_WIDTH+2);
        goto out;
    }

    /************** compare the 1st and 2th pictures, Determine whether filtering is needed********************/
    if (Adjust_ImageClassification(p_ImgBuf1, p_ImgBuf2, FPSENSOR_IMAGE_HEIGHT*FPSENSOR_IMAGE_WIDTH+2 ))
    {
        //SENSOR_FpCaptureImageEx(p_ImgBuf3, FPSENSOR_IMAGE_HEIGHT*FPSENSOR_IMAGE_WIDTH+2, 10);
		SENSOR_FpCaptureImage_1(p_ImgBuf3, 0, 0, FPSENSOR_IMAGE_WIDTH, FPSENSOR_IMAGE_HEIGHT, 200);
        /************** compare the 1st and 3th pictures, Judge whether sliding****************/
        if (0 == Adjust_ImageClassification(p_ImgBuf1, p_ImgBuf3, FPSENSOR_IMAGE_HEIGHT*FPSENSOR_IMAGE_WIDTH+2 ))
        {
            memcpy(image, p_ImgBuf1, FPSENSOR_IMAGE_HEIGHT*FPSENSOR_IMAGE_WIDTH+2);
            goto out;
        } 
        /************** Judge whether take the white picture****************/
        if (Adjust_is_Image_white(p_ImgBuf3 +(FPSENSOR_IMAGE_HEIGHT-2)* FPSENSOR_IMAGE_WIDTH, 2*FPSENSOR_IMAGE_WIDTH))
        {
            memcpy(image, p_ImgBuf1, FPSENSOR_IMAGE_HEIGHT*FPSENSOR_IMAGE_WIDTH+2);
            goto out;
        }
        
        //SENSOR_FpCaptureImageEx(p_ImgBuf4, FPSENSOR_IMAGE_HEIGHT*FPSENSOR_IMAGE_WIDTH+2, 10);
        //SENSOR_FpCaptureImageEx(p_ImgBuf5, FPSENSOR_IMAGE_HEIGHT*FPSENSOR_IMAGE_WIDTH+2, 10);
        SENSOR_FpCaptureImage_1(p_ImgBuf4, 0, 0, FPSENSOR_IMAGE_WIDTH, FPSENSOR_IMAGE_HEIGHT, 200);
		SENSOR_FpCaptureImage_1(p_ImgBuf5, 0, 0, FPSENSOR_IMAGE_WIDTH, FPSENSOR_IMAGE_HEIGHT, 200);
        /************** compare the 1st and 5th pictures, Judge whether sliding****************/
        if (0 == Adjust_ImageClassification(p_ImgBuf1, p_ImgBuf5, FPSENSOR_IMAGE_HEIGHT*FPSENSOR_IMAGE_WIDTH+2 ))
        {
            memcpy(image, p_ImgBuf1, FPSENSOR_IMAGE_HEIGHT*FPSENSOR_IMAGE_WIDTH+2);
            goto out;
        } 

        Adjust_ImageProgerss(p_ImgBuf1, p_ImgBuf2, p_ImgBuf3, p_ImgBuf4, p_ImgBuf5, FPSENSOR_IMAGE_HEIGHT*FPSENSOR_IMAGE_WIDTH+2, image);
    }
    
    /************** Judge whether the last image is white****************/
    if (Adjust_is_Image_white(image, FPSENSOR_IMAGE_HEIGHT*FPSENSOR_IMAGE_WIDTH+2))
    {
        memcpy(image, p_ImgBuf1, FPSENSOR_IMAGE_HEIGHT*FPSENSOR_IMAGE_WIDTH+2);
    }

out:
    Ucas_memInit();      //free Ramspace 
    return 0;
}


