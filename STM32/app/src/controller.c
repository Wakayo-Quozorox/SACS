/*
 * controller.c
 *
 *  Created on: 17 mai 2023
 *      Author: nbouc
 */

#include "controller.h"
#include "bsp.h"

#define RECEIVE_TIMEOUT 	100
#define LED_PACKET_SIZE		1

int controllerMain(void) {
	uint32_t curtime = 0;
	uint8_t ledData[] = {LED_TOGGLE};
	frameSACS_s packetLed = {SID1, ACK, LED_PACKET_SIZE, ledData, 0};
	frameSACS_s receivedPacket;
	uint8_t receiveStatus;

	BSP_LED_Init();

	while(1)
	{
		/* Send LED_TOGGLE packet */
		if(APP_SACS_send(packetLed) != SEND_OK)
		{
			return SEND_ERROR;
		} else {
			#if DEBUG
				my_printf("Send OK\r\n");
			#endif
		}

		/* Receive data from subordinate */
		receiveStatus = APP_SACS_receive(&receivedPacket, RECEIVE_TIMEOUT);
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

		processDataController(&receivedPacket);

		/* Wait for a second and blink LED */
		BSP_delay_ms(800);
		BSP_LED_On();
		BSP_delay_ms(200);
		BSP_LED_Off();
		
	}

	/* Should never go there */
	return EXIT_FAILURE;
}

int processDataController(frameSACS_s *toProcess) {
	#if DEBUG
		my_printf("Data received:\r\n");
		for (int i = 0; i < MAX_DATA_SIZE; ++i)
		{
			my_printf("%d\r\n", toProcess->data[i]);
		}
	#endif

	/* Cette portion n'est valide que pour l'exemple de la LED simple
	 * Il faudra l'adapter si on modifie les échanges
	 */
	if(toProcess->data[0] == LED_TOGGLE)
	{
		BSP_LED_On();
		BSP_delay_ms(200);
		BSP_LED_Off();
		BSP_delay_ms(200);
		BSP_LED_On();
		BSP_delay_ms(200);
		BSP_LED_Off();
		return EXIT_SUCCESS;
	} else {
		BSP_LED_On();
		BSP_delay_ms(1000);
		BSP_LED_Off();
		return EXIT_FAILURE;
	}
}
