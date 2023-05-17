/*
 * comSX1272.h
 *
 *  Created on: 24 août 2020
 *      Author: Arnaud
 */

#ifndef BSP_INC_COMSX1272_H_
#define BSP_INC_COMSX1272_H_

#include "stm32f0xx.h"

/*
 * SPI functions
 */

void BSP_SPI1_Init();
uint8_t BSP_SX1272_Read(uint8_t register_address);
void BSP_SX1272_Write(uint8_t register_address, uint8_t data);

#endif /* BSP_INC_COMSX1272_H_ */
