/******************************************************************************

                  版权所有 (C), 2014-2024, 深圳市魔力信息技术有限公司

 ******************************************************************************
  文 件 名   : fpsapi.h
  版 本 号   : 初稿
  作    者   : roy
  生成日期   : 2015年5月13日
  最近修改   :
  功能描述   : FPS API interface
  函数列表   :
  修改历史   :
  1.日    期   : 2015年5月13日
    作    者   : roy
    修改内容   : 创建文件
******************************************************************************/

#ifndef __FPS_API_H__
#define __FPS_API_H__

#define MITAFIS_API

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char           u8;
typedef unsigned short          u16;
typedef unsigned int            u32;
typedef signed char             s8;
typedef signed short            s16;
typedef signed int              s32;

typedef struct
{
	u8*    pData;           //Pointer to Raw Image Buffer
	u8     nHeight;         //Raw Image Height
	u8     nWidth;          //Raw Image Width
	u8     nBpp;            //Bits per Pixel
	u8     nReserved;       //For 4 bytes alignment, and reserved for future use
} RawImageInfo_t,*pRawImageInfo;

typedef struct
{
	u16 nLearnScoreThres;  //When Feature Matching, Match Score > nLearningScoreThres, Learn this feature
	u16 nMatchScoreThres;    //When Feature Matching, Match Score > nMatchScoreThres, Same Finger; Others, Different Finger.
                             //匹配得分超过该值则两枚指纹为同一手指；否则为不同手指；
	u16 nEnrollScoreThres;   //When Enrollment, Match Score >nEnrollScoreThres, Stitch Ftr; Others, No Stitching.
                             //注册时，匹配得分超过该值则拼接；否则则不拼接；
	u16 nMaxFtrSize;         //The limit of Enrollment Feature Size.
                             //指纹特征Buffer Size上限,若FeatureExtraction提取出特征超过上限则返回错误码
	u8  nEnrollMaxNum;       //The limit of finger number when enrollment.
                             //注册时输入指纹图像的最大数目；
	u8  nQualityControl;     //Image quality control parameter. 
                             //根据不同图像质量去决定Extraction的相关行为，取值范围0~3;0-严格，3-宽松。
	u8  nCoverageRate;       //Fingerprint image coverage rate, if < nCoverageRate, reject this finger.
	                         //指纹图像占整图的百分比，低于该百分比，该图像将被拒绝。
	s8  strUID[33];          //Chip UID，Verify version validation.
                             //芯片UID，用于验证版本有效性。
	u8  nEnrollStrategyCoverageTh;//输入特征与模板特征重合面积超过该阈值时，返回PR_ENROLL_ALMOST_NOMOVE，默认阈值为60，即（60%）
	u8  nFadingSpeed;         //学习速度
	u8  nMaxUpdateNum;        //最大更新次数
	u8  nMaxSetNum;           //学习到的特征幅数，不小于nEnrollMaxNum
	u16 nTestSingleScoreTh;   //注册时，测试是否同一手指的分数阈值
	//u16 nSensorResolution;    //Sensor分辨率(423/508)
	u16 nRegisterPoolSize;    //Sensor分辨率(423/508)
	u8  nRegisterMaxNum;
	u8  nMoistnessGoodFingerThr;         //注册时湿度阈值
    u8  nMoistnessUpdateThr;         //自学习湿度阈值
	u8  nUseNewEnrollStrategy;         //启用新注册策略开关 1：开 0：关 ；默认为1
	u8 nSelfLearningStrategy;//位开关。默认值255（策略全打开）。
	//1: 湿度更新策略；2：前景增益更新策略；4：面积增益更新策略；8：强制更新策略；16：强制更新策略中的匹配效率更新策略
	u8 nReserved[7];
} AfisParams_t,*pAfisParams;

/*****************************************************************************
 函 数 名  : SetAfisParams
 功能描述  : 设定指纹算法所需的控制参数，用于初始化。
 输入参数  : const AfisParams_t* pFpParams: 
            定义Extraction, Matching and Enrollment所需要的参数；
 输出参数  : 无
 返 回 值  : s32, 0--OK，other- Setting Fail.
 *****************************************************************************/
MITAFIS_API s32 SetAfisParams(const AfisParams_t* pFpParams);

