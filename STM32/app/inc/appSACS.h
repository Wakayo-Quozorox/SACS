/*
 * appSACS.h
 *
 *  Created on: 7 juin 2023
 *      Author: leila.medina
 */

#ifndef APP_INC_APPSACS_H_
#define APP_INC_APPSACS_H_

#include "SX1272.h"

uint8_t APP_SACS_send(uint8_t sid,uint8_t ack, uint8_t	size_data, uint8_t* data);

#endif /* APP_INC_APPSACS_H_ */
