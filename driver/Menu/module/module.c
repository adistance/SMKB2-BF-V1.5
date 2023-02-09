#include "module.h"
#include "trace.h"

#include "mlapi.h"
#include "driver_sensor.h"
#include "driver_uart.h"
#if 1
#define MODULE_PRINT_INFO0   APP_PRINT_TRACE0
#define MODULE_PRINT_INFO1   APP_PRINT_TRACE1
#define MODULE_PRINT_INFO2   APP_PRINT_TRACE2
#define MODULE_PRINT_INFO3   APP_PRINT_TRACE3
#define MODULE_PRINT_INFO4   APP_PRINT_TRACE4
#else
#define MODULE_PRINT_INFO0(...)
#define MODULE_PRINT_INFO1(...)
#define MODULE_PRINT_INFO2(...)
#define MODULE_PRINT_INFO3(...)
#define MODULE_PRINT_INFO4(...)
#endif


unsigned char menu_module_init(void)
{
    sensor_comp_code sensor_rv;
    
    sensor_rv = Sensor_Init();
    if(sensor_rv != SENSOR_COMP_CODE_OK)
    {
        MODULE_PRINT_INFO1("[FINGERPRINT] Sensor_Init ret:%d\r\n", sensor_rv);
        return COMP_CODE_HARDWARE_ERROR;
    }
	else
	{
		MODULE_PRINT_INFO0("[FINGERPRINT] Sensor_Init success!");
	}
    
    return 0;
}




