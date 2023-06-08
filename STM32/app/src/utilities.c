/**
 * @file utilities.c
 * @author Nicolas Bouchery
 * @brief Various utilities functions that don't fit in any other file
 * @version 0.1
 * @date 2023-06-08
 * 
 *  
 */

#include "utilities.h"

void shortBlink(uint8_t nbBlink) {
	for (uint8_t i = 0; i < nbBlink; ++i)
	{
		BSP_LED_On();
		BSP_DELAY_ms(200);
		BSP_LED_Off();
		BSP_DELAY_ms(200);
	}
}

void longBlink(uint8_t nbBlink) {
	for (uint8_t i = 0; i < nbBlink; ++i)
	{
		BSP_LED_On();
		BSP_DELAY_ms(1000);
		BSP_LED_Off();
		BSP_DELAY_ms(1000);
	}
}
