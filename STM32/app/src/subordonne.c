
/*
 * subordonne.c
 *
 *  Created on: 08 june 2023
 *      Author: Maxime C
 */
#include "subordonne.h"
#include "bsp.h"
#define RECEIVE_TIMEOUT 	100
#define LED_PACKET_SIZE		1
BSP_LED_Init()

int subordonneMain(void) {
	uint32_t curtime = 0;
	frameSACS_s packetLed = {SID1, ACK, LED_PACKET_SIZE, LED_TOGGLE, 0};
	frameSACS_s receivedPacket;
	uint8_t receiveStatus;

	while(1)
	{
		/* Send ACK packet */
		if(APP_SACS_send(packetLed) != SEND_OK)
		{
			return SEND_ERROR;
		} else {
			#if DEBUG
				my_printf("Send OK\r\n");
			#endif
		}

		/* Receive data from controller */
		receiveStatus = APP_SACS_receive(&receivedPacket, RECEIVE_TIMEOUT) != RECEIVE_OK);
		switch (receiveStatus)
		{
		case RECEIVE_OK:
			#if DEBUG
				my_printf("Receive OK\r\n");
                frameSACS_s packetLed = {SID1, ACK, LED_PACKET_SIZE, LED_TOGGLE, 0};
                if (LED_TOGGLE==0x12)
                {
                    BSP_LED_Toggle()
                }
			#endif
			break;
		case RECEIVE_ERROR:
			#if DEBUG
				my_printf("Receive ERROR\r\n");
                frameSACS_s packetLed = {SID1,NACK,LED_PACKET_SIZE,LED_TOGGLE,0};
			#endif
			break;
		
		default:
			#if DEBUG
				my_printf("Unmanaged receive error\r\n");
                frameSACS_s packetLed = {SID1,NACK,LED_PACKET_SIZE,LED_TOGGLE,0};
			#endif
			break;
		}

		processData(receivedPacket.data);

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