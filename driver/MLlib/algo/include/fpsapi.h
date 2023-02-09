/******************************************************************************

                  ��Ȩ���� (C), 2014-2024, ������ħ����Ϣ�������޹�˾

 ******************************************************************************
  �� �� ��   : fpsapi.h
  �� �� ��   : ����
  ��    ��   : roy
  ��������   : 2015��5��13��
  ����޸�   :
  ��������   : FPS API interface
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2015��5��13��
    ��    ��   : roy
    �޸�����   : �����ļ�
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
                             //ƥ��÷ֳ�����ֵ����öָ��Ϊͬһ��ָ������Ϊ��ͬ��ָ��
	u16 nEnrollScoreThres;   //When Enrollment, Match Score >nEnrollScoreThres, Stitch Ftr; Others, No Stitching.
                             //ע��ʱ��ƥ��÷ֳ�����ֵ��ƴ�ӣ�������ƴ�ӣ�
	u16 nMaxFtrSize;         //The limit of Enrollment Feature Size.
                             //ָ������Buffer Size����,��FeatureExtraction��ȡ���������������򷵻ش�����
	u8  nEnrollMaxNum;       //The limit of finger number when enrollment.
                             //ע��ʱ����ָ��ͼ��������Ŀ��
	u8  nQualityControl;     //Image quality control parameter. 
                             //���ݲ�ͬͼ������ȥ����Extraction�������Ϊ��ȡֵ��Χ0~3;0-�ϸ�3-���ɡ�
	u8  nCoverageRate;       //Fingerprint image coverage rate, if < nCoverageRate, reject this finger.
	                         //ָ��ͼ��ռ��ͼ�İٷֱȣ����ڸðٷֱȣ���ͼ�񽫱��ܾ���
	s8  strUID[33];          //Chip UID��Verify version validation.
                             //оƬUID��������֤�汾��Ч�ԡ�
	u8  nEnrollStrategyCoverageTh;//����������ģ�������غ������������ֵʱ������PR_ENROLL_ALMOST_NOMOVE��Ĭ����ֵΪ60������60%��
	u8  nFadingSpeed;         //ѧϰ�ٶ�
	u8  nMaxUpdateNum;        //�����´���
	u8  nMaxSetNum;           //ѧϰ����������������С��nEnrollMaxNum
	u16 nTestSingleScoreTh;   //ע��ʱ�������Ƿ�ͬһ��ָ�ķ�����ֵ
	//u16 nSensorResolution;    //Sensor�ֱ���(423/508)
	u16 nRegisterPoolSize;    //Sensor�ֱ���(423/508)
	u8  nRegisterMaxNum;
	u8  nMoistnessGoodFingerThr;         //ע��ʱʪ����ֵ
    u8  nMoistnessUpdateThr;         //��ѧϰʪ����ֵ
	u8  nUseNewEnrollStrategy;         //������ע����Կ��� 1���� 0���� ��Ĭ��Ϊ1
	u8 nSelfLearningStrategy;//λ���ء�Ĭ��ֵ255������ȫ�򿪣���
	//1: ʪ�ȸ��²��ԣ�2��ǰ��������²��ԣ�4�����������²��ԣ�8��ǿ�Ƹ��²��ԣ�16��ǿ�Ƹ��²����е�ƥ��Ч�ʸ��²���
	u8 nReserved[7];
} AfisParams_t,*pAfisParams;

/*****************************************************************************
 �� �� ��  : SetAfisParams
 ��������  : �趨ָ���㷨����Ŀ��Ʋ��������ڳ�ʼ����
 �������  : const AfisParams_t* pFpParams: 
            ����Extraction, Matching and Enrollment����Ҫ�Ĳ�����
 �������  : ��
 �� �� ֵ  : s32, 0--OK��other- Setting Fail.
 *****************************************************************************/
MITAFIS_API s32 SetAfisParams(const AfisParams_t* pFpParams);

/*****************************************************************************
 �� �� ��  : GetDefaultAfisParams
 ��������  : �õ�ָ���㷨����Ŀ��Ʋ���Ĭ�ϣ���Ϊ��ʼ���Ĳο�ֵ��
 �������  : const AfisParams_t* pFpParams: 
            ����Extraction, Matching and Enrollment����Ҫ�Ĳ�����
 �������  : ��
 �� �� ֵ  : s32, 0--OK��other- Setting Fail.
 *****************************************************************************/
