
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
    uint8_t datarcv

	while(1)
	{
        /* Receive data from controller */
		receiveStatus = APP_SACS_receive(&receivedPacket, RECEIVE_TIMEOUT) != RECEIVE_OK);
		switch (receiveStatus)
		{
		case RECEIVE_OK:
			#if DEBUG
				my_printf("Receive OK\r\n");
			#endif
            datarcv=receivedPacket.data[0];
            switch (datarcv)
            {
                case LED_ON:
                    BSP_LED_On();
                    break;
                case LED_OFF:
                    BSP_LED_Off();
                    break;
                case LED_TOGGLE:
                    BSP_LED_Toggle();
                    break;
            }
			break;
		case RECEIVE_ERROR:
			#if DEBUG
				my_printf("Receive ERROR\r\n");
			#endif
            packetLed.ack=NACK;
			break;
		
		default:
			#if DEBUG
				my_printf("Unmanaged receive error\r\n");
			#endif
            packetLed.ack=NACK;
			break;

		/* Send ACK packet */
		if(APP_SACS_send(packetLed) != SEND_OK)
		{
			return SEND_ERROR;
		} else {
			#if DEBUG
				my_printf("Send OK\r\n");
			#endif
		}

		
		}

	}
	/* Should never go there */
	return EXIT_FAILURE;
}
int processDatasub(uint8_t *data) {
	/* Insert code here */
	#if DEBUG
		my_printf("Data received:\r\n");
		for (int i = 0; i < MAX_DATA_SIZE; ++i)
		{
			my_printf("%d\r\n", data[i]);
		}
	#endif
}