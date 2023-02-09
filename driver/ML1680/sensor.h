#ifndef _SENSOR_H
#define _SENSOR_H

//#include "mhscpu.h"
#include "stdbool.h"
#include "stdint.h"

#if 0
#ifndef uint8_t
#define uint8_t  unsigned char
#endif

#ifndef uint32_t
#define uint32_t unsigned int
#endif

#ifndef int32_t
#define int32_t  int
#endif

#ifndef uint16_t
#define uint16_t unsigned short
#endif
#endif

#define FPC1260_CMD_READ_INTERRUPT_WITHCLEAR        0x1C //Includes clearing the interrupt flag
#define FPC1260_CMD_READ_INTERRUPT_NOCLEAR          0x18 //Read the interrupt data without clearing the IRQ flag 
#define FPC1260_CMD_READ_HWID                       0xFC
#define FPC1260_PRODUCT_ID                          (0x0311)


#define FPS_SETNUM_MAX                              (32)
#define FPS_SETNUM_MID                              (8)

#define SENSOR_IMG_BUFFER_MAX                      	(0x3000)//(5120+32)//80*64 //由于图像缓冲去在注册查重过程中会用来缓存特征数据，所以图像缓冲区最小要和特征缓冲宏保持一致
#define SENSOR_FTR_BUFFER_TMP          				(1024)
#define SENSOR_FTR_BUFFER_MAX          				(0x3000)

#define FPSENSOR_IMAGE_HEIGHT 						(80u)
#define FPSENSOR_IMAGE_WIDTH  						(64u)
#define FPSENSOR_IMAGE_SIZE  						((FPSENSOR_IMAGE_HEIGHT*FPSENSOR_IMAGE_WIDTH)+32)


#define SENSOR_OK                                   (0)
#define SENSOR_ERROR_ID                             (1)
#define SENSOR_ERROR_NO_FINGER                      (2)
#define SENSOR_ERROR_CAPTURE                        (3)
#define SENSOR_ERROR_FORCE_QUIT                     (4)
#define SENSOR_ERROR_UNQUALIFIED_IMAGE              (5)
#define SENSOR_ERROR_STORAGE_FULL                   (6)
#define SENSOR_ERROR_FAILURE                        (7)
#define SENSOR_ERROR_REPEAT_FP_INDEX                (8)
#define SENSOR_ERROR_MEGER_ERROR                    (9)
#define SENSOR_ERROR_WRONG_INDEX                    (10)
#define SENSOR_ERROR_HARDWARE_ERROR                 (11)
#define SENSOR_ADJUST_GAIN_ERROR                    (12)
#define SENSOR_INIT_ERROR                   	    (0xff)



//#define FINGER_STATUS_THRES 0x0080
#define FINGER_STATUS_THRES 0xe000

#define SLEEP_MODE_CLK 16000


typedef struct fpsensor_adc
{
	uint8_t shift;
	uint8_t gain; 
	uint8_t pixel; 
	uint8_t et1;
}fpsensor_adc_t;

typedef enum {
    DISPLAY_FORWORD = 0x0b,
    DISPLAY_INVERSE = 0x03,
}  fpsensor_invert_color_t;

typedef enum
{
    FPSENSOR_IRQ_REG_BIT_FINGER_DOWN   = 1 << 0,
    FPSENSOR_IRQ_REG_BIT_ERROR         = 1 << 2,
    FPSENSOR_IRQ_REG_BIT_FIFO_NEW_DATA = 1 << 5,
    FPSENSOR_IRQ_REG_BIT_COMMAND_DONE  = 1 << 7,
    FPSENSOR_IRQ_REG_BITS_RESET        = 0xff
} fpsensor_irq_reg_t;

typedef enum
{

	FPSENSOR_CMD_FINGER_PRESENT_QUERY           = 0x24,
	FPSENSOR_CMD_WAIT_FOR_FINGER_PRESENT        = 0x28,
	FPSENSOR_CMD_ACTIVATE_SLEEP_MODE            = 0x34,
	FPSENSOR_CMD_ACTIVATE_DEEP_SLEEP_MODE       = 0x38,
	FPSENSOR_CMD_ACTIVATE_IDLE_MODE             = 0x20,
	FPSENSOR_CMD_CAPTURE_IMAGE                  = 0x30,
	FPSENSOR_CMD_READ_IMAGE                     = 0x2C,
	FPSENSOR_CMD_SOFT_RESET                     = 0xF8
	
} fpsensor_cmd_t;

typedef enum
{
	FPSENSOR_OK               =0x00,
	FPSENSOR_SPI_ERROR        =0x01,
	FPSENSOR_TIMER_ERROR      =0x02,
	FPSENSOR_GPIO_ERROR       =0x04,
	FPSENSOR_BUFFER_ERROR     =0x08,
	FPSENSOR_TIMEOUT_ERROR    =0x10,
	FPSENSOR_NOFNGR_ERROR     =0x20
}fpsensor_error_t;