MITAFIS_API s32 GetDefaultAfisParams(AfisParams_t* pFpParams);

/*****************************************************************************
 �� �� ��  : CalcMaxFtrSize
 ��������  : ���������ѧϰָ��ͼ���������������Ҫ��FTR�ռ��С��
 �������  : const s32 nMaxSetNum: 
            ������ѧϰָ��ͼ�����
 �������  : ��
 �� �� ֵ  : s32, ��������Ҫ�Ŀռ��С����λ���ֽ�.
 *****************************************************************************/
MITAFIS_API s32 CalcMaxFtrSize(const s32 nMaxSetNum);

/*****************************************************************************
 �� �� ��  : GetAfisParams
 ��������  : �õ�ָ���㷨��ǰ���õĿ��Ʋ��������ڳ�ʼ����
 �������  : const AfisParams_t* pFpParams: 
            ����Extraction, Matching and Enrollment����Ҫ�Ĳ�����
 �������  : ��
 �� �� ֵ  : s32, 0--OK��other- Setting Fail.
 *****************************************************************************/
MITAFIS_API s32 GetAfisParams(AfisParams_t* pFpParams);

/*****************************************************************************
�� �� �� : ExtractFeature
�������� : ָ��������ȡ�㷨
������� : const RawImageInfo_t* pRawImage:
           ** Pointer to RawImage Info structure
		   ** �����ָ��ͼ��ṹָ��
		   u8* pFtrBuffer:
		   **  Pointer to storage buffer of extraction feature.
           **  ��Buffer�ռ����ⲿ����,�ռ��С����Ϊ 888 Bytes,�����洢��ȡ������ָ��������Ϣ
		   u32* pBufferLength:
		   **  Pointer to feature buffer length 
		   **  ���ʵ�ʵ���ȡ������С
�� �� ֵ : s32 32 bits Error Code
*****************************************************************************/
MITAFIS_API s32 ExtractFeature(const RawImageInfo_t* pRawImage, 
	u8* pFtrBuffer, u32* pBufferLength);

/*****************************************************************************
 �� �� ��  : MatchFeature
 ��������  : ָ�������ȶ�
 �������  : [input] u8 * pTplFtrBuff
             ** Pointer to enrolled template feature buffer.
			 ** ��ע���ģ������
             [input] u32 pTplFtrBuffLen
			 ** Pointer to enrolled template feature size.
			 ** ��ע���ģ��������С
             [input] u8 * pProbeFtrBuff
			 ** Pointer to probe feature buffer by way of feature extraction
			 ** ���ȶԵ�������ָ������
             [input] u32 pProbeFtrBuffLen
			 ** Pointer to probe feature buffer size.
			 ** ���ȶԵ�������ָ��������С
			 u32 *nScore
			 ** Pointer to Matching score.
			 ** ƥ���������ö��ָ������������
 �� �� ֵ  : s32 32 bits Error Code
*****************************************************************************/
MITAFIS_API s32 MatchFeature(u8* pTplFtrBuff, u32 * pTplFtrBuffLen, 
	u8* pProbeFtrBuff, u32 pProbeFtrBuffLen, u32* nScore);

/*****************************************************************************
 �� �� ��  : MatchFeatureForDebug
 ��������  : ָ�������ȶ�, ������ѧϰɾ������������
 �������  : [input] u8 * pTplFtrBuff
             ** Pointer to enrolled template feature buffer.
			 ** ��ע���ģ������
             [input] u32 pTplFtrBuffLen
			 ** Pointer to enrolled template feature size.
			 ** ��ע���ģ��������С
             [input] u8 * pProbeFtrBuff
			 ** Pointer to probe feature buffer by way of feature extraction
			 ** ���ȶԵ�������ָ������
             [input] u32 pProbeFtrBuffLen
			 ** Pointer to probe feature buffer size.
			 ** ���ȶԵ�������ָ��������С
			 u32 *nScore
			 ** Pointer to Matching score.
			 ** ƥ���������ö��ָ������������
			 s32 *pDeleteIndex
			 ** the index of the deleted template.
			 ** ��ѧϰɾ������������
 �� �� ֵ  : s32 32 bits Error Code
*****************************************************************************/
MITAFIS_API s32 MatchFeatureForDebug(u8* pTplFtrBuff, u32 * pTplFtrBuffLen, 
	u8* pProbeFtrBuff, u32 pProbeFtrBuffLen, u32* nScore, s32 *pDeleteIndex);