/*****************************************************************************
 函 数 名  : GetDefaultAfisParams
 功能描述  : 得到指纹算法所需的控制参数默认，作为初始化的参考值。
 输入参数  : const AfisParams_t* pFpParams: 
            定义Extraction, Matching and Enrollment所需要的参数；
 输出参数  : 无
 返 回 值  : s32, 0--OK，other- Setting Fail.
 *****************************************************************************/
MITAFIS_API s32 GetDefaultAfisParams(AfisParams_t* pFpParams);

/*****************************************************************************
 函 数 名  : CalcMaxFtrSize
 功能描述  : 根据最大自学习指纹图像幅数，计算所需要的FTR空间大小。
 输入参数  : const s32 nMaxSetNum: 
            最大的自学习指纹图像幅数
 输出参数  : 无
 返 回 值  : s32, 特征所需要的空间大小，单位：字节.
 *****************************************************************************/
MITAFIS_API s32 CalcMaxFtrSize(const s32 nMaxSetNum);

/*****************************************************************************
 函 数 名  : GetAfisParams
 功能描述  : 得到指纹算法当前所用的控制参数，用于初始化。
 输入参数  : const AfisParams_t* pFpParams: 
            定义Extraction, Matching and Enrollment所需要的参数；
 输出参数  : 无
 返 回 值  : s32, 0--OK，other- Setting Fail.
 *****************************************************************************/
MITAFIS_API s32 GetAfisParams(AfisParams_t* pFpParams);

/*****************************************************************************
函 数 名 : ExtractFeature
功能描述 : 指纹特征提取算法
输入参数 : const RawImageInfo_t* pRawImage:
           ** Pointer to RawImage Info structure
		   ** 输入的指纹图像结构指针
		   u8* pFtrBuffer:
		   **  Pointer to storage buffer of extraction feature.
           **  此Buffer空间由外部开辟,空间大小上限为 888 Bytes,用来存储提取出来的指纹特征信息
		   u32* pBufferLength:
		   **  Pointer to feature buffer length 
		   **  输出实际的提取特征大小
返 回 值 : s32 32 bits Error Code
*****************************************************************************/
MITAFIS_API s32 ExtractFeature(const RawImageInfo_t* pRawImage, 
	u8* pFtrBuffer, u32* pBufferLength);

/*****************************************************************************
 函 数 名  : MatchFeature
 功能描述  : 指纹特征比对
 输入参数  : [input] u8 * pTplFtrBuff
             ** Pointer to enrolled template feature buffer.
			 ** 已注册的模板特征
             [input] u32 pTplFtrBuffLen
			 ** Pointer to enrolled template feature size.
			 ** 已注册的模板特征大小
             [input] u8 * pProbeFtrBuff
			 ** Pointer to probe feature buffer by way of feature extraction
			 ** 欲比对的输入手指的特征
             [input] u32 pProbeFtrBuffLen
			 ** Pointer to probe feature buffer size.
			 ** 欲比对的输入手指的特征大小
			 u32 *nScore
			 ** Pointer to Matching score.
			 ** 匹配分数，两枚手指的相似性评估
 返 回 值  : s32 32 bits Error Code
*****************************************************************************/
MITAFIS_API s32 MatchFeature(u8* pTplFtrBuff, u32 * pTplFtrBuffLen, 
	u8* pProbeFtrBuff, u32 pProbeFtrBuffLen, u32* nScore);

/*****************************************************************************
 函 数 名  : MatchFeatureForDebug
 功能描述  : 指纹特征比对, 返回自学习删除特征的索引
 输入参数  : [input] u8 * pTplFtrBuff
             ** Pointer to enrolled template feature buffer.
			 ** 已注册的模板特征
             [input] u32 pTplFtrBuffLen
			 ** Pointer to enrolled template feature size.
			 ** 已注册的模板特征大小
             [input] u8 * pProbeFtrBuff
			 ** Pointer to probe feature buffer by way of feature extraction
			 ** 欲比对的输入手指的特征
             [input] u32 pProbeFtrBuffLen
			 ** Pointer to probe feature buffer size.
			 ** 欲比对的输入手指的特征大小
			 u32 *nScore
			 ** Pointer to Matching score.
			 ** 匹配分数，两枚手指的相似性评估
			 s32 *pDeleteIndex
			 ** the index of the deleted template.
			 ** 自学习删除特征的索引
 返 回 值  : s32 32 bits Error Code
*****************************************************************************/
MITAFIS_API s32 MatchFeatureForDebug(u8* pTplFtrBuff, u32 * pTplFtrBuffLen, 
	u8* pProbeFtrBuff, u32 pProbeFtrBuffLen, u32* nScore, s32 *pDeleteIndex);


