/*
 * controller.h
 *
 *  Created on: 17 mai 2023
 *      Author: nbouc
 */

#ifndef APP_INC_CONTROLLER_H_
#define APP_INC_CONTROLLER_H_

#include "main.h"
#include "appSACS.h"
#include "appSX1272.h"
#include "delay.h"

/* Uncomment to show debug messages to console */
#define CONTROLLER_DEBUG

#define CON_RECEIVE_TIMEOUT 10000
#define CON_SEND_RETRIES 3

/**
 * @brief Run the controller core function
 * 
 * @return int 1 if error, should never return otherwise
 */
int controllerMain(void);

/**
 * @brief   Send an LED command to a subordinate with acknowledge management
 *          In case of timeout, the command is sent again up to CON_TIMEOUT_RETRIES times
 * 
 * @param subID     ID of the subordinate
 * @param command   Command to send
 * @return int      1 if error, 0 otherwise
 */
int controllerSendCommand(const uint8_t subID, const uint8_t command); 

#endif /* APP_INC_CONTROLLER_H_ */
