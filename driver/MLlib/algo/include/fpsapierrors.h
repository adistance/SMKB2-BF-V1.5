/******************************************************************************

                  ��Ȩ���� (C), 2014-2024, ������ħ����Ϣ�������޹�˾

 ******************************************************************************
  �� �� ��   : fpsapierrors.h
  �� �� ��   : ����
  ��    ��   : roy
  ��������   : 2014��11��2��
  ����޸�   :
  ��������   : Definition of return values of AFIS system.

  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2014��11��2��
    ��    ��   : roy
    �޸�����   : �����ļ�
  2.��    ��   : 2016��05��2��
	��    ��   : Henry
	�޸�����   : �����޶�
******************************************************************************/

#ifndef __FPS_API_ERRORS_H__
#define __FPS_API_ERRORS_H__

//00---Interface
#define PR_OK                                           (0x00000000)
#define PR_OK_AND_UPDATE_TPL                            (0x00000001)

#define PR_API_NULL_POINTER                             (0x00010001)
#define PR_API_IMG_SIZE_ERROR                           (0x00010002)
#define PR_API_MEM_OUT_OF_SPACE                         (0x00010003)
#define PR_API_INVALID_VERSION                          (0x00010005)
#define PR_API_INVALID_FEATURE                          (0x00010006)
#define PR_DATA_POINTER_NOT_4_ALIGN                     (0x00010007)

//01---Extraction
#define PR_EXTBASE_ARGU_ERROR                           (0x01010201)
#define PR_EXTBASE_EX_FATAL_ERROR                       (0x01010202)
#define PR_EXTBASE_NULL_PARAM                           (0x01010301)
#define PR_EXTBASE_LOW_COVERAGE                         (0x01110102)/*(0x01010302)*/
#define PR_RDGPRD_ARGU_ERROR                            (0x01020101)
#define PR_RDGPRD_NONE_VALID_BLOCK                      (0x01020201)
#define PR_RDGPRD_POOR_QUALITY                          (0x01020301)
#define PR_RDGPRD_POOR_CYCLE_QUALITY                    (0x01020501)
#define PR_RDGPRD_CYCLE_INVALID_PERIOD                  (0x01020502)
#define PR_EXTBIN_ARGU_ERROR                            (0x01030101)
#define PR_EXTSKT_FEW_MINUTIAE                          (0x01050101)
#define PR_EXTMINU_FEW_MINUNUM                          (0x01060101)
#define PR_EXTMINU_QC_LOW                               (0x01060102)
#define PR_EXTMINU_QC_HIGH                              (0x01060103)
#define PR_EXTMINU_QC_DEFUALT                           (0x01060105)
#define PR_EXTMINU_FATAL_ERROR                          (0x01060201)
#define PR_EXTMINU_LINE_TOP_SHIFT                       (0x01060301)
#define PR_EXTMINU_BRANCH_TOP_SHIFT                     (0x01060501)
#define PR_EXTMINU_BIFUR_DATA_ERROR                     (0x01060601)
#define PR_EXTMINU_DEL_MINUTIA_ERROR                    (0x01060701)
#define PR_IMG_PROCESS_ARGU_ERROR                       (0x01080101)
#define PR_IMG_CHECK_POOR_QUALITY                       (0x01080201)
#define PR_IMG_IS_BG                                    (0x01080202)
#define PR_IMG_IS_MISINVOKED                            (0x01080203)

//02--Enrollment
#define PR_ENROLL_OK                                    (0x02000000) //ע������
#define PR_ENROLL_FAIL                                  (0x02000001) //��ǰ��ѹ��ָ��ע��ʧ�ܣ�������ѹע�ἴ��
#define PR_ENROLL_LARGE_MOVE                            (0x02000002) //ע��������ƥ���жϳ�ע��ͼ���ƶ�����Ŀǰû�з���
#define PR_ENROLL_ALMOST_NOMOVE                         (0x02000003) //ע��������ƥ���жϳ�ע��ͼ�񼸺�û���ƶ���������ʾ�û���΢��Ų����ָ
#define PR_ENROLL_BUFFER_OVERFLOW                       (0x02000005) //Ŀǰû���õ�
#define PR_ENROLL_COMPLETE                              (0x02000006) //ע����������������ע��ɹ�
#define PR_ENROLL_EXCEEDED                              (0X02000007) //Ŀǰû���õ�
#define PR_ENROLL_LOW_IMAGE_QUALITY                     (0X02000008) //Ŀǰû���õ�
#define PR_ENROLL_LOW_COVERAGE                          (0X02000009) //��ǰ��ѹ��ָ�Ƹ�����̫�ͣ�������ע�ᣬ������ѹע�ἴ��
#define PR_ERROLL_NOT_SINGLE_FINGER                     (0X0200000A) //Ŀǰû���õ�
#define PR_ERROLL_FEA_HEAD_ERROR						(0X0200000B) //��Ӧ�ó��ֵĴ���������֣�˵���ڴ汻�������Ĵܸġ���������ע�ᣬ��¼�ø��������д����־�����������ǡ�
#define PR_ERROLL_UNSUCCESSFUL							(0X0200000C) //ע�������������������Ǿ����жϣ�ָ������������ƥ��ȶ�̫����ע������ʧ�ܡ���������ע�ᡣ


