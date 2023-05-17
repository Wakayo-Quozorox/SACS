/*
 * bsp.h
 *
 *  Created on: 23 août 2020
 *      Author: Arnaud
 */

#ifndef BSP_INC_BSP_H_
#define BSP_INC_BSP_H_

#include "stm32f0xx.h"

/*
 * LED driver functions
 */

void	BSP_LED_Init	(void);
void	BSP_LED_On	(void);
void	BSP_LED_Off	(void);
void	BSP_LED_Toggle	(void);

/*
 * Push-Button driver functions
 */

void		BSP_PB_Init		(void);
uint8_t	BSP_PB_GetState	(void);

void BSP_TIMER_Timebase_Init	(void);
void BSP_NVIC_Init();
int BSP_millis();


/*
 * USART driver functions
 */

void	BSP_Console_Init	(void);

#endif /* BSP_INC_BSP_H_ */
