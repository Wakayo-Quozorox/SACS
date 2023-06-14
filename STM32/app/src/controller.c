/*
 * controller.c
 *
 *  Created on: 17 mai 2023
 *      Author: nbouc
 */

#include "controller.h"

int controllerMain(void) {
	
	while(1)
	{
		if (controllerSendCommand(SID3, LED_ON) != EXIT_SUCCESS)
		{
			my_printf("Error sending command to SID3\r\n");
		} else {
			my_printf("Command sent to SID3\r\n");
		}
		BSP_DELAY_ms(5000);

		if (controllerSendCommand(SID4, LED_OFF) != EXIT_SUCCESS)
		{
			my_printf("Error sending command to SID4\r\n");
		} else {
			my_printf("Command sent to SID4\r\n");
		}
		BSP_DELAY_ms(5000);
	}

	/* Should never go there */
	return EXIT_FAILURE;
}

int controllerSendCommand(uint8_t command, uint8_t subID)
{
	frameSACS_s packetToSend = {subID, ACK, LED_PACKET_SIZE, {command}, 0};
	frameSACS_s receivedPacket = {0};
	uint8_t receiveStatus = RECEIVE_ERROR;
	uint8_t retry = 0;
	
	/* Test args values */
	if (command != LED_ON && command != LED_OFF && command != LED_TOGGLE)
	{
		#ifdef CONTROLLER_DEBUG
			my_printf("Wrong command\r\n");
		#endif
		return EXIT_FAILURE;
	} else if (subID > SID6) 
	{
		#ifdef CONTROLLER_DEBUG
			my_printf("Wrong subID\r\n");
		#endif
		return EXIT_FAILURE;
	}

	/* Sending packet */
	#ifdef CONTROLLER_DEBUG
		my_printf("Sending to sub %d...\r\n", subID);
	#endif

	APP_SX1272_quietSetup();
	while (retry < CON_SEND_RETRIES && receiveStatus != RECEIVE_OK )
	{
		if(APP_SACS_send(packetToSend) != SEND_OK)
		{
			#ifdef CONTROLLER_DEBUG
				my_printf("Send ERROR\r\n");
			#endif
			return SEND_ERROR;
		} else 
		{
			#ifdef CONTROLLER_DEBUG
				my_printf("Send OK\r\n");
			#endif
		}

		/* Receive data from subordinate */
		#ifdef CONTROLLER_DEBUG
			my_printf("Receiving from sub %d... Retry %d\r\n", subID, retry);
		#endif

		receiveStatus = APP_SACS_receive(&receivedPacket, CON_RECEIVE_TIMEOUT);
		switch (receiveStatus)
		{
		case RECEIVE_OK:
			#ifdef CONTROLLER_DEBUG
				my_printf("Receive OK\r\n");
			#endif
			if (receivedPacket.data[0] == packetToSend.data[0] && receivedPacket.ack == ACK)
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
		retry++;
	}

	if (retry >= CON_SEND_RETRIES)
	{
		#ifdef CONTROLLER_DEBUG
			my_printf("Send failed\r\n");
		#endif
		return SEND_ERROR;
	} else 
	{
		#ifdef CONTROLLER_DEBUG
			my_printf("Send OK\r\n");
		#endif
		APP_SX1272_quietSetup();

		return SEND_OK;
	}
}