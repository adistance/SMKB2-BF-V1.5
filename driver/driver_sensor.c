#include "driver_sensor.h"
#include "string.h"
#include <trace.h>
#include "board.h"

#include "rtl876x_rcc.h"
#include "rtl876x_gpio.h"
#include "rtl876x_nvic.h"

#include "driver_delay.h"

#include "sensor.h"

#include "os_mem.h"

#include "driver_flash.h"
#include "driver_spi.h"
#include "filesys.h"

#include "fpsapi.h"
#include "menu_sleep.h"

#if 0
#define SENSOR_PRINT_INFO0   APP_PRINT_TRACE0
#define SENSOR_PRINT_INFO1   APP_PRINT_TRACE1
#define SENSOR_PRINT_INFO2   APP_PRINT_TRACE2
#define SENSOR_PRINT_INFO3   APP_PRINT_TRACE3
#define SENSOR_PRINT_INFO4   APP_PRINT_TRACE4
#else
#define SENSOR_PRINT_INFO0(...)
#define SENSOR_PRINT_INFO1(...)
#define SENSOR_PRINT_INFO2(...)
#define SENSOR_PRINT_INFO3(...)
#define SENSOR_PRINT_INFO4(...)
#endif

#define SENSOR_WIDTH                                                (64)
#define SENSOR_LENGTH                                               (80)

#define ICN7000_CMD_REG_STATUS                                      0x14
#define ICN7000_CMD_READ_INTERRUPT_NOCLEAR                          0x18 //Read the interrupt data without clearing the IRQ flag
#define ICN7000_CMD_READ_INTERRUPT_WITHCLEAR                        0x1C //Includes clearing the interrupt flag
#define ICN7000_CMD_FINGERPRESENT                                   0x20 //Instant finger present check
#define ICN7000_CMD_WAIT_FOR_FINGERPRESENT                          0x24 //Wait for finger present
#define ICN7000_CMD_SLEEP_MODE                                      0x28 //Activate sleep mode
#define ICN7000_CMD_DEEP_SLEEP_MODE                                 0x2C //Activate deep sleep mode
#define ICN7000_CMD_IDLE_MODE                                       0x34 //Activate idle mode
#define ICN7000_CMD_IMG_CAPT_SIZE                                   0x54
#define ICN7000_CMD_IMAGE_SETUP                                     0x5C
#define ICN7000_CMD_FINGER_DRIVE_CONF                               0x8C
#define ICN7000_CMD_ADC_SHIFT_GAIN                                  0xA0
#define ICN7000_CMD_PXL_CTRL                                        0xA8
#define ICN7000_CMD_CAPTURE_IMG                                     0xC0 //Capture Single Image
#define ICN7000_CMD_READ_IMG_DATA                                   0xC4 //Read image data for a previously captured image.
#define ICN7000_REG_ANA_CFG1                                        0xC8
#define ICN7000_REG_ANA_CFG2                                        0xCC
#define ICN7000_CMD_FINGER_PRESENT_STATUS                           0xD4
#define ICN7000_CMD_RDT                                             0xF4
#define ICN7000_CMD_SOFT_RESET                                      0xF8 //Soft reset
#define ICN7000_CMD_READ_HWID                                       0xFC
#define ICN7000_CMD_FNGR_DET_THRES                                  0xD8
#define ICN7000_CMD_FNGR_DET_CNTR                                   0xDC
#define ICN7000_PXL_BIAS_CTRL                                       (0x80)
#define ICN7000_PRODUCT_ID                                          (0x7332) //ML1680


extern uint8_t spiSensorTransfer(unsigned char *txBuf, unsigned char *rxBuf, unsigned int len, bool leave_cs_asserted);

typedef struct{
    unsigned int sensor_clibration_len;
    unsigned char sensor_calibration_data[256];
    unsigned int chk;                                       //CRC
}__attribute__((packed)) ST_STORAGE_FP_SENSOR;              //sum

ST_STORAGE_FP_SENSOR g_stFpcPara = {0};

__align(4) uint8_t g_ImgBuf[SENSOR_IMG_BUFFER_SIZE] = {0};  

uint8_t g_QuitAtOnce = RESET;

void Sensor_set_quit()
{
	g_QuitAtOnce = 1;
}

void gpio_sensor_pinmux_config(void)
{
    Pinmux_Config(SENSOR_INT_PIN, DWGPIO);
}

