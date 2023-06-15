/*
 * SX1272.c
 *
 *  Created on: 24 ao�t 2020
 *      Author: Arnaud
 */

#include "SX1272.h"
#include "comSX1272.h"
#include "delay.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"
#include "bsp.h"
#include "main.h"

SX1272status currentstate;

const double SignalBwLog[] =
{
    5.0969100130080564143587833158265,
    5.397940008672037609572522210551,
    5.6989700043360188047862611052755
};

int millis()
{
	return BSP_millis();
}

/**
 * Find maximum between two numbers.
 */
int max(float num1, float num2)
{
    return (num1 > num2 ) ? num1 : num2;
}

/*
 Function: Sets the module ON.
 Returns: Integer that determines if there has been any error
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
*/
uint8_t BSP_SX1272_ON(int type_mod)
{
	uint8_t state = 2;

	// Initialize class variables
	currentstate._bandwidth = BW_125;
	currentstate._codingRate = CR_5;
	currentstate._spreadingFactor = SF_7;
	currentstate._channel = CH_868v1;
	currentstate._header = HEADER_ON;
	currentstate._CRC = CRC_OFF;
	currentstate._modem = FSK;
	currentstate._power = 15;
	currentstate._packetNumber = 0;
	currentstate._reception = CORRECT_PACKET;
	currentstate._retries = 0;
	currentstate._maxRetries = 3;
	currentstate.packet_sent.retry = currentstate._retries;

	//Configure the MISO, MOSI, CS, SPCR.
	//BSP_SPI1_Init();

	//BSP_DELAY_ms(100);
	//BSP_DELAY_ms(100);
	//BSP_DELAY_ms(100);

	// Set Maximum Over Current Protection
	state = BSP_SX1272_setMaxCurrent(0x1B);

	if( state == 0 )
	{

	}
	else
	{
		return 1;
	}

	if(type_mod==0)
	{
		// set LoRa mode
		state = BSP_SX1272_setLORA();
	}
	else
	{
		state = BSP_SX1272_setFSK();
	}
	return state;
}

/*
 Function: Sets the module OFF.
 Returns: 0
*/
void BSP_SX1272_OFF(int type_mod)
{
	if(type_mod==0)
	{
		BSP_SX1272_Write(REG_OP_MODE, LORA_STANDBY_MODE); // FSK standby mode
		BSP_SX1272_Write(REG_OP_MODE, LORA_SLEEP_MODE); // FSK standby mode
	}
	else
	{
		BSP_SX1272_Write(REG_OP_MODE, FSK_STANDBY_MODE); // FSK standby mode
		BSP_SX1272_Write(REG_OP_MODE, FSK_SLEEP_MODE); // FSK standby mode
	}
}

/*
 * Function: Clears the interruption flags
 *
 * LoRa Configuration registers are accessed through the SPI interface.
 * Registers are readable in all device mode including Sleep. However, they
 * should be written only in Sleep and Stand-by modes.
 *
 * Returns: Nothing
*/
void BSP_SX1272_clearFlags()
{
    uint8_t st0;

	// Save the previous status
	st0 = BSP_SX1272_Read(REG_OP_MODE);

	if( currentstate._modem == LORA )
	{
		/// LoRa mode
		// Stdby mode to write in registers
		BSP_SX1272_Write(REG_OP_MODE, LORA_STANDBY_MODE);
		// LoRa mode flags register
		BSP_SX1272_Write(REG_IRQ_FLAGS, 0xFF);
		// Getting back to previous status
		BSP_SX1272_Write(REG_OP_MODE, st0);

		#if (SX1272_debug_mode > 1)
			my_printf("## LoRa flags cleared ##\r\n");
		#endif
	}
	else
	{
		/// FSK mode
		// Stdby mode to write in registers
		BSP_SX1272_Write(REG_OP_MODE, FSK_STANDBY_MODE);
		// FSK mode flags1 register
		BSP_SX1272_Write(REG_IRQ_FLAGS1, 0xFF);
		// FSK mode flags2 register
		BSP_SX1272_Write(REG_IRQ_FLAGS2, 0xFF);
		// Getting back to previous status
		BSP_SX1272_Write(REG_OP_MODE, st0);

		#if (SX1272_debug_mode > 1)
			my_printf("## FSK flags cleared ##\r\n");
		#endif
	}
}

/*
 Function: Sets the module in LoRa mode.
 Returns:  Integer that determines if there has been any error
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
*/
uint8_t BSP_SX1272_setLORA()
{
    uint8_t state = 2;
    uint8_t st0;

	#if (SX1272_debug_mode > 1)
		my_printf("\r\n");
		my_printf("Starting 'BSP_SX1272_setLORA'\r\n");
	#endif

	BSP_SX1272_Write(REG_OP_MODE, FSK_SLEEP_MODE);    // Sleep mode (mandatory to set LoRa mode)
	BSP_SX1272_Write(REG_OP_MODE, LORA_SLEEP_MODE);    // LoRa sleep mode
	BSP_SX1272_Write(REG_OP_MODE, LORA_STANDBY_MODE);	// LoRa standby mode

	BSP_SX1272_Write(REG_MAX_PAYLOAD_LENGTH,MAX_LENGTH);

	// Set RegModemConfig1 to Default values
	BSP_SX1272_Write(REG_MODEM_CONFIG1, 0x08);
	// Set RegModemConfig2 to Default values
	BSP_SX1272_Write(REG_MODEM_CONFIG2, 0x74);

	//BSP_DELAY_ms(100);

	st0 = BSP_SX1272_Read(REG_OP_MODE);	// Reading config mode
	if( st0 == LORA_STANDBY_MODE )
	{ // LoRa mode
		currentstate._modem = LORA;
		state = 0;
		#if (SX1272_debug_mode > 1)
			my_printf("## LoRa set with success ##\r\n");
			my_printf("\r\n");
		#endif
	}
	else
	{ // FSK mode
		currentstate._modem = FSK;
		state = 1;
		#if (SX1272_debug_mode > 1)
			my_printf("** There has been an error while setting LoRa **\r\n");
			my_printf("\r\n");
		#endif
	}
	return state;
}

/*
 Function: Sets the module in FSK mode.
 Returns:   Integer that determines if there has been any error
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
*/
uint8_t BSP_SX1272_setFSK()
{
	uint8_t state = 2;
    uint8_t st0;
    uint8_t config1;

	#if (SX1272_debug_mode > 1)
		my_printf("\r\n");
		my_printf("Starting 'setFSK'\r\n");
	#endif

  BSP_SX1272_Write(REG_OP_MODE, LORA_SLEEP_MODE);
	BSP_SX1272_Write(REG_OP_MODE, FSK_SLEEP_MODE);	// Sleep mode (mandatory to change mode)
	BSP_SX1272_Write(REG_OP_MODE, FSK_STANDBY_MODE);	// FSK standby mode

	/////////////////////////////////////////////////////////////////////////////////////////
  // Config REG_PACKET_CONFIG1
	config1 = BSP_SX1272_Read(REG_PACKET_CONFIG1);
	config1 = config1 & 0b01111101;		// clears bits 8 and 1 from REG_PACKET_CONFIG1
	config1 = config1 | 0b00000100;		// sets bit 2 from REG_PACKET_CONFIG1
  BSP_SX1272_Write(REG_PACKET_CONFIG1,config1);	// Packet format fixe, AddressFiltering = NodeAddress + BroadcastAddress
  /////////////////////////////////////////////////////////////////////////////////////////

	BSP_SX1272_Write(REG_FIFO_THRESH, 0x80);	// condition to start packet tx

	/////////////////////////////////////////////////////////////////////////////////////////
  // Config REG_SYNC_CONFIG
	config1 = BSP_SX1272_Read(REG_SYNC_CONFIG);
	config1 = config1 & 0b00111111; //Auto-restart off, sync word detection on, sync size = 4 words
	BSP_SX1272_Write(REG_SYNC_CONFIG,config1);

  BSP_SX1272_Write(REG_FDEV_MSB,0x07);
  BSP_SX1272_Write(REG_FDEV_LSB,0xFF);

  BSP_SX1272_Write(REG_BITRATE_MSB,0x68);
  BSP_SX1272_Write(REG_BITRATE_LSB,0x2B);

  /////////////////////////////////////////////////////////////////////////////////////////
  // Config REG_PACKET_CONFIG2
  BSP_SX1272_Write(REG_PACKET_CONFIG2,0x40); // packet mode
  /////////////////////////////////////////////////////////////////////////////////////////

	BSP_DELAY_ms(100);

	st0 = BSP_SX1272_Read(REG_OP_MODE);	// Reading config mode
	if( st0 == FSK_STANDBY_MODE )
	{ // FSK mode
		currentstate._modem = FSK;
		state = 0;
		#if (SX1272_debug_mode > 1)
			my_printf("## FSK set with success ##\r\n");
			my_printf("\r\n");
		#endif
	}
	else
	{ // LoRa mode
		currentstate._modem = LORA;
		state = 1;
		#if (SX1272_debug_mode > 1)
			my_printf("** There has been an error while setting FSK **\r\n");
			my_printf("\r\n");
		#endif
	}
	return state;
}

/*
 Function: Gets the bandwidth, coding rate and spreading factor of the LoRa modulation.
 Returns: Integer that determines if there has been any error
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
*/
uint8_t BSP_SX1272_getMode()
{
	uint8_t st0;
	int8_t state = 2;
	uint8_t value = 0x00;

	#if (SX1272_debug_mode > 1)
	  my_printf("\r\n");
	  my_printf("Starting 'getMode'\r\n");
	#endif

	// Save the previous status
	st0 = BSP_SX1272_Read(REG_OP_MODE);
	// Setting LoRa mode
	if( currentstate._modem == FSK )
	{
		BSP_SX1272_setLORA();
	}
	value = BSP_SX1272_Read(REG_MODEM_CONFIG1);
	currentstate._bandwidth = (value >> 6);   				// Storing 2 MSB from REG_MODEM_CONFIG1 (=currentstate._bandwidth)
	currentstate._codingRate = (value >> 3) & 0x07;  		// Storing third, forth and fifth bits from
	value = BSP_SX1272_Read(REG_MODEM_CONFIG2);	// REG_MODEM_CONFIG1 (=currentstate._codingRate)
	currentstate._spreadingFactor = (value >> 4) & 0x0F; 	// Storing 4 MSB from REG_MODEM_CONFIG2 (=currentstate._spreadingFactor)
	state = 1;

	if( BSP_SX1272_isBW(currentstate._bandwidth) )		// Checking available values for:
	{								//		currentstate._bandwidth
		if( BSP_SX1272_isCR(currentstate._codingRate) )		//		currentstate._codingRate
		{							//		currentstate._spreadingFactor
			if( BSP_SX1272_isSF(currentstate._spreadingFactor) )
			{
				state = 0;
			}
		}
	}

	#if (SX1272_debug_mode > 1)
	  my_printf("## Parameters from configuration mode are:\r\n");
	  my_printf("Bandwidth: ");
	  my_printf("%d",currentstate._bandwidth);
	  my_printf("\r\n");
	  my_printf("\t Coding Rate: ");
	  my_printf("%d",currentstate._codingRate);
	  my_printf("\r\n");
	  my_printf("\t Spreading Factor: ");
	  my_printf("%d",currentstate._spreadingFactor);
	  my_printf(" ##\r\n");
	  my_printf("\r\n");
	#endif

	BSP_SX1272_Write(REG_OP_MODE, st0);	// Getting back to previous status
	return state;
}

/*
 Function: Sets the bandwidth, coding rate and spreading factor of the LoRa modulation.
 Returns: Integer that determines if there has been any error
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
   state = -1 --> Forbidden command for this protocol
 Parameters:
   mode: mode number to set the required BW, SF and CR of LoRa modem.
*/
int8_t BSP_SX1272_setMode(uint8_t mode)
{
	int8_t state = 2;
	uint8_t st0;
	uint8_t config1 = 0x00;
	uint8_t config2 = 0x00;

	#if (SX1272_debug_mode > 1)
		my_printf("\r\n");
		my_printf("Starting 'setMode'\r\n");
	#endif

	st0 = BSP_SX1272_Read(REG_OP_MODE);		// Save the previous status

	// 'setMode' function only can be called in LoRa mode
	if( currentstate._modem == FSK )
	{
		BSP_SX1272_setLORA();
	}

	// LoRa standby mode
	BSP_SX1272_Write(REG_OP_MODE, LORA_STANDBY_MODE);

	switch (mode)
	{
		// mode 1 (better reach, medium time on air)
		case 1: 	BSP_SX1272_setCR(CR_5);		// CR = 4/5
					BSP_SX1272_setSF(SF_12);		// SF = 12
					BSP_SX1272_setBW(BW_125);		// BW = 125 KHz
					break;

		// mode 2 (medium reach, less time on air)
		case 2: 	BSP_SX1272_setCR(CR_5);		// CR = 4/5
					BSP_SX1272_setSF(SF_12);		// SF = 12
					BSP_SX1272_setBW(BW_250);		// BW = 250 KHz
					break;

		// mode 3 (worst reach, less time on air)
		case 3: 	BSP_SX1272_setCR(CR_5);		// CR = 4/5
					BSP_SX1272_setSF(SF_10);		// SF = 10
					BSP_SX1272_setBW(BW_125);		// BW = 125 KHz
					break;

		// mode 4 (better reach, low time on air)
		case 4: 	BSP_SX1272_setCR(CR_5);		// CR = 4/5
					BSP_SX1272_setSF(SF_12);		// SF = 12
					BSP_SX1272_setBW(BW_500);		// BW = 500 KHz
					break;

		// mode 5 (better reach, medium time on air)
		case 5: 	BSP_SX1272_setCR(CR_5);		// CR = 4/5
					BSP_SX1272_setSF(SF_10);		// SF = 10
					BSP_SX1272_setBW(BW_250);		// BW = 250 KHz
					break;

		// mode 6 (better reach, worst time-on-air)
		case 6: 	BSP_SX1272_setCR(CR_5);		// CR = 4/5
					BSP_SX1272_setSF(SF_11);		// SF = 11
					BSP_SX1272_setBW(BW_500);		// BW = 500 KHz
					break;

		// mode 7 (medium-high reach, medium-low time-on-air)
		case 7: 	BSP_SX1272_setCR(CR_5);		// CR = 4/5
					BSP_SX1272_setSF(SF_9);		// SF = 9
					BSP_SX1272_setBW(BW_250);		// BW = 250 KHz
					break;

		// mode 8 (medium reach, medium time-on-air)
		case 8:		BSP_SX1272_setCR(CR_5);		// CR = 4/5
					BSP_SX1272_setSF(SF_9);		// SF = 9
					BSP_SX1272_setBW(BW_500);		// BW = 500 KHz
					break;

		// mode 9 (medium-low reach, medium-high time-on-air)
		case 9: 	BSP_SX1272_setCR(CR_5);		// CR = 4/5
					BSP_SX1272_setSF(SF_8);		// SF = 8
					BSP_SX1272_setBW(BW_500);		// BW = 500 KHz
					break;

		// mode 10 (worst reach, less time_on_air)
		case 10:	BSP_SX1272_setCR(CR_5);		// CR = 4/5
					BSP_SX1272_setSF(SF_7);		// SF = 7
					BSP_SX1272_setBW(BW_500);		// BW = 500 KHz
					break;

		default:	state = -1; // The indicated mode doesn't exist

	};


	// Check proper register configuration
	if( state == -1 )	// if state = -1, don't change its value
	{
		#if (SX1272_debug_mode > 1)
			my_printf("** The indicated mode doesn't exist, ");
			my_printf("please select from 1 to 10 **\r\n");
		#endif
	}
	else
	{
		state = 1;
		config1 = BSP_SX1272_Read(REG_MODEM_CONFIG1);
		switch (mode)
		{	//		Different way to check for each mode:
			// (config1 >> 3) ---> take out bits 7-3 from REG_MODEM_CONFIG1 (=currentstate._bandwidth & currentstate._codingRate together)
			// (config2 >> 4) ---> take out bits 7-4 from REG_MODEM_CONFIG2 (=currentstate._spreadingFactor)

			// mode 1: BW = 125 KHz, CR = 4/5, SF = 12.
			case 1:  if( (config1 >> 3) == 0x01 )
						{  config2 = BSP_SX1272_Read(REG_MODEM_CONFIG2);
						if( (config2 >> 4) == SF_12 )
							{
							state = 0;
							}
						}
 					 break;


			// mode 2: BW = 250 KHz, CR = 4/5, SF = 12.
			case 2:  if( (config1 >> 3) == 0x09 )
						{  config2 = BSP_SX1272_Read(REG_MODEM_CONFIG2);
						if( (config2 >> 4) == SF_12 )
							{
							state = 0;
							}
						}
 					 break;

			// mode 3: BW = 125 KHz, CR = 4/5, SF = 10.
			case 3:  if( (config1 >> 3) == 0x01 )
						{  config2 = BSP_SX1272_Read(REG_MODEM_CONFIG2);
						if( (config2 >> 4) == SF_10 )
							{
							state = 0;
							}
						}
 					 break;

			// mode 4: BW = 500 KHz, CR = 4/5, SF = 12.
			case 4:  if( (config1 >> 3) == 0x11 )
						{  config2 = BSP_SX1272_Read(REG_MODEM_CONFIG2);
						if( (config2 >> 4) == SF_12 )
							{
							state = 0;
							}
						}
 					 break;

			// mode 5: BW = 250 KHz, CR = 4/5, SF = 10.
			case 5:  if( (config1 >> 3) == 0x09 )
						{  config2 = BSP_SX1272_Read(REG_MODEM_CONFIG2);
						if( (config2 >> 4) == SF_10 )
							{
							state = 0;
							}
						}
 					 break;

			// mode 6: BW = 500 KHz, CR = 4/5, SF = 11.
			case 6:  if( (config1 >> 3) == 0x11 )
						{  config2 = BSP_SX1272_Read(REG_MODEM_CONFIG2);
						if( (config2 >> 4) == SF_11 )
							{
							state = 0;
							}
						}
 					 break;

			// mode 7: BW = 250 KHz, CR = 4/5, SF = 9.
			case 7:  if( (config1 >> 3) == 0x09 )
						{  config2 = BSP_SX1272_Read(REG_MODEM_CONFIG2);
						if( (config2 >> 4) == SF_9 )
							{
							state = 0;
							}
						}
 					 break;

			// mode 8: BW = 500 KHz, CR = 4/5, SF = 9.
			case 8:  if ((config1 >> 3) == 0x11)
						{  config2 = BSP_SX1272_Read(REG_MODEM_CONFIG2);
						if( (config2 >> 4) == SF_9 )
							{
							state = 0;
							}
						}
 					 break;

			// mode 9: BW = 500 KHz, CR = 4/5, SF = 8.
			case 9:  if( (config1 >> 3) == 0x11 )
						{  config2 = BSP_SX1272_Read(REG_MODEM_CONFIG2);
						if( (config2 >> 4) == SF_8 )
							{
							state = 0;
							}
						}
 					 break;

			// mode 10: BW = 500 KHz, CR = 4/5, SF = 7.
			case 10: if( (config1 >> 3) == 0x11 )
						{  config2 = BSP_SX1272_Read(REG_MODEM_CONFIG2);
						if( (config2 >> 4) == SF_7 )
							{
							state = 0;
							}
						}
		}// end switch
	}

	#if (SX1272_debug_mode > 1)
	if( state == 0 )
	{
		my_printf("## Mode ");
		my_printf("%d",mode);
		my_printf(" configured with success ##\r\n");
	}
	else
	{
		my_printf("** There has been an error while configuring mode ");
		my_printf("%d",mode);
		my_printf(". **\r\n");
	}
	#endif

	// Getting back to previous status
	BSP_SX1272_Write(REG_OP_MODE, st0);

	return state;
}

