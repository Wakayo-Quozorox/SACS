/*
 * controller.c
 *
 *  Created on: 17 mai 2023
 *      Author: nbouc
 */

#include "controller.h"

#define RECEIVE_TIMEOUT 	100
#define LED_PACKET_SIZE		1

int controllerMain(void) {
	uint32_t curtime = 0;
	uint8_t bufferData[MAX_DATA_SIZE];
	uint8_t receiveStatus;

	while(1)
	{
		/* Send LED_TOGGLE packet */
		if(APP_SACS_send(SID1, LED_PACKET_SIZE, LED_TOGGLE) != SEND_OK)
		{
			return SEND_ERROR;
		} else {
			#if DEBUG
				my_printf("Send OK\r\n");
			#endif
		}

		/* Receive data from subordonate */
		receiveStatus = APP_SACS_receive(SID1, bufferData, RECEIVE_TIMEOUT) != RECEIVE_OK)
		switch (receiveStatus)
		{
		case RECEIVE_OK:
			#if DEBUG
				my_printf("Receive OK\r\n");
			#endif
			break;
		case RECEIVE_ERROR:
			#if DEBUG
				my_printf("Receive ERROR\r\n");
			#endif
			break;
		
		default:
			#if DEBUG
				my_printf("Unmanaged receive error\r\n");
			#endif
			break;
		}

		processData(bufferData);

		/* Wait for a second */
		BSP_delay_ms(1000);
	}

	/* Should never go there */
	return EXIT_FAILURE;
}

int processData(uint8_t *data) {
	/* Insert code here */
	#if DEBUG
		my_printf("Data received:\r\n");
		for (int i = 0; i < MAX_DATA_SIZE; ++i)
		{
			my_printf("%d\r\n", data[i]);
		}
	#endif
}
