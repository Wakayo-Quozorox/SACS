/*
 * subordonne.c
 *
 *  Created on: 08 june 2023
 *      Author: Maxime C
 */

#include "subordonne.h"
#include "bsp.h"
#include "appSACS.h"

#define RECEIVE_TIMEOUT 	100
#define LED_PACKET_SIZE		1


int subordonneMain(void) {
	uint32_t curtime = 0;
	frameSACS_s packetLed = {SID1, ACK, LED_PACKET_SIZE, LED_TOGGLE, 0};
	frameSACS_s receivedPacket;
	uint8_t receiveStatus;

	while(1)
	{
		BSP_LED_Init()

        /* Receive data from controller */
		receiveStatus = APP_SACS_receive(&receivedPacket, RECEIVE_TIMEOUT) != RECEIVE_OK);
		switch (receiveStatus)
		{
		case RECEIVE_OK:
			#if DEBUG
				my_printf("Receive OK\r\n");
			#endif
			/* Verification de l'action à réaliser */
            switch (receivedPacket.data[0]);
            {
                case LED_ON:
                    BSP_LED_On();
					packetLed.ack = ACK;
                    break;
                case LED_OFF:
                    BSP_LED_Off();
					packetLed.ack = ACK;
                    break;
                case LED_TOGGLE:
                    BSP_LED_Toggle();
					packetLed.ack = ACK;
                    break;
				default:
					/* Incorrect value */
					packetLed.ack = NACK;
            }
			break;

		case RECEIVE_ERROR:
			#if DEBUG
				my_printf("Receive ERROR\r\n");
			#endif
            packetLed.ack = NACK;
			break;
		
		default:
			#if DEBUG
				my_printf("Unmanaged receive error\r\n");
			#endif
            packetLed.ack = NACK;
			break;		
		}
	
		/* Send ACK packet */
		if(APP_SACS_send(packetLed) != SEND_OK)
		{
			#if DEBUG
				my_printf("Send ERROR\r\n");
			#endif
			return SEND_ERROR;
		} else {
			#if DEBUG
				my_printf("Send OK\r\n");
			#endif
		}
	}
	/* Should never go there */
	return EXIT_FAILURE;
}