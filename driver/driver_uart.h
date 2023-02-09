#ifndef DRIVER_UART_H
#define DRIVER_UART_H

#include <rtl876x.h>
#include <app_task.h>
#include "board.h"

#include "menu_manage.h"

#define UART   UART0
extern bool uart_tx_flag;

void global_data_uart_init(void);
void board_uart_init(void);
void driver_uart_init(void);
void io_uart_dlps_enter(void);
void io_uart_dlps_exit(void);

void io_handle_uart_msg(T_MENU_MSG *io_uart_msg);

void io_uart_background_task_deal(T_MENU_MSG *io_uart_msg);
void uart_test(void);
void uart_mode_set(bool bFlag);
bool uart_mode_get(void);
void USART_SendData(uint8_t *data, uint16_t vCount);
void driver_uart_int_disable(void);

#endif
