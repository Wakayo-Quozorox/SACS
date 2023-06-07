/*
 * appSACS.h
 *
 *  Created on: 7 juin 2023
 *      Author: leila.medina
 */

#ifndef APP_INC_APPSACS_H_
#define APP_INC_APPSACS_H_

#include "SX1272.h"

typedef struct frameSACS{
	uint8_t sid;       // SLAVE ID
	uint8_t ack;       // ACKNOWLEDGEMENT
	uint8_t size_data; // SIZE DATA
	uint8_t data[16];  // DATA
	uint8_t crc;	   // CRC
} frameSACS_s;

uint8_t APP_SACS_send(frameSACS_s frame);
uint8_t APP_SACS_receive(frameSACS_s* frame, uint32_t timeOut);

#endif /* APP_INC_APPSACS_H_ */