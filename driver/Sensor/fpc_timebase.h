/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#ifndef FPC_TIMEBASE_H
#define FPC_TIMEBASE_H

/**
 * @file    fpc_timebase.h
 * @brief   Timebase based on a system tick.
 *
 * Supplies tick counter and wait operation(s).
 */

#include <stdint.h>

/**
 * @brief Function that returns a bool deciding if WFI mode should be entered or not.
 * @return true enter WFI, false don't enter WFI
 */
typedef bool (*wfi_check_t)(void);


/**
 * @brief Reads the system tick counter.
 *
 * @return Tick count since fpc_timebase_init() call. [ms]
 */
uint32_t fpc_timebase_get_tick(void);


/**
 * @brief Busy wait.
 *
 * @param[in] ms  Time to wait [ms].
 * 0 => return immediately
 * 1 => wait at least 1ms etc.
 */
void fpc_timebase_busy_wait(uint32_t ms);


/**
 * @brief Initializes timebase. Starts system tick counter.
 */
void fpc_timebase_init(void);

#endif /* FPC_TIMEBASE_H */