/*
 Function: Indicates if module is configured in implicit or explicit header mode.
 Returns: Integer that determines if there has been any error
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
*/
uint8_t	BSP_SX1272_getHeader()
{
	int8_t state = 2;

	#if (SX1272_debug_mode > 1)
		my_printf("\r\n");
		my_printf("Starting 'getHeader'\r\n");
	#endif

	// take out bit 2 from REG_MODEM_CONFIG1 indicates ImplicitHeaderModeOn
	if( bitRead(BSP_SX1272_Read(REG_MODEM_CONFIG1), 2) == 0 )
	{ // explicit header mode (ON)
		currentstate._header = HEADER_ON;
		state = 1;
	}
	else
	{ // implicit header mode (OFF)
		currentstate._header = HEADER_OFF;
		state = 1;
	}

	state = 0;

	if( currentstate._modem == FSK )
	{ // header is not available in FSK mode
		#if (SX1272_debug_mode > 1)
			my_printf("## Notice that FSK mode packets hasn't header ##\r\n");
			my_printf("\r\n");
		#endif
	}
	else
	{ // header in LoRa mode
		#if (SX1272_debug_mode > 1)
			my_printf("## Header is ");
			if( currentstate._header == HEADER_ON )
			{
				my_printf("in explicit header mode ##\r\n");
			}
			else
			{
				my_printf("in implicit header mode ##\r\n");
			}
			my_printf("\r\n");
		#endif
	}
	return state;
}

/*
 Function: Sets the module in explicit header mode (header is sent).
 Returns: Integer that determines if there has been any error
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
   state = -1 --> Forbidden command for this protocol
*/
int8_t	BSP_SX1272_setHeaderON()
{
  int8_t state = 2;
  uint8_t config1;

  #if (SX1272_debug_mode > 1)
	  my_printf("\r\n");
	  my_printf("Starting 'setHeaderON'\r\n");
  #endif

  if( currentstate._modem == FSK )
  {
	  state = -1;		// header is not available in FSK mode
	  #if (SX1272_debug_mode > 1)
		  my_printf("## FSK mode packets hasn't header ##\r\n");
		  my_printf("\r\n");
	  #endif
  }
  else
  {
	config1 = BSP_SX1272_Read(REG_MODEM_CONFIG1);	// Save config1 to modify only the header bit
	if( currentstate._spreadingFactor == 6 )
	{
		state = -1;		// Mandatory headerOFF with SF = 6
		#if (SX1272_debug_mode > 1)
			my_printf("## Mandatory implicit header mode with spreading factor = 6 ##\r\n");
		#endif
	}
	else
	{
		config1 = config1 & 0b11111011;			// clears bit 2 from config1 = headerON
		BSP_SX1272_Write(REG_MODEM_CONFIG1,config1);	// Update config1
	}
	if( currentstate._spreadingFactor != 6 )
	{ // checking headerON taking out bit 2 from REG_MODEM_CONFIG1
		config1 = BSP_SX1272_Read(REG_MODEM_CONFIG1);
		if( bitRead(config1, 2) == HEADER_ON )
		{
			state = 0;
			currentstate._header = HEADER_ON;
			#if (SX1272_debug_mode > 1)
				my_printf("## Header has been activated ##\r\n");
				my_printf("\r\n");
			#endif
		}
		else
		{
			state = 1;
		}
	}
  }
  return state;
}

/*
 Function: Sets the module in implicit header mode (header is not sent).
 Returns: Integer that determines if there has been any error
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
   state = -1 --> Forbidden command for this protocol
*/
int8_t	BSP_SX1272_setHeaderOFF()
{
	uint8_t state = 2;
	uint8_t config1;

	#if (SX1272_debug_mode > 1)
		my_printf("\r\n");
		my_printf("Starting 'setHeaderOFF'\r\n");
	#endif

	if( currentstate._modem == FSK )
	{
		// header is not available in FSK mode
		state = -1;
		#if (SX1272_debug_mode > 1)
			my_printf("## Notice that FSK mode packets hasn't header ##\r\n");
			my_printf("\r\n");
		#endif
	}
	else
	{
		// Read config1 to modify only the header bit
		config1 = BSP_SX1272_Read(REG_MODEM_CONFIG1);

		// sets bit 2 from REG_MODEM_CONFIG1 = headerOFF
		config1 = config1 | 0b00000100;
		// Update config1
		BSP_SX1272_Write(REG_MODEM_CONFIG1,config1);

		// check register
		config1 = BSP_SX1272_Read(REG_MODEM_CONFIG1);
		if( bitRead(config1, 2) == HEADER_OFF )
		{
			// checking headerOFF taking out bit 2 from REG_MODEM_CONFIG1
			state = 0;
			currentstate._header = HEADER_OFF;

			#if (SX1272_debug_mode > 1)
			    my_printf("## Header has been desactivated ##\r\n");
			    my_printf("\r\n");
			#endif
		}
		else
		{
			state = 1;
			#if (SX1272_debug_mode > 1)
				my_printf("** Header hasn't been desactivated ##\r\n");
				my_printf("\r\n");
			#endif
		}
	}
	return state;
}

/*
 Function: Indicates if module is configured with or without checking CRC.
 Returns: Integer that determines if there has been any error
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
*/
uint8_t	BSP_SX1272_getCRC()
{
	int8_t state = 2;
	uint8_t value;

	#if (SX1272_debug_mode > 1)
		my_printf("\r\n");
		my_printf("Starting 'getCRC'\r\n");
	#endif

	if( currentstate._modem == LORA )
	{ // LoRa mode

		// take out bit 1 from REG_MODEM_CONFIG1 indicates RxPayloadCrcOn
		value = BSP_SX1272_Read(REG_MODEM_CONFIG1);
		if( bitRead(value, 1) == CRC_OFF )
		{ // CRCoff
			currentstate._CRC = CRC_OFF;
			#if (SX1272_debug_mode > 1)
				my_printf("## CRC is desactivated ##\r\n");
				my_printf("\r\n");
			#endif
			state = 0;
		}
		else
		{ // CRCon
			currentstate._CRC = CRC_ON;
			#if (SX1272_debug_mode > 1)
				my_printf("## CRC is activated ##\r\n");
				my_printf("\r\n");
			#endif
			state = 0;
		}
	}
	else
	{ // FSK mode

		// take out bit 2 from REG_PACKET_CONFIG1 indicates CrcOn
		value = BSP_SX1272_Read(REG_PACKET_CONFIG1);
		if( bitRead(value, 4) == CRC_OFF )
		{ // CRCoff
			currentstate._CRC = CRC_OFF;
			#if (SX1272_debug_mode > 1)
				my_printf("## CRC is desactivated ##\r\n");
				my_printf("\r\n");
			#endif
			state = 0;
		}
		else
		{ // CRCon
			currentstate._CRC = CRC_ON;
			#if (SX1272_debug_mode > 1)
				my_printf("## CRC is activated ##\r\n");
				my_printf("\r\n");
			#endif
			state = 0;
		}
	}
	if( state != 0 )
	{
		state = 1;
		#if (SX1272_debug_mode > 1)
			my_printf("** There has been an error while getting configured CRC **\r\n");
			my_printf("\r\n");
		#endif
	}
	return state;
}

/*
 Function: Sets the module with CRC on.
 Returns: Integer that determines if there has been any error
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
*/
uint8_t	BSP_SX1272_setCRC_ON()
{
  uint8_t state = 2;
  uint8_t config1;

  #if (SX1272_debug_mode > 1)
	  my_printf("\r\n");
	  my_printf("Starting 'setCRC_ON'\r\n");
  #endif

  if( currentstate._modem == LORA )
  { // LORA mode
	config1 = BSP_SX1272_Read(REG_MODEM_CONFIG1);	// Save config1 to modify only the CRC bit
	config1 = config1 | 0b00000010;				// sets bit 1 from REG_MODEM_CONFIG1 = CRC_ON
	BSP_SX1272_Write(REG_MODEM_CONFIG1,config1);

	state = 1;

	config1 = BSP_SX1272_Read(REG_MODEM_CONFIG1);
	if( bitRead(config1, 1) == CRC_ON )
	{ // take out bit 1 from REG_MODEM_CONFIG1 indicates RxPayloadCrcOn
		state = 0;
		currentstate._CRC = CRC_ON;
		#if (SX1272_debug_mode > 1)
			my_printf("## CRC has been activated ##\r\n");
			my_printf("\r\n");
		#endif
	}
  }
  else
  { // FSK mode
	config1 = BSP_SX1272_Read(REG_PACKET_CONFIG1);	// Save config1 to modify only the CRC bit
	config1 = config1 | 0b00010000;				// set bit 4 and 3 from REG_MODEM_CONFIG1 = CRC_ON
	BSP_SX1272_Write(REG_PACKET_CONFIG1,config1);

	state = 1;

	config1 = BSP_SX1272_Read(REG_PACKET_CONFIG1);
	if( bitRead(config1, 4) == CRC_ON )
	{ // take out bit 4 from REG_PACKET_CONFIG1 indicates CrcOn
		state = 0;
		currentstate._CRC = CRC_ON;
		#if (SX1272_debug_mode > 1)
			my_printf("## CRC has been activated ##\r\n");
			my_printf("\r\n");
		#endif
	}
  }
  if( state != 0 )
  {
	  state = 1;
	  #if (SX1272_debug_mode > 1)
		  my_printf("** There has been an error while setting CRC ON **\r\n");
		  my_printf("\r\n");
	  #endif
  }
  return state;
}

/*
 Function: Sets the module with CRC off.
 Returns: Integer that determines if there has been any error
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
*/
uint8_t	BSP_SX1272_setCRC_OFF()
{
  int8_t state = 2;
  uint8_t config1;

  #if (SX1272_debug_mode > 1)
	  my_printf("\r\n");
	  my_printf("Starting 'setCRC_OFF'\r\n");
  #endif

  if( currentstate._modem == LORA )
  { // LORA mode
  	config1 = BSP_SX1272_Read(REG_MODEM_CONFIG1);	// Save config1 to modify only the CRC bit
	config1 = config1 & 0b11111101;				// clears bit 1 from config1 = CRC_OFF
	BSP_SX1272_Write(REG_MODEM_CONFIG1,config1);

	config1 = BSP_SX1272_Read(REG_MODEM_CONFIG1);
	if( (bitRead(config1, 1)) == CRC_OFF )
	{ // take out bit 1 from REG_MODEM_CONFIG1 indicates RxPayloadCrcOn
	  state = 0;
	  currentstate._CRC = CRC_OFF;
	  #if (SX1272_debug_mode > 1)
		  my_printf("## CRC has been desactivated ##\r\n");
		  my_printf("\r\n");
	  #endif
	}
  }
  else
  { // FSK mode
	config1 = BSP_SX1272_Read(REG_PACKET_CONFIG1);	// Save config1 to modify only the CRC bit
	config1 = config1 & 0b11101111;				// clears bit 4 from config1 = CRC_OFF
	BSP_SX1272_Write(REG_PACKET_CONFIG1,config1);

	config1 = BSP_SX1272_Read(REG_PACKET_CONFIG1);
	if( bitRead(config1, 4) == CRC_OFF )
	{ // take out bit 4 from REG_PACKET_CONFIG1 indicates RxPayloadCrcOn
		state = 0;
		currentstate._CRC = CRC_OFF;
		#if (SX1272_debug_mode > 1)
		    my_printf("## CRC has been desactivated ##\r\n");
		    my_printf("\r\n");
	    #endif
	}
  }
  if( state != 0 )
  {
	  state = 1;
	  #if (SX1272_debug_mode > 1)
		  my_printf("** There has been an error while setting CRC OFF **\r\n");
		  my_printf("\r\n");
	  #endif
  }
  return state;
}

/*
 Function: Checks if SF is a valid value.
 Returns: uint8_t that's '1' if the SF value exists and
		  it's '0' if the SF value does not exist.
 Parameters:
   spr: spreading factor value to check.
*/
uint8_t	BSP_SX1272_isSF(uint8_t spr)
{
  #if (SX1272_debug_mode > 1)
	  my_printf("\r\n");
	  my_printf("Starting 'BSP_SX1272_isSF'\r\n");
  #endif

  // Checking available values for currentstate._spreadingFactor
  switch(spr)
  {
	  case SF_6:
	  case SF_7:
	  case SF_8:
	  case SF_9:
	  case SF_10:
	  case SF_11:
	  case SF_12:	return 1;
					break;

	  default:		return 0;
  }
  #if (SX1272_debug_mode > 1)
	  my_printf("## Finished 'BSP_SX1272_isSF' ##\r\n");
	  my_printf("\r\n");
  #endif
}

/*
 Function: Gets the SF within the module is configured.
 Returns: Integer that determines if there has been any error
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
   state = -1 --> Forbidden command for this protocol
*/
int8_t	BSP_SX1272_getSF()
{
  int8_t state = 2;
  uint8_t config2;

  #if (SX1272_debug_mode > 1)
	  my_printf("\r\n");
	  my_printf("Starting 'getSF'\r\n");
  #endif

  if( currentstate._modem == FSK )
  {
	  state = -1;		// SF is not available in FSK mode
	  #if (SX1272_debug_mode > 1)
		  my_printf("** FSK mode hasn't spreading factor **\r\n");
		  my_printf("\r\n");
	  #endif
  }
  else
  {
	// take out bits 7-4 from REG_MODEM_CONFIG2 indicates currentstate._spreadingFactor
	config2 = (BSP_SX1272_Read(REG_MODEM_CONFIG2)) >> 4;
	currentstate._spreadingFactor = config2;
	state = 1;

	if( (config2 == currentstate._spreadingFactor) && BSP_SX1272_isSF(currentstate._spreadingFactor) )
	{
		state = 0;
		#if (SX1272_debug_mode > 1)
			my_printf("## Spreading factor is ");
			my_printf("%d",currentstate._spreadingFactor);
			my_printf(" ##\r\n");
			my_printf("\r\n");
		#endif
	}
  }
  return state;
}