//03--Match
#define PR_MATCH_ARGU_ERROR                             (0x03010101)
#define PR_MATCH_NONE_MINUTIAE_ERROR                    (0x03010102)
#define PR_MATCH_SAME_FINGER                            (0x03010103)
#define PR_MATCH_DIFFERENT_FINGER                       (0x03010105)
#define PR_MATCH_UNSURE_FINGER                          (0x03010106)
#define PR_MATCH_PAT_BNC_ARGU_ERROR                     (0x03010107)
#define PR_MATCH_FEW_MINUTIAE_ERROR                     (0x03010108)
#define PR_MATCH_MATCHPAIRS_ERROR                       (0x03010109)
#define PR_MATCH_OUT_RANGE_ERROR                        (0x0301010A)
#define PR_MATCH_CELLPOS_OUT_ERROR                      (0x0301010B)
#define PR_MATCH_OUT_MEMORY_LIMIT                       (0x0301010C) 
#define PR_MATCH_SPT_FEA_DECODE_ERROR                   (0x0301010D)
#define PR_MATCH_FTR_UPDATED                            (0X0301010E)
#define PR_MATCH_FTR_HEAD_UPDATED                       (0X0301010F)

//04--Spectral Match
#define PR_SM_OUT_OF_MEMORY								(0x04020101)
#define PR_SM_ANALYZE_SPTL_FTR_ERROR					(0x04020102)

//05--add by Nemo
#define PR_SPECTRAL_EXTRACT_ERROR						(0x05010101)
#define PR_ENCODE_SPECTRAL_FEATURE_ERROR				(0x05010102)
#define PR_DECODE_SPECTRAL_FEATURE_ERROR				(0x05010103)
#define PR_CONBINE_MATCH_REPRESENTATION_ERROR			(0x05010104)
#define PR_SPECTRAL_MATCH_DIFFERENT_FINGER				(0x05010105)
#define PR_SPECTRAL_FEATURE_ERROR						(0x05010106)
#define PR_BUILD_SMD_ERROR								(0x05010107)


//06--Triplet
#define PR_TRIPLET_MEM_ALLOC_ERROR                      (0x06010101)
#define PR_TRIPLET_TRIANGLE_SPACE_LIMIT                 (0x06010102)    
#define PR_TRIPLET_EDGE_SPACE_LIMIT                     (0x06010103)
#define PR_TRIPLET_FEA_NUM_LIMIT                        (0x06010104)


//07--pair
#define PR_PAIR_MEM_ALLOC_ERROR                         (0x07010101)

//08--Update
#define PR_UPDATE_LOW_COVERAGE                          (0x08010101)
#define PR_ENROLL_LOW_GLOBALQLT                         (0x08010102)
#define PR_ENROLL_LOW_MOISTNESS                         (0x08010103)

//00---Interface
//#define PR_OK                                           (0x00000000)
//#define PR_API_IMG_SIZE_ERROR                           (0x00010002)

//01---Extraction
#define PR_PSCORE_IS_NULL								(0x01110101)
//#define PR_EXTBASE_LOW_COVERAGE                         (0x01110102)
#define PR_EXTBASE_FEW_FEATURE                          (0x01110103)

//08---Gaussian
#define PR_ROW_FILTER_ERROR								(0x08010101)
#define PR_COL_FILTER_ERROR								(0x08010102)

//09--StructFunction
#define PR_MATCREAT_ERROR								(0x09010101)
#define PR_MATCOPY_ERROR								(0x09010102)
#define PR_MATCONVERTU2TOU1_ERROR						(0x09010103)
#define PR_MATCONVERTU1TOU2_ERROR						(0x09010104)

//10---BaseFunction
#define PR_MEMSET_ERROR									(0x10010101)
#define PR_MEMCOPY_ERROR								(0x10010102)
#define PR_FREE_ERROR									(0x10010103)




#define PR_MEMMALLOC_ERROR						    (2)
#define PR_IMG_ERROR								(3)

#endif /* __FPS_API_ERRORS_H__ */