/*****************************************************************************
 �� �� ��  : MatchFeatureForOverlapCheck
 ��������  : ָ�������ȶ�, �����Ƿ�����������غϹ���
 �������  : [input] u8 * pTplFtrBuff
             ** Pointer to enrolled template feature buffer.
			 ** ע���ģ������
             [input] u32 pTplFtrBuffLen
			 ** Pointer to enrolled template feature size.
			 ** ע���ģ��������С
             [input] u8 * pProbeFtrBuff
			 ** Pointer to probe feature buffer by way of feature extraction
			 ** ���ȶԵ�������ָ������
             [input] u32 pProbeFtrBuffLen
			 ** Pointer to probe feature buffer size.
			 ** ���ȶԵ�������ָ��������С
			 u32 *pTooOverlap
			 ** Pointer to check flag.
			 ** ����������ģ�������غ����������ֵ��nEnrollStrategyCoverageTh��ʱ����1��û�г����򷵻�0
 �� �� ֵ  : s32 32 bits Error Code
*****************************************************************************/
MITAFIS_API s32 MatchFeatureForOverlapCheck(u8* pTplFtrBuff, u32 nTplFtrBuffLen, 
	u8* pProbeFtrBuff, u32 nProbeFtrBuffLen, u32* pTooOverlap);


/*****************************************************************************
 �� �� ��  : MatchFeatureForOverlap
 ��������  : ָ�������ȶ�, ������ģ��������������غ���
 �������  : [input] u8 * pTplFtrBuff
             ** Pointer to enrolled template feature buffer.
			 ** ע���ģ������
             [input] u32 pTplFtrBuffLen
			 ** Pointer to enrolled template feature size.
			 ** ע���ģ��������С
             [input] u8 * pProbeFtrBuff
			 ** Pointer to probe feature buffer by way of feature extraction
			 ** ���ȶԵ�������ָ������
             [input] u32 pProbeFtrBuffLen
			 ** Pointer to probe feature buffer size.
			 ** ���ȶԵ�������ָ��������С
			 u32* pMaxOverlap
			 ** Pointer to the Maximum overlap.
			 ** ����������ģ�������е�������������غ�����ƥ�䲻����Ϊ0
 �� �� ֵ  : s32 32 bits Error Code
*****************************************************************************/
MITAFIS_API s32 MatchFeatureForOverlap(u8* pTplFtrBuff, u32 nTplFtrBuffLen, 
	u8* pProbeFtrBuff, u32 nProbeFtrBuffLen, u32* pMaxOverlap);


/*****************************************************************************
�� �� �� : EnrollFingerChip
�������� : ָ��ע�����
������� : const RawImageInfo_t* pRawImage
           ** Pointer to RawImage_Info structure
		   ** �����ָ��ͼ��ṹָ��
		   u8** pFtrBuffer
		   **  Pointer to storage buffer of enrollment feature
		   **  ��Buffer�ռ����ڲ�������,�ռ��С������1024Byte,�����洢ע���ָ������
		   u32* nTotalLength
		   **  Pointer to buffer length
		   **  ���ʵ�ʵ�������С
		   u32 * nProgress
		   **  Pointer to the enrollment progress
		   **  ���ע��Ľ���
�� �� ֵ : s32 32 bits Error Code
*****************************************************************************/
MITAFIS_API s32 EnrollFingerChip(const RawImageInfo_t* pRawImage, u8** pBuffer, 
	u32* nTotalLength, u32* nProgress);


/*****************************************************************************
�� �� �� : MlEnrollFingerChipWithFeature
�������� : ͨ��ָ����������ע�����
������� : uint8 *pInputFea
           ** Pointer to Input Feature
		   ** �����ָ������ָ��
		   uint32 nInputFeaSize
		   **  �����ָ��������С
		   u32* nTotalLength
		   **  Pointer to buffer length
		   **  ���ʵ�ʵ�������С
		   u32 * nProgress
		   **  Pointer to the enrollment progress
		   **  ���ע��Ľ��ȣ���ʼʹ��ʱ�����*nProgressֵӦ���ⲿ����Ϊ0��
�� �� ֵ : s32 32 bits Error Code
*****************************************************************************/
MITAFIS_API s32 EnrollFeature(const u8 *pInputFea, u8** pBuffer, 
	u32* nTotalLength, u32* nProgress);