/*
 Function: Sets the indicated SF in the module.
 Returns: Integer that determines if there has been any error
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
 Parameters:
   spr: spreading factor value to set in LoRa modem configuration.
*/
uint8_t	BSP_SX1272_setSF(uint8_t spr)
{
	uint8_t st0;
	int8_t state = 2;
	uint8_t config1;
	uint8_t config2;

	#if (SX1272_debug_mode > 1)
		my_printf("\r\n");
		my_printf("Starting 'BSP_SX1272_setSF'\r\n");
	#endif

	st0 = BSP_SX1272_Read(REG_OP_MODE);	// Save the previous status

	if( currentstate._modem == FSK )
	{
		/// FSK mode
		#if (SX1272_debug_mode > 1)
			my_printf("## Notice that FSK hasn't Spreading Factor parameter, ");
			my_printf("so you are configuring it in LoRa mode ##\r\n");
		#endif
		state = BSP_SX1272_setLORA();				// Setting LoRa mode
	}
	else
	{
		/// LoRa mode
		// LoRa standby mode
		BSP_SX1272_Write(REG_OP_MODE, LORA_STANDBY_MODE);

		// Read config1 to modify only the LowDataRateOptimize
		config1 = (BSP_SX1272_Read(REG_MODEM_CONFIG1));
		// Read config2 to modify SF value (bits 7-4)
		config2 = (BSP_SX1272_Read(REG_MODEM_CONFIG2));

		switch(spr)
		{
			case SF_6:
					config2 = config2 & 0b01101111;	// clears bits 7 & 4 from REG_MODEM_CONFIG2
					config2 = config2 | 0b01100000;	// sets bits 6 & 5 from REG_MODEM_CONFIG2
					break;
			case SF_7:
					config2 = config2 & 0b01111111;	// clears bits 7 from REG_MODEM_CONFIG2
					config2 = config2 | 0b01110000;	// sets bits 6, 5 & 4
					break;

			case SF_8:
					config2 = config2 & 0b10001111;	// clears bits 6, 5 & 4 from REG_MODEM_CONFIG2
					config2 = config2 | 0b10000000;	// sets bit 7 from REG_MODEM_CONFIG2
					break;

			case SF_9:
					config2 = config2 & 0b10011111;	// clears bits 6, 5 & 4 from REG_MODEM_CONFIG2
					config2 = config2 | 0b10010000;	// sets bits 7 & 4 from REG_MODEM_CONFIG2
					break;

			case SF_10:	config2 = config2 & 0b10101111;	// clears bits 6 & 4 from REG_MODEM_CONFIG2
					config2 = config2 | 0b10100000;	// sets bits 7 & 5 from REG_MODEM_CONFIG2
					break;

			case SF_11:
					config2 = config2 & 0b10111111;	// clears bit 6 from REG_MODEM_CONFIG2
					config2 = config2 | 0b10110000;	// sets bits 7, 5 & 4 from REG_MODEM_CONFIG2
					BSP_SX1272_getBW();
					//if( currentstate._bandwidth == BW_125 )
					{ // LowDataRateOptimize (Mandatory with SF_11 if BW_125)
						config1 = config1 | 0b00000001;
					}
					break;

			case SF_12:
					config2 = config2 & 0b11001111;	// clears bits 5 & 4 from REG_MODEM_CONFIG2
					config2 = config2 | 0b11000000;	// sets bits 7 & 6 from REG_MODEM_CONFIG2
					//if( currentstate._bandwidth == BW_125 )
					{ // LowDataRateOptimize (Mandatory with SF_12 if BW_125)
						config1 = config1 | 0b00000001;
					}
					break;
	}

	// Check if it is neccesary to set special settings for SF=6
	if( spr == SF_6 )
	{
		// Mandatory headerOFF with SF = 6 (Implicit mode)
		BSP_SX1272_setHeaderOFF();

		// Set the bit field DetectionOptimize of
		// register RegLoRaDetectOptimize to value "0b101".
		BSP_SX1272_Write(REG_DETECT_OPTIMIZE, 0x05);

		// Write 0x0C in the register RegDetectionThreshold.
		BSP_SX1272_Write(REG_DETECTION_THRESHOLD, 0x0C);
	}
	else
	{
		// LoRa detection Optimize: 0x03 --> SF7 to SF12
		BSP_SX1272_Write(REG_DETECT_OPTIMIZE, 0x03);

		// LoRa detection threshold: 0x0A --> SF7 to SF12
		BSP_SX1272_Write(REG_DETECTION_THRESHOLD, 0x0A);
	}

	// sets bit 2-0 (AgcAutoOn and SymbTimout) for any SF value
	config2 = config2 | 0b00000111;

	// Update 'config1' and 'config2'
	BSP_SX1272_Write(REG_MODEM_CONFIG1, config1);
	BSP_SX1272_Write(REG_MODEM_CONFIG2, config2);

	// Read 'config1' and 'config2' to check update
	config1 = (BSP_SX1272_Read(REG_MODEM_CONFIG1));
	config2 = (BSP_SX1272_Read(REG_MODEM_CONFIG2));

	// (config2 >> 4) ---> take out bits 7-4 from REG_MODEM_CONFIG2 (=currentstate._spreadingFactor)
	// bitRead(config1, 0) ---> take out bits 1 from config1 (=LowDataRateOptimize)
	switch(spr)
	{
		case SF_6:	if(		((config2 >> 4) == spr)
						&& 	(bitRead(config2, 2) == 1)
						&& 	(currentstate._header == HEADER_OFF))
					{
						state = 0;
					}
					break;
		case SF_7:	if(		((config2 >> 4) == 0x07)
						 && (bitRead(config2, 2) == 1))
					{
						state = 0;
					}
					break;
		case SF_8:	if(		((config2 >> 4) == 0x08)
						 && (bitRead(config2, 2) == 1))
					{
						state = 0;
					}
					break;
		case SF_9:	if(		((config2 >> 4) == 0x09)
						 && (bitRead(config2, 2) == 1))
					{
						state = 0;
					}
					break;
		case SF_10:	if(		((config2 >> 4) == 0x0A)
						 && (bitRead(config2, 2) == 1))
					{
						state = 0;
					}
					break;
		case SF_11:	if(		((config2 >> 4) == 0x0B)
						 && (bitRead(config2, 2) == 1)
						 && (bitRead(config1, 0) == 1))
					{
						state = 0;
					}
					break;
		case SF_12:	if(		((config2 >> 4) == 0x0C)
						 && (bitRead(config2, 2) == 1)
						 && (bitRead(config1, 0) == 1))
					{
						state = 0;
					}
					break;
		default:	state = 1;
	}
  }

  BSP_SX1272_Write(REG_OP_MODE, st0);	// Getting back to previous status

  if( BSP_SX1272_isSF(spr) )
  { // Checking available value for currentstate._spreadingFactor
		state = 0;
		currentstate._spreadingFactor = spr;
		#if (SX1272_debug_mode > 1)
		    my_printf("## Spreading factor ");
		    my_printf("%d",currentstate._spreadingFactor);
		    my_printf(" has been successfully set ##\r\n");
		    my_printf("\r\n");
		#endif
  }
  else
  {
	  if( state != 0 )
	  {
		  #if (SX1272_debug_mode > 1)
		      my_printf("** There has been an error while setting the spreading factor **");
		      my_printf("\r\n");
		  #endif
	  }
  }
  return state;
}

/*
 Function: Checks if BW is a valid value.
 Returns: uint8_t that's '1' if the BW value exists and
		  it's '0' if the BW value does not exist.
 Parameters:
   band: bandwidth value to check.
*/
uint8_t	BSP_SX1272_isBW(uint16_t band)
{
  #if (SX1272_debug_mode > 1)
	  my_printf("\r\n");
	  my_printf("Starting 'BSP_SX1272_isBW'\r\n");
  #endif

  // Checking available values for currentstate._bandwidth
  switch(band)
  {
	  case BW_125:
	  case BW_250:
	  case BW_500:	return 1;
					break;

	  default:		return 0;
  }
  #if (SX1272_debug_mode > 1)
	  my_printf("## Finished 'BSP_SX1272_isBW' ##\r\n");
	  my_printf("\r\n");
  #endif
}

/*
 Function: Gets the BW within the module is configured.
 Returns: Integer that determines if there has been any error
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
   state = -1 --> Forbidden command for this protocol
*/
int8_t	BSP_SX1272_getBW()
{
  uint8_t state = 2;
  uint8_t config1;

  #if (SX1272_debug_mode > 1)
	  my_printf("\r\n");
	  my_printf("Starting 'getBW'\r\n");
  #endif

  if( currentstate._modem == FSK )
  {
	  state = -1;		// BW is not available in FSK mode
	  #if (SX1272_debug_mode > 1)
		  my_printf("** FSK mode hasn't bandwidth **\r\n");
		  my_printf("\r\n");
	  #endif
  }
  else
  {
	  // take out bits 7-6 from REG_MODEM_CONFIG1 indicates currentstate._bandwidth
	  config1 = (BSP_SX1272_Read(REG_MODEM_CONFIG1)) >> 6;
	  currentstate._bandwidth = config1;

	  if( (config1 == currentstate._bandwidth) && BSP_SX1272_isBW(currentstate._bandwidth) )
	  {
		  state = 0;
		  #if (SX1272_debug_mode > 1)
			  my_printf("## Bandwidth is ");
			  my_printf("%d",currentstate._bandwidth);
			  my_printf(" ##\r\n");
			  my_printf("\r\n");
		  #endif
	  }
	  else
	  {
		  state = 1;
		  #if (SX1272_debug_mode > 1)
			  my_printf("** There has been an error while getting bandwidth **");
			  my_printf("\r\n");
		  #endif
	  }
  }
  return state;
}

/*
 Function: Sets the indicated BW in the module.
 Returns: Integer that determines if there has been any error
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
 Parameters:
   band: bandwith value to set in LoRa modem configuration.
*/
int8_t	BSP_SX1272_setBW(uint16_t band)
{
  uint8_t st0;
  int8_t state = 2;
  uint8_t config1;

  #if (SX1272_debug_mode > 1)
	  my_printf("\r\n");
	  my_printf("Starting 'BSP_SX1272_setBW'\r\n");
  #endif

  st0 = BSP_SX1272_Read(REG_OP_MODE);	// Save the previous status

  if( currentstate._modem == FSK )
  {
	  #if (SX1272_debug_mode > 1)
		  my_printf("## Notice that FSK hasn't Bandwidth parameter, ");
		  my_printf("so you are configuring it in LoRa mode ##\r\n");
	  #endif
	  state = BSP_SX1272_setLORA();
  }
  BSP_SX1272_Write(REG_OP_MODE, LORA_STANDBY_MODE);	// LoRa standby mode
  config1 = (BSP_SX1272_Read(REG_MODEM_CONFIG1));	// Save config1 to modify only the BW
  switch(band)
  {
	  case BW_125:  config1 = config1 & 0b00111111;	// clears bits 7 & 6 from REG_MODEM_CONFIG1
	  	  	  	  	BSP_SX1272_getSF();
					if( currentstate._spreadingFactor == 11 )
					{ // LowDataRateOptimize (Mandatory with BW_125 if SF_11)
						config1 = config1 | 0b00000001;
					}
					if( currentstate._spreadingFactor == 12 )
					{ // LowDataRateOptimize (Mandatory with BW_125 if SF_12)
						config1 = config1 | 0b00000001;
					}
					break;
	  case BW_250:  config1 = config1 & 0b01111111;	// clears bit 7 from REG_MODEM_CONFIG1
					config1 = config1 | 0b01000000;	// sets bit 6 from REG_MODEM_CONFIG1
					break;
	  case BW_500:  config1 = config1 & 0b10111111;	//clears bit 6 from REG_MODEM_CONFIG1
					config1 = config1 | 0b10000000;	//sets bit 7 from REG_MODEM_CONFIG1
					break;
  }
  BSP_SX1272_Write(REG_MODEM_CONFIG1,config1);		// Update config1

  config1 = (BSP_SX1272_Read(REG_MODEM_CONFIG1));
  // (config1 >> 6) ---> take out bits 7-6 from REG_MODEM_CONFIG1 (=currentstate._bandwidth)
  switch(band)
  {
	   case BW_125: if( (config1 >> 6) == BW_125 )
					{
						state = 0;
						if( currentstate._spreadingFactor == 11 )
						{
							if( bitRead(config1, 0) == 1 )
							{ // LowDataRateOptimize
								state = 0;
							}
							else
							{
								state = 1;
							}
						}
						if( currentstate._spreadingFactor == 12 )
						{
							if( bitRead(config1, 0) == 1 )
							{ // LowDataRateOptimize
								state = 0;
							}
							else
							{
								state = 1;
							}
						}
					}
					break;
	   case BW_250: if( (config1 >> 6) == BW_250 )
					{
						state = 0;
					}
					break;
	   case BW_500: if( (config1 >> 6) == BW_500 )
					{
						state = 0;
					}
					break;
  }

  if( !BSP_SX1272_isBW(band) )
  {
	  state = 1;
	  #if (SX1272_debug_mode > 1)
		  my_printf("** Bandwidth ");
		  my_printf("%d",band);
		  my_printf(" is not a correct value **\r\n");
		  my_printf("\r\n");
	  #endif
  }
  else
  {
	  currentstate._bandwidth = band;
	  #if (SX1272_debug_mode > 1)
		  my_printf("## Bandwidth ");
		  my_printf("%d",band);
		  my_printf(" has been successfully set ##\r\n");
		  my_printf("\r\n");
	  #endif
  }
  BSP_SX1272_Write(REG_OP_MODE, st0);	// Getting back to previous status
  return state;
}

/*
 Function: Checks if CR is a valid value.
 Returns: uint8_t that's '1' if the CR value exists and
		  it's '0' if the CR value does not exist.
 Parameters:
   cod: coding rate value to check.
*/
uint8_t	BSP_SX1272_isCR(uint8_t cod)
{
  #if (SX1272_debug_mode > 1)
	  my_printf("\r\n");
	  my_printf("Starting 'BSP_SX1272_isCR'\r\n");
  #endif

  // Checking available values for currentstate._codingRate
  switch(cod)
  {
	  case CR_5:
	  case CR_6:
	  case CR_7:
	  case CR_8:	return 1;
					break;

	  default:		return 0;
  }
  #if (SX1272_debug_mode > 1)
	  my_printf("## Finished 'BSP_SX1272_isCR' ##\r\n");
	  my_printf("\r\n");
  #endif
}

/*
 Function: Indicates the CR within the module is configured.
 Returns: Integer that determines if there has been any error
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
   state = -1 --> Forbidden command for this protocol
*/
int8_t	BSP_SX1272_getCR()
{
  int8_t state = 2;
  uint8_t config1;

  #if (SX1272_debug_mode > 1)
	  my_printf("\r\n");
	  my_printf("Starting 'getCR'\r\n");
  #endif

  if( currentstate._modem == FSK )
  {
	  state = -1;		// CR is not available in FSK mode
	  #if (SX1272_debug_mode > 1)
		  my_printf("** FSK mode hasn't coding rate **\r\n");
		  my_printf("\r\n");
	  #endif
  }
  else
  {
	// take out bits 7-3 from REG_MODEM_CONFIG1 indicates currentstate._bandwidth & currentstate._codingRate
	config1 = (BSP_SX1272_Read(REG_MODEM_CONFIG1)) >> 3;
	config1 = config1 & 0b00000111;	// clears bits 7-5 ---> clears currentstate._bandwidth
	currentstate._codingRate = config1;
	state = 1;

	if( (config1 == currentstate._codingRate) && BSP_SX1272_isCR(currentstate._codingRate) )
	{
		state = 0;
		#if (SX1272_debug_mode > 1)
			my_printf("## Coding rate is ");
			my_printf("%d",currentstate._codingRate);
			my_printf(" ##\r\n");
			my_printf("\r\n");
		#endif
	}
  }
  return state;
}

/*
 Function: Sets the indicated CR in the module.
 Returns: Integer that determines if there has been any error
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
   state = -1 --> Forbidden command for this protocol
 Parameters:
   cod: coding rate value to set in LoRa modem configuration.
*/
int8_t	BSP_SX1272_setCR(uint8_t cod)
{
  uint8_t st0;
  int8_t state = 2;
  uint8_t config1;

  #if (SX1272_debug_mode > 1)
	  my_printf("\r\n");
	  my_printf("Starting 'BSP_SX1272_setCR'\r\n");
  #endif

  st0 = BSP_SX1272_Read(REG_OP_MODE);		// Save the previous status

  if( currentstate._modem == FSK )
  {
	  #if (SX1272_debug_mode > 1)
		  my_printf("## Notice that FSK hasn't Coding Rate parameter, ");
		  my_printf("so you are configuring it in LoRa mode ##\r\n");
	  #endif
	  state = BSP_SX1272_setLORA();
  }
  else
  {
	  BSP_SX1272_Write(REG_OP_MODE, LORA_STANDBY_MODE);		// Set Standby mode to write in registers

	  config1 = BSP_SX1272_Read(REG_MODEM_CONFIG1);	// Save config1 to modify only the CR
	  switch(cod)
	  {
		 case CR_5: config1 = config1 & 0b11001111;	// clears bits 5 & 4 from REG_MODEM_CONFIG1
					config1 = config1 | 0b00001000;	// sets bit 3 from REG_MODEM_CONFIG1
					break;
		 case CR_6: config1 = config1 & 0b11010111;	// clears bits 5 & 3 from REG_MODEM_CONFIG1
					config1 = config1 | 0b00010000;	// sets bit 4 from REG_MODEM_CONFIG1
					break;
		 case CR_7: config1 = config1 & 0b11011111;	// clears bit 5 from REG_MODEM_CONFIG1
					config1 = config1 | 0b00011000;	// sets bits 4 & 3 from REG_MODEM_CONFIG1
					break;
		 case CR_8: config1 = config1 & 0b11100111;	// clears bits 4 & 3 from REG_MODEM_CONFIG1
					config1 = config1 | 0b00100000;	// sets bit 5 from REG_MODEM_CONFIG1
					break;
	  }
	  BSP_SX1272_Write(REG_MODEM_CONFIG1, config1);		// Update config1

	  config1 = BSP_SX1272_Read(REG_MODEM_CONFIG1);
	  // ((config1 >> 3) & 0b0000111) ---> take out bits 5-3 from REG_MODEM_CONFIG1 (=currentstate._codingRate)
	  switch(cod)
	  {
		 case CR_5: if( ((config1 >> 3) & 0b0000111) == 0x01 )
					{
						state = 0;
					}
					break;
		 case CR_6: if( ((config1 >> 3) & 0b0000111) == 0x02 )
					{
						state = 0;
					}
					break;
		 case CR_7: if( ((config1 >> 3) & 0b0000111) == 0x03 )
					{
						state = 0;
					}
					break;
		 case CR_8: if( ((config1 >> 3) & 0b0000111) == 0x04 )
					{
						state = 0;
					}
					break;
	  }
  }

  if( BSP_SX1272_isCR(cod) )
  {
	  currentstate._codingRate = cod;
	  #if (SX1272_debug_mode > 1)
		  my_printf("## Coding Rate ");
		  my_printf("%d",cod);
		  my_printf(" has been successfully set ##\r\n");
		  my_printf("\r\n");
	  #endif
  }
  else
  {
	  state = 1;
	  #if (SX1272_debug_mode > 1)
		  my_printf("** There has been an error while configuring Coding Rate parameter **\r\n");
		  my_printf("\r\n");
	  #endif
  }
  BSP_SX1272_Write(REG_OP_MODE,st0);	// Getting back to previous status
  return state;
}

/*
 Function: Checks if channel is a valid value.
 Returns: uint8_t that's '1' if the CR value exists and
		  it's '0' if the CR value does not exist.
 Parameters:
   ch: frequency channel value to check.
*/
uint8_t	BSP_SX1272_isChannel(uint32_t ch)
{
  #if (SX1272_debug_mode > 1)
	  my_printf("\r\n");
	  my_printf("Starting 'BSP_SX1272_isChannel'\r\n");
  #endif

  // Checking available values for currentstate._channel
  switch(ch)
  {
    case CH_868v1:
    case CH_868v3:
    case CH_868v5:
	  case CH_10_868:
	  case CH_11_868:
	  case CH_12_868:
	  case CH_13_868:
	  case CH_14_868:
	  case CH_15_868:
	  case CH_16_868:
	  case CH_17_868:
	  case CH_00_900:
	  case CH_01_900:
	  case CH_02_900:
	  case CH_03_900:
	  case CH_04_900:
	  case CH_05_900:
	  case CH_06_900:
	  case CH_07_900:
	  case CH_08_900:
	  case CH_09_900:
	  case CH_10_900:
	  case CH_11_900:
	  case CH_12_900:	return 1;
						break;

	  default:			return 0;
  }
  #if (SX1272_debug_mode > 1)
	  my_printf("## Finished 'BSP_SX1272_isChannel' ##\r\n");
	  my_printf("\r\n");
  #endif
}

