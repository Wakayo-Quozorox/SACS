/*
 * appSACS.c
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
// Envoi de la trame
///////////////////////////////////////////////////////////////
// Fonction qui envoie une trame SACS
// où frame est une structure contenant les paramètres de la trame
// Renvoie: - 0 si tout est OK
//   		- 1 si erreur pendant l'execution de la commande
//			- 2 si la commande n'a pas été executée
uint8_t APP_SACS_send(frameSACS_s frame)
{
	uint8_t dest_address = TX_Addr;
	uint16_t LgMsg = 0;
	uint8_t error = 0;
	uint8_t size = 2+frame.sizeData+2+1; // START(1 byte) + ID/ACK/SIZE_DATA (1 byte) + DATA (SIZE_DATA byte) + CRC (2 bytes) + END(1 byte)
	uint8_t payload[size];

	// START OF FRAME //
	payload[0] =  0b10101010; // Sequence of 1 and 0

	//ID SLAVE + ACKNOWLEDGE + SIZE_DATA //
	payload[1] = frame.sid<<5 | frame.ack <<4 | (frame.sizeData-1);

	// DATA //
	for(uint8_t i=2; i<frame.sizeData+2; i++)
		payload[i]=frame.data[i-2];

	// CRC //
	APP_SACS_setCRC(payload,size);

	// END OF FRAME //
	payload[size-1]= 0b00000000;

    LgMsg=sizeof(payload) / sizeof(payload[0]);
    error = BSP_SX1272_sendPacketTimeout(dest_address,payload,LgMsg,WaitTxMax);
    
	return error;
}

///////////////////////////////////////////////////////////////
// Reception de la trame
///////////////////////////////////////////////////////////////
// Fonction qui envoie une trame SACS
// où frame est une structure contenant les paramètres de la trame
// et timeOut est le temps limite pour que la commande s'execute.
// Renvoie: - 0 si tout est OK
//   		- 1 si erreur pendant l'execution de la commande
//			- 2 si la commande n'a pas été executee
//          - 3 Le CRC reçu ne correspond pas au CRC calculé, les données sont invalides
uint8_t APP_SACS_receive(frameSACS_s* frame, uint32_t timeOut)
{
	uint8_t error = 0;
	uint8_t sizePayload = 0;
	uint8_t payload[MAX_SIZE_PAYLOAD] = {0};

    error = BSP_SX1272_receivePacketTimeout(timeOut);

    if (error == 0){ // La reception a été effectuee
    	sizePayload = currentstate._payloadlength;  // taille totale de la payload
        // Remplissage de la payload avec les donnees recues
        my_printf("\n\r");
        my_printf("TRAME: ");
        for(uint8_t i = 0; i<sizePayload; i++)
        {
        	payload[i] = currentstate.packet_received.data[i];
        	my_printf("%x",payload[i]);
        	my_printf(" ");
        }

        // Remplissage de la structure frame
        // SID //
        frame->sid = payload[1]>>5;

        // ACKNOWLEDGE //
        frame->ack = payload[1]>>4 && MASK_ACKNOLEDGE;

        // SIZE DATA //
        frame->sizeData = sizePayload-5;

        // DATA //
        my_printf("\n\r");
        my_printf("DONNEE: ");
        for(int i = 2; i<frame->sizeData+2; i++)
        {
        	frame->data[i-2]=payload[i];
        	my_printf("%c",frame->data[i-2]);
        }
        my_printf("\n\r");
        // CRC //
        frame->crc = (uint16_t)payload[sizePayload - 3] << 8 | payload[sizePayload - 2];
        error = APP_SACS_checkCRC(payload, sizePayload); //Check CRC
    }else
    {
    	my_printf("Reception Error\r\n");
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
    return 3; // Le CRC reçu ne correspond pas au CRC calculé, les données sont invalides
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

