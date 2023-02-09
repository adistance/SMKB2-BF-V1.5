/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

/**
 * @file    fpc_timebase.c
 * @brief   TODO
 */

#include "fpc_bep_types.h"
#include "fpc_timebase.h"

#include "os_sched.h"
#include "driver_delay.h"

void fpc_timebase_busy_wait(uint32_t delay)
{
    /* Ensure minimum delay or skip if delay is zero*/
    if (delay) 
    {
        delay++;
        delay_ms(delay);
    }
}

uint32_t fpc_timebase_get_tick(void)
{
    return os_sys_tick_get();   //os_sys_time_get();
}

void fpc_timebase_init(void)
{
    return ;
}

fpc_bep_result_t fpc_sensor_wfi(uint16_t timeout_ms, wfi_check_t enter_wfi, bool enter_wfi_mode)
{
    fpc_bep_result_t res = FPC_BEP_RESULT_OK;
    
    return res;
}