/*
 Function: Indicates the frequency channel within the module is configured.
 Returns: Integer that determines if there has been any error
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
*/
uint8_t BSP_SX1272_getChannel()
{
  uint8_t state = 2;
  uint32_t ch;
  uint8_t freq3;
  uint8_t freq2;
  uint8_t freq1;

  #if (SX1272_debug_mode > 1)
	  my_printf("\r\n");
	  my_printf("Starting 'getChannel'\r\n");
  #endif

  freq3 = BSP_SX1272_Read(REG_FRF_MSB);	// frequency channel MSB
  freq2 = BSP_SX1272_Read(REG_FRF_MID);	// frequency channel MID
  freq1 = BSP_SX1272_Read(REG_FRF_LSB);	// frequency channel LSB
  ch = ((uint32_t)freq3 << 16) + ((uint32_t)freq2 << 8) + (uint32_t)freq1;
  currentstate._channel = ch;						// frequency channel

  if( (currentstate._channel == ch) && BSP_SX1272_isChannel(currentstate._channel) )
  {
	  state = 0;
	  #if (SX1272_debug_mode > 1)
		  my_printf("## Frequency channel is ");
		  my_printf("%d",currentstate._channel);
		  my_printf(" ##\r\n");
		  my_printf("\r\n");
	  #endif
  }
  else
  {
	  state = 1;
  }
  return state;
}

/*
 Function: Sets the indicated channel in the module.
 Returns: Integer that determines if there has been any error
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
   state = -1 --> Forbidden command for this protocol
 Parameters:
   ch: frequency channel value to set in configuration.
*/
int8_t BSP_SX1272_setChannel(uint32_t ch)
{
  uint8_t st0;
  int8_t state = 2;
  unsigned int freq3;
  unsigned int freq2;
  uint8_t freq1;
  uint32_t freq;

  #if (SX1272_debug_mode > 1)
	  my_printf("\r\n");
	  my_printf("Starting 'setChannel'\r\n");
  #endif

  st0 = BSP_SX1272_Read(REG_OP_MODE);	// Save the previous status
  if( currentstate._modem == LORA )
  {
	  // LoRa Stdby mode in order to write in registers
	  BSP_SX1272_Write(REG_OP_MODE, LORA_STANDBY_MODE);
  }
  else
  {
	  // FSK Stdby mode in order to write in registers
	  BSP_SX1272_Write(REG_OP_MODE, FSK_STANDBY_MODE);
  }

  freq3 = ((ch >> 16) & 0x0FF);		// frequency channel MSB
  freq2 = ((ch >> 8) & 0x0FF);		// frequency channel MIB
  freq1 = (ch & 0xFF);				// frequency channel LSB

  BSP_SX1272_Write(REG_FRF_MSB, freq3);
  BSP_SX1272_Write(REG_FRF_MID, freq2);
  BSP_SX1272_Write(REG_FRF_LSB, freq1);

  // storing MSB in freq channel value
  freq3 = (BSP_SX1272_Read(REG_FRF_MSB));
  freq = (freq3 << 8) & 0xFFFFFF;

  // storing MID in freq channel value
  freq2 = (BSP_SX1272_Read(REG_FRF_MID));
  freq = (freq << 8) + ((freq2 << 8) & 0xFFFFFF);

  // storing LSB in freq channel value
  freq = freq + ((BSP_SX1272_Read(REG_FRF_LSB)) & 0xFFFFFF);

  if( freq == ch )
  {
    state = 0;
    currentstate._channel = ch;
    #if (SX1272_debug_mode > 1)
		my_printf("## Frequency channel ");
		my_printf("%d",ch);
		my_printf(" has been successfully set ##\r\n");
		my_printf("\r\n");
	#endif
  }
  else
  {
    state = 1;
  }

  if( !BSP_SX1272_isChannel(ch) )
  {
	 state = -1;
	 #if (SX1272_debug_mode > 1)
		 my_printf("** Frequency channel ");
		 my_printf("%d",ch);
		 my_printf("is not a correct value **\r\n");
		 my_printf("\r\n");
	 #endif
  }

  BSP_SX1272_Write(REG_OP_MODE, st0);	// Getting back to previous status
  return state;
}

/*
 Function: Gets the signal power within the module is configured.
 Returns: Integer that determines if there has been any error
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
*/
uint8_t BSP_SX1272_getPower()
{
  uint8_t state = 2;
  uint8_t value = 0x00;

  #if (SX1272_debug_mode > 1)
	  my_printf("\r\n");
	  my_printf("Starting 'getPower'\r\n");
  #endif

  value = BSP_SX1272_Read(REG_PA_CONFIG);
  state = 1;

  currentstate._power = value;
  if( (value > -1) && (value < 16) )
  {
	    state = 0;
		#if (SX1272_debug_mode > 1)
			my_printf("## Output power is ");
			my_printf("%d",currentstate._power);
			my_printf(" ##\r\n");
			my_printf("\r\n");
		#endif
	}

  return state;
}

/*
 Function: Sets the signal power indicated as input to the module.
 Returns: Integer that determines if there has been any error
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
   state = -1 --> Forbidden command for this protocol
 Parameters:
   pow: power option to set in configuration. The input value range is from
   0 to 14 dBm.
*/
int8_t BSP_SX1272_setPower(uint8_t pow)
{
  uint8_t st0;
  int8_t state = 2;
  uint8_t value = 0x00;

  #if (SX1272_debug_mode > 1)
	  my_printf("\r\n");
	  my_printf("Starting 'setPower'\r\n");
  #endif

  st0 = BSP_SX1272_Read(REG_OP_MODE);	  // Save the previous status
  if( currentstate._modem == LORA )
  { // LoRa Stdby mode to write in registers
	  BSP_SX1272_Write(REG_OP_MODE, LORA_STANDBY_MODE);
  }
  else
  { // FSK Stdby mode to write in registers
	  BSP_SX1272_Write(REG_OP_MODE, FSK_STANDBY_MODE);
  }

  if ( (pow >= 0) && (pow < 15) )
  {
	  currentstate._power = pow;
  }
  else
  {
	  state = -1;
	  #if (SX1272_debug_mode > 1)
		  my_printf("## Power value is not valid ##\r\n");
		  my_printf("\r\n");
	  #endif
  }

  BSP_SX1272_Write(REG_PA_CONFIG, currentstate._power);	// Setting output power value
  value = BSP_SX1272_Read(REG_PA_CONFIG);

  if( value == currentstate._power )
  {
	  state = 0;
	  #if (SX1272_debug_mode > 1)
		  my_printf("## Output power has been successfully set ##\r\n");
		  my_printf("\r\n");
	  #endif
  }
  else
  {
	  state = 1;
  }

  BSP_SX1272_Write(REG_OP_MODE, st0);	// Getting back to previous status
  return state;
}


/*
 Function: Gets the preamble length from the module.
 Returns: Integer that determines if there has been any error
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
*/
uint8_t BSP_SX1272_getPreambleLength()
{
	int8_t state = 2;
	uint8_t p_length;

	#if (SX1272_debug_mode > 1)
		my_printf("\r\n");
		my_printf("Starting 'getPreambleLength'\r\n");
	#endif

	state = 1;
	if( currentstate._modem == LORA )
  	{ // LORA mode
  		p_length = BSP_SX1272_Read(REG_PREAMBLE_MSB_LORA);
  		// Saving MSB preamble length in LoRa mode
		currentstate._preamblelength = (p_length << 8) & 0xFFFF;
		p_length = BSP_SX1272_Read(REG_PREAMBLE_LSB_LORA);
  		// Saving LSB preamble length in LoRa mode
		currentstate._preamblelength = currentstate._preamblelength + (p_length & 0xFFFF);
		#if (SX1272_debug_mode > 1)
			my_printf("## Preamble length configured is ");
			my_printf("%d",currentstate._preamblelength);
			my_printf(" ##");
			my_printf("\r\n");
		#endif
	}
	else
	{ // FSK mode
		p_length = BSP_SX1272_Read(REG_PREAMBLE_MSB_FSK);
		// Saving MSB preamble length in FSK mode
		currentstate._preamblelength = (p_length << 8) & 0xFFFF;
		p_length = BSP_SX1272_Read(REG_PREAMBLE_LSB_FSK);
		// Saving LSB preamble length in FSK mode
		currentstate._preamblelength = currentstate._preamblelength + (p_length & 0xFFFF);
		#if (SX1272_debug_mode > 1)
			my_printf("## Preamble length configured is ");
			my_printf("%d",currentstate._preamblelength);
			my_printf(" ##");
			my_printf("\r\n");
		#endif
	}
	state = 0;
	return state;
}

/*
 Function: Sets the preamble length in the module
 Returns: Integer that determines if there has been any error
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
 Parameters:
   l: length value to set as preamble length.
*/
uint8_t BSP_SX1272_setPreambleLength(uint16_t l)
{
	uint8_t st0;
	uint8_t p_length;
	int8_t state = 2;

	#if (SX1272_debug_mode > 1)
		my_printf("\r\n");
		my_printf("Starting 'setPreambleLength'\r\n");
	#endif

	st0 = BSP_SX1272_Read(REG_OP_MODE);	// Save the previous status
	state = 1;
	if( currentstate._modem == LORA )
  	{ // LoRa mode
  		BSP_SX1272_Write(REG_OP_MODE, LORA_STANDBY_MODE);    // Set Standby mode to write in registers
  		p_length = ((l >> 8) & 0x0FF);
  		// Storing MSB preamble length in LoRa mode
		BSP_SX1272_Write(REG_PREAMBLE_MSB_LORA, p_length);
		p_length = (l & 0x0FF);
		// Storing LSB preamble length in LoRa mode
		BSP_SX1272_Write(REG_PREAMBLE_LSB_LORA, p_length);
	}
	else
	{ // FSK mode
		BSP_SX1272_Write(REG_OP_MODE, FSK_STANDBY_MODE);    // Set Standby mode to write in registers
		p_length = ((l >> 8) & 0x0FF);
  		// Storing MSB preamble length in FSK mode
		BSP_SX1272_Write(REG_PREAMBLE_MSB_FSK, p_length);
		p_length = (l & 0x0FF);
  		// Storing LSB preamble length in FSK mode
		BSP_SX1272_Write(REG_PREAMBLE_LSB_FSK, p_length);
	}

	state = 0;
	#if (SX1272_debug_mode > 1)
		my_printf("## Preamble length ");
		my_printf("%d",l);
		my_printf(" has been successfully set ##\r\n");
		my_printf("\r\n");
	#endif

	BSP_SX1272_Write(REG_OP_MODE, st0);	// Getting back to previous status
	return state;
}

/*
 Function: Gets the payload length from the module.
 Returns: Integer that determines if there has been any error
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
*/
uint8_t BSP_SX1272_getPayloadLength()
{
	uint8_t state = 2;

	#if (SX1272_debug_mode > 1)
		my_printf("\r\n");
		my_printf("Starting 'getPayloadLength'\r\n");
	#endif

	if( currentstate._modem == LORA )
  	{ // LORA mode
  		// Saving payload length in LoRa mode
		currentstate._payloadlength = BSP_SX1272_Read(REG_PAYLOAD_LENGTH_LORA);
		state = 1;
	}
	else
	{ // FSK mode
  		// Saving payload length in FSK mode
		currentstate._payloadlength = BSP_SX1272_Read(REG_PAYLOAD_LENGTH_FSK);
		state = 1;
	}

	#if (SX1272_debug_mode > 1)
		my_printf("## Payload length configured is ");
		my_printf("%d",currentstate._payloadlength);
		my_printf(" ##\r\n");
		my_printf("\r\n");
	#endif

	state = 0;
	return state;
}

/*
 Function: Sets the packet length in the module.
 Returns: Integer that determines if there has been any error
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
   state = -1 --> Forbidden command for this protocol
 Parameters:
   l: length value to set as payload length.
*/
int8_t BSP_SX1272_setPacketLength(uint8_t l)
{
	uint8_t st0;
	uint8_t value = 0x00;
	int8_t state = 2;

	#if (SX1272_debug_mode > 1)
		my_printf("\r\n");
		my_printf("Starting 'setPacketLength'\r\n");
	#endif

	st0 = BSP_SX1272_Read(REG_OP_MODE);	// Save the previous status
	//----
	//	truncPayload(l);
	currentstate.packet_sent.length = l;
	//
	if( currentstate._modem == LORA )
  	{ // LORA mode
  		BSP_SX1272_Write(REG_OP_MODE, LORA_STANDBY_MODE);    // Set LoRa Standby mode to write in registers
		BSP_SX1272_Write(REG_PAYLOAD_LENGTH_LORA, currentstate.packet_sent.length);
		// Storing payload length in LoRa mode
		value = BSP_SX1272_Read(REG_PAYLOAD_LENGTH_LORA);
	}
	else
	{ // FSK mode
		BSP_SX1272_Write(REG_OP_MODE, FSK_STANDBY_MODE);    //  Set FSK Standby mode to write in registers
		BSP_SX1272_Write(REG_PAYLOAD_LENGTH_FSK, currentstate.packet_sent.length);
		// Storing payload length in FSK mode
		value = BSP_SX1272_Read(REG_PAYLOAD_LENGTH_FSK);
	}

	if( currentstate.packet_sent.length == value )
	{
		state = 0;
		#if (SX1272_debug_mode > 1)
			my_printf("## Packet length ");
			my_printf("%d",currentstate.packet_sent.length);
			my_printf(" has been successfully set ##\r\n");
			my_printf("\r\n");
		#endif
	}
	else
	{
		state = 1;
	}

	BSP_SX1272_Write(REG_OP_MODE, st0);	// Getting back to previous status
  	//BSP_DELAY_ms(250);
	return state;
}

/*
 Function: Gets the node address in the module.
 Returns: Integer that determines if there has been any error
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
*/
uint8_t BSP_SX1272_getNodeAddress()
{
	uint8_t st0 = 0;
	uint8_t state = 2;

	#if (SX1272_debug_mode > 1)
		my_printf("\r\n");
		my_printf("Starting 'getNodeAddress'\r\n");
	#endif

	if( currentstate._modem == LORA )
	{
		// Nothing to read
		// node address is stored in currentstate._nodeAddress attribute
		state = 0;
	}
	else
	{
		// FSK mode
		st0 = BSP_SX1272_Read(REG_OP_MODE);	// Save the previous status

		// Allowing access to FSK registers while in LoRa standby mode
		BSP_SX1272_Write(REG_OP_MODE, LORA_STANDBY_FSK_REGS_MODE);

		// Read node address
		currentstate._nodeAddress = BSP_SX1272_Read(REG_NODE_ADRS);

		// Getting back to previous status
		BSP_SX1272_Write(REG_OP_MODE, st0);

		// update state
		state = 0;
	}

	#if (SX1272_debug_mode > 1)
		my_printf("## Node address configured is ");
		my_printf("%d",currentstate._nodeAddress);
		my_printf(" ##\r\n");
		my_printf("\r\n");
	#endif
	return state;
}

/*
 Function: Sets the node address in the module.
 Returns: Integer that determines if there has been any error
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
   state = -1 --> Forbidden command for this protocol
 Parameters:
   addr: address value to set as node address.
*/
int8_t BSP_SX1272_setNodeAddress(uint8_t addr)
{
	uint8_t st0;
	uint8_t value;
	uint8_t state = 2;

	#if (SX1272_debug_mode > 1)
		my_printf("\r\n");
		my_printf("Starting 'setNodeAddress'\r\n");
	#endif

	// check address value is within valid range
	if( addr > 255 )
	{
		state = -1;
		#if (SX1272_debug_mode > 1)
			my_printf("** Node address must be less than 255 **\r\n");
			my_printf("\r\n");
		#endif
	}
	else
	{
		// Saving node address
		currentstate._nodeAddress = addr;
		st0 = BSP_SX1272_Read(REG_OP_MODE);	  // Save the previous status

		// in LoRa mode
		state = 0;

		if( currentstate._modem == LORA )
		{
			// in LoRa mode, address is SW controlled
			// set status to success
			state = 0;
		}
		else if( currentstate._modem == FSK )
		{
			//Set FSK Standby mode to write in registers
			BSP_SX1272_Write(REG_OP_MODE, FSK_STANDBY_MODE);

			// Storing node and broadcast address
			BSP_SX1272_Write(REG_NODE_ADRS, addr);
			BSP_SX1272_Write(REG_BROADCAST_ADRS, BROADCAST_0);

			value = BSP_SX1272_Read(REG_NODE_ADRS);
			BSP_SX1272_Write(REG_OP_MODE, st0);		// Getting back to previous status

			if( value == currentstate._nodeAddress )
			{
				state = 0;
				#if (SX1272_debug_mode > 1)
					my_printf("## Node address ");
					my_printf("%d",currentstate._nodeAddress);
					my_printf(" has been successfully set ##\r\n");
					my_printf("\r\n");
				#endif
			}
			else
			{
				state = 1;
				#if (SX1272_debug_mode > 1)
					my_printf("** There has been an error while setting address ##\r\n");
					my_printf("\r\n");
				#endif
			}
		}
	}
	return state;
}

/*
 Function: Gets the SNR value in LoRa mode.
 Returns: Integer that determines if there has been any error
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
   state = -1 --> Forbidden command for this protocol
*/
int8_t BSP_SX1272_getSNR()
{	// getSNR exists only in LoRa mode
  int8_t state = 2;
  uint8_t value;

  #if (SX1272_debug_mode > 1)
	  my_printf("\r\n");
	  my_printf("Starting 'getSNR'\r\n");
  #endif

  if( currentstate._modem == LORA )
  { // LoRa mode
	  state = 1;
	  value = BSP_SX1272_Read(REG_PKT_SNR_VALUE);
	  if( value & 0x80 ) // The SNR sign bit is 1
	  {
		  // Invert and divide by 4
		  value = ( ( ~value + 1 ) & 0xFF ) >> 2;
          currentstate._SNR = -value;
      }
      else
      {
		  // Divide by 4
		  currentstate._SNR = ( value & 0xFF ) >> 2;
	  }
	  state = 0;
	  #if (SX1272_debug_mode > 0)
		  my_printf("## SNR value is ");
		  my_printf("%d",currentstate._SNR);
		  my_printf(" ##\r\n");
		  my_printf("\r\n");
	  #endif
  }
  else
  { // forbidden command if FSK mode
	state = -1;
	#if (SX1272_debug_mode > 0)
		my_printf("** SNR does not exist in FSK mode **\r\n");
		my_printf("\r\n");
	#endif
  }
  return state;
}