void gpio_sensor_pad_config(void)
{
    Pad_Config(SENSOR_RST_PIN, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    
    Pad_Config(SENSOR_INT_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE, PAD_OUT_LOW);
}

void gpio_sensor_enter_dlps_config(void)
{
	Pad_Config(SENSOR_INT_PIN, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_DOWN, PAD_OUT_DISABLE, PAD_OUT_LOW);
	Pad_Config(SENSOR_RST_PIN, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
	System_WakeUpPinEnable(SENSOR_INT_PIN, PAD_WAKEUP_POL_HIGH, PAD_WK_DEBOUNCE_DISABLE);   			
}

void gpio_sensor_exit_dlps_config(void)
{
    Pad_Config(SENSOR_RST_PIN, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
	Pad_Config(SENSOR_INT_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE, PAD_OUT_LOW);
	
}

void driver_sensor_gpio_init(void)
{
    RCC_PeriphClockCmd(APBPeriph_GPIO, APBPeriph_GPIO_CLOCK, ENABLE);
 
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin        = GPIO_GetPin(SENSOR_INT_PIN);
    GPIO_InitStruct.GPIO_Mode       = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_ITCmd      = ENABLE;
    GPIO_InitStruct.GPIO_ITTrigger  = GPIO_INT_Trigger_EDGE;
    GPIO_InitStruct.GPIO_ITPolarity = GPIO_INT_POLARITY_ACTIVE_HIGH;
    GPIO_InitStruct.GPIO_ITDebounce = GPIO_INT_DEBOUNCE_ENABLE;
    GPIO_InitStruct.GPIO_DebounceTime = 10;
    GPIO_Init(&GPIO_InitStruct);

    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = SENSOR_INT_IRQ;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
    
    /* Enable interrupt */
    GPIO_MaskINTConfig(GPIO_GetPin(SENSOR_INT_PIN), DISABLE);
    GPIO_INTConfig(GPIO_GetPin(SENSOR_INT_PIN), ENABLE);
}

void GPIO2_Handler(void)
{    
    GPIO_INTConfig(GPIO_GetPin(SENSOR_INT_PIN), DISABLE);    
    GPIO_MaskINTConfig(GPIO_GetPin(SENSOR_INT_PIN), ENABLE);        
     
    GPIO_ClearINTPendingBit(GPIO_GetPin(SENSOR_INT_PIN));    
    GPIO_MaskINTConfig(GPIO_GetPin(SENSOR_INT_PIN), DISABLE);    
    GPIO_INTConfig(GPIO_GetPin(SENSOR_INT_PIN), ENABLE);
}


void Sensor_reset_control(bool state)
{
    if(state == true)
    {
        Pad_Config(SENSOR_RST_PIN, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_LOW);
    }
    else
    {
        Pad_Config(SENSOR_RST_PIN, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    }
}

void Sensor_Reset(void)
{   
    Sensor_reset_control(true);
    delay_ms(2);
  	Sensor_reset_control(false);
    delay_ms(2);
}

#if 0
/*****************************************************************************
 函 数 名  : Sensor_GetHwId 功能描述  : 获取硬件ID
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
uint32_t Sensor_GetHwId(void)
{
    uint8_t spi_tx[3];
    uint8_t spi_rx[3];

    spi_tx[0] = ICN7000_CMD_READ_HWID;
    spi_tx[1] = 0x00;
    spi_tx[2] = 0x00;
    spi_rx[0] = 0x00;
    spi_rx[1] = 0x00;
    spi_rx[2] = 0x00;

    spiSensorTransfer(spi_tx, spi_rx, 3, false);

    return ((spi_rx[1] << 8) | spi_rx[2]);
}

/*****************************************************************************
 函 数 名  : Sensor_ReadInterrupt  功能描述  : 请清中断
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
uint8_t Sensor_ReadInterrupt(bool bClearFlag)
{
    uint8_t spi_tx[2];
    uint8_t spi_rx[2];

    if (bClearFlag)
    {
        spi_tx[0] = ICN7000_CMD_READ_INTERRUPT_WITHCLEAR;
    }
    else
    {
        spi_tx[0] = ICN7000_CMD_READ_INTERRUPT_NOCLEAR;
    }

    spi_tx[1] = 0x00;
    spi_rx[0] = 0x00;
    spi_rx[1] = 0x00;

    spiSensorTransfer(spi_tx, spi_rx, 2, false);

    return spi_rx[1];
}
#endif

#if 0
fpc_bep_result_t Sensor_Calibration(void)
{
    fpc_bep_result_t result = FPC_BEP_RESULT_INVALID_ARGUMENT;
    size_t size;
    uint8_t *cal_data;
    fpc_bep_sensor_param_t param;

    param.nbr_of_finger_present_zones = 9;
    param.driver_mechanism = FPC_BEP_INTERRUPT_DRIVEN;
    param.reset = FPC_BEP_HARD_RESET;

    flashReadBuffer((unsigned char *)&g_stFpcPara, FPC_INIT_PARA_ADDRESS, sizeof(g_stFpcPara), EM_FLASH_CTRL_FTL);            //read 

    if ((160 == g_stFpcPara.sensor_clibration_len) && (g_stFpcPara.chk == CRC32_calc((unsigned char *)&g_stFpcPara , sizeof(g_stFpcPara) - 4)))//长度固定
    {
        result = fpc_bep_sensor_init(sensor, &g_stFpcPara.sensor_calibration_data[0], &param);
        
        if (result != FPC_BEP_RESULT_OK)
        {
            SENSOR_PRINT_INFO1("fpc_bep_sensor_init result = %d", result);
        }
        return result;
    }
    else
    {
        result = fpc_bep_sensor_init(sensor, NULL, &param);
        if (result == FPC_BEP_RESULT_OK)
        {
            result = fpc_bep_cal_get(&cal_data, &size);
            if (result == FPC_BEP_RESULT_OK)
            {
                g_stFpcPara.sensor_clibration_len = size;                                       //组合sensor数据
                memcpy(&g_stFpcPara.sensor_calibration_data, cal_data, size);
                g_stFpcPara.chk = CRC32_calc((unsigned char *)&g_stFpcPara , sizeof(g_stFpcPara) - 4);
                
                flashWriteBuffer((unsigned char *)&g_stFpcPara, FPC_INIT_PARA_ADDRESS, sizeof(g_stFpcPara), EM_FLASH_CTRL_FTL);
            }
            else
            {
                SENSOR_PRINT_INFO1("fpc_bep_cal_get result = %d", result);
            }
        }
        SENSOR_PRINT_INFO1("fpc_bep_sensor_init result:%d", result);
    }
    return result;
}

sensor_comp_code Sensor_Init(bool freeFlag)
{
    uint32_t id;    
    uint8_t state; 
    fpc_bep_result_t result;
    uint8_t retry = 3;
    
    sensor_comp_code rv = SENSOR_COMP_CODE_OK;

retry:    
    Ucas_memInit();

    spi_sem_check();
    Sensor_Reset();
    spi_sem_give();
    
    state = Sensor_ReadInterrupt(false);
    if (state != 0xFF)
    {
        SENSOR_PRINT_INFO1("[FPC1260-0] after reset, irq state is 0x%x.\r\n", state);
        rv = SENSOR_COMP_CODE_IRQ_ERROR;
        goto sensor_err;
    }
    
    id = Sensor_GetHwId();
    if(id == 0x0000 || id == 0xFFFF)
    {
        SENSOR_PRINT_INFO1("[FPC1260-2] get id: 0x%x.\r\n", id);
        rv = SENSOR_COMP_CODE_ID_ERROR;
        goto sensor_err;
    }
    
    if(!freeFlag)
    {
        SENSOR_PRINT_INFO1("[FPC1260-2] Fpsensor HWID = 1%04x\r\n", id );    //读ID
    }   
    
    if(freeFlag)
    {
        result = fpc_bep_sensor_release();
        if (result != FPC_BEP_RESULT_OK)
        {
            SENSOR_PRINT_INFO1("fpc_bep_sensor_release() res=%d\r\n", result);
            rv = SENSOR_COMP_CODE_RELEASE_ERROR;
            goto sensor_err;
        }
    }
    

    result = Sensor_Calibration();
    if (result != FPC_BEP_RESULT_OK)
    {
        SENSOR_PRINT_INFO1("Sensor_Calibration() res=%d\r\n", result);
        rv = SENSOR_COMP_CODE_CALIBRATION_ERROR;
        
        if(retry--)
            goto retry;
        
        goto sensor_err;
    }
       
sensor_err:   
    return rv;
}
#endif


sensor_finger_status Sensor_FingerCheck(void)
{
#if 0
    fpc_bep_result_t result = FPC_BEP_RESULT_OK;
    fpc_bep_finger_status_t finger_present;

    result = fpc_bep_check_finger_present(&finger_present);
    if ((result == FPC_BEP_RESULT_OK) && (finger_present == FPC_BEP_FINGER_STATUS_PRESENT))
    {
        return SENSOR_FINGER_PRESENT;
    }
    else
    {
        return SENSOR_FINGER_NO_PRESENT;
    }
#endif
    
    if (1 ==  SENSOR_FpsensorFingerDown())
    {
        return SENSOR_FINGER_PRESENT;
    }
    else
    {
        return SENSOR_FINGER_NO_PRESENT;
    }
}

#if 0
sensor_comp_code Sensor_WaitAndCapture(unsigned int wait_time)
{
    uint32_t tick = 0;
    
    sensor_comp_code ret = SENSOR_COMP_CODE_OK;
    fpc_bep_image_t* image = NULL;
    fpc_bep_result_t result = FPC_BEP_RESULT_OK;
    size_t raw_image_size = 0;
    uint8_t *raw_image;
    

    /********************************************/
    //初始化Sensor芯片
//    ret = Sensor_Init(true);
//    if(SENSOR_COMP_CODE_OK != ret)
//    {
//        return ret;
//    }
    /********************************************/

    while (tick < (wait_time))
    {
        if (g_QuitAtOnce)
        {
            g_QuitAtOnce = 0;
            return SENSOR_COMP_CODE_QUIT;
        }
        
        if(Sensor_FingerCheck())
        {
        
#if(TEST_FPS_SPEED == 1)
            timerStart('C');
#endif
            image = fpc_bep_image_new();
            if(NULL == image)
            {
                fpc_bep_image_delete(&image);
                Ucas_memInit();
                return SENSOR_COMP_CODE_MALLOC;
            }
            result = fpc_bep_capture(image);
            switch (result)
            {
                case FPC_BEP_RESULT_OK:
                    raw_image_size = fpc_bep_image_get_size(image);
                    if(0 != raw_image_size)
                    {
                        raw_image = fpc_bep_image_get_pixels(image);
                        if(raw_image != NULL)
                        {
                            memcpy(SENSOR_IMAGE_BUFFER, raw_image, raw_image_size);
//                            dumpBuff(SENSOR_IMAGE_BUFFER, 176*64);
                            ret = SENSOR_COMP_CODE_OK;
                        }
                        else
                        {
                            ret = SENSOR_COMP_CODE_CAPTURE;
                        }

                    }
                    else
                    {
                        ret = SENSOR_COMP_CODE_SIZE;
                    }
                    break;

                case FPC_BEP_RESULT_IMAGE_CAPTURE_ERROR:
                case FPC_BEP_RESULT_TIMEOUT:
                case FPC_BEP_FINGER_NOT_STABLE:
                case FPC_BEP_RESULT_GENERAL_ERROR:
                    SENSOR_PRINT_INFO1("fpc_bep_capture UNQUALIFIED res=%d\r\n", result);
                    ret = SENSOR_COMP_CODE_UNQUALIFIED;
                    break;

                case FPC_BEP_RESULT_BROKEN_SENSOR:
                    ret = SENSOR_COMP_CODE_HARDWARE;
                    break;
                
                case FPC_BEP_RESULT_INTERNAL_ERROR:
                    ret = SENSOR_COMP_CODE_MALLOC;
                    break;
                
                default:
                    SENSOR_PRINT_INFO1("fpc_bep_capture default res=%d\r\n", result);
                    ret = SENSOR_COMP_CODE_DEFAULT;
                    break;
            }
            fpc_bep_image_delete(&image);
            Ucas_memInit();

#if(TEST_FPS_SPEED == 1)
            timerEnd();
            if(ret != SENSOR_COMP_CODE_OK)
            {
                SENSOR_PRINT_INFO0("capture unqualified image\r\n");
            }
#endif
            return ret;
        }
        else
        {
            delay_ms(10);
        }
		menu_sleep_event_control(MENU_SLEEP_EVENT_WAKEUP_BIT, true);
        tick++;
    }

//    SENSOR_PRINT_INFO0("fpc_bep_capture timer out\r\n");
    return SENSOR_COMP_CODE_NO_FINGER;
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
void Sensor_GetSpiImgInfo(uint8_t ** pImg, uint32_t *pLen)
{
    *pImg = SENSOR_IMAGE_BUFFER+2;
    *pLen = 80*64;
}

/*****************************************************************************
 函 数 名  : SENSOR_GetAdcGain
 功能描述  : 获取SENSOR增益
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
void Sensor_GetAdcGain(uint8_t *pShift, uint8_t *pGain, uint8_t *pPxlCtrl)
{
    *pShift   = 21;
    *pGain    = 6;
    *pPxlCtrl = 4 & 0x14;
}

/*****************************************************************************
 函 数 名  : Sensor_Sleep
 功能描述  : Sensor休眠
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
sensor_comp_code Sensor_Sleep(void)
{
#if 0
    fpc_bep_result_t result;
    /*if(Sensor_FingerCheck())
    {
        SENSOR_PRINT_INFO0("[SLEEP] finger down\r\n");
        return SENSOR_COMP_CODE_PRESS_FINGER;
    }*/

    result = fpc_bep_sensor_sleep(800/* (800/4)ms poll period */);
    if (result != FPC_BEP_RESULT_OK)
    {
        SENSOR_PRINT_INFO0("[SLEEP] finger FPC_BEP_RESULT_NG\r\n");
        return SENSOR_COMP_CODE_SLEEP_ERROR;
    }
#endif
    SENSOR_Sleep();

    return SENSOR_COMP_CODE_OK;
}
