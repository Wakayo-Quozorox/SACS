/*
 * delay.h
 *
 *  Created on: 6 août 2017
 *      Author: Laurent
 */

#ifndef BSP_INC_DELAY_H_
#define BSP_INC_DELAY_H_

#include "stm32f0xx.h"

/*
 * Software counting delays
 */

void BSP_DELAY_ms	(uint32_t delay);
void BSP_DELAY_us	(uint32_t delay);

void BSP_DELAY_TIM_init		(void);
void BSP_DELAY_TIM_ms		(uint16_t ms);

#endif /* BSP_INC_DELAY_H_ */