/*
 Function: Gets the current value of RSSI.
 Returns: Integer that determines if there has been any error
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
*/
uint8_t BSP_SX1272_getRSSI()
{
	uint8_t state = 2;
	int rssi_mean = 0;
	int total = 5;

	#if (SX1272_debug_mode > 1)
		my_printf("\r\n");
		my_printf("Starting 'getRSSI'\r\n");
	#endif

	if( currentstate._modem == LORA )
	{
		/// LoRa mode
		// get mean value of RSSI
		for(int i = 0; i < total; i++)
		{
			currentstate._RSSI = -OFFSET_RSSI + BSP_SX1272_Read(REG_RSSI_VALUE_LORA);
			rssi_mean += currentstate._RSSI;
		}
		rssi_mean = rssi_mean / total;
		currentstate._RSSI = rssi_mean;

		state = 0;
		#if (SX1272_debug_mode > 0)
			my_printf("## RSSI value is ");
			my_printf("%d",currentstate._RSSI);
			my_printf(" ##\r\n");
			my_printf("\r\n");
		#endif
	}
	else
	{
		/// FSK mode
		// get mean value of RSSI
		for(int i = 0; i < total; i++)
		{
			currentstate._RSSI = -(BSP_SX1272_Read(REG_RSSI_VALUE_FSK) >> 1);
			rssi_mean += currentstate._RSSI;
		}
		rssi_mean = rssi_mean / total;
		currentstate._RSSI = rssi_mean;

		state = 0;
		#if (SX1272_debug_mode > 0)
			my_printf("## RSSI value is ");
			my_printf("%d",currentstate._RSSI);
			my_printf(" ##\r\n");
			my_printf("\r\n");
		#endif
	}

	return state;
}

/*
 Function: Gets the RSSI of the last packet received in LoRa mode.
 Returns: Integer that determines if there has been any error
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
   state = -1 --> Forbidden command for this protocol
*/
int16_t BSP_SX1272_getRSSIpacket()
{	// RSSIpacket only exists in LoRa
  int8_t state = 2;

  #if (SX1272_debug_mode > 1)
	  my_printf("\r\n");
	  my_printf("Starting 'getRSSIpacket'\r\n");
  #endif

  state = 1;
  if( currentstate._modem == LORA )
  { // LoRa mode
	  state = BSP_SX1272_getSNR();
	  if( state == 0 )
	  {
		  if( currentstate._SNR < 0 )
		  {
			  currentstate._RSSIpacket = -NOISE_ABSOLUTE_ZERO + 10.0 * SignalBwLog[currentstate._bandwidth] + NOISE_FIGURE + ( double )currentstate._SNR;
			  state = 0;
		  }
		  else
		  {
			  currentstate._RSSIpacket = BSP_SX1272_Read(REG_PKT_RSSI_VALUE);
			  currentstate._RSSIpacket = -OFFSET_RSSI + ( double )currentstate._RSSIpacket;
			  state = 0;
		  }
	  #if (SX1272_debug_mode > 0)
		  my_printf("## RSSI packet value is ");
		  my_printf("%d",currentstate._RSSIpacket);
  		  my_printf(" ##\r\n");
		  my_printf("\r\n");
	  #endif
	  }
  }
  else
  { // RSSI packet doesn't exist in FSK mode
	state = -1;
	#if (SX1272_debug_mode > 0)
		my_printf("** RSSI packet does not exist in FSK mode **\r\n");
		my_printf("\r\n");
	#endif
  }
  return state;
}

/*
 Function: It sets the maximum number of retries.
 Returns: Integer that determines if there has been any error
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
   state = -1 -->
*/
uint8_t BSP_SX1272_setRetries(uint8_t ret)
{
	uint8_t state = 2;

	#if (SX1272_debug_mode > 1)
		my_printf("\r\n");
		my_printf("Starting 'setRetries'\r\n");
	#endif

	state = 1;
	if( ret > MAX_RETRIES )
	{
		state = -1;
		#if (SX1272_debug_mode > 1)
			my_printf("** Retries value can't be greater than ");
			my_printf("%d",MAX_RETRIES);
			my_printf(" **\r\n");
			my_printf("\r\n");
		#endif
	}
	else
	{
		currentstate._maxRetries = ret;
		state = 0;
		#if (SX1272_debug_mode > 1)
			my_printf("## Maximum retries value = ");
			my_printf("%d",currentstate._maxRetries);
			my_printf(" ##\r\n");
			my_printf("\r\n");
		#endif
	}
	return state;
}

/*
 Function: Gets the current supply limit of the power amplifier, protecting battery chemistries.
 Returns: Integer that determines if there has been any error
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
 Parameters:
   rate: value to compute the maximum current supply. Maximum current is 45+5*'rate' [mA]
*/
uint8_t BSP_SX1272_getMaxCurrent()
{
	int8_t state = 2;
	uint8_t value;

	#if (SX1272_debug_mode > 1)
		my_printf("\r\n");
		my_printf("Starting 'getMaxCurrent'\r\n");
	#endif

	state = 1;
	currentstate._maxCurrent = BSP_SX1272_Read(REG_OCP);

	// extract only the OcpTrim value from the OCP register
	currentstate._maxCurrent &= 0b00011111;

	if( currentstate._maxCurrent <= 15 )
	{
		value = (45 + (5 * currentstate._maxCurrent));
	}
	else if( currentstate._maxCurrent <= 27 )
	{
		value = (-30 + (10 * currentstate._maxCurrent));
	}
	else
	{
		value = 240;
	}

	currentstate._maxCurrent = value;
	#if (SX1272_debug_mode > 1)
		my_printf("## Maximum current supply configured is ");
		my_printf("%d",value);
		my_printf(" mA ##\r\n");
		my_printf("\r\n");
	#endif
	state = 0;
	return state;
}

/*
 Function: Limits the current supply of the power amplifier, protecting battery chemistries.
 Returns: Integer that determines if there has been any error
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
   state = -1 --> Forbidden parameter value for this function
 Parameters:
   rate: value to compute the maximum current supply. Range: 0x00 to 0x1B. The
   Maximum current is:
	Imax = 45+5*OcpTrim [mA] 	if OcpTrim <= 15 (120 mA) /
	Imax = -30+10*OcpTrim [mA] 	if 15 < OcpTrim <= 27 (130 to 240 mA)
	Imax = 240mA 				for higher settings
*/
int8_t BSP_SX1272_setMaxCurrent(uint8_t rate)
{
	int8_t state = 2;
	uint8_t st0;

	#if (SX1272_debug_mode > 1)
		my_printf("\r\n");
		my_printf("Starting 'setMaxCurrent'\r\n");
	#endif

	// Maximum rate value = 0x1B, because maximum current supply = 240 mA
	if (rate > 0x1B)
	{
		state = -1;
		#if (SX1272_debug_mode > 1)
			my_printf("** Maximum current supply is 240 mA, ");
			my_printf("so maximum parameter value must be 27 (DEC) or 0x1B (HEX) **\r\n");
			my_printf("\r\n");
		#endif
	}
	else
	{
		// Enable Over Current Protection
		rate |= 0b00100000;

		state = 1;
		st0 = BSP_SX1272_Read(REG_OP_MODE);	// Save the previous status
		if( currentstate._modem == LORA )
		{ // LoRa mode
			BSP_SX1272_Write(REG_OP_MODE, LORA_STANDBY_MODE);	// Set LoRa Standby mode to write in registers
		}
		else
		{ // FSK mode
			BSP_SX1272_Write(REG_OP_MODE, FSK_STANDBY_MODE);	// Set FSK Standby mode to write in registers
		}
		BSP_SX1272_Write(REG_OCP, rate);		// Modifying maximum current supply
		BSP_SX1272_Write(REG_OP_MODE, st0);		// Getting back to previous status
		state = 0;
	}
	return state;
}

/*
 Function: Gets the content of different registers.
 Returns: Integer that determines if there has been any error
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
*/
uint8_t BSP_SX1272_getRegs()
{
	int8_t state = 2;
	uint8_t state_f = 2;

	#if (SX1272_debug_mode > 1)
		my_printf("\r\n");
		my_printf("Starting 'getRegs'\r\n");
	#endif

	state_f = 1;
	state = BSP_SX1272_getMode();			// Stores the BW, CR and SF.
	if( state == 0 )
	{
		state = BSP_SX1272_getPower();		// Stores the power.
	}
	else
	{
		state_f = 1;
		#if (SX1272_debug_mode > 1)
			my_printf("** Error getting mode **\r\n");
		#endif
	}
 	if( state == 0 )
	{
		state = BSP_SX1272_getChannel();	// Stores the channel.
	}
	else
	{
		state_f = 1;
		#if (SX1272_debug_mode > 1)
			my_printf("** Error getting power **\r\n");
		#endif
	}
	if( state == 0 )
	{
		state = BSP_SX1272_getCRC();		// Stores the CRC configuration.
	}
	else
	{
		state_f = 1;
		#if (SX1272_debug_mode > 1)
			my_printf("** Error getting channel **\r\n");
		#endif
	}
	if( state == 0 )
	{
		state = BSP_SX1272_getHeader();	// Stores the header configuration.
	}
	else
	{
		state_f = 1;
		#if (SX1272_debug_mode > 1)
			my_printf("** Error getting CRC **\r\n");
		#endif
	}
	if( state == 0 )
	{
		state = BSP_SX1272_getPreambleLength();	// Stores the preamble length.
	}
	else
	{
		state_f = 1;
		#if (SX1272_debug_mode > 1)
			my_printf("** Error getting header **\r\n");
		#endif
	}
	if( state == 0 )
	{
		state = BSP_SX1272_getPayloadLength();		// Stores the payload length.
	}
	else
	{
		state_f = 1;
		#if (SX1272_debug_mode > 1)
			my_printf("** Error getting preamble length **\r\n");
		#endif
	}
	if( state == 0 )
	{
		state = BSP_SX1272_getNodeAddress();		// Stores the node address.
	}
	else
	{
		state_f = 1;
		#if (SX1272_debug_mode > 1)
			my_printf("** Error getting payload length **\r\n");
		#endif
	}
	if( state == 0 )
	{
		state = BSP_SX1272_getMaxCurrent();		// Stores the maximum current supply.
	}
	else
	{
		state_f = 1;
		#if (SX1272_debug_mode > 1)
			my_printf("** Error getting node address **\r\n");
		#endif
	}
	if( state == 0 )
	{
		state_f = BSP_SX1272_getTemp();		// Stores the module temperature.
	}
	else
	{
		state_f = 1;
		#if (SX1272_debug_mode > 1)
			my_printf("** Error getting maximum current supply **\r\n");
		#endif
	}
	if( state_f != 0 )
	{
		#if (SX1272_debug_mode > 1)
			my_printf("** Error getting temperature **\r\n");
			my_printf("\r\n");
		#endif
	}
	return state_f;
}

/*
 Function: It truncs the payload length if it is greater than 0xFF.
 Returns: Integer that determines if there has been any error
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
*/
uint8_t BSP_SX1272_truncPayload(uint16_t length16)
{
	uint8_t state = 2;

	state = 1;

	#if (SX1272_debug_mode > 1)
		my_printf("\r\n");
		my_printf("Starting 'truncPayload'\r\n");
	#endif

	if( length16 > MAX_PAYLOAD )
	{
		currentstate._payloadlength = MAX_PAYLOAD;
	}
	else
	{
		currentstate._payloadlength = (length16 & 0xFF);
	}
	state = 0;

	return state;
}

/*
 Function: It sets an currentstate.ACK in FIFO in order to send it.
 Returns: Integer that determines if there has been any error
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
*/
uint8_t BSP_SX1272_setACK()
{
	uint8_t state = 2;

	#if (SX1272_debug_mode > 1)
		my_printf("\r\n");
		my_printf("Starting 'BSP_SX1272_setACK'\r\n");
	#endif

	BSP_SX1272_clearFlags();	// Initializing flags

	if( currentstate._modem == LORA )
	{ // LoRa mode
		BSP_SX1272_Write(REG_OP_MODE, LORA_STANDBY_MODE);	// Stdby LoRa mode to write in FIFO
	}
	else
	{ // FSK mode
		BSP_SX1272_Write(REG_OP_MODE, FSK_STANDBY_MODE);	// Stdby FSK mode to write in FIFO
	}

	// Setting currentstate.ACK length in order to send it
	state = BSP_SX1272_setPacketLength(ACK_LENGTH);
	if( state == 0 )
	{
		// Setting currentstate.ACK
		memset( &currentstate.ACK, 0x00, sizeof(currentstate.ACK) );
		currentstate.ACK.dst = currentstate.packet_received.src; // currentstate.ACK destination is packet source
		currentstate.ACK.src = currentstate.packet_received.dst; // currentstate.ACK source is packet destination
		currentstate.ACK.packnum = currentstate.packet_received.packnum; // packet number that has been correctly received
		currentstate.ACK.length = 0;		  // length = 0 to show that's an currentstate.ACK
		currentstate.ACK.data[0] = currentstate._reception;	// CRC of the received packet

		// Setting address pointer in FIFO data buffer
		BSP_SX1272_Write(REG_FIFO_ADDR_PTR, 0x00);
		BSP_SX1272_Write(REG_FIFO_TX_BASE_ADDR, 0x00);

		state = 1;

		// Writing currentstate.ACK to send in FIFO
		BSP_SX1272_Write(REG_FIFO, currentstate.ACK.dst); 		// Writing the destination in FIFO
		BSP_SX1272_Write(REG_FIFO, currentstate.ACK.src);		// Writing the source in FIFO
		BSP_SX1272_Write(REG_FIFO, currentstate.ACK.packnum);	// Writing the packet number in FIFO
		BSP_SX1272_Write(REG_FIFO, currentstate.ACK.length); 	// Writing the packet length in FIFO
		BSP_SX1272_Write(REG_FIFO, currentstate.ACK.data[0]);	// Writing the currentstate.ACK in FIFO

		#if (SX1272_debug_mode > 0)
			my_printf("## currentstate.ACK set and written in FIFO ##\r\n");
			// Print the complete currentstate.ACK if debug_mode
			my_printf("## currentstate.ACK to send:\r\n");
			my_printf("%d",currentstate.ACK.dst);			 	// Printing destination
			my_printf("|");
			my_printf("%d",currentstate.ACK.src);			 	// Printing source
			my_printf("|");
			my_printf("%d",currentstate.ACK.packnum);			// Printing currentstate.ACK number
			my_printf("|");
			my_printf("%d",currentstate.ACK.length);				// Printing currentstate.ACK length
			my_printf("|");
			my_printf("%d",currentstate.ACK.data[0]);			// Printing currentstate.ACK payload
			my_printf(" ##\r\n");
			my_printf("\r\n");
		#endif

		state = 0;
		currentstate._reception = CORRECT_PACKET;		// Updating value to next packet

		BSP_DELAY_ms(500);
	}
	return state;
}

/*
 Function: Configures the module to BSP_SX1272_receive information.
 Returns: Integer that determines if there has been any error
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
*/
uint8_t BSP_SX1272_receive()
{
	uint8_t state = 1;

	#if (SX1272_debug_mode > 1)
		my_printf("\r\n");
		my_printf("Starting 'BSP_SX1272_receive'\r\n");
	#endif

	// Initializing currentstate.packet_received struct
	memset( &currentstate.packet_received, 0x00, sizeof(currentstate.packet_received) );

	// Setting Testmode
	BSP_SX1272_Write(0x31,0x43);
	// Set LowPnTxPllOff
	BSP_SX1272_Write(REG_PA_RAMP, 0x09);
	// Set LNA gain: Highest gain. LnaBoost:Improved sensitivity
	BSP_SX1272_Write(REG_LNA, 0x23);
	// Setting address pointer in FIFO data buffer
	BSP_SX1272_Write(REG_FIFO_ADDR_PTR, 0x00);
	// change RegSymbTimeoutLsb
	BSP_SX1272_Write(REG_SYMB_TIMEOUT_LSB, 0xFF);
	// Setting current value of reception buffer pointer
	BSP_SX1272_Write(REG_FIFO_RX_BASE_ADDR, 0x00);

	// Proceed depending on the protocol selected
	if( currentstate._modem == LORA )
	{
		/// LoRa mode
		// With MAX_LENGTH gets all packets with length < MAX_LENGTH
		state = BSP_SX1272_setPacketLength(MAX_LENGTH);
		// Set LORA mode - Rx
		BSP_SX1272_Write(REG_OP_MODE, LORA_RX_MODE);

		#if (SX1272_debug_mode > 1)
			my_printf("## Receiving LoRa mode activated with success ##\r\n");
			my_printf("%d",millis());
		#endif
	}
	else
	{
		/// FSK mode
		state = BSP_SX1272_setPacketLength(currentstate._payloadlength + OFFSET_PAYLOADLENGTH);

		// FSK mode - Rx
		BSP_SX1272_Write(REG_OP_MODE, FSK_RX_MODE);
		#if (SX1272_debug_mode > 1)
			my_printf("## Receiving FSK mode activated with success ##\r\n");
			my_printf("\r\n");
		#endif
	}

	#if (SX1272_debug_mode > 1)
		//showRxRegisters();
	#endif

	return state;
}

/*
 Function: Configures the module to BSP_SX1272_receive information.
 Returns: Integer that determines if there has been any error
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
*/
uint8_t BSP_SX1272_receivePacketMAXTimeout()
{
	return BSP_SX1272_receivePacketTimeout(MAX_TIMEOUT);
}

