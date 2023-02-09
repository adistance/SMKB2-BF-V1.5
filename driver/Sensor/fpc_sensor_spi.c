/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

/**
 * @file    fpc_sensor_spi.c
 * @brief   Driver for SPI master.
 */

#include <stdint.h>
//#include <stdbool.h>
#include <stdlib.h>

#include "fpc_sensor_spi.h"

#include "board.h"
#include <trace.h>

#include "driver_spi.h"
#include "driver_sensor.h"
#include "rtl876x_gpio.h"


extern uint8_t spiSensorTransfer(unsigned char *txBuf, unsigned char *rxBuf, unsigned int len, bool leave_cs_asserted);

volatile bool fpc_sensor_irq_active;

void fpc_sensor_spi_init(uint32_t speed_hz)
{
	speed_hz = speed_hz;
    
    spi_sem_check();
	driver_spi_init();
    spi_sem_give();
//	fpc_sensor_irq_active = FALSE;
}

fpc_bep_result_t fpc_sensor_spi_write_read(uint8_t *write, uint8_t *read, uint32_t size,
        bool leave_cs_asserted)
{
    fpc_bep_result_t result;
    uint8_t status;

    status = spiSensorTransfer(write, read, size, leave_cs_asserted);
    switch (status)
    {
        case 0:
            result = FPC_BEP_RESULT_OK;
            break;
        case 3:
            result = FPC_BEP_RESULT_NO_RESOURCE;
            break;
        default:
            result = FPC_BEP_RESULT_INTERNAL_ERROR;
            break;
    }
    return result;
}

bool fpc_sensor_spi_check_irq(void)
{
    return fpc_sensor_irq_active;
}

bool fpc_sensor_spi_read_irq(void)
{
    bool active = false;
    if (fpc_sensor_irq_active) 
	{
        fpc_sensor_irq_active = false;
        active = true;
    }
    return active;
}

/*
	sensor进入普通休眠后，检测sensor是否被唤醒，再次进入普通休眠
*/
bool fpc_sleep_check_and_set(void)
{
	if (fpc_sensor_irq_active) 
	{
        fpc_sensor_irq_active = false;
		while(Sensor_Sleep()){};
		return false;
    }
	
	return true;
}

void fpc_sensor_spi_reset(bool state)
{
	Sensor_reset_control(state);
}

void GPIO2_Handler(void)
{
    GPIO_INTConfig(GPIO_GetPin(SENSOR_INT_PIN), DISABLE);
    GPIO_MaskINTConfig(GPIO_GetPin(SENSOR_INT_PIN), ENABLE);
    
    fpc_sensor_irq_active = true;
    
    GPIO_ClearINTPendingBit(GPIO_GetPin(SENSOR_INT_PIN));
    GPIO_MaskINTConfig(GPIO_GetPin(SENSOR_INT_PIN), DISABLE);
    GPIO_INTConfig(GPIO_GetPin(SENSOR_INT_PIN), ENABLE);
}
