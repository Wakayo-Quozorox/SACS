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
	// START(1 byte) + ID/ACK/SIZE_DATA (1 byte) + DATA (SIZE_DATA byte) + CRC (2 bytes) + END(1 byte)
	uint8_t size = NB_BYTE_SOF+NB_BYTE_PARAM+frame.sizeData+NB_BYTE_CRC+NB_BYTE_EOF;
	uint8_t payload[size];

	// START OF FRAME //
	payload[0] =  START_OF_FRAME; // Sequence of 1 and 0

	//ID SLAVE + ACKNOWLEDGE + SIZE_DATA //
	payload[1] = frame.sid<<SHIFT_SID | frame.ack <<SHIFT_ACK | (frame.sizeData-1);

	// DATA //
	for(uint8_t i=NB_BYTE_BEFORE_DATA; i<frame.sizeData+NB_BYTE_BEFORE_DATA; i++)
		payload[i]=frame.data[i-NB_BYTE_BEFORE_DATA];

	// CRC //
	APP_SACS_setCRC(payload,size);

	// END OF FRAME //
	payload[size-1]= END_OF_FRAME;

    LgMsg=sizeof(payload) / sizeof(payload[0]);
    error = BSP_SX1272_sendPacketTimeout(dest_address,payload,LgMsg,WaitTxMax);

	return error;
}

// Fonction qui recoit une trame SACS
// où frame est une structure contenant les parametres de la trame
// et timeOut est le temps limite pour que la commande s'execute.
// Renvoie: - 0 si tout est OK
//   		- 1 si erreur pendant l'execution de la commande
//			- 2 si la commande n'a pas été executee
//          - 3 Le CRC reçu ne correspond pas au CRC calcule, les donnees sont invalides
//          - 4 La taille de la trame recue est superieur a la taille maximaale
uint8_t APP_SACS_receive(frameSACS_s* frame, uint32_t timeOut)
{
	uint8_t error = 0;
	uint8_t sizePayload = 0;
	uint8_t payload[MAX_PAYLOAD_SIZE] = {0};

    error = BSP_SX1272_receivePacketTimeout(timeOut); // Réceptionne tout ce qui passe avec un timeout
    if (error != RECEIVE_OK)
    {
    	my_printf("Problème de réception\n\r");
    }
    else // on continue
    {
		sizePayload = currentstate._payloadlength;  // Taille totale de la payload
		if (sizePayload >= MAX_PAYLOAD_SIZE) // On vérifie que le message reçu n'a pas une taille supérieure a la taille max de la trame
		{
			my_printf("La taille de la trame reçue est superieure a la taille maximale \n\r");
			error = SIZE_ERROR; // La trame reçue n'a pas la bonne taille
		}
		else // on continue
		{
			my_printf("TRAME: ");

			for(uint8_t i = 0; i<sizePayload; i++) // Remplissage de la payload avec les donnees recues
			{
				payload[i] = currentstate.packet_received.data[i];
				my_printf("%x",payload[i]);
				my_printf(" ");
			}
			my_printf("\n\r");

			error = APP_SACS_checkCRC(payload, sizePayload); // On check la validité des données reçues

			if (error != RECEIVE_OK)  // Les données sont invalides
			{
				my_printf("CRC invalide, trame ignorée !\r\n");
			}
			else // Si les données reçues sont valides, on affiche et on remplit la structure de la trame
			{
				// SID //
				frame->sid = payload[1]>>SHIFT_SID;
				my_printf("SID : %x \n\r",frame->sid);
				// ACKNOWLEDGE //
				frame->ack = payload[1]>>SHIFT_ACK && MASK_ACKNOLEDGE;
				my_printf("ACK : %x \n\r",frame->ack);

				// SIZE DATA //
				frame->sizeData = sizePayload-(NB_BYTE_BEFORE_DATA+NB_BYTE_AFTER_DATA);
				my_printf("SIZE DATA : %x \n\r",frame->sizeData);

				// DATA //
				my_printf("DONNEE: ");

				for(int i = 2; i<frame->sizeData+2; i++)
				{
					frame->data[i-NB_BYTE_BEFORE_DATA]=payload[NB_BYTE_BEFORE_DATA];
					my_printf("%x",frame->data[i-NB_BYTE_BEFORE_DATA]);
				}
				my_printf("\n\r");

				// CRC //
				frame->crc = (uint16_t)payload[sizePayload - NB_BYTE_AFTER_DATA] << BYTE_SIZE | payload[sizePayload - (NB_BYTE_AFTER_DATA-1)];
				error = APP_SACS_checkCRC(payload, sizePayload); //Check CRC
			}
		}
    }

	return error;
}

