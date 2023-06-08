/**
 * @file utilities.h
 * @author Nicolas Bouchery
 * @brief Various utilities functions that don't fit in any other file
 * @version 0.1
 * @date 2023-06-08
 * 
 * 
 */

#ifndef UTILITIES_H
#define UTILITIES_H

#include "stm32f0xx.h"
#include "bsp.h"
#include "delay.h"

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

#endif /* UTILITIES_H */