/*****************************************************************************
 函 数 名  : MatchFeatureForOverlapCheck
 功能描述  : 指纹特征比对, 返回是否与库里特征重合过大
 输入参数  : [input] u8 * pTplFtrBuff
             ** Pointer to enrolled template feature buffer.
			 ** 注册的模板特征
             [input] u32 pTplFtrBuffLen
			 ** Pointer to enrolled template feature size.
			 ** 注册的模板特征大小
             [input] u8 * pProbeFtrBuff
			 ** Pointer to probe feature buffer by way of feature extraction
			 ** 欲比对的输入手指的特征
             [input] u32 pProbeFtrBuffLen
			 ** Pointer to probe feature buffer size.
			 ** 欲比对的输入手指的特征大小
			 u32 *pTooOverlap
			 ** Pointer to check flag.
			 ** 输入特征与模板特征重合面积超过阈值（nEnrollStrategyCoverageTh）时返回1，没有超过则返回0
 返 回 值  : s32 32 bits Error Code
*****************************************************************************/
MITAFIS_API s32 MatchFeatureForOverlapCheck(u8* pTplFtrBuff, u32 nTplFtrBuffLen, 
	u8* pProbeFtrBuff, u32 nProbeFtrBuffLen, u32* pTooOverlap);


/*****************************************************************************
 函 数 名  : MatchFeatureForOverlap
 功能描述  : 指纹特征比对, 返回与模板中特征的最大重合量
 输入参数  : [input] u8 * pTplFtrBuff
             ** Pointer to enrolled template feature buffer.
			 ** 注册的模板特征
             [input] u32 pTplFtrBuffLen
			 ** Pointer to enrolled template feature size.
			 ** 注册的模板特征大小
             [input] u8 * pProbeFtrBuff
			 ** Pointer to probe feature buffer by way of feature extraction
			 ** 欲比对的输入手指的特征
             [input] u32 pProbeFtrBuffLen
			 ** Pointer to probe feature buffer size.
			 ** 欲比对的输入手指的特征大小
			 u32* pMaxOverlap
			 ** Pointer to the Maximum overlap.
			 ** 输入特征与模板特征中单个特征的最大重合良，匹配不上则为0
 返 回 值  : s32 32 bits Error Code
*****************************************************************************/
MITAFIS_API s32 MatchFeatureForOverlap(u8* pTplFtrBuff, u32 nTplFtrBuffLen, 
	u8* pProbeFtrBuff, u32 nProbeFtrBuffLen, u32* pMaxOverlap);


/*****************************************************************************
函 数 名 : EnrollFingerChip
功能描述 : 指纹注册过程
输入参数 : const RawImageInfo_t* pRawImage
           ** Pointer to RawImage_Info structure
		   ** 输入的指纹图像结构指针
		   u8** pFtrBuffer
		   **  Pointer to storage buffer of enrollment feature
		   **  此Buffer空间由内部所配置,空间大小不超过1024Byte,用来存储注册的指纹特征
		   u32* nTotalLength
		   **  Pointer to buffer length
		   **  输出实际的特征大小
		   u32 * nProgress
		   **  Pointer to the enrollment progress
		   **  输出注册的进度
返 回 值 : s32 32 bits Error Code
*****************************************************************************/
MITAFIS_API s32 EnrollFingerChip(const RawImageInfo_t* pRawImage, u8** pBuffer, 
	u32* nTotalLength, u32* nProgress);