/*
 Function: Configures the module to BSP_SX1272_receive information.
 Returns: Integer that determines if there has been any error
   state = 6  --> Timeout
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
*/
uint8_t BSP_SX1272_receivePacketTimeout(uint32_t wait)
{
	uint8_t state = 2;
	uint8_t state_f = 2;

	#if (SX1272_debug_mode > 1)
		my_printf("\r\n");
		my_printf("Starting 'receivePacketTimeout'\r\n");
	#endif

	// set RX mode
	state = BSP_SX1272_receive();

	// if RX mode is set correctly then wait for data
	if( state == 0 )
	{
		// Wait for a new packet for 'wait' time
		if( BSP_SX1272_availableData(wait) )
		{
			// If packet received, getPacket
			state_f = BSP_SX1272_getPacket(MAX_TIMEOUT);
		}
		else
		{
			state_f = 6;
		}
	}
	else
	{
		state_f = state;
	}

	return state_f;
}

/*
 Function: Configures the module to BSP_SX1272_receive information and send an currentstate.ACK.
 Returns: Integer that determines if there has been any error
   state = 4  --> The command has been executed but the packet received is incorrect
   state = 3  --> The command has been executed but there is no packet received
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
*/
uint8_t BSP_SX1272_receivePacketTimeoutACK(uint32_t wait)
{
	uint8_t state = 2;
	uint8_t state_f = 2;


	#if (SX1272_debug_mode > 1)
		my_printf("\r\n");
		my_printf("Starting 'receivePacketTimeoutACK'\r\n");
	#endif

	// set RX mode
	state = BSP_SX1272_receive();

	// if RX mode is set correctly then wait for data
	if( state == 0 )
	{
		// Wait for a new packet for 'wait' time
		if( BSP_SX1272_availableData(wait) )
		{
			// If packet received, getPacket
			state = BSP_SX1272_getPacket(MAX_TIMEOUT);
		}
		else
		{
			state = 1;
			state_f = 3;  // There is no packet received
		}
	}
	else
	{
		state = 1;
		state_f = 1; // There has been an error with the 'BSP_SX1272_receive' function
	}


	if( (state == 0) || (state == 3) )
	{
		if( currentstate._reception == INCORRECT_PACKET )
		{
			state_f = 4;  // The packet has been incorrectly received
		}
		else
		{
			state_f = 1;  // The packet has been correctly received
		}
		state = BSP_SX1272_setACK();
		if( state == 0 )
		{
			state = BSP_SX1272_sendWithTimeout(currentstate._sendTime);
			if( state == 0 )
			{
			state_f = 0;
			#if (SX1272_debug_mode > 1)
				my_printf("This last packet was an currentstate.ACK, so ...\r\n");
				my_printf("currentstate.ACK successfully sent\r\n");
				my_printf("\r\n");
			#endif
			}
			else
			{
				state_f = 1; // There has been an error with the 'sendWithTimeout' function
			}
		}
		else
		{
			state_f = 1; // There has been an error with the 'BSP_SX1272_setACK' function
		}
	}
	else
	{
		state_f = 1;
	}
	return state_f;
}

/*
 Function: Configures the module to BSP_SX1272_receive all the information on air.
 Returns: Integer that determines if there has been any error
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
*/
uint8_t BSP_SX1272_receiveAll(uint32_t wait)
{
	uint8_t state = 2;
	uint8_t config1;

	#if (SX1272_debug_mode > 1)
		my_printf("\r\n");
		my_printf("Starting 'receiveAll'\r\n");
	#endif

	if( currentstate._modem == FSK )
	{
		/// FSK mode
		BSP_SX1272_Write(REG_OP_MODE, FSK_STANDBY_MODE);		// Setting standby FSK mode
		config1 = BSP_SX1272_Read(REG_PACKET_CONFIG1);
		config1 = config1 & 0b11111001;			// clears bits 2-1 from REG_PACKET_CONFIG1
		BSP_SX1272_Write(REG_PACKET_CONFIG1, config1);		// AddressFiltering = None
	}

	#if (SX1272_debug_mode > 1)
		my_printf("## Address filtering desactivated ##\r\n");
		my_printf("\r\n");
	#endif

	// Setting Rx mode
	state = BSP_SX1272_receive();

	if( state == 0 )
	{
		// Getting all packets received in wait
		state = BSP_SX1272_getPacket(wait);
	}
	return state;
}

/*
 Function: If a packet is received, checks its destination.
 Returns: uint8_t that's '1' if the packet is for the module and
		  it's '0' if the packet is not for the module.
 Parameters:
   wait: time to wait while there is no a valid header received.
*/
uint8_t	BSP_SX1272_availableData(uint32_t wait)
{
	uint8_t value;
	uint8_t header = 0;
	uint8_t forme = 0;
	unsigned long previous;

	// update attribute
	currentstate._hreceived = 0;

	#if (SX1272_debug_mode > 0)
		my_printf("\r\n");
		my_printf("Starting 'availableData'\r\n");
	#endif

	previous = millis();

	if( currentstate._modem == LORA )
	{
		/// LoRa mode
		// read REG_IRQ_FLAGS
		value = BSP_SX1272_Read(REG_IRQ_FLAGS);

		// Wait to ValidHeader interrupt in REG_IRQ_FLAGS
		while( (bitRead(value, 4) == 0) && (millis()-previous < (unsigned long)wait) )
		{
			// read REG_IRQ_FLAGS
			value = BSP_SX1272_Read(REG_IRQ_FLAGS);

			// Condition to avoid an overflow (DO NOT REMOVE)
			if( millis() < previous )
			{
				previous = millis();
			}
		}

		// Check if ValidHeader was received
		if( bitRead(value, 4) == 1 )
		{
			#if (SX1272_debug_mode > 0)
				my_printf("## Valid Header received in LoRa mode ##\r\n");
			#endif
			currentstate._hreceived = 1;
			while( (header == 0) && (millis()-previous < (unsigned long)wait) )
			{
				// Wait for the increment of the RX buffer pointer
				header = BSP_SX1272_Read(REG_FIFO_RX_BASE_ADDR);

				// Condition to avoid an overflow (DO NOT REMOVE)
				if( millis() < previous )
				{
					previous = millis();
				}
			}

			// If packet received: Read first uint8_t of the received packet
			if( header != 0 )
			{
				currentstate._destination = BSP_SX1272_Read(REG_FIFO);
			}
		}
		else
		{
			forme = 0;
			currentstate._hreceived = 0;
			#if (SX1272_debug_mode > 0)
				my_printf("** The timeout has expired **\r\n");
				my_printf("\r\n");
			#endif
		}
	}
	else
	{
		/// FSK mode
		// read REG_IRQ_FLAGS2
		value = BSP_SX1272_Read(REG_IRQ_FLAGS2);
		// Wait to Payload Ready interrupt
		while( (bitRead(value, 5) == 0) && (millis() - previous < wait) )
		{
			value = BSP_SX1272_Read(REG_IRQ_FLAGS2);
			// Condition to avoid an overflow (DO NOT REMOVE)
			if( millis() < previous )
			{
				previous = millis();
			}
		}// end while (millis)
		if( bitRead(value, 5) == 1 )	// something received
		{
			currentstate._hreceived = 1;
			#if (SX1272_debug_mode > 0)
				my_printf("## Valid Preamble detected in FSK mode ##\r\n");
			#endif
			// Reading first uint8_t of the received packet
			currentstate._destination = BSP_SX1272_Read(REG_FIFO);
		}
		else
		{
			forme = 0;
			currentstate._hreceived = 0;
			#if (SX1272_debug_mode > 0)
				my_printf("** The timeout has expired **\r\n");
				my_printf("\r\n");
			#endif

       /*
      if(bitRead(value, 5) == 1)
      {
        while(bitRead(value, 6) == 0)
        {
         my_printf("data: ");
         my_printf(BSP_SX1272_Read(REG_FIFO));
         value = BSP_SX1272_Read(REG_IRQ_FLAGS2);
         //my_printf("irq: ");
         //my_printf(value);
        }

      }*/
		}
	}


	/* We use 'currentstate._hreceived' because we need to ensure that 'currentstate._destination' value
	 * is correctly updated and is not the 'currentstate._destination' value from the
	 * previously packet
	 */
	if( currentstate._hreceived == 1 )
	{
		#if (SX1272_debug_mode > 0)
			my_printf("## Checking destination ##\r\n");
			my_printf("Destination address = ");
			my_printf("%d",currentstate._destination);
			my_printf("Node address = ");
			my_printf("%d",currentstate._nodeAddress);
			my_printf("Broadcast address = ");
			my_printf("%d\r\n",BROADCAST_0);
		#endif

		// Checking destination
		if( (currentstate._destination == currentstate._nodeAddress) || (currentstate._destination == BROADCAST_0) )
		{ // LoRa or FSK mode
			forme = 1;
			#if (SX1272_debug_mode > 0)
				my_printf("## Packet received is for me ##\r\n");
			#endif
		}
		else
		{
			forme = 0;
			#if (SX1272_debug_mode > 0)
				my_printf("## Packet received is not for me ##\r\n");
				my_printf("%d",millis());
			#endif

			// If it is not a correct destination address, then change to
			// STANDBY to minimize power consumption
			if( currentstate._modem == LORA )
			{
				// Setting standby LoRa mode
				BSP_SX1272_Write(REG_OP_MODE, LORA_STANDBY_MODE);
			}
			else
			{
				// Setting standby FSK mode
				BSP_SX1272_Write(REG_OP_MODE, FSK_STANDBY_MODE);
			}
		}
	}
	else
	{
		// If timeout has expired, then change to
		// STANDBY to minimize power consumption
		if( currentstate._modem == LORA )
		{
			// Setting standby LoRa mode
//~ 			BSP_SX1272_Write(REG_OP_MODE, LORA_STANDBY_MODE);
		}
		else
		{
			// Setting standby FSK mode
			BSP_SX1272_Write(REG_OP_MODE, FSK_STANDBY_MODE);
		}
	}
	return forme;
}

/*
 Function: It gets and stores a packet if it is received before MAX_TIMEOUT expires.
 Returns:  Integer that determines if there has been any error
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
*/
uint8_t BSP_SX1272_getPacketMAXTimeout()
{
	return BSP_SX1272_getPacket(MAX_TIMEOUT);
}

/*
 Function: It gets and stores a packet if it is received before ending 'wait' time.
 Returns:  Integer that determines if there has been any error
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
   state = -1 --> Forbidden parameter value for this function
 Parameters:
   wait: time to wait while there is no a valid header received.
*/
int8_t BSP_SX1272_getPacket(uint32_t wait)
{
	uint8_t state = 2;
	uint8_t state_f = 2;
	uint8_t value = 0x00;
	unsigned long previous;
	uint8_t p_received = 0;

	#if (SX1272_debug_mode > 0)
		my_printf("\r\n");
		my_printf("Starting 'getPacket'\r\n");
	#endif

	previous = millis();

	if( currentstate._modem == LORA )
	{
		/// LoRa mode
		// read REG_IRQ_FLAGS
		value = BSP_SX1272_Read(REG_IRQ_FLAGS);

		// Wait until the packet is received (RxDone flag) or the timeout expires
		while( (bitRead(value, 6) == 0) && (millis()-previous < (unsigned long)wait) )
		{
			value = BSP_SX1272_Read(REG_IRQ_FLAGS);

			// Condition to avoid an overflow (DO NOT REMOVE)
			if( millis() < previous )
			{
				previous = millis();
			}
		}

		// Check if 'RxDone' is 1 and 'PayloadCrcError' is correct
		if( (bitRead(value, 6) == 1) && (bitRead(value, 5) == 0) )
		{
			// packet received & CRC correct
			p_received = 1;	// packet correctly received
			currentstate._reception = CORRECT_PACKET;
			#if (SX1272_debug_mode > 0)
				my_printf("## Packet correctly received in LoRa mode ##\r\n");
			#endif
		}
		else
		{
			if( bitRead(value, 6) != 1 )
			{
				#if (SX1272_debug_mode > 0)
					my_printf("NOT 'RxDone' flag\r\n");
				#endif
			}

			if( currentstate._CRC != CRC_ON )
			{
				#if (SX1272_debug_mode > 0)
					my_printf("NOT 'CRC_ON' enabled\r\n");
				#endif
			}

			if( (bitRead(value, 5) == 0) && (currentstate._CRC == CRC_ON) )
			{
				// CRC is correct
				currentstate._reception = CORRECT_PACKET;
			}
			else
			{
				// CRC incorrect
				currentstate._reception = INCORRECT_PACKET;
				state = 3;
				#if (SX1272_debug_mode > 0)
					my_printf("** The CRC is incorrect **\r\n");
					my_printf("\r\n");
				#endif
			}
		}

	}
	else
	{
		/// FSK mode
		value = BSP_SX1272_Read(REG_IRQ_FLAGS2);
    currentstate._reception = CORRECT_PACKET;
    p_received = 1;

    BSP_SX1272_getRSSI();
    //my_printf("RSSI: ");
    //my_printf(currentstate._RSSI, DEC);

    BSP_DELAY_ms(500);// laid !

		/*
		while( (bitRead(value, 2) == 0) && (millis() - previous < wait) )
		{
			value = BSP_SX1272_Read(REG_IRQ_FLAGS2);
			// Condition to avoid an overflow (DO NOT REMOVE)
			if( millis() < previous )
			{
				previous = millis();
			}
		} // end while (millis)
    */

/*
		if( bitRead(value, 2) == 1 )
		{ // packet received
 			if( (bitRead(value, 1) == 1) && (currentstate._CRC == CRC_ON) )
			{ // CRC correct
				currentstate._reception = CORRECT_Pcurrentstate.ACKET;
				p_received = 1;
				#if (SX1272_debug_mode > 0)
					my_printf("## Packet correctly received in FSK mode ##\r\n");
				#endif
			}
			else
			{ // CRC incorrect
				currentstate._reception = INCORRECT_Pcurrentstate.ACKET;
				state = 3;
				p_received = 0;
				#if (SX1272_debug_mode > 0)
					my_printf("## Packet incorrectly received in FSK mode ##\r\n");
				#endif
			}
		}
		else
		{
			#if (SX1272_debug_mode > 0)
				my_printf("** The timeout has expired **\r\n");
				my_printf("\r\n");
			#endif
		}
*/

		BSP_SX1272_Write(REG_OP_MODE, FSK_STANDBY_MODE);	// Setting standby FSK mode
	}

	/* If a new packet was received correctly, now the information must be
	 * filled inside the structures of the class
	 */
	if( p_received == 1 )
	{
		// Store the packet
		if( currentstate._modem == LORA )
		{
			/// LoRa
			// Setting address pointer in FIFO data buffer
			BSP_SX1272_Write(REG_FIFO_ADDR_PTR, 0x00);
			// Storing first uint8_t of the received packet
			currentstate.packet_received.dst = BSP_SX1272_Read(REG_FIFO);
		}
		else
		{
			/// FSK
			value = BSP_SX1272_Read(REG_PACKET_CONFIG1);
			if( (bitRead(value, 2) == 0) && (bitRead(value, 1) == 0) )
			{
				// Storing first uint8_t of the received packet
				currentstate.packet_received.dst = BSP_SX1272_Read(REG_FIFO);
			}
			else
			{
				// Storing first uint8_t of the received packet
				currentstate.packet_received.dst = currentstate._destination;
			}
		}

		// Reading second uint8_t of the received packet
		// Reading third uint8_t of the received packet
		// Reading fourth uint8_t of the received packet
		currentstate.packet_received.src = BSP_SX1272_Read(REG_FIFO);
		currentstate.packet_received.packnum = BSP_SX1272_Read(REG_FIFO);
		currentstate.packet_received.length = BSP_SX1272_Read(REG_FIFO);

		// calculate the payload length
		if( currentstate._modem == LORA )
		{
			currentstate._payloadlength = currentstate.packet_received.length - OFFSET_PAYLOADLENGTH;
		}
   else
   {
    if(currentstate.packet_received.length>OFFSET_PAYLOADLENGTH)
      currentstate._payloadlength = currentstate.packet_received.length - OFFSET_PAYLOADLENGTH;
    else
      currentstate._payloadlength=currentstate.packet_received.length;
   }

		// check if length is incorrect
		if( currentstate.packet_received.length > (MAX_LENGTH + 1) )
		{
			#if (SX1272_debug_mode > 0)
				my_printf("Corrupted packet, length must be less than 256\r\n");
			#endif
		}
		else
		{
			// Store payload in 'data'
			for(unsigned int i = 0; i < currentstate._payloadlength; i++)
			{
				currentstate.packet_received.data[i] = BSP_SX1272_Read(REG_FIFO);
			}
			// Store 'retry'
			currentstate.packet_received.retry = BSP_SX1272_Read(REG_FIFO);

			// Print the packet if debug_mode
			#if (SX1272_debug_mode > 0)
				my_printf("## Packet received:\r\n");
				my_printf("%d",currentstate.packet_received.dst);			 	// Printing destination
				my_printf("|");
				my_printf("%d",currentstate.packet_received.src);			 	// Printing source
				my_printf("|");
				my_printf("%d",currentstate.packet_received.packnum);			// Printing packet number
				my_printf("|");
				my_printf("%d",currentstate.packet_received.length);			// Printing packet length
				my_printf("|");
				for(unsigned int i = 0; i < currentstate._payloadlength; i++)
				{
					my_printf("%d",currentstate.packet_received.data[i]);		// Printing payload
					my_printf("|");
				}
				my_printf("%d",currentstate.packet_received.retry);			// Printing number retry
				my_printf(" ##\r\n");
				my_printf("\r\n");
			#endif
			state_f = 0;
		}
	}
	else
	{
		// if packet was NOT received
		state_f = 1;
		if( (currentstate._reception == INCORRECT_PACKET) && (currentstate._retries < currentstate._maxRetries) && (state != 3) )
		{
			currentstate._retries++;
			#if (SX1272_debug_mode > 0)
				my_printf("## Retrying to send the last packet ##\r\n");
				my_printf("\r\n");
			#endif
		}
	}

	// Setting address pointer in FIFO data buffer to 0x00 again
	if( currentstate._modem == LORA )
	{
		BSP_SX1272_Write(REG_FIFO_ADDR_PTR, 0x00);
	}

	// Initializing flags
	BSP_SX1272_clearFlags();

	if( wait > MAX_WAIT )
	{
		state_f = -1;
		#if (SX1272_debug_mode > 0)
			my_printf("** The timeout must be smaller than 12.5 seconds **\r\n");
			my_printf("\r\n");
		#endif
	}

	return state_f;
}

