/*
 * app_sacs.c
 *
 *  Created on: 17 mai 2023
 *      Author: leila.medina
 */

#include "main.h"
#include "appSACS.h"
#include "appSX1272.h"
#include "SX1272.h"
#include "comSX1272.h"
#include "string.h"
#include "delay.h"


///////////////////////////////////////////////////////////////
// Declaration variables globales
///////////////////////////////////////////////////////////////
extern	SX1272status currentstate;



///////////////////////////////////////////////////////////////
// Envoie de la trame
///////////////////////////////////////////////////////////////
uint8_t APP_SACS_send(frameSACS_s frame)
{
	uint8_t dest_address = TX_Addr;
	uint16_t LgMsg = 0;
	uint8_t error = 0;
	uint8_t size = 2+frame.size_data+1; // START(1 byte) + ID/ACK/SIZE_DATA (1 byte) + DATA (SIZE_DATA byte) + END(1 byte)
	uint8_t payload[size];

	// START OF FRAME //
	payload[0] =  0b10101010; // Sequence of 1 and 0

	//ID SLAVE + ACKNOWLEDGE + SIZE_DATA //
	payload[1] = frame.sid<<5 | frame.ack <<4 | (frame.size_data-1);

	// DATA //
	for(uint8_t i=2; i<frame.size_data+2; i++)
		payload[i]=frame.data[i-2];

	// END OF FRAME //
	payload[size-1]= 0b00000000;

    LgMsg=sizeof(payload) / sizeof(payload[0]);
    error = BSP_SX1272_sendPacketTimeout(dest_address,payload,LgMsg,WaitTxMax);

	return error;
}

///////////////////////////////////////////////////////////////
// Reception de la trame
///////////////////////////////////////////////////////////////
uint8_t APP_SACS_receive(frameSACS_s* frame, uint32_t timeOut)
{
	uint8_t error = 0;
	uint8_t size_payload = 0;
	uint8_t payload[MAX_SIZE_PAYLOAD];

    error = BSP_SX1272_receivePacketTimeout(WaitRxMax);

    if (error == 0){
        size_payload = currentstate.packet_received.length;
        for(int i = 0; i<size_payload; i++)
        	payload[i] = currentstate.packet_received.data[i];
        frame->sid = payload[1]>>5;
        frame->ack = payload[1]>>4 && MASK_ACKNOLEDGE;
        frame->size_data = size_payload-3;
        for(int i = 2; i<frame->size_data+2; i++)
        	frame->data[i-2]=payload[i];
    }

	return error;

}
