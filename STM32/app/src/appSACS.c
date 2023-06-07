/*
 * app_sacs.c
 *
 *  Created on: 17 mai 2023
 *      Author: leila.medina
 */

#include "main.h"
#include "appSX1272.h"
#include "SX1272.h"
#include "comSX1272.h"
#include "string.h"
#include "delay.h"


///////////////////////////////////////////////////////////////
// Declaration variables globales
///////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////////////
// Creation de trame
///////////////////////////////////////////////////////////////
uint8_t APP_SACS_send(uint8_t sid,uint8_t ack, uint8_t	size_data, uint8_t* data)
{
	uint8_t dest_address = TX_Addr;
	uint16_t LgMsg = 0;
	uint8_t error = 0;
	uint8_t size = 2+size_data+1; // START(1 byte) + ID/ACK/SIZE_DATA (1 byte) + DATA (SIZE_DATA byte) + END(1 byte)
	uint8_t payload[size];

	// START OF FRAME //
	payload[0] =  0b10101010; // Sequence of 1 and 0

	//ID SLAVE + ACKNOWLEDGE + SIZE_DATA //
	payload[1] = sid<<5 | ack <<4 | size_data;

	// DATA //
	for(uint8_t i=2; i<size_data+2; i++)
		payload[i]=data[i];

	// END OF FRAME //
	payload[size]=	0b00000000;

    LgMsg=sizeof(payload) / sizeof(payload[0]);
    error = BSP_SX1272_sendPacketTimeout(dest_address,payload,LgMsg,WaitTxMax);

	return error;
}
