#include "driver_uart.h"

/* Includes ------------------------------------------------------------------*/
#include "rtl876x_nvic.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_rcc.h"
#include "rtl876x_uart.h"

#include <string.h>
#include <os_sync.h>
#include "app_task.h"
#include "app_msg.h"
#include "os_sched.h"

#include "trace.h"
#include "mlapi.h"
#include "test_mode.h"
#include "driver_motor.h"
#include "driver_led.h"
#include "driver_sensor.h"

#include "filesys.h"
#include "files_ftrsys.h"
#include "system_setting.h"
#include "ftl.h"
#include "tuya_ble_app_production_test.h"

/** @brief  UART_BaudRate_Table
  *         div ovsr ovsr_adj :These three parameters set the baud rate calibration parameters of UART.
    baudrate    |   div     |   ovsr    |   ovsr_adj
--------------------------------------------------------
    1200Hz      |   2589    |   7       |   0x7F7
    9600Hz      |   271     |   10      |   0x24A
    14400Hz     |   271     |   5       |   0x222
    19200Hz     |   165     |   7       |   0x5AD
    28800Hz     |   110     |   7       |   0x5AD
    38400Hz     |   85      |   7       |   0x222
    57600Hz     |   55      |   7       |   0x5AD
    76800Hz     |   35      |   9       |   0x7EF
    115200Hz    |   20      |   12      |   0x252
    128000Hz    |   25      |   7       |   0x555
    153600Hz    |   15      |   12      |   0x252
    230400Hz    |   10      |   12      |   0x252
    460800Hz    |   5       |   12      |   0x252
    500000Hz    |   8       |   5       |   0
    921600Hz    |   4       |   5       |   0x3F7
    1000000Hz   |   4       |   5       |   0
    1382400Hz   |   2       |   9       |   0x2AA
    1444400Hz   |   2       |   8       |   0x5F7
    1500000Hz   |   2       |   8       |   0x492
    1843200Hz   |   2       |   5       |   0x3F7
    2000000Hz   |   2       |   5       |   0
    2100000Hz   |   1       |   14      |   0x400
    2764800Hz   |   1       |   9       |   0x2AA
    3000000Hz   |   1       |   8       |   0x492
    3250000Hz   |   1       |   7       |   0x112
    3692300Hz   |   1       |   5       |   0x5F7
    3750000Hz   |   1       |   5       |   0x36D
    4000000Hz   |   1       |   5       |   0
    6000000Hz   |   1       |   1       |   0x36D
-----------------------------------------------------
*/ /* End of UART_BaudRate_Table */

//const UART_BaudRate_TypeDef BaudRate_Table[10] =
//{
//    {271, 10, 0x24A}, // BAUD_RATE_9600
//    {165, 7,  0x5AD}, // BAUD_RATE_19200
//    {20,  12, 0x252}, // BAUD_RATE_115200
//    {10,  12, 0x252}, // BAUD_RATE_230400
//    {5,   12, 0x252}, // BAUD_RATE_460800
//    {4,   5,  0x3F7}, // BAUD_RATE_921600
//    {2,   5,  0},     // BAUD_RATE_2000000
//    {1,   8,  0x492}, // BAUD_RATE_3000000
//    {1,   5,  0},     // BAUD_RATE_4000000
//    {1,   1,  0x36D}, // BAUD_RATE_6000000
//};

/* Globals ------------------------------------------------------------------*/
#define UART_RX_BUF_MAX_LEN        512
static uint8_t UART_Recv_Buf[UART_RX_BUF_MAX_LEN+2] = {0};
static uint16_t UART_Recv_Buf_Lenth = 0;
//uint8_t UART_Send_Buf[256];
//因为使用usb线上电有可能会一直触发串口接收中断导致死机，所以当串口多次接收错误数据就屏蔽串口功能
static uint8_t s_u8UartErrorCnt = 0;   //串口错误接收次数
bool uart_tx_flag = false;   	//串口TX发送标志位

extern void *uart_sem_handle;

void global_data_uart_init(void)
{
    memset(UART_Recv_Buf, 0, sizeof(UART_Recv_Buf));
}

void uart_mode_set(bool bFlag)
{
	uart_tx_flag = bFlag;
}

bool uart_mode_get(void)
{
	return uart_tx_flag;
}

void driver_uart_int_disable(void)
{
	UART_INTConfig(UART0, UART_INT_RD_AVA, DISABLE);
    UART_INTConfig(UART0, UART_INT_RX_IDLE, DISABLE);
}