/*
 Function: It sets the packet destination.
 Returns:  Integer that determines if there has been any error
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
 Parameters:
   dest: destination value of the packet sent.
*/
int8_t BSP_SX1272_setDestination(uint8_t dest)
{
	int8_t state = 2;

	#if (SX1272_debug_mode > 1)
		my_printf("\r\n");
		my_printf("Starting 'setDestination'\r\n");
	#endif

	state = 1;
	currentstate._destination = dest; // Storing destination in a global variable
	currentstate.packet_sent.dst = dest;	 // Setting destination in packet structure
	currentstate.packet_sent.src = currentstate._nodeAddress; // Setting source in packet structure
	currentstate.packet_sent.packnum = currentstate._packetNumber;	// Setting packet number in packet structure
	currentstate._packetNumber++;
	state = 0;

	#if (SX1272_debug_mode > 1)
		my_printf("## Destination ");
		my_printf("%d",currentstate._destination);
		my_printf(" successfully set ##\r\n");
		my_printf("## Source ");
		my_printf("%d",currentstate.packet_sent.src);
		my_printf(" successfully set ##\r\n");
		my_printf("## Packet number ");
		my_printf("%d",currentstate.packet_sent.packnum);
		my_printf(" successfully set ##\r\n");
		my_printf("\r\n");
	#endif
	return state;
}

/*
 Function: It sets the timeout according to the configured mode.
 Link: http://www.semtech.com/images/datasheet/sx1272.pdf
 Returns: Integer that determines if there has been any error
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
*/
uint8_t BSP_SX1272_setTimeout()
{
	uint8_t state = 2;
	uint16_t delay;

	#if (SX1272_debug_mode > 1)
		my_printf("\r\n");
		my_printf("Starting 'setTimeout'\r\n");
	#endif

	state = 1;
	if( currentstate._modem == LORA )
	{
		// calculate 'delay'
		delay = ((0.1*currentstate._sendTime) + 1);

		float Tpacket = BSP_SX1272_timeOnAir(currentstate._payloadlength);

		// calculate final send/BSP_SX1272_receive timeout adding an offset and a random value
		currentstate._sendTime = (uint16_t) Tpacket + (rand()%delay) + 1000;

		#if (SX1272_debug_mode > 2)
			my_printf("Tsym (ms):");
			my_printf(Tsym);
			my_printf("Tpreamble (ms):");
			my_printf(Tpreamble);
			my_printf("payloadSymbNb:");
			my_printf(payloadSymbNb);
			my_printf("Tpacket:");
			my_printf(Tpacket);
		#endif

		// update state
		state = 0;
	}
	else
	{
		// update state
		currentstate._sendTime = MAX_TIMEOUT;

		// update state
		state = 0;
	}

	#if (SX1272_debug_mode > 1)
		my_printf("Timeout to send/BSP_SX1272_receive is: ");
		my_printf("%d",currentstate._sendTime);
	#endif

	return state;
}


/*
 Function: It gets the theoretical value of the time-on-air of the packet
 Link: http://www.semtech.com/images/datasheet/sx1272.pdf
 Returns: Float that determines the time-on-air
*/
/*float BSP_SX1272_timeOnAir()
{
	return timeOnAir( currentstate._payloadlength );
}*/

/*
 Function: It gets the theoretical value of the time-on-air of the packet
 Link: http://www.semtech.com/images/datasheet/sx1272.pdf
 Returns: Float that determines the time-on-air
*/
float BSP_SX1272_timeOnAir( uint16_t payloadlength )
{
	float BW;
	float DE = 0;
	float SF = currentstate._spreadingFactor;
	float PL = payloadlength + OFFSET_PAYLOADLENGTH;
	float H = currentstate._header;
	float CR = currentstate._codingRate;

	// Dara rate optimization enabled if SF is 11 or 12
	if( SF > 10) DE = 1.0;
	else DE = 0.0;

	// payload correction
	if( payloadlength == 0 ) PL = 255;

	// Bandwidth value setting
	if( currentstate._bandwidth == BW_125 ) 		BW = 125.0;
	else if( currentstate._bandwidth == BW_250 ) BW = 250.0;
	else if( currentstate._bandwidth == BW_500 ) BW = 500.0;
	else BW = 125.0;

	// Calculation steps:
	float Tsym = pow(2,SF)/(BW); // ms
	float Tpreamble = (8+4.25)*Tsym;// ms
	float argument1 = ceil( (8.0*PL-4.0*SF+28.0+16.0-20.0*H)/(4.0*(SF-2.0*DE)) )*(CR+4.0);
	float argument2 = 0;
	float payloadSymbNb = 8 + max( argument1, argument2);
	float Tpayload = payloadSymbNb * Tsym;
	float Tpacket = Tpreamble + Tpayload;

	return Tpacket;
}


/*
 Function: It sets a char array payload packet in a packet struct.
 Returns:  Integer that determines if there has been any error
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
*/
//uint8_t BSP_SX1272_setPayload(char *payload)
//{
//	uint8_t state = 2;
//	uint8_t state_f = 2;
//	uint16_t length16;
//
//	#if (SX1272_debug_mode > 1)
//		my_printf("\r\n");
//		my_printf("Starting 'setPayload'\r\n");
//	#endif
//
//	state = 1;
//	length16 = (uint16_t)strlen(payload);
//	state = BSP_SX1272_truncPayload(length16);
//	if( state == 0 )
//	{
//		// fill data field until the end of the string
//		for(unsigned int i = 0; i < currentstate._payloadlength; i++)
//		{
//			currentstate.packet_sent.data[i] = payload[i];
//		}
//	}
//	else
//	{
//		state_f = state;
//	}
//
//	// In the case of FSK mode, the max payload is more restrictive
//	if( ( currentstate._modem == FSK ) && ( currentstate._payloadlength > MAX_PAYLOAD_FSK ) )
//	{
//		currentstate._payloadlength = MAX_PAYLOAD_FSK;
//		state = 1;
//		#if (SX1272_debug_mode > 1)
//			my_printf("In FSK, payload length must be less than 60 uint8_ts.\r\n");
//			my_printf("\r\n");
//		#endif
//	}
//
//	// Set length with the actual counter value
//	// Setting packet length in packet structure
//	state_f = BSP_SX1272_setPacketLength(currentstate._payloadlength + OFFSET_PAYLOADLENGTH);
//	return state_f;
//}

/*
 Function: It sets an uint8_t array payload packet in a packet struct.
 Returns:  Integer that determines if there has been any error
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
*/
uint8_t BSP_SX1272_setPayload(uint8_t *payload)
{
	uint8_t state = 2;

	#if (SX1272_debug_mode > 1)
		my_printf("\r\n");
		my_printf("Starting 'setPayload'\r\n");
	#endif

	state = 1;
	if( ( currentstate._modem == FSK ) && ( currentstate._payloadlength > MAX_PAYLOAD_FSK ) )
	{
		currentstate._payloadlength = MAX_PAYLOAD_FSK;
		state = 1;
		#if (SX1272_debug_mode > 1)
			my_printf("In FSK, payload length must be less than 60 uint8_ts.\r\n");
			my_printf("\r\n");
		#endif
	}
	for(unsigned int i = 0; i < currentstate._payloadlength; i++)
	{
		currentstate.packet_sent.data[i] = payload[i];	// Storing payload in packet structure
	}
	// set length with the actual counter value
    state = BSP_SX1272_setPacketLength(currentstate._payloadlength + OFFSET_PAYLOADLENGTH);	// Setting packet length in packet structure
	return state;
}

/*
 Function: It sets a packet struct in FIFO in order to send it.
 Returns:  Integer that determines if there has been any error
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
*/
//uint8_t BSP_SX1272_setPacket(uint8_t dest, char *payload)
//{
//	int8_t state = 2;
//	uint8_t st0;
//
//	#if (SX1272_debug_mode > 1)
//		my_printf("\r\n");
//		my_printf("Starting 'setPacket'\r\n");
//	#endif
//
//	// Save the previous status
//	st0 = BSP_SX1272_Read(REG_OP_MODE);
//	// Initializing flags
//	BSP_SX1272_clearFlags();
//
//	// Updating incorrect value
//	currentstate._reception = CORRECT_PACKET;
//
//
//	if (currentstate._retries == 0)
//	{
//		// Updating these values only if it is the first try
//		// Setting destination in packet structure
//		state = BSP_SX1272_setDestination(dest);
//		if( state == 0 )
//		{
//			state = BSP_SX1272_setPayload(payload);
//		}
//	}
//	else
//	{
//		state = BSP_SX1272_setPacketLength(currentstate._payloadlength + OFFSET_PAYLOADLENGTH);
//		currentstate.packet_sent.retry = currentstate._retries;
//		#if (SX1272_debug_mode > 0)
//			my_printf("** Retrying to send last packet ");
//			my_printf("%d",currentstate._retries);
//			my_printf(" time **\r\n");
//		#endif
//	}
//
//	// Setting address pointer in FIFO data buffer
//	BSP_SX1272_Write(REG_FIFO_TX_BASE_ADDR, 0x00);
//	BSP_SX1272_Write(REG_FIFO_ADDR_PTR, 0x00);
//	if( state == 0 )
//	{
//		state = 1;
//		// Writing packet to send in FIFO
//		BSP_SX1272_Write(REG_FIFO, currentstate.packet_sent.dst); 		// Writing the destination in FIFO
//		BSP_SX1272_Write(REG_FIFO, currentstate.packet_sent.src);		// Writing the source in FIFO
//		BSP_SX1272_Write(REG_FIFO, currentstate.packet_sent.packnum);	// Writing the packet number in FIFO
//		BSP_SX1272_Write(REG_FIFO, currentstate.packet_sent.length); 	// Writing the packet length in FIFO
//		for( uint16_t i = 0; i < currentstate._payloadlength; i++)
//		{
//			BSP_SX1272_Write(REG_FIFO, currentstate.packet_sent.data[i]);  // Writing the payload in FIFO
//		}
//		BSP_SX1272_Write(REG_FIFO, currentstate.packet_sent.retry);		// Writing the number retry in FIFO
//		state = 0;
//		#if (SX1272_debug_mode > 0)
//			my_printf("## Packet set and written in FIFO ##\r\n");
//			// Print the complete packet if debug_mode
//			my_printf("## Packet to send: ");
//			my_printf("%d",currentstate.packet_sent.dst);			 	// Printing destination
//			my_printf("|");
//			my_printf("%d",currentstate.packet_sent.src);			 	// Printing source
//			my_printf("|");
//			my_printf("%d",currentstate.packet_sent.packnum);			// Printing packet number
//			my_printf("|");
//			my_printf("%d",currentstate.packet_sent.length);			// Printing packet length
//			my_printf("|");
//			for( uint16_t i = 0; i < currentstate._payloadlength; i++)
//			{
//				my_printf("%d",currentstate.packet_sent.data[i]);		// Printing payload
//				my_printf("|");
//			}
//			my_printf("%d",currentstate.packet_sent.retry);			// Printing retry number
//			my_printf(" ##\r\n");
//		#endif
//	}
//	BSP_SX1272_Write(REG_OP_MODE, st0);	// Getting back to previous status
//	return state;
//}

/*
 Function: It sets a packet struct in FIFO in order to sent it.
 Returns:  Integer that determines if there has been any error
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
*/
uint8_t BSP_SX1272_setPacket(uint8_t dest, uint8_t *payload)
{
	int8_t state = 2;
	uint8_t st0;

	#if (SX1272_debug_mode > 1)
		my_printf("\r\n");
		my_printf("Starting 'setPacket'\r\n");
	#endif

	st0 = BSP_SX1272_Read(REG_OP_MODE);	// Save the previous status
	BSP_SX1272_clearFlags();	// Initializing flags

	if( currentstate._modem == LORA )
	{ // LoRa mode
		BSP_SX1272_Write(REG_OP_MODE, LORA_STANDBY_MODE);	// Stdby LoRa mode to write in FIFO
	}
	else
	{ // FSK mode
		BSP_SX1272_Write(REG_OP_MODE, FSK_STANDBY_MODE);	// Stdby FSK mode to write in FIFO
	}

	currentstate._reception = CORRECT_PACKET;	// Updating incorrect value to send a packet (old or new)
	if(currentstate._retries == 0)
	{ // Sending new packet
		state = BSP_SX1272_setDestination(dest);	// Setting destination in packet structure
		if( state == 0 )
		{
			state = BSP_SX1272_setPayload(payload);
		}
	}
	else
	{
		state = BSP_SX1272_setPacketLength(currentstate._payloadlength + OFFSET_PAYLOADLENGTH);
		currentstate.packet_sent.retry = currentstate._retries;
		#if (SX1272_debug_mode > 0)
			my_printf("** Retrying to send last packet ");
			my_printf(currentstate._retries, DEC);
			my_printf(" time **\r\n");
		#endif
	}
	BSP_SX1272_Write(REG_FIFO_TX_BASE_ADDR, 0x00);
	BSP_SX1272_Write(REG_FIFO_ADDR_PTR, 0x00);  // Setting address pointer in FIFO data buffer
	if( state == 0 )
	{
		state = 1;
		// Writing packet to send in FIFO
		BSP_SX1272_Write(REG_FIFO, currentstate.packet_sent.dst); 		// Writing the destination in FIFO
		BSP_SX1272_Write(REG_FIFO, currentstate.packet_sent.src);		// Writing the source in FIFO
		BSP_SX1272_Write(REG_FIFO, currentstate.packet_sent.packnum);	// Writing the packet number in FIFO
		BSP_SX1272_Write(REG_FIFO, currentstate.packet_sent.length); 	// Writing the packet length in FIFO
		for(unsigned int i = 0; i < currentstate._payloadlength; i++)
		{
			BSP_SX1272_Write(REG_FIFO, currentstate.packet_sent.data[i]);  // Writing the payload in FIFO
		}
		BSP_SX1272_Write(REG_FIFO, currentstate.packet_sent.retry);		// Writing the number retry in FIFO
		state = 0;
		#if (SX1272_debug_mode > 0)
			my_printf("## Packet set and written in FIFO ##\r\n");
			// Print the complete packet if debug_mode
			my_printf("## Packet to send: ");
			my_printf(currentstate.packet_sent.dst);			 	// Printing destination
			my_printf("|");
			my_printf(currentstate.packet_sent.src);			 	// Printing source
			my_printf("|");
			my_printf(currentstate.packet_sent.packnum);			// Printing packet number
			my_printf("|");
			my_printf(currentstate.packet_sent.length);			// Printing packet length
			my_printf("|");
			for(unsigned int i = 0; i < currentstate._payloadlength; i++)
			{
				my_printf(currentstate.packet_sent.data[i]);		// Printing payload
				my_printf("|");
			}
			my_printf(currentstate.packet_sent.retry);			// Printing retry number
			my_printf(" ##\r\n");
		#endif
	}
	BSP_SX1272_Write(REG_OP_MODE, st0);	// Getting back to previous status
	return state;
}

/*
 Function: Configures the module to transmit information.
 Returns: Integer that determines if there has been any error
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
*/
uint8_t BSP_SX1272_sendWithMAXTimeout()
{
	return BSP_SX1272_sendWithTimeout(MAX_TIMEOUT);
}

/*
 Function: Configures the module to transmit information.
 Returns: Integer that determines if there has been any error
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
*/
uint8_t BSP_SX1272_sendWithTimeout(uint32_t wait)
{
	uint8_t state = 2;
	uint8_t value = 0x00;
	unsigned long previous;

	#if (SX1272_debug_mode > 1)
		my_printf("\r\n");
		my_printf("Starting 'sendWithTimeout'\r\n");
	#endif

	// wait to TxDone flag
	previous = millis();
	if( currentstate._modem == LORA )
	{
		/// LoRa mode
		// Initializing flags
		BSP_SX1272_clearFlags();
		// LORA mode - Tx
		BSP_SX1272_Write(REG_OP_MODE, LORA_TX_MODE);

		value = BSP_SX1272_Read(REG_IRQ_FLAGS);

		// Wait until the packet is sent (TX Done flag) or the timeout expires
		while ((bitRead(value, 3) == 0) && (millis() - previous < wait))
		{
			value = BSP_SX1272_Read(REG_IRQ_FLAGS);
			// Condition to avoid an overflow (DO NOT REMOVE)
			if( millis() < previous )
			{
				previous = millis();
			}
		}
		state = 1;
	}
	else
	{
    // Initializing flags
		/// FSK mode
		BSP_SX1272_Write(REG_OP_MODE, FSK_TX_MODE);  // FSK mode - Tx

		value = BSP_SX1272_Read(REG_IRQ_FLAGS2);
		// Wait until the packet is sent (Packet Sent flag) or the timeout expires
		while ((bitRead(value, 3) == 0) && (millis() - previous < wait))
		{
			value = BSP_SX1272_Read(REG_IRQ_FLAGS2);

			// Condition to avoid an overflow (DO NOT REMOVE)
			if( millis() < previous )
			{
				previous = millis();
			}
		}
		state = 1;
	}
	if( bitRead(value, 3) == 1 )
	{
		state = 0;	// Packet successfully sent
		#if (SX1272_debug_mode > 1)
			my_printf("## Packet successfully sent ##\r\n");
			my_printf("\r\n");
		#endif
	}
	else
	{
		if( state == 1 )
		{
			#if (SX1272_debug_mode > 1)
				my_printf("** Timeout has expired **\r\n");
				my_printf("\r\n");
			#endif
		}
		else
		{
			#if (SX1272_debug_mode > 1)
				my_printf("** There has been an error and packet has not been sent **\r\n");
				my_printf("\r\n");
			#endif
		}
	}

	// Initializing flags
	BSP_SX1272_clearFlags();
	return state;
}

/*
 Function: Configures the module to transmit information.
 Returns: Integer that determines if there has been any error
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
*/
//uint8_t BSP_SX1272_sendPacketMAXTimeout(uint8_t dest, char *payload)
//{
//	return BSP_SX1272_sendPacketTimeout(dest, payload, MAX_TIMEOUT);
//}

/*
 Function: Configures the module to transmit information.
 Returns: Integer that determines if there has been any error
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
*/
uint8_t BSP_SX1272_sendPacketMAXTimeout(	uint8_t dest,
											uint8_t *payload,
											uint16_t length16)
{
	return BSP_SX1272_sendPacketTimeout(dest, payload, length16, MAX_TIMEOUT);
}

