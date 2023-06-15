/*
 * subordonne.h
 *
 *  Created on: 08 june 2023
 *      Author: Maxime C
 */
#ifndef APP_INC_SUBORDONNE_H_
#define APP_INC_SUBORDONNE_H_

#include "main.h"
#include "appSACS.h"
#include "delay.h"
#include "appSX1272.h"
#include "bsp.h"

#define SUBORDONNE_DEBUG 		1
#define SUB_RECEIVE_TIMEOUT 	20000

#define MYSID SID3

int subordonneMain(void);

#endif /* APP_INC_CONTROLLER_H_ */