typedef enum
{

	FPSENSOR_REG_FPC_STATUS                  = 0x14,  /* RO, 1 bytes  */
	FPSENSOR_REG_READ_INTERRUPT              = 0x18,  /* RO, 1 byte   */
	FPSENSOR_REG_READ_INTERRUPT_WITH_CLEAR   = 0x1C,  /* RO, 1 byte   */
	FPSENSOR_REG_READ_ERROR_WITH_CLEAR       = 0x0C,  /* RO, 1 byte   */
    FPSENSOR_REG_DBG_CFG                     = 0x54,
	FPSENSOR_REG_MISO_EDGE_RIS_EN            = 0x64,  /* WO, 1 byte   */
	FPSENSOR_REG_FPC_CONFIG                  = 0x60,  /* RW, 1 byte   */
	FPSENSOR_REG_IMG_SMPL_SETUP              = 0xA4,  /* RW, 3 bytes  */
	FPSENSOR_REG_CLOCK_CONFIG                = 0x6C,  /* RW, 1 byte   */
	FPSENSOR_REG_IMG_CAPT_SIZE               = 0xA8,  /* RW, 4 bytes  */
	FPSENSOR_REG_IMAGE_SETUP                 = 0xAC,  /* RW, 1 byte   */
	FPSENSOR_REG_ADC_TEST_CTRL               = 0x3C,  /* RW, 1 byte   */
	FPSENSOR_REG_IMG_RD                      = 0xA0,  /* RW, 1 byte   */
	FPSENSOR_REG_SAMPLE_PX_DLY               = 0xB8,  /* RW, 8 bytes  */
	FPSENSOR_REG_PXL_RST_DLY                 = 0xB4,  /* RW, 1 byte   */
	FPSENSOR_REG_TST_COL_PATTERN_EN          = 0x50,  /* RW, 2 bytes  */
	FPSENSOR_REG_CLK_BIST_RESULT             = 0x44,  /* RW, 4 bytes  */
	FPSENSOR_REG_ADC_WEIGHT_SETUP            = 0x98,  /* RW, 1 byte   */
	FPSENSOR_REG_ANA_TEST_MUX                = 0x5C,  /* RW, 4 bytes  */
	FPSENSOR_REG_FINGER_DRIVE_CONF           = 0xC4,  /* RW, 1 byte   */
	FPSENSOR_REG_FINGER_DRIVE_DLY            = 0xC0,  /* RW, 1 byte   */
	FPSENSOR_REG_OSC_TRIM                    = 0xCC,  /* RW, 2 bytes  */
	FPSENSOR_REG_ADC_WEIGHT_TABLE            = 0x9C,  /* RW, 10 bytes */
	FPSENSOR_REG_ADC_SETUP                   = 0x90,  /* RW, 5 bytes  */
	FPSENSOR_REG_ADC_SHIFT_GAIN              = 0x94,  /* RW, 2 bytes  */
	FPSENSOR_REG_BIAS_TRIM                   = 0x68,  /* RW, 1 byte   */
	FPSENSOR_REG_PXL_CTRL                    = 0xB0,  /* RW, 2 bytes  */
	FPSENSOR_REG_FPC_DEBUG                   = 0xF4,  /* RO, 1 bytes  */
	FPSENSOR_REG_FINGER_PRESENT_STATUS       = 0x80,  /* RO, 2 bytes  */
	FPSENSOR_REG_HWID                        = 0x00,  /* RO, 2 bytes  */
	FPSENSOR_REG_VID                         = 0x04,
	FPSENSOR_REG_ANA_CFG1                    = 0xD0,
	FPSENSOR_REG_ANA_CFG2                    = 0xD4,
	FPSENSOR_REG_RDT                         = 0xF0,
	FPSENSOR_ADDRESS_REMAP                   = 0x08,
	/* --- fpsensor/ specific --- */
	FPSENSOR_REG_FNGR_DET_THRES              = 0x84,  /* RW, 1 byte   */
	FPSENSOR_REG_FNGR_DET_CNTR               = 0x88,  /* RW, 2 bytes  */
	FPSENSOR_REG_FNGR_DET_VAL                = 0x8C,
	/* --- fpsensor1 specific --- */
	FPSENSOR1_REG_OFFSET                      = 1000, /* Not a register ! */
	FPSENSOR1_REG_FNGR_DET_THRES              = 1216, /* RW, 4 byte   */
	FPSENSOR1_REG_FNGR_DET_CNTR               = 1220, /* RW, 4 bytes  */

	FPSENSOR_FGR_DET_SUB_COL                  = 0x70,
	FPSENSOR_FGR_DET_SUB_ROW                  = 0x74,
	FPSENSOR_SLP_DET_SUB                      = 0x78,
	FPSENSOR_FGR_DET_NUM                      = 0x7C,
	
} fpsensor_reg_t;