/*
 Function: Configures the module to transmit information.
 Returns: Integer that determines if there has been any error
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
*/
//uint8_t BSP_SX1272_sendPacketTimeout(uint8_t dest, char *payload, uint32_t wait)
//{
//	uint8_t state = 2;
//
//	#if (SX1272_debug_mode > 1)
//		my_printf("\r\n");
//		my_printf("Starting 'sendPacketTimeout'\r\n");
//		my_printf("Passe dans sendPacketTimeout(uint8_t dest, char *payload, uint32_t wait)\r\n");
//	#endif
//
//	state = BSP_SX1272_setPacket(dest, payload);	// Setting a packet with 'dest' destination
//	if (state == 0)								// and writing it in FIFO.
//	{
//		state = BSP_SX1272_sendWithTimeout(wait);	// Sending the packet
//	}
//	return state;
//}

/*
 Function: Configures the module to transmit information.
 Returns: Integer that determines if there has been any error
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
*/
// FONCTION MODIFIEE POUR
uint8_t BSP_SX1272_sendPacketTimeout(	uint8_t dest,
										uint8_t *payload,
										uint16_t length16,
										uint32_t wait)
{
	uint8_t state = 2;
	uint8_t state_f = 2;
	uint8_t varable;
	varable = BSP_SX1272_Read(REG_OP_MODE);
	#if (SX1272_debug_mode > 1)
		my_printf("\r\n");
		my_printf("Starting 'sendPacketTimeout'\r\n");
		my_printf("Passe dans sendPacketTimeout(uint8_t dest, uint8_t *payload, uint16_t length16, uint32_t wait)\r\n");
	#endif

	state = BSP_SX1272_truncPayload(length16);
	if( state == 0 )
	{
		state_f = BSP_SX1272_setPacket(dest, payload);	// Setting a packet with 'dest' destination
	}
	else
	{
		state_f = state;
	}
	if( state_f == 0 )								// and writing it in FIFO.
	{
		state_f = BSP_SX1272_sendWithTimeout(wait);	// Sending the packet
	}
	//varable = BSP_SX1272_Read(REG_OP_MODE);
	BSP_SX1272_Write(REG_OP_MODE, varable);
	return state_f;
}

/*
 Function: Configures the module to transmit information.
 Returns: Integer that determines if there has been any error
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
*/
//uint8_t BSP_SX1272_sendPacketMAXTimeoutACK(uint8_t dest, char *payload)
//{
//	return BSP_SX1272_sendPacketTimeoutACK(dest, payload, MAX_TIMEOUT);
//}

/*
 Function: Configures the module to transmit information and BSP_SX1272_receive an currentstate.ACK.
 Returns: Integer that determines if there has been any error
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
*/
uint8_t BSP_SX1272_sendPacketMAXTimeoutACK(uint8_t dest,
											uint8_t *payload,
											uint16_t length16)
{
	return BSP_SX1272_sendPacketTimeoutACK(dest, payload, length16, MAX_TIMEOUT);
}

/*
 Function: Configures the module to transmit information and BSP_SX1272_receive an currentstate.ACK.
 Returns: Integer that determines if there has been any error
   state = 9  --> The currentstate.ACK lost (no data available)
   state = 8  --> The currentstate.ACK lost
   state = 7  --> The currentstate.ACK destination incorrectly received
   state = 6  --> The currentstate.ACK source incorrectly received
   state = 5  --> The currentstate.ACK number incorrectly received
   state = 4  --> The currentstate.ACK length incorrectly received
   state = 3  --> N-currentstate.ACK received
   state = 2  --> The currentstate.ACK has not been received
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
*/
//uint8_t BSP_SX1272_sendPacketTimeoutACK(	uint8_t dest,
//											char *payload,
//											uint32_t wait)
//{
//	uint8_t state = 2;
//	uint8_t state_f = 2;
//
//	#if (SX1272_debug_mode > 1)
//		my_printf("\r\n");
//		my_printf("Starting 'sendPacketTimeouACK'\r\n");
//	#endif
//
//	state = BSP_SX1272_sendPacketTimeout(dest, payload, wait);	// Sending packet to 'dest' destination
//	if( state == 0 )
//	{
//		state = BSP_SX1272_receive();	// Setting Rx mode to wait an currentstate.ACK
//	}
//	else
//	{
//		state_f = 1;
//	}
//	if( state == 0 )
//	{
//		if( BSP_SX1272_availableData(MAX_TIMEOUT) )
//		{
//			state_f = BSP_SX1272_getACK(MAX_TIMEOUT);	// Getting currentstate.ACK
//		}
//		else
//		{
//			state_f = 9;
//		}
//	}
//	else
//	{
//		state_f = 1;
//	}
//
//	return state_f;
//}

/*
 Function: Configures the module to transmit information and BSP_SX1272_receive an currentstate.ACK.
 Returns: Integer that determines if there has been any error
   state = 9  --> The currentstate.ACK lost (no data available)
   state = 8  --> The currentstate.ACK lost
   state = 7  --> The currentstate.ACK destination incorrectly received
   state = 6  --> The currentstate.ACK source incorrectly received
   state = 5  --> The currentstate.ACK number incorrectly received
   state = 4  --> The currentstate.ACK length incorrectly received
   state = 3  --> N-currentstate.ACK received
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
*/
uint8_t BSP_SX1272_sendPacketTimeoutACK(	uint8_t dest,
											uint8_t *payload,
											uint16_t length16,
											uint32_t wait)
{
	uint8_t state = 2;
	uint8_t state_f = 2;

	#if (SX1272_debug_mode > 1)
		my_printf("\r\n");
		my_printf("Starting 'sendPacketTimeoutACK'\r\n");
	#endif

	state = BSP_SX1272_sendPacketTimeout(dest, payload, length16, wait);	// Sending packet to 'dest' destination
	if( state == 0 )
	{
		state = BSP_SX1272_receive();	// Setting Rx mode to wait an currentstate.ACK
	}
	else
	{
		state_f = 1;
	}
	if( state == 0 )
	{
		if( BSP_SX1272_availableData(MAX_TIMEOUT) )
		{
			state_f = BSP_SX1272_getACK(MAX_TIMEOUT);	// Getting currentstate.ACK
		}
		else
		{
			state_f = 9;
		}
	}
	else
	{
		state_f = 1;
	}

	return state_f;
}

/*
 Function: It gets and stores an currentstate.ACK if it is received, before ending 'wait' time.
 Returns: Integer that determines if there has been any error
   state = 8  --> The currentstate.ACK lost
   state = 7  --> The currentstate.ACK destination incorrectly received
   state = 6  --> The currentstate.ACK source incorrectly received
   state = 5  --> The currentstate.ACK number incorrectly received
   state = 4  --> The currentstate.ACK length incorrectly received
   state = 3  --> N-currentstate.ACK received
   state = 2  --> The currentstate.ACK has not been received
   state = 1  --> not used (reserved)
   state = 0  --> The currentstate.ACK has been received with no errors
 Parameters:
   wait: time to wait while there is no a valid header received.
*/
uint8_t BSP_SX1272_getACK(uint32_t wait)
{
	uint8_t state = 2;
	uint8_t value = 0x00;
	unsigned long previous;
	uint8_t a_received = 0;

	#if (SX1272_debug_mode > 1)
		my_printf("\r\n");
		my_printf("Starting 'geACK'\r\n");
	#endif

    previous = millis();

	if( currentstate._modem == LORA )
	{ // LoRa mode
	    value = BSP_SX1272_Read(REG_IRQ_FLAGS);
		// Wait until the currentstate.ACK is received (RxDone flag) or the timeout expires
		while ((bitRead(value, 6) == 0) && (millis() - previous < wait))
		{
			value = BSP_SX1272_Read(REG_IRQ_FLAGS);
			if( millis() < previous )
			{
				previous = millis();
			}
		}
		if( bitRead(value, 6) == 1 )
		{ // currentstate.ACK received
			a_received = 1;
		}
		// Standby para minimizar el consumo
		BSP_SX1272_Write(REG_OP_MODE, LORA_STANDBY_MODE);	// Setting standby LoRa mode
	}
	else
	{ // FSK mode
		value = BSP_SX1272_Read(REG_IRQ_FLAGS2);
		// Wait until the packet is received (RxDone flag) or the timeout expires
		while ((bitRead(value, 2) == 0) && (millis() - previous < wait))
		{
			value = BSP_SX1272_Read(REG_IRQ_FLAGS2);
			if( millis() < previous )
			{
				previous = millis();
			}
		}
		if( bitRead(value, 2) == 1 )
		{ // currentstate.ACK received
			a_received = 1;
		}
		// Standby para minimizar el consumo
		BSP_SX1272_Write(REG_OP_MODE, FSK_STANDBY_MODE);	// Setting standby FSK mode
	}

	if( a_received )
	{
//----	BSP_SX1272_Write(REG_FIFO_ADDR_PTR, 0x00);  // Setting address pointer in FIFO data buffer
		// Storing the received currentstate.ACK
		currentstate.ACK.dst = currentstate._destination;
		currentstate.ACK.src = BSP_SX1272_Read(REG_FIFO);
		currentstate.ACK.packnum = BSP_SX1272_Read(REG_FIFO);
		currentstate.ACK.length = BSP_SX1272_Read(REG_FIFO);
		currentstate.ACK.data[0] = BSP_SX1272_Read(REG_FIFO);

		// Checking the received currentstate.ACK
		if( currentstate.ACK.dst == currentstate.packet_sent.src )
		{
			if( currentstate.ACK.src == currentstate.packet_sent.dst )
			{
				if( currentstate.ACK.packnum == currentstate.packet_sent.packnum )
				{
					if( currentstate.ACK.length == 0 )
					{
						if( currentstate.ACK.data[0] == CORRECT_PACKET )
						{
							state = 0;
							#if (SX1272_debug_mode > 0)
							// Printing the received currentstate.ACK
							my_printf("## currentstate.ACK received:\r\n");
							my_printf("%d",currentstate.ACK.dst);
							my_printf("|");
							my_printf("%d",currentstate.ACK.src);
							my_printf("|");
							my_printf("%d",currentstate.ACK.packnum);
							my_printf("|");
							my_printf("%d",currentstate.ACK.length);
							my_printf("|");
							my_printf("%d",currentstate.ACK.data[0]);
							my_printf(" ##\r\n");
							my_printf("\r\n");
							#endif
						}
						else
						{
							state = 3;
							#if (SX1272_debug_mode > 0)
								my_printf("** N-currentstate.ACK received **\r\n");
								my_printf("\r\n");
							#endif
						}
					}
					else
					{
						state = 4;
						#if (SX1272_debug_mode > 0)
							my_printf("** currentstate.ACK length incorrectly received **\r\n");
							my_printf("\r\n");
						#endif
					}
				}
				else
				{
					state = 5;
					#if (SX1272_debug_mode > 0)
						my_printf("** currentstate.ACK number incorrectly received **\r\n");
						my_printf("\r\n");
					#endif
				}
			}
			else
			{
				state = 6;
				#if (SX1272_debug_mode > 0)
					my_printf("** currentstate.ACK source incorrectly received **\r\n");
					my_printf("\r\n");
				#endif
			}
		}
		else
		{
			state = 7;
			#if (SX1272_debug_mode > 0)
				my_printf("** currentstate.ACK destination incorrectly received **\r\n");
				my_printf("\r\n");
			#endif
		}
	}
	else
	{
		state = 8;
		#if (SX1272_debug_mode > 0)
			my_printf("** currentstate.ACK lost **\r\n");
			my_printf("\r\n");
		#endif
	}
	BSP_SX1272_clearFlags();	// Initializing flags
	return state;
}

/*
 Function: Configures the module to transmit information with retries in case of error.
 Returns: Integer that determines if there has been any error
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
*/
//uint8_t BSP_SX1272_sendPacketMAXTimeoutACKRetries(	uint8_t dest,
//													char  *payload)
//{
//	return BSP_SX1272_sendPacketTimeoutACKRetries(dest, payload, MAX_TIMEOUT);
//}

/*
 Function: Configures the module to transmit information with retries in case of error.
 Returns: Integer that determines if there has been any error
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
*/
uint8_t BSP_SX1272_sendPacketMAXTimeoutACKRetries(	uint8_t dest,
													uint8_t *payload,
													uint16_t length16)
{
	return BSP_SX1272_sendPacketTimeoutACKRetries(dest, payload, length16, MAX_TIMEOUT);
}



/*
 Function: Configures the module to transmit information with retries in case of error.
 Returns: Integer that determines if there has been any error
   state = 9  --> The currentstate.ACK lost (no data available)
   state = 8  --> The currentstate.ACK lost
   state = 7  --> The currentstate.ACK destination incorrectly received
   state = 6  --> The currentstate.ACK source incorrectly received
   state = 5  --> The currentstate.ACK number incorrectly received
   state = 4  --> The currentstate.ACK length incorrectly received
   state = 3  --> N-currentstate.ACK received
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
*/
/*uint8_t BSP_SX1272_sendPacketTimeoutACKRetries(uint8_t dest,
												char *payload,
												uint32_t wait)
{
	uint8_t state = 2;

	#if (SX1272_debug_mode > 1)
		my_printf("\r\n");
		my_printf("Starting 'sendPacketTimeoutACKRetries'\r\n");
	#endif

	// Sending packet to 'dest' destination and waiting an currentstate.ACK response.
	state = 1;
	while( (state != 0) && (currentstate._retries <= currentstate._maxRetries) )
	{
		state = BSP_SX1272_sendPacketTimeoutACK(dest, payload, wait);
		currentstate._retries++;
	}
	currentstate._retries = 0;

	return state;
}*/

/*
 Function: Configures the module to transmit information with retries in case of error.
 Returns: Integer that determines if there has been any error
   state = 9  --> The currentstate.ACK lost (no data available)
   state = 8  --> The currentstate.ACK lost
   state = 7  --> The currentstate.ACK destination incorrectly received
   state = 6  --> The currentstate.ACK source incorrectly received
   state = 5  --> The currentstate.ACK number incorrectly received
   state = 4  --> The currentstate.ACK length incorrectly received
   state = 3  --> N-currentstate.ACK received
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
*/
uint8_t BSP_SX1272_sendPacketTimeoutACKRetries(uint8_t dest,
												uint8_t *payload,
												uint16_t length16,
												uint32_t wait)
{
	uint8_t state = 2;

	#if (SX1272_debug_mode > 1)
		my_printf("\r\n");
		my_printf("Starting 'sendPacketTimeoutACKRetries'\r\n");
	#endif

	// Sending packet to 'dest' destination and waiting an currentstate.ACK response.
	state = 1;
	while( (state != 0) && (currentstate._retries <= currentstate._maxRetries) )
	{
		state = BSP_SX1272_sendPacketTimeoutACK(dest, payload, length16, wait);
		currentstate._retries++;
	}
	currentstate._retries = 0;

	return state;
}

/*
 Function: It gets the temperature from the measurement block module.
 Returns: Integer that determines if there has been any error
   state = 2  --> The command has not been executed
   state = 1  --> There has been an error while executing the command
   state = 0  --> The command has been executed with no errors
*/
uint8_t BSP_SX1272_getTemp()
{
	uint8_t st0;
	uint8_t state = 2;

	#if (SX1272_debug_mode > 1)
		my_printf("\r\n");
		my_printf("Starting 'getTemp'\r\n");
	#endif

	st0 = BSP_SX1272_Read(REG_OP_MODE);	// Save the previous status

	if( currentstate._modem == LORA )
	{ // Allowing access to FSK registers while in LoRa standby mode
		BSP_SX1272_Write(REG_OP_MODE, LORA_STANDBY_FSK_REGS_MODE);
	}

	state = 1;
	// Saving temperature value
	currentstate._temp = BSP_SX1272_Read(REG_TEMP);
	if( currentstate._temp & 0x80 ) // The SNR sign bit is 1
	{
		// Invert and divide by 4
		currentstate._temp = ( ( ~currentstate._temp + 1 ) & 0xFF );
    }
    else
    {
		// Divide by 4
		currentstate._temp = ( currentstate._temp & 0xFF );
	}


	#if (SX1272_debug_mode > 1)
		my_printf("## Temperature is: ");
		my_printf("%d",currentstate._temp);
		my_printf(" ##\r\n");
		my_printf("\r\n");
	#endif

	if( currentstate._modem == LORA )
	{
		BSP_SX1272_Write(REG_OP_MODE, st0);	// Getting back to previous status
	}

	state = 0;
	return state;
}

/*
 Function: It sets the CAD mode to search Channel Activity Detection
 Returns: Integer that determines if there has been any error
   state = 1   --> Channel Activity Detected
   state = 0  --> Channel Activity NOT Detected
*/
uint8_t BSP_SX1272_cadDetected()
{
	uint8_t val = 0;

	// get actual time
	unsigned long time = millis();

	// set LNA
	BSP_SX1272_Write(REG_LNA,0x23);
	BSP_SX1272_clearFlags();

	BSP_SX1272_getRSSI();

	#if (SX1272_debug_mode > 1)
		my_printf("Inside CAD DETECTION -> RSSI: ");
		my_printf("%d",currentstate._RSSI);
	#endif

	if( currentstate._modem == LORA )
	{
		#if (SX1272_debug_mode > 1)
			my_printf("Set CAD mode\r\n");
		#endif

		// Setting LoRa CAD mode
		BSP_SX1272_Write(REG_OP_MODE,0x87);
	}

	// Wait for IRQ CadDone
    val = BSP_SX1272_Read(REG_IRQ_FLAGS);
    while((bitRead(val,2) == 0) && (millis()-time)<10000 )
    {
      val = BSP_SX1272_Read(REG_IRQ_FLAGS);
    }

	// After waiting or detecting CadDone
	// check 'CadDetected' bit in 'RegIrqFlags' register
    if(bitRead(val,0) == 1)
    {
		#if (SX1272_debug_mode > 1)
			my_printf("CAD 1\r\n");
		#endif
		return 1;
	}

	#if (SX1272_debug_mode > 1)
		my_printf("CAD 0\r\n");
	#endif
	return 0;

}


