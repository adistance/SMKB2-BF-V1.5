#ifndef DRIVER_RTC_H
#define DRIVER_RTC_H

typedef struct
{
	uint8_t seconds;
	uint8_t minutes;
	uint8_t hours ;
	uint8_t month;
	uint8_t date;
	uint8_t year;  
}st_times;

void driver_rtc_init(void);
void rtc_local_time_updata(unsigned int timestamp);
void rtc_set_time(unsigned int timestamp);
uint32_t rtc_get_time(void);
void rtc_update_in_dlps(void);

#endif