void board_uart_init(void)
{
    Pad_Config(UART_TX_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);
    Pad_Config(UART_RX_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);

    Pinmux_Config(UART_TX_PIN, UART0_TX);
    Pinmux_Config(UART_RX_PIN, UART0_RX);

	memset(UART_Recv_Buf, 0, sizeof(UART_Recv_Buf));
}

void driver_uart_init(void)
{
    UART_DeInit(UART0);
    
    RCC_PeriphClockCmd(APBPeriph_UART0, APBPeriph_UART0_CLOCK, ENABLE);

    /* uart init */
    UART_InitTypeDef UART_InitStruct;
    UART_StructInit(&UART_InitStruct);

#ifdef SY_PROTOCOL_FALG //晟元协议标志 在keil5 c/c++ option选项里面定义了
	 //57600
    UART_InitStruct.UART_Div               = 55;
    UART_InitStruct.UART_Ovsr              = 7;
    UART_InitStruct.UART_OvsrAdj           = 0x5AD;
#endif

    UART_Init(UART0, &UART_InitStruct);

    //enable rx interrupt and line status interrupt
    UART_INTConfig(UART0, UART_INT_RD_AVA, ENABLE);
    UART_INTConfig(UART0, UART_INT_RX_IDLE, ENABLE);

    /*  Enable UART IRQ  */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel         = UART0_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd      = (FunctionalState)ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_Init(&NVIC_InitStruct);
    s_u8UartErrorCnt = 0;

    uint8_t data = 0x55;
    USART_SendData(&data, 1);
    DBG_DIRECT("[UART] driver_uart_init");
    return;
}

/**
  * @brief  IO enter dlps call back function.
  * @param  No parameter.
  * @return void
  */
void io_uart_dlps_enter(void)
{
    /* Switch pad to Software mode */
    Pad_ControlSelectValue(UART_TX_PIN, PAD_SW_MODE);
    Pad_ControlSelectValue(UART_RX_PIN, PAD_SW_MODE);

    System_WakeUpPinEnable(UART_RX_PIN, PAD_WAKEUP_POL_LOW, PAD_WK_DEBOUNCE_DISABLE);
}

/**
  * @brief  IO exit dlps call back function.
  * @param  No parameter.
  * @return void
  */
void io_uart_dlps_exit(void)
{
    /* Switch pad to Pinmux mode */
    Pad_ControlSelectValue(UART_TX_PIN, PAD_PINMUX_MODE);
    Pad_ControlSelectValue(UART_RX_PIN, PAD_PINMUX_MODE);
}

/**
  * @brief  UARt send data continuous.
  * @param  No parameter.
  * @return void
  */
void uart_senddata_continuous(UART_TypeDef *UARTx, const uint8_t *pSend_Buf, uint16_t vCount)
{
    uint8_t count;
	
    while (vCount / UART_TX_FIFO_SIZE > 0)
    {
        while (UART_GetFlagStatus(UARTx, UART_FLAG_TX_FIFO_EMPTY) == 0);
        for (count = UART_TX_FIFO_SIZE; count > 0; count--)
        {
            UARTx->RB_THR = *pSend_Buf++;
        }
        vCount -= UART_TX_FIFO_SIZE;
    }

    while (UART_GetFlagStatus(UARTx, UART_FLAG_TX_FIFO_EMPTY) == 0);
    while (vCount--)
    {
        UARTx->RB_THR = *pSend_Buf++;
    }
}


void USART_SendData(uint8_t *data, uint16_t vCount)
{
	if(uart_tx_flag)
		uart_senddata_continuous(UART0, data, vCount);
}

void io_handle_uart_msg(T_MENU_MSG *io_uart_msg)
{
    uint16_t subtype = io_uart_msg->subtype; 
    if (IO_MSG_UART_RX == subtype)
    {
        ML_COMMAND_Task();
    }

}


void io_uart_background_task_deal(T_MENU_MSG *io_uart_msg)      //对接收到的数据进行处理，存放在缓冲区里
{
    uint8_t *p_buf = io_uart_msg->u.buf;
    //uint16_t subtype = io_uart_msg->subtype;
    uint16_t len = io_uart_msg->len;            //io_uart_msg->len;
    uint16_t i = 0;

	if(p_buf[0] == 0x66 || p_buf[1] == 0xAA)
	{	
		uart_tx_flag = true;
		tuya_ble_app_production_test_process(0, p_buf, len);
	}
	else
	{
		for (i=0; i<len; i++)
	    {
	        USART_RecvData(p_buf[i]);                       //数据解释存放
	    }
	}    

    os_sem_give(uart_sem_handle);                       //释放等待接收
    
//    menu_sleep_event_control(MENU_SLEEP_EVENT_UART_BIT, true);  //串口数据接收处理
}