// Fonction qui calcule le CRC avec le Polynôme CRC-16-CCITT
// où payload est la trame sous forme de tableau et sizeCRC est la taille du tableau à checker
// Renvoie les 16bits du CRC dans un uint16_t
// ** correspond a CRC-CCITT (0xFFFF) sur ce site https://www.lammertbies.nl/comm/info/crc-calculation
uint16_t APP_SACS_calculateCRC(uint8_t *payload, uint8_t sizeCRC)
{
  uint16_t crc = INIT_CRC; // CRC initial avec tous les bits à 1 (valeur maximale pour un CRC 16 bits)

  // Parcours de chaque octet des données
  for (uint8_t i = 0; i < sizeCRC; i++) {
    crc ^= (uint16_t)payload[i] << BYTE_SIZE; // XOR du CRC avec l'octet courant, décalé de 8 bits vers la gauche

    // Parcours de chaque bit de l'octet
    for (uint8_t j = 0; j < BYTE_SIZE; j++) {
      if (crc & MASK_CRC_MSB) // Vérifie si le bit de poids le plus élevé est à 1
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
  uint16_t receivedCRC = (uint16_t)payload[size - NB_BYTE_AFTER_DATA] << BYTE_SIZE | payload[size - (NB_BYTE_AFTER_DATA-1)]; // Récupération du CRC reçu des 2 derniers octets
  uint16_t calculatedCRC = APP_SACS_calculateCRC(payload, size - NB_BYTE_AFTER_DATA); // Calcul du CRC pour les données reçues (sans les 2 octets du CRC et la fin de trame)

  if (receivedCRC == calculatedCRC)
  {
    return CRC_OK; // Le CRC reçu correspond au CRC calculé, les données sont valides
  }
  else
  {
    return CRC_ERROR; // Le CRC reçu ne correspond pas au CRC calculé, les données sont invalides
  }
}


// Fonction qui calcule le CRC a partir de la trame et qui affecte le CRC a la trame (partie CHECK)
// Prend en entrée la trame et sa taille totale
// Met a jour la trame avec le CRC correspondant
void APP_SACS_setCRC(uint8_t *payload, uint8_t size)
{
	uint16_t crc = 0;

	crc = APP_SACS_calculateCRC(payload, size - NB_BYTE_AFTER_DATA); // Calcul du CRC a partir des données utiles de la trame (tout sauf crc et fin de trame)

	payload[size - NB_BYTE_AFTER_DATA] = (uint8_t)((crc >> BYTE_SIZE) & MASK_RST_MSBYTE); // Extraction du octet de poids fort (bits 8 à 15)
	payload[size - (NB_BYTE_AFTER_DATA-1)] = (uint8_t)(crc & MASK_RST_MSBYTE); // Extraction du octet de poids faible (bits 0 à 7)

	return;
}

// Fonction qui recoit une trame SACS et verifie l'ID subordonne
// où frame est une structure contenant les paramètres de la trame
// timeOut est le temps limite pour que la commande s'execute
// et subId est l'identifiant du subordonne
// Renvoie: - 0 si tout est OK
//   		- 1 si erreur pendant l'execution de la commande
//			- 2 si la commande n'a pas été executee
//          - 3 Le CRC reçu ne correspond pas au CRC calcule, les donnees sont invalides
//          - 4 La taille de la trame recue est superieur a la taille maximale
//          - 5 La trame recue n'est pas attribuee au subordonne
uint8_t APP_SACS_receiveSub(frameSACS_s* frame, uint32_t timeOut, uint8_t subId)
{
	uint8_t status = 0;
	frameSACS_s frameReceive;

	status = APP_SACS_receive(&frameReceive,timeOut);

	if (status != RECEIVE_OK) // Erreur de reception par le subordonne
	{
		my_printf("Problème de réception par le subordonné: ");
		my_printf("error %d \n\r",status);
	}else
	{
		if(frameReceive.sid == subId) // Le subordonne est concerne par la trame reçue
		{
			frame->sid      = frameReceive.sid;
			frame->ack      = frameReceive.ack;
			frame->sizeData = frameReceive.sizeData;
			for(int i = 0; i<frame->sizeData ; i++)
				frame->data[i] = frameReceive.data[i];
			frame->crc      = frameReceive.crc;
		}else // La trame n'est pas attribuee au subordonne
		{
			status = RECEIVE_SUB_NC;
		}
	}

	return status;

}

