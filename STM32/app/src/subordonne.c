/*
 * subordonne.c
 *
 *  Created on: 08 june 2023
 *      Author: Maxime C
 */

#include "subordonne.h"

/* CODE QUI GERE LE SUBORDONNE */
/* Attend en boucle un message de la part du contrôleur et renvoie un paquet d'acknowledge ou de
 * non acknowledge en fonction du message reçu */
int subordonneMain(void)
{
	frameSACS_s receivedPacket;
	uint8_t receiveStatus;
	uint8_t sendStatus;

	while(1)
	{
        /* Receive data from controller */
		receiveStatus = APP_SACS_receiveSub(&receivedPacket, SUB_RECEIVE_TIMEOUT, SID3);
	
		//setup SX1272
		APP_SX1272_quietSetup();

		switch (receiveStatus)
		{
		case RECEIVE_OK:
			#if SUBORDONNE_DEBUG
				my_printf("Receive OK\r\n");
			#endif
			/* Verification de l'action à réaliser */
            switch (receivedPacket.data[0])
            {
                case LED_ON:
                    // BSP_LED_On();
                	receivedPacket.ack = ACK;
                    break;
                case LED_OFF:
                    // BSP_LED_Off();
                	receivedPacket.ack = ACK;
                    break;
                case LED_TOGGLE:
                    // BSP_LED_Toggle();
                	receivedPacket.ack = ACK;
                    break;
                default:
					#if SUBORDONNE_DEBUG
						my_printf("Unmanaged command\r\n");
					#endif
					receivedPacket.ack = NACK;
            }
			break;

		case RECEIVE_ERROR:
			#if SUBORDONNE_DEBUG
				my_printf("Receive ERROR\r\n");
			#endif
			receivedPacket.ack = NACK;
			break;

		case RECEIVE_TIMEOUT_ERROR:
			#if SUBORDONNE_DEBUG
				my_printf("RECEIVE TIMEOUT ERROR\r\n");
			#endif
			break;

		case RECEIVE_FAILED:
			#if SUBORDONNE_DEBUG
				my_printf("Receive FAIL\r\n");
			#endif
			receivedPacket.ack = NACK;
			break;

		case CRC_ERROR :
			#if SUBORDONNE_DEBUG
				my_printf("CRC ERROR\r\n");
			#endif
			receivedPacket.ack = NACK;
			break;

		case SIZE_ERROR :
			#if SUBORDONNE_DEBUG
				my_printf("SIZE ERROR\r\n");
			#endif
			receivedPacket.ack = NACK;
			break;

		case RECEIVE_SUB_NC :
			#if SUBORDONNE_DEBUG
				my_printf("Receive but not for me\r\n");
			#endif
			break;

		default:
			#if SUBORDONNE_DEBUG
				my_printf("Unmanaged receive error\r\n");
			#endif
			receivedPacket.ack = NACK;
		}

		if ((receiveStatus != RECEIVE_SUB_NC) && (receiveStatus != RECEIVE_TIMEOUT_ERROR))
		{
			BSP_DELAY_ms(1000); // Laisse un peu le temps

			receivedPacket.sid = CONTROLLER_ID; // Reset l'identifiant
			sendStatus = APP_SACS_send(receivedPacket); /* Send ACK packet */
			
			//setup SX1272
			APP_SX1272_quietSetup();

			if(sendStatus != SEND_OK)
			{
				#if SUBORDONNE_DEBUG
					my_printf("Send ERROR\r\n");
				#endif
				return SEND_ERROR;
			}
			else
			{
				#if SUBORDONNE_DEBUG
					my_printf("Send OK\r\n");
				#endif
			}
		}
		else	
		{
			// rien du tout
		}
	}
	/* Should never go there */
	return EXIT_FAILURE;
}
