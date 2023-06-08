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
// Fonction qui calcule le CRC avec le Polynôme CRC-16-CCITT
// où payload est la trame sous forme de tableau et sizeCRC est la taille du tableau à checker
// Renvoie les 16bits du CRC dans un uint16_t
// ** correspond a CRC-CCITT (0xFFFF) sur ce site https://www.lammertbies.nl/comm/info/crc-calculation
uint16_t APP_SACS_calculateCRC(uint8_t *payload, uint8_t sizeCRC)
{
  uint16_t crc = 0xFFFF; // CRC initial avec tous les bits à 1 (valeur maximale pour un CRC 16 bits)

  // Parcours de chaque octet des données
  for (uint8_t i = 0; i < sizeCRC; i++) {
    crc ^= (uint16_t)payload[i] << 8; // XOR du CRC avec l'octet courant, décalé de 8 bits vers la gauche

    // Parcours de chaque bit de l'octet
    for (uint8_t j = 0; j < 8; j++) {
      if (crc & 0x8000) // Vérifie si le bit de poids le plus élevé est à 1
        crc = (crc << 1) ^ CRC16_POLY; // Décalage du CRC vers la gauche et XOR avec le polynôme
      else
        crc <<= 1; // Décalage du CRC vers la gauche
    }
  }

  return crc; // Renvoie le CRC calculé
}

// Fonction pour vérifier la validité des données avec le CRC
// où payload est la trame sous forme de tableau et size est la taille du tableau
// Renvoie 0 si tout est OK et 1 si erreur
uint8_t APP_SACS_checkCRC(uint8_t *payload, uint8_t size)
{
  uint16_t receivedCRC = (uint16_t)payload[size - 3] << 8 | payload[size - 2]; // Récupération du CRC reçu des 2 derniers octets
  uint16_t calculatedCRC = APP_SACS_calculateCRC(payload, size - 3); // Calcul du CRC pour les données reçues (sans les 2 octets du CRC et la fin de trame)

  if (receivedCRC == calculatedCRC)
  {
    return 0; // Le CRC reçu correspond au CRC calculé, les données sont valides
  }
  else
  {
    return 1; // Le CRC reçu ne correspond pas au CRC calculé, les données sont invalides
  }
}


// Fonction qui calcule le CRC a partir de la trame et qui affecte le CRC a la trame (partie CHECK)
// Prend en entrée la trame et sa taille totale
// Met a jour la trame avec le CRC correspondant
void APP_SACS_setCRC(uint8_t *payload, uint8_t size)
{
	uint16_t crc = 0;

	crc = APP_SACS_calculateCRC(payload, size - 3); // Calcul du CRC a partir des données utiles de la trame (tout sauf crc et fin de trame)

	payload[size - 3] = (uint8_t)((crc >> 8) & 0xFF); // Extraction du octet de poids fort (bits 8 à 15)
	payload[size - 2] = (uint8_t)(crc & 0xFF); // Extraction du octet de poids faible (bits 0 à 7)

	return;
}