/*****************************************************************************
函 数 名 : MlEnrollFingerChipWithFeature
功能描述 : 通过指纹特征进行注册过程
输入参数 : uint8 *pInputFea
           ** Pointer to Input Feature
		   ** 输入的指纹特征指针
		   uint32 nInputFeaSize
		   **  输入的指纹特征大小
		   u32* nTotalLength
		   **  Pointer to buffer length
		   **  输出实际的特征大小
		   u32 * nProgress
		   **  Pointer to the enrollment progress
		   **  输出注册的进度（初始使用时传入的*nProgress值应在外部设置为0）
返 回 值 : s32 32 bits Error Code
*****************************************************************************/
MITAFIS_API s32 EnrollFeature(const u8 *pInputFea, u8** pBuffer, 
	u32* nTotalLength, u32* nProgress);

/*****************************************************************************
函 数 名 : AbortEnrollment
功能描述 : 中断指纹注册
输入参数 : 无
返 回 值 : s32 32 bits Error Code
*****************************************************************************/
MITAFIS_API s32 AbortEnrollment(void);

/*****************************************************************************
函 数 名 : GetAfisVersion
功能描述 : 获得算法库的版本号
输入参数 : 无
返 回 值 : 版本号
*****************************************************************************/
MITAFIS_API const char* GetAfisVersion(void);

/*****************************************************************************
函 数 名 : IsSingleFeature
功能描述 : 判断当前特征集是否只含一个手指的特征
输入参数 : [input] u8 * pFtrBuff
           ** Feature buffer.
           ** 特征Buffer
           [input] u32 nFtrBuffLen
           ** Feature size.
           ** 模板特征大小
返 回 值 : s32 
           1,   indicating feature buffer contains only one feature
		   0,   indicating feature buffer contains more than one or no features
*****************************************************************************/
MITAFIS_API s32 IsSingleFeature(u8* pFtrBuff, u32 nFtrBuffLen);

/*****************************************************************************
函 数 名 : IsAvailableImage
功能描述 : 判断当前采图面积是否足够，以及是否为极端湿手指
输入参数 : const RawImageInfo_t* pRawImage:
           ** Pointer to RawImage Info structure
		   ** 输入的指纹图像结构指针
返 回 值 : 错误码
*****************************************************************************/
MITAFIS_API s32 IsAvailableImage(const RawImageInfo_t* pRawImage);

/*****************************************************************************
函 数 名 : AlgoGlobalInit
功能描述 : AlgoGlobalInit
输入参数 : void
返 回 值 : void
*****************************************************************************/
MITAFIS_API void AlgoGlobalInit(void);

/*****************************************************************************
函 数 名 : Ucas_memInit
功能描述 : Mem初始化
输入参数 : void
返 回 值 : void
*****************************************************************************/
MITAFIS_API void  Ucas_memInit(void);

/*****************************************************************************
函 数 名 : Ucas_malloc
功能描述 : Mem内存申请
输入参数 : [input] sint32 nSize
           ** 申请内存的长度(Byte)
返 回 值 : void *
           ** NULL： 申请异常
           ** 其他值：返回内存空间首地址
*****************************************************************************/
MITAFIS_API void* Ucas_malloc(u32 nSize);

/*****************************************************************************
函 数 名 : Ucas_free
功能描述 : Mem内存释放
输入参数 : [input] void *pBuff
           ** 释放内存的首地址
返 回 值 : void
*****************************************************************************/
MITAFIS_API void Ucas_free(void **pBuff);

/*****************************************************************************
函 数 名 : IsValidFeature
功能描述 : 判断大特征是否有效
输入参数 : u8 * pTplFtrBuff,特征首地址
           uint32 nTplFtrBuffLen，特征长度
           ** 释放内存的首地址
返 回 值 : void
*****************************************************************************/
MITAFIS_API s32 IsValidFeature(u8 * pTplFtrBuff, u32 nTplFtrBuffLen);

/*****************************************************************************
函 数 名 : GetAfisMemory
功能描述 : 获取Afis内存首地址
输出参数 : u8 **pMem, 内存首地址
           u32 *pLen，内存长度
返 回 值 : s32 0-正常，其他-异常
*****************************************************************************/
MITAFIS_API s32 GetAfisMemory(u8 **pMem, u32 *pLen);

/*****************************************************************************
函 数 名 : AfisMemInit
功能描述 : Afis内存初始化
输出参数 : 无
返 回 值 : 无
*****************************************************************************/
void AfisMemInit(void);

#ifdef __cplusplus
}// extern "C"
#endif

#endif /*__FPS_API_H__  */

