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
int processDataController(frameSACS_s *toProcess);

/**
 * @brief Blink the LED a certain number of times
 * with a period of 400ms
 * 
 * @param nbBlink Number of times the LED should blink
 */
void shortBlink(uint8_t nbBlink);

/**
 * @brief Blink the LED a certain number of times
 * with a period of 2s
 * 
 * @param nbBlink Number of times the LED should blink
 */
void longBlink(uint8_t nbBlink);

#endif /* APP_INC_CONTROLLER_H_ */
