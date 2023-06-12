/*
 * controller.h
 *
 *  Created on: 17 mai 2023
 *      Author: nbouc
 */

#ifndef APP_INC_CONTROLLER_H_
#define APP_INC_CONTROLLER_H_

#include "appSACS.h"
#include "delay.h"
#include "utilities.h"
#include "main.h"

#define CONTROLLER_DEBUG 1

/**
 * @brief Run the controller core function
 * 
 * @return int 1 if error, should never return otherwise
 */
int controllerMain(void);

#endif /* APP_INC_CONTROLLER_H_ */
