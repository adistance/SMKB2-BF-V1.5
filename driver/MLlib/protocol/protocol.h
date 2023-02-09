/*
 * protocol.h
 *
 *  Created on: 2020Äê8ÔÂ15ÈÕ
 *      Author: sks
 */

#ifndef APPLICATION_PROTOCOL_PROTOCOL_H_
#define APPLICATION_PROTOCOL_PROTOCOL_H_

#include "mlapi.h"

#define ML_PROTOCOL                                    (0x00)
#define TZ_PROTOCOL                                    (0x01)
#define SY_PROTOCOL                                    (0x02)
#define BR_PROTOCOL                                    (0x03)

void protocol_command_cpy(P_ML_CMD_REQ_DATA pReq);

void UTRNS_SendFrame_ML(P_ML_CMD_RESP_DATA pFrame);



#endif /* APPLICATION_PROTOCOL_PROTOCOL_H_ */
