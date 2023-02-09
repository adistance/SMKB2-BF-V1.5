#ifndef DRIVER_WDG_H
#define DRIVER_WDG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "rtl876x_rcc.h"
#include "rtl876x_wdg.h"

/* Defines ------------------------------------------------------------------*/

void driver_wdg_init(void);
void wdg_feed(void);


#ifdef __cplusplus
}
#endif

#endif
