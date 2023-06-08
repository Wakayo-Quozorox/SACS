/*
 * subordonne.h
 *
 *  Created on: 08 june 2023
 *      Author: Maxime C
 */
#ifndef APP_INC_SUBORDONNE_H_
#define APP_INC_SUBORDONNE_H_

#include "appSACS.h"

#define SID1		0x03
#define LED_ON		0x11
#define LED_OFF		0x10
#define LED_TOGGLE	0x12

/* temporaire, à intégrer dans appSACS.h */
#define MAX_DATA_SIZE    16
#define SEND_ERROR	    1
#define SEND_OK		    0
#define RECEIVE_ERROR	1
#define RECEIVE_OK	    0   
#define ACK             1
#define NACK            0

int subordonneMain(void);

#endif /* APP_INC_CONTROLLER_H_ */