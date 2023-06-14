/*
 * controller.c
 *
 *  Created on: 17 mai 2023
 *      Author: nbouc
 */

#include "controller.h"

int controllerMain(void) {
	frameSACS_s packetLed = {SID3, ACK, LED_PACKET_SIZE, {LED_ON}, 0};
	frameSACS_s receivedPacket = {0};
	uint8_t receiveStatus;

	while(1)
	{
		/* Send LED_TOGGLE packet */
		if(APP_SACS_send(packetLed) != SEND_OK)
		{
			#ifdef CONTROLLER_DEBUG
				my_printf("Send ERROR\r\n");
			#endif
			//return SEND_ERROR;
		} else {
			#ifdef CONTROLLER_DEBUG
				my_printf("Send OK\r\n");
			#endif
		}

		/* Receive data from subordinate */

		receiveStatus = APP_SACS_receive(&receivedPacket, RECEIVE_TIMEOUT);
		switch (receiveStatus)
		{
		case RECEIVE_OK:
			#ifdef CONTROLLER_DEBUG
				my_printf("Receive OK\r\n");
			#endif
			if (receivedPacket.data[0] == packetLed.data[0] && receivedPacket.ack == ACK)
			{
				my_printf("Good packet received\r\n");
			}
			break;
		case RECEIVE_ERROR:
			#ifdef CONTROLLER_DEBUG
				my_printf("Receive ERROR\r\n");
			#endif
			break;
		case RECEIVE_FAILED:
			#ifdef CONTROLLER_DEBUG
				my_printf("Command not executed\r\n");
			#endif
			break;
		case CRC_ERROR:
			#ifdef CONTROLLER_DEBUG
				my_printf("CRC ERROR\r\n");
			#endif
			break;
		case RECEIVE_TIMEOUT_ERROR:
			#ifdef CONTROLLER_DEBUG
				my_printf("RECEIVE TIMEOUT ERROR\r\n");
			#endif
			break;

		default:
			#ifdef CONTROLLER_DEBUG
				my_printf("Unmanaged receive ERROR\r\n");
			#endif
			break;
		}

		BSP_DELAY_ms(5000);
	}

	/* Should never go there */
	return EXIT_FAILURE;
}