/*
 * controller.h
 *
 *  Created on: 17 mai 2023
 *      Author: nbouc
 */

#ifndef APP_INC_CONTROLLER_H_
#define APP_INC_CONTROLLER_H_

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

/**
 * @brief Run the controller core function
 * 
 * @return int 1 if error, should never return otherwise
 */
int controllerMain(void);
/**
 * @brief Process the data received from the subordinate
 * 
 * @param toProcess 
 * @return int EXIT_SUCCESS if data is the same as sent, EXIT_FAILURE otherwise
 */
int processDataController(frameSASCS_s *toProcess);

#endif /* APP_INC_CONTROLLER_H_ */
