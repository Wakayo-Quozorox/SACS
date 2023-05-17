/*
 * controller.c
 *
 *  Created on: 17 mai 2023
 *      Author: nbouc
 */


#include "bsp.h"

int controllerMain(void) {
	uint32_t curtime = 0;

	while(1)
	{
		curtime = BSP_millis();
		if((curtime%1000)==0)//send every 1000ms
		{
			APP_SACS_send(SID1,2,LED_ON);

			i++;
		}
	}


}
