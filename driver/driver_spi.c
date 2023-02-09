#include "driver_spi.h"
#include <trace.h>
#include "string.h"

#include <os_sched.h>
#include <os_sync.h>

#include "rtl876x_rcc.h"
#include "rtl876x_spi.h"
#include "rtl876x_gdma.h"
#include "rtl876x_nvic.h"

#define GDMA_MOSI_Channel_Num                1
#define GDMA_MOSI_Channel                    GDMA_Channel1
#define GDMA_MOSI_Channel_IRQn               GDMA0_Channel1_IRQn
#define GDMA_MOSI_Channel_Handler            GDMA0_Channel1_Handler

#define GDMA_MISO_Channel_Num                2
#define GDMA_MISO_Channel                    GDMA_Channel2
#define GDMA_MISO_Channel_IRQn               GDMA0_Channel2_IRQn
#define GDMA_MISO_Channel_Handler            GDMA0_Channel2_Handler

#define GDMA_MAX_SIZE                       (0x800)
GDMA_LLIDef GDMA_MOSI_LLIStruct[8];
GDMA_LLIDef GDMA_MISO_LLIStruct[8];

uint8_t recv_dummy  = 0xFF;
uint8_t send_dummy  = 0xFF;

extern void *spi_sem_handle;

void gpio_spi_pinmux_config(void)
{
    Pinmux_Config(SPI_MOSI_PIN, SPI0_MO_MASTER);
    Pinmux_Config(SPI_MISO_PIN, SPI0_MI_MASTER);
    Pinmux_Config(SPI_CLK_PIN, SPI0_CLK_MASTER);
}