/*****************************************************************************
�� �� �� : AbortEnrollment
�������� : �ж�ָ��ע��
������� : ��
�� �� ֵ : s32 32 bits Error Code
*****************************************************************************/
MITAFIS_API s32 AbortEnrollment(void);

/*****************************************************************************
�� �� �� : GetAfisVersion
�������� : ����㷨��İ汾��
������� : ��
�� �� ֵ : �汾��
*****************************************************************************/
MITAFIS_API const char* GetAfisVersion(void);

/*****************************************************************************
�� �� �� : IsSingleFeature
�������� : �жϵ�ǰ�������Ƿ�ֻ��һ����ָ������
������� : [input] u8 * pFtrBuff
           ** Feature buffer.
           ** ����Buffer
           [input] u32 nFtrBuffLen
           ** Feature size.
           ** ģ��������С
�� �� ֵ : s32 
           1,   indicating feature buffer contains only one feature
		   0,   indicating feature buffer contains more than one or no features
*****************************************************************************/
MITAFIS_API s32 IsSingleFeature(u8* pFtrBuff, u32 nFtrBuffLen);

/*****************************************************************************
�� �� �� : IsAvailableImage
�������� : �жϵ�ǰ��ͼ����Ƿ��㹻���Լ��Ƿ�Ϊ����ʪ��ָ
������� : const RawImageInfo_t* pRawImage:
           ** Pointer to RawImage Info structure
		   ** �����ָ��ͼ��ṹָ��
�� �� ֵ : ������
*****************************************************************************/
MITAFIS_API s32 IsAvailableImage(const RawImageInfo_t* pRawImage);

/*****************************************************************************
�� �� �� : AlgoGlobalInit
�������� : AlgoGlobalInit
������� : void
�� �� ֵ : void
*****************************************************************************/
MITAFIS_API void AlgoGlobalInit(void);

/*****************************************************************************
�� �� �� : Ucas_memInit
�������� : Mem��ʼ��
������� : void
�� �� ֵ : void
*****************************************************************************/
MITAFIS_API void  Ucas_memInit(void);

/*****************************************************************************
�� �� �� : Ucas_malloc
�������� : Mem�ڴ�����
������� : [input] sint32 nSize
           ** �����ڴ�ĳ���(Byte)
�� �� ֵ : void *
           ** NULL�� �����쳣
           ** ����ֵ�������ڴ�ռ��׵�ַ
*****************************************************************************/
MITAFIS_API void* Ucas_malloc(u32 nSize);

/*****************************************************************************
�� �� �� : Ucas_free
�������� : Mem�ڴ��ͷ�
������� : [input] void *pBuff
           ** �ͷ��ڴ���׵�ַ
�� �� ֵ : void
*****************************************************************************/
MITAFIS_API void Ucas_free(void **pBuff);

/*****************************************************************************
�� �� �� : IsValidFeature
�������� : �жϴ������Ƿ���Ч
������� : u8 * pTplFtrBuff,�����׵�ַ
           uint32 nTplFtrBuffLen����������
           ** �ͷ��ڴ���׵�ַ
�� �� ֵ : void
*****************************************************************************/
MITAFIS_API s32 IsValidFeature(u8 * pTplFtrBuff, u32 nTplFtrBuffLen);

/*****************************************************************************
�� �� �� : GetAfisMemory
�������� : ��ȡAfis�ڴ��׵�ַ
������� : u8 **pMem, �ڴ��׵�ַ
           u32 *pLen���ڴ泤��
�� �� ֵ : s32 0-����������-�쳣
*****************************************************************************/
MITAFIS_API s32 GetAfisMemory(u8 **pMem, u32 *pLen);

/*****************************************************************************
�� �� �� : AfisMemInit
�������� : Afis�ڴ��ʼ��
������� : ��
�� �� ֵ : ��
*****************************************************************************/
void AfisMemInit(void);

#ifdef __cplusplus
}// extern "C"
#endif

#endif /*__FPS_API_H__  */

