/*
 * appSACS.h
 *
 *  Created on: 7 juin 2023
 *      Author: leila.medina
 */

#ifndef APP_INC_APPSACS_H_
#define APP_INC_APPSACS_H_

#include "SX1272.h"

#define MAX_SIZE_DATA 16
#define MAX_SIZE_PAYLOAD 19

#define CRC16_POLY 0x1021 // Polynôme CRC-16-CCITT

#define MASK_ACKNOLEDGE 0b00000001

typedef struct frameSACS_s {
	uint8_t sid;                  // SLAVE ID
	uint8_t ack;                  // ACKNOWLEDGEMENT
	uint8_t sizeData;            // SIZE DATA
	uint8_t data[MAX_SIZE_DATA];  // DATA
	uint16_t crc;	              // CRC
} frameSACS_s;

// Fonction qui envoie une trame SACS
// où frame est une structure contenant les paramètres de la trame
// Renvoie: - 0 si tout est OK
//   		- 1 si erreur pendant l'execution de la commande
//			- 2 si la commande n'a pas été executée
uint8_t APP_SACS_send(frameSACS_s frame);

// Fonction qui envoie une trame SACS
// où frame est une structure contenant les paramètres de la trame
// et timeOut est le temps limite pour que la commande s'execute.
// Renvoie: - 0 si tout est OK
//   		- 1 si erreur pendant l'execution de la commande
//			- 2 si la commande n'a pas été executée
//          - 3 Le CRC reçu ne correspond pas au CRC calculé, les données sont invalides
uint8_t APP_SACS_receive(frameSACS_s* frame, uint32_t timeOut);

// Fonction qui calcule le CRC avec le Polynôme CRC-16-CCITT
// où payload est la trame sous forme de tableau et sizeCRC est la taille du tableau à checker
// Renvoie les 16bits du CRC dans un uint16_t
uint16_t APP_SACS_calculateCRC(uint8_t *payload, uint8_t sizeCRC);

// Fonction pour vérifier la validité des données avec le CRC
// où payload est la trame sous forme de tableau et size est la taille du tableau
// Renvoie 0 si tout est OK et 1 si erreur
uint8_t APP_SACS_checkCRC(uint8_t *payload, uint8_t size);

// Fonction qui calcule le CRC a partir de la trame et qui affecte le CRC a la trame (partie CHECK)
// Prend en entrée la trame et sa taille totale
// Met a jour la trame avec le CRC correspondant
void APP_SACS_setCRC(uint8_t *payload, uint8_t size);

#endif /* APP_INC_APPSACS_H_ */