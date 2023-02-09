#ifndef DRIVER_SPI_H
#define DRIVER_SPI_H

#include <rtl876x.h>
#include <app_task.h>
#include "board.h"

#define SPI_MODE_NONE                           (0)
#define SPI_MODE_FLASH                          (1)
#define SPI_MODE_SENSOR                         (2)

#define SPIMASTER_STATUS_OK                     (0)
#define SPIMASTER_STATUS_INIT_ERROR             (1)
#define SPIMASTER_STATUS_START_ERROR            (2)
#define SPIMASTER_STATUS_TIMEROUT_ERROR         (3)

#define SPI_FLASH_CS_ENABLE                     //(Pad_Config(SPI_F_CS_PIN, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_LOW))
#define SPI_FLASH_CS_DISABLE                    //(Pad_Config(SPI_F_CS_PIN, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH))

#define SPI_SENSOR_CS_ENABLE                    (Pad_Config(SPI_S_CS_PIN, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_LOW))
#define SPI_SENSOR_CS_DISABLE                   (Pad_Config(SPI_S_CS_PIN, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH))
       

void gpio_spi_pinmux_config(void);
void gpio_spi_pad_config(void);
void gpio_spi_enter_dlps_config(void);
void gpio_spi_exit_dlps_config(void);


void driver_spi_init(void);
void spi_sem_check(void);
void spi_sem_give(void);

#endif