void gpio_spi_pad_config(void)
{
    //Pad_Config(SPI_F_CS_PIN, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pad_Config(SPI_S_CS_PIN, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    
    
    Pad_Config(SPI_MOSI_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pad_Config(SPI_MISO_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);
    Pad_Config(SPI_CLK_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
}

void gpio_spi_enter_dlps_config(void)
{

    Pad_Config(SPI_S_CS_PIN, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
    
    Pad_Config(SPI_MOSI_PIN, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
    Pad_Config(SPI_MISO_PIN, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
    Pad_Config(SPI_CLK_PIN,  PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
}

void gpio_spi_exit_dlps_config(void)
{
   // Pad_Config(SPI_F_CS_PIN, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pad_Config(SPI_S_CS_PIN, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    
    
    Pad_Config(SPI_MOSI_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pad_Config(SPI_MISO_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);
    Pad_Config(SPI_CLK_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
}

void spi_sem_check(void)
{
    while(1)
    {
        if(os_sem_take(spi_sem_handle, 0) == true)
        {
            break;
        }
        else
        {
            os_delay(10);
            APP_PRINT_INFO0("spi wait\r\n");
        }
    }
}

void spi_sem_give(void)
{
    os_sem_give(spi_sem_handle);
}

uint8_t spiTransferWait(void)
{
    uint32_t tickCount = os_sys_tick_get();
    while(GDMA_GetChannelStatus(GDMA_MISO_Channel_Num) == SET)
    {
        if((os_sys_tick_get() - tickCount) > 1000)
        {
            APP_PRINT_INFO0("spi transfer wait time out\r\n");
            return SPIMASTER_STATUS_TIMEROUT_ERROR;
        }
    }
    
    return SPIMASTER_STATUS_OK;
}

/***************************************************************
****************SPI 外设初始化接口******************************
****************************************************************/
void driver_spi_init(void)
{
    SPI_InitTypeDef  SPI_InitStruct;

    SPI_DeInit(SPI0_PORT);
    RCC_PeriphClockCmd(APBPeriph_SPI0, APBPeriph_SPI0_CLOCK, ENABLE);

    /*----------------------SPI init---------------------------------*/
    SPI_StructInit(&SPI_InitStruct);
    SPI_InitStruct.SPI_Direction            = SPI_Direction_FullDuplex; //SPI_Direction_FullDuplex;
    SPI_InitStruct.SPI_Mode                 = SPI_Mode_Master;
    SPI_InitStruct.SPI_DataSize             = SPI_DataSize_8b;
    SPI_InitStruct.SPI_CPOL                 = SPI_CPOL_Low;
    SPI_InitStruct.SPI_CPHA                 = SPI_CPHA_1Edge;
    SPI_InitStruct.SPI_FrameFormat          = SPI_Frame_Motorola;
    SPI_InitStruct.SPI_BaudRatePrescaler    = SPI_BaudRatePrescaler_12;

    SPI_InitStruct.SPI_NDF                  = 2;//0x4000;    //指定EEPROM模式下的触发条件。此参数应该是读取数据长度的值。
    
    SPI_InitStruct.SPI_RxDmaEn              = ENABLE;
    SPI_InitStruct.SPI_RxWaterlevel         = 1;
    SPI_InitStruct.SPI_RxThresholdLevel     = 2;//0x4000;
    
    SPI_InitStruct.SPI_TxDmaEn              = ENABLE;
    SPI_InitStruct.SPI_TxWaterlevel         = 2;
    SPI_InitStruct.SPI_TxThresholdLevel     = 2; //0x4000;
    
    SPI_Init(SPI0_PORT, &SPI_InitStruct);
    
    SPI_Cmd(SPI0_PORT, ENABLE);  
}

/***************************************************************
****************SPI DMA初始化接口*******************************
****************************************************************/
void SPI0_SendAndRecvBuffDma(uint8_t *SendBuf , uint32_t SendLen , uint8_t *RecvBuf , uint32_t RecvLen)  
{
    RCC_PeriphClockCmd(APBPeriph_GDMA, APBPeriph_GDMA_CLOCK, ENABLE);
    
    GDMA_InitTypeDef GDMA_InitStruct;
    uint32_t i, transfer_tmp, transfer_less;
    
    transfer_tmp = SendLen/GDMA_MAX_SIZE;
    transfer_less = SendLen%GDMA_MAX_SIZE;
    if(transfer_less != 0)
    {
        transfer_tmp += 1;
    }
    
//    APP_PRINT_INFO3("SendLen:%d, transfer_tmp:%d, transfer_less:%d\r\n", SendLen, transfer_tmp, transfer_less);
    /*---------------------GDMA MISO initial------------------------------*/
    GDMA_StructInit(&GDMA_InitStruct);
    GDMA_InitStruct.GDMA_ChannelNum          = GDMA_MISO_Channel_Num;
    GDMA_InitStruct.GDMA_DIR                 = GDMA_DIR_PeripheralToMemory;
    
    GDMA_InitStruct.GDMA_BufferSize          = GDMA_MAX_SIZE;        
    GDMA_InitStruct.GDMA_SourceDataSize      = GDMA_DataSize_Byte;              //格式配置
    GDMA_InitStruct.GDMA_DestinationDataSize = GDMA_DataSize_Byte;
    GDMA_InitStruct.GDMA_SourceMsize         = GDMA_Msize_1;
    GDMA_InitStruct.GDMA_DestinationMsize    = GDMA_Msize_1;
    
    GDMA_InitStruct.GDMA_SourceInc           = DMA_SourceInc_Fix;
    GDMA_InitStruct.GDMA_SourceAddr          = (uint32_t)SPI0_PORT->DR;
    GDMA_InitStruct.GDMA_SourceHandshake     = GDMA_Handshake_SPI0_RX;
    
    if(RecvBuf != NULL)
    {
        GDMA_InitStruct.GDMA_DestinationInc  = DMA_DestinationInc_Inc;
        GDMA_InitStruct.GDMA_DestinationAddr = (uint32_t)RecvBuf;
    }
    else
    {
        GDMA_InitStruct.GDMA_DestinationInc  = DMA_DestinationInc_Fix;
        GDMA_InitStruct.GDMA_DestinationAddr = (uint32_t)(&recv_dummy);
    }
   
    GDMA_InitStruct.GDMA_ChannelPriority     = 1;
    GDMA_InitStruct.GDMA_Multi_Block_En      = 1;
    GDMA_InitStruct.GDMA_Multi_Block_Mode    = LLI_TRANSFER;
    GDMA_InitStruct.GDMA_Multi_Block_Struct  = (uint32_t)GDMA_MISO_LLIStruct;
    
    for(i=0; i<transfer_tmp; i++)
    {
        if(RecvBuf != NULL)
        {
            GDMA_MISO_LLIStruct[i].DAR = (uint32_t)(&RecvBuf[i*GDMA_MAX_SIZE]);
        }
        else
        {
            GDMA_MISO_LLIStruct[i].DAR = (uint32_t)(&recv_dummy);
        }
        
        GDMA_MISO_LLIStruct[i].SAR = (uint32_t)SPI0_PORT->DR;
        if(i == transfer_tmp - 1)
        {
            GDMA_MISO_LLIStruct[i].LLP = 0;
            /* configure low 32 bit of CTL register */
            GDMA_MISO_LLIStruct[i].CTL_LOW = BIT(0)
                                        | (GDMA_InitStruct.GDMA_DestinationDataSize << 1)
                                        | (GDMA_InitStruct.GDMA_SourceDataSize << 4)
                                        | (GDMA_InitStruct.GDMA_DestinationInc << 7)
                                        | (GDMA_InitStruct.GDMA_SourceInc << 9)
                                        | (GDMA_InitStruct.GDMA_DestinationMsize << 11)
                                        | (GDMA_InitStruct.GDMA_SourceMsize << 14)
                                        | (GDMA_InitStruct.GDMA_DIR << 20);
            
            /* configure high 32 bit of CTL register */
            if(transfer_less != 0)
            {
                GDMA_MISO_LLIStruct[i].CTL_HIGH = transfer_less;
            }
            else
            {
                GDMA_MISO_LLIStruct[i].CTL_HIGH = GDMA_InitStruct.GDMA_BufferSize;
            }
        }
        else
        {
            GDMA_MISO_LLIStruct[i].LLP = (uint32_t)&GDMA_MISO_LLIStruct[i + 1];
            GDMA_MISO_LLIStruct[i].CTL_LOW = BIT(0)
                                        | (GDMA_InitStruct.GDMA_DestinationDataSize << 1)
                                        | (GDMA_InitStruct.GDMA_SourceDataSize << 4)
                                        | (GDMA_InitStruct.GDMA_DestinationInc << 7)
                                        | (GDMA_InitStruct.GDMA_SourceInc << 9)
                                        | (GDMA_InitStruct.GDMA_DestinationMsize << 11)
                                        | (GDMA_InitStruct.GDMA_SourceMsize << 14)
                                        | (GDMA_InitStruct.GDMA_DIR << 20)
                                        | (GDMA_InitStruct.GDMA_Multi_Block_Mode & LLP_SELECTED_BIT);
            
            /* configure high 32 bit of CTL register */
            GDMA_MISO_LLIStruct[i].CTL_HIGH = GDMA_InitStruct.GDMA_BufferSize;
        }
    }
    GDMA_Init(GDMA_MISO_Channel, &GDMA_InitStruct);
    
    /*-----------------GDMA MISO IRQ-----------------------------*/
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = GDMA_MISO_Channel_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 2;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
    GDMA_INTConfig(GDMA_MISO_Channel_Num, GDMA_INT_Block, ENABLE);      /* Enable transfer interrupt */
    
    /*---------------------GDMA MOSI initial------------------------------*/
    GDMA_StructInit(&GDMA_InitStruct);
    GDMA_InitStruct.GDMA_ChannelNum         = GDMA_MOSI_Channel_Num;
    GDMA_InitStruct.GDMA_DIR                = GDMA_DIR_MemoryToPeripheral;
    GDMA_InitStruct.GDMA_BufferSize         = GDMA_MAX_SIZE;     
    
    GDMA_InitStruct.GDMA_SourceDataSize      = GDMA_DataSize_Byte;              //格式配置
    GDMA_InitStruct.GDMA_DestinationDataSize = GDMA_DataSize_Byte;
    GDMA_InitStruct.GDMA_SourceMsize         = GDMA_Msize_1;
    GDMA_InitStruct.GDMA_DestinationMsize    = GDMA_Msize_1;
    
    if(SendBuf != NULL)
    {
        GDMA_InitStruct.GDMA_SourceInc       = DMA_SourceInc_Inc;                //数据源累加   
        GDMA_InitStruct.GDMA_SourceAddr      = (uint32_t)SendBuf;
    }
    else
    {
        GDMA_InitStruct.GDMA_SourceInc       = DMA_SourceInc_Fix;
        GDMA_InitStruct.GDMA_SourceAddr      = (uint32_t)(&send_dummy);
    }

    GDMA_InitStruct.GDMA_DestinationInc      = DMA_DestinationInc_Fix;          //目标地址固定    
    GDMA_InitStruct.GDMA_DestinationAddr     = (uint32_t)SPI0_PORT->DR;
    GDMA_InitStruct.GDMA_DestHandshake       = GDMA_Handshake_SPI0_TX;
    
    GDMA_InitStruct.GDMA_ChannelPriority     = 1;
    GDMA_InitStruct.GDMA_Multi_Block_En      = 1;
    GDMA_InitStruct.GDMA_Multi_Block_Mode    = LLI_TRANSFER;
    GDMA_InitStruct.GDMA_Multi_Block_Struct  = (uint32_t)GDMA_MOSI_LLIStruct;
    
    for(i=0; i<transfer_tmp; i++)
    {
        if(SendBuf != NULL)
        {
            GDMA_MOSI_LLIStruct[i].SAR = (uint32_t)(&SendBuf[i*GDMA_MAX_SIZE]);        
        }
        else
        {
            GDMA_MOSI_LLIStruct[i].SAR = (uint32_t)(&send_dummy);
        }
        
        GDMA_MOSI_LLIStruct[i].DAR = (uint32_t)SPI0_PORT->DR;
        if(i == transfer_tmp - 1)
        {
            GDMA_MOSI_LLIStruct[i].LLP = 0;
            /* configure low 32 bit of CTL register */
            GDMA_MOSI_LLIStruct[i].CTL_LOW = BIT(0)
                                        | (GDMA_InitStruct.GDMA_DestinationDataSize << 1)
                                        | (GDMA_InitStruct.GDMA_SourceDataSize << 4)
                                        | (GDMA_InitStruct.GDMA_DestinationInc << 7)
                                        | (GDMA_InitStruct.GDMA_SourceInc << 9)
                                        | (GDMA_InitStruct.GDMA_DestinationMsize << 11)
                                        | (GDMA_InitStruct.GDMA_SourceMsize << 14)
                                        | (GDMA_InitStruct.GDMA_DIR << 20);
            
            /* configure high 32 bit of CTL register */
            if(transfer_less != 0)
            {
                GDMA_MOSI_LLIStruct[i].CTL_HIGH = transfer_less;
            }
            else
            {
                GDMA_MOSI_LLIStruct[i].CTL_HIGH = GDMA_InitStruct.GDMA_BufferSize;
            }
        }
        else
        {
            GDMA_MOSI_LLIStruct[i].LLP = (uint32_t)&GDMA_MOSI_LLIStruct[i + 1];
            GDMA_MOSI_LLIStruct[i].CTL_LOW = BIT(0)
                                        | (GDMA_InitStruct.GDMA_DestinationDataSize << 1)
                                        | (GDMA_InitStruct.GDMA_SourceDataSize << 4)
                                        | (GDMA_InitStruct.GDMA_DestinationInc << 7)
                                        | (GDMA_InitStruct.GDMA_SourceInc << 9)
                                        | (GDMA_InitStruct.GDMA_DestinationMsize << 11)
                                        | (GDMA_InitStruct.GDMA_SourceMsize << 14)
                                        | (GDMA_InitStruct.GDMA_DIR << 20)
                                        | (GDMA_InitStruct.GDMA_Multi_Block_Mode & LLP_SELECTED_BIT);
            
            /* configure high 32 bit of CTL register */
            GDMA_MOSI_LLIStruct[i].CTL_HIGH = GDMA_InitStruct.GDMA_BufferSize;
        }
    }
    GDMA_Init(GDMA_MOSI_Channel, &GDMA_InitStruct);
    
    /*-----------------GDMA MOSI IRQ-----------------------------*/
    NVIC_InitStruct.NVIC_IRQChannel = GDMA_MOSI_Channel_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
    GDMA_INTConfig(GDMA_MOSI_Channel_Num, GDMA_INT_Block, ENABLE);      /* Enable transfer interrupt */
    
    GDMA_Cmd(GDMA_MISO_Channel_Num, ENABLE);
    GDMA_Cmd(GDMA_MOSI_Channel_Num, ENABLE);
}

/***************************************************************
****************SPI DMA中断实现接口*****************************
****************************************************************/
void GDMA_MOSI_Channel_Handler(void)
{
    GDMA_INTConfig(GDMA_MOSI_Channel_Num, GDMA_INT_Block, DISABLE);
    
//    DBG_DIRECT("GDMA_MOSI_Channel_Handler");
    
    GDMA_ClearINTPendingBit(GDMA_MOSI_Channel_Num, GDMA_INT_Block);
    GDMA_INTConfig(GDMA_MOSI_Channel_Num, GDMA_INT_Block, ENABLE);
}

void GDMA_MISO_Channel_Handler(void)
{
    GDMA_INTConfig(GDMA_MISO_Channel_Num, GDMA_INT_Block, DISABLE);
    
    spi_sem_give();
//    DBG_DIRECT("GDMA_MISO_Channel_Handler");
    
    GDMA_ClearINTPendingBit(GDMA_MISO_Channel_Num, GDMA_INT_Block);
    GDMA_INTConfig(GDMA_MISO_Channel_Num, GDMA_INT_Block, ENABLE);
}

/***************************************************************
****************FLASH 操作SPI接口*******************************
****************************************************************/
uint8_t spiFlashTransfer(unsigned char *txBuf, unsigned char *rxBuf, unsigned int len, bool leave_cs_asserted)
{
    uint8_t rv = SPIMASTER_STATUS_OK;
    
    spi_sem_check();
    
    SPI_FLASH_CS_ENABLE;
    SPI0_SendAndRecvBuffDma(txBuf, len, rxBuf, len);
    
    spiTransferWait();
    
    if (!leave_cs_asserted)
    {
        SPI_FLASH_CS_DISABLE;
    }
    return rv;
}

uint8_t spiFlashTransfer_match(unsigned char *txBuf, unsigned char *rxBuf, unsigned int len)
{
    uint8_t rv = SPIMASTER_STATUS_OK;
    
    spi_sem_check();
    SPI0_SendAndRecvBuffDma(txBuf, len, rxBuf, len);
    
    return rv;
}

uint8_t spiFlashTransfer_match_wait(void)
{
    uint8_t rv = SPIMASTER_STATUS_OK;
    
    rv = spiTransferWait();
    if(SPIMASTER_STATUS_TIMEROUT_ERROR == rv)
    {
        APP_PRINT_INFO0("spi flash match wait time out\r\n");
    }
    
    SPI_FLASH_CS_DISABLE;
    return rv;
}

/***************************************************************
****************SENSOR 操作SPI接口******************************
****************************************************************/
uint8_t spiSensorTransfer(unsigned char *txBuf, unsigned char *rxBuf, unsigned int len, bool leave_cs_asserted)
{
    uint8_t rv = SPIMASTER_STATUS_OK;
    
    spi_sem_check();
    
    SPI_SENSOR_CS_ENABLE;
    SPI0_SendAndRecvBuffDma(txBuf, len, rxBuf, len);

    spiTransferWait();
    
    if (!leave_cs_asserted)
    {
        SPI_SENSOR_CS_DISABLE;
    }
    return rv;
}

