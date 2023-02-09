#ifndef FINGERPRINT_H
#define FINGERPRINT_H

#include <rtl876x.h>

void background_msg_set_fp(unsigned char subtype);
bool fp_admin_match(uint8_t u8OkCnt);


#endif
