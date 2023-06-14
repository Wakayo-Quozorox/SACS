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
#include "delay.h"

/* Uncomment to show debug messages to console */
#define CONTROLLER_DEBUG
#define CON_RECEIVE_TIMEOUT 10000


/**
 * @brief Run the controller core function
 * 
 * @return int 1 if error, should never return otherwise
 */
int controllerMain(void);

#endif /* APP_INC_CONTROLLER_H_ */