typedef enum{
    NO_HEAVY_PRESS_CHECK    = 0,
    HEAVY_PRESS_CHECK       = 1,
}TYPE_EM_CHECK_FLAG;

typedef struct{
    uint8_t x;
    uint8_t y;
    uint8_t width;
    uint8_t hight;
}__attribute__((packed)) ST_ROI, *PST_ROI;

typedef struct{
    ST_ROI stRoi[12];
    uint32_t roi_num;
    uint32_t area;
    uint32_t sto_num;
}__attribute__((packed)) ST_ROI_CTRL, *PST_ROI_CTRL;

#if 0
typedef struct{
    uint8_t  *pFeature;
    uint32_t length;
    uint32_t progress;
    uint32_t storage_addr;
}ENROLL_PARA, *PENROLL_PARA;
#endif

#if !defined(MAX)
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif
#if !defined(MIN)
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#if TEST_SAVE_PIC
extern uint8_t g_ftrbuffTemp[128];
#endif

#define SPI_IMAGE_BUFFER                                            (g_ImgBuf + 8)
extern uint8_t g_EnrollFtrBuf[SENSOR_FTR_BUFFER_MAX];
extern uint8_t g_ImgBuf[SENSOR_IMG_BUFFER_MAX]; 
extern uint8_t g_QuitAtOnce;
extern uint8_t g_FingerPresent;
extern uint8_t g_FtrInfoUpdateFlag;
//extern ENROLL_PARA g_stEnrollPara;

void SENSOR_GetSpiImgInfo(uint8_t ** pImg, uint32_t *pLen);
int32_t SENSOR_WaitAndCapture(uint32_t wait_time, TYPE_EM_CHECK_FLAG unFlag);
int32_t SENSOR_WaitAndCapture_1(uint32_t wait_time, TYPE_EM_CHECK_FLAG unFlag);

uint8_t SENSOR_FpsensorFingerDown(void);
int32_t SENSOR_ImgJudgment(uint8_t *Imgbuff);
int32_t SENSOR_Extract(void);
void SENSOR_AfisInitEx(int nEnrollMaxNum, uint8_t nMaxSetNum);
void SENSOR_AfisInit(int nEnrollMaxNum, uint8_t nMaxSetNum);
int32_t SENSOR_Enrollment(uint8_t num, uint8_t *pProgress, uint32_t storage_index);
int32_t SENSOR_StorageFeature(uint32_t storage_index);
int32_t SENSOR_Match(uint16_t *Fp_Index);
int32_t SENSOR_Sleep(void);
int32_t SENSOR_DeepSleep(void);
int32_t SENSOR_DeepSleep_1(void);

int32_t SENSOR_MatchEx(uint16_t *Fp_Index);
uint8_t SENSOR_ActiveSleepMode(int32_t fngerDectPeriod);
uint8_t SENSOR_ActiveSleepMode_1(void);

void SENSOR_CropImage(void);
uint8_t SENSOR_SetAdcGain(uint8_t shift, uint8_t gain, uint8_t pxl);
void SENSOR_GetAdcGain(uint8_t *pShift, uint8_t *pGain, uint8_t *pPxlCtrl);
uint8_t SENSOR_Init(void);

uint8_t SENSOR_FpCaptureImageEx(uint8_t* buffer, uint32_t length, uint32_t timeout_seconds);
uint8_t SENSOR_FpCaptureImage_1(uint8_t* buffer, uint32_t x , uint32_t y , uint32_t ImageWidth , uint32_t ImageHeight , uint32_t timeout_seconds);
uint8_t fpsensor_reg_fngr_det_cntr(void);
uint8_t fpsensor_unknow_cmd(void);
uint8_t fpsensor_finger_drive_config(void);
uint8_t fpsensor_image_rd(uint8_t data);
uint8_t fpsensor_smpl_setup(void);
uint8_t fpsensor_reg_ana_cfg1(void);
uint8_t fpsensor_reg_ana_cfg2(void);
uint8_t SENSOR_SetCaptureCrop(uint8_t *buffer, uint32_t length, uint32_t rowStart,uint32_t rowCount,
			uint32_t colStart,uint32_t colGroup);
uint8_t SENSOR_SetInverse_1(uint8_t *buffer, uint32_t length, uint8_t color);

void SENSOR_Common(void);
int32_t SENSOR_MegerFtr(void);
extern void SENSOR_RecentIndexInit(void);

#endif