void printf_mem()
{
	APP_PRINT_INFO1("FTR_HEAD_ADDRESS is 			%d", FTR_HEAD_ALL_LENGTH);
	APP_PRINT_INFO1("FTR_HEAD_BACKUP_ADDRESS is 	%d", FTR_HEAD_ALL_LENGTH);
	APP_PRINT_INFO1("FTR_INFO_BASE_ADDRESS is 		%d", LENG_SINGERFTRINFO);
	//APP_PRINT_INFO1("FPC_INIT_PARA_ADDRESS is 		%d", sizeof(ST_STORAGE_FP_SENSOR));
	APP_PRINT_INFO1("SYS_INIT_PARA_ADDRESS is 		%d", sizeof(SYS_SETTING));
	APP_PRINT_INFO1("SYS_INIT_PARA_BACK_ADDRESS is 	%d", sizeof(SYS_SETTING));
}

void io_uart_handle_data_deal(void)                     //将接收到的数据发送到本地缓冲区
{
    //APP_PRINT_INFO2("[UART] IDLE len:%d, data:%b", UART_Recv_Buf_Lenth, TRACE_BINARY(UART_Recv_Buf_Lenth, &UART_Recv_Buf[0]));
		
    if(os_sched_is_start() == true)
    {    
        if(os_sem_take(uart_sem_handle, 0) == true)                     //等待本地缓冲区处理完成
        {
            T_MENU_MSG io_uart_msg;
            
            io_uart_msg.type = BACKGROUND_MSG_TYPE_UART;
            io_uart_msg.subtype = IO_MSG_UART_RX;
            io_uart_msg.len = UART_Recv_Buf_Lenth;
            io_uart_msg.u.buf = (void *)(&UART_Recv_Buf);
            background_task_msg_send(&io_uart_msg);
        }
        else
        {
//            APP_PRINT_INFO0("[UART] io_uart_handle_data_deal take sem fail");                //在接收
        }
    }
    else
    {
        APP_PRINT_INFO0("[UART] os is not start"); 
    }
    UART_Recv_Buf_Lenth = 0;
}

void io_uart_handler_data_recv(void)                        //每个byte数据接收
{
    uint16_t lenth = 0;
    uint16_t lenth_end = 0;
    
    lenth = UART_GetRxFIFODataLen(UART0);
    lenth_end = UART_Recv_Buf_Lenth + lenth;
    
    if(lenth_end >= UART_RX_BUF_MAX_LEN)                    
    {
        //lenth = lenth_end - UART_RX_BUF_MAX_LEN;        //长度过长，舍弃后面的信息，避免越界
        APP_PRINT_INFO0("[UART] [ERROR] recv buff len is too long!!!");
		s_u8UartErrorCnt++;
    }
    else	
    {
    	UART_ReceiveData(UART0, &UART_Recv_Buf[UART_Recv_Buf_Lenth], lenth);
    	UART_Recv_Buf_Lenth += lenth;

    }
    
//    APP_PRINT_INFO2("[UART] RECV DATA len:%d, read_data = %b,", lenth, TRACE_BINARY(lenth, &UART_Recv_Buf[UART_Recv_Buf_Lenth-lenth]));
}


void UART0_Handler()
{   
    /* Read interrupt id */
    uint32_t int_status = UART_GetIID(UART0);

    /* Disable interrupt */
    UART_INTConfig(UART0, UART_INT_RD_AVA, DISABLE);
    
    if (UART_GetFlagStatus(UART0, UART_FLAG_RX_IDLE) == SET)
    {
        UART_INTConfig(UART0, UART_INT_RX_IDLE, DISABLE);
        io_uart_handle_data_deal();                         //处理
        
        UART_ClearRxFIFO(UART0);
		if(s_u8UartErrorCnt < 3)
        	UART_INTConfig(UART0, UART_INT_RX_IDLE, ENABLE);
   }

    switch (int_status & 0x0E)
    {
        case UART_INT_ID_RX_DATA_TIMEOUT:
            {
                io_uart_handler_data_recv();                //收
                break;
            }
            
        case UART_INT_ID_LINE_STATUS:
            break;

        case UART_INT_ID_RX_LEVEL_REACH:
            {
                io_uart_handler_data_recv();                //收
                break;
            }
 
        case UART_INT_ID_TX_EMPTY:
            break;

        case UART_INT_ID_MODEM_STATUS:
            break;

        default:
            break;
    }

    if(s_u8UartErrorCnt < 3)
	{
		/* enable interrupt again */
    	UART_INTConfig(UART0, UART_INT_RD_AVA, ENABLE);
	}
}



