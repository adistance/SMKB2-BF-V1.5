#ifndef __SENSORADJUST_H__
#define __SENSORADJUST_H__
//#include "mhscpu.h"


#define FILTERING_ALGO_ON                   (1)             //ÖÐÖµÂË²¨¿ªÆô

uint32_t SENSOR_AdjustGainStart(uint8_t *pImgBuf);
uint8_t Adjust_read_image_Wave_filtering(uint8_t *image);

#endif 
