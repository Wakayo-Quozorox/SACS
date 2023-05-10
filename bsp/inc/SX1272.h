/*
 * SX1272.h
 *
 *  Created on: 24 ao�t 2020
 *      Author: Arnaud
 */

#ifndef BSP_INC_SX1272_H_
#define BSP_INC_SX1272_H_

#include "stm32f0xx.h"
#include <stdlib.h>
#include <stdint.h>

#ifndef inttypes_h
	#include <inttypes.h>
#endif


/******************************************************************************
 * Definitions & Declarations
 *****************************************************************************/

#define SX1272_debug_mode 0

// pins used for SS and Reset signals of Arduino MEGA 2560 board (reset signal is optional)
#define SX1272_RST  A0
#define SX1272_SS 10


// MACROS //
#define bitRead(value, bit) (((value) >> (bit)) & 0x01)  // read a bit
#define bitSet(value, bit) ((value) |= (1UL << (bit)))    // set bit to '1'
#define bitClear(value, bit) ((value) &= ~(1UL << (bit))) // set bit to '0'


// REGISTERS //

#define        REG_FIFO        					0x00
#define        REG_OP_MODE        				0x01
#define        REG_BITRATE_MSB    				0x02
#define        REG_BITRATE_LSB    				0x03
#define        REG_FDEV_MSB   					0x04
#define        REG_FDEV_LSB    					0x05
#define        REG_FRF_MSB    					0x06
#define        REG_FRF_MID    					0x07
#define        REG_FRF_LSB    					0x08
#define        REG_PA_CONFIG    				0x09
#define        REG_PA_RAMP    					0x0A
#define        REG_OCP    						0x0B
#define        REG_LNA    						0x0C
#define        REG_RX_CONFIG    				0x0D
#define        REG_FIFO_ADDR_PTR  				0x0D
#define        REG_RSSI_CONFIG   				0x0E
#define        REG_FIFO_TX_BASE_ADDR 		    0x0E
#define        REG_RSSI_COLLISION    			0x0F
#define        REG_FIFO_RX_BASE_ADDR   			0x0F
#define        REG_RSSI_THRESH    				0x10
#define        REG_FIFO_RX_CURRENT_ADDR   		0x10
#define        REG_RSSI_VALUE_FSK	    		0x11
#define        REG_IRQ_FLAGS_MASK    			0x11
#define        REG_RX_BW		    			0x12
#define        REG_IRQ_FLAGS	    			0x12
#define        REG_AFC_BW		    			0x13
#define        REG_RX_NB_uint8_tS	    			0x13
#define        REG_OOK_PEAK	    				0x14
#define        REG_RX_HEADER_CNT_VALUE_MSB  	0x14
#define        REG_OOK_FIX	    				0x15
#define        REG_RX_HEADER_CNT_VALUE_LSB  	0x15
#define        REG_OOK_AVG	 					0x16
#define        REG_RX_PACKET_CNT_VALUE_MSB  	0x16
#define        REG_RX_PACKET_CNT_VALUE_LSB  	0x17
#define        REG_MODEM_STAT	  				0x18
#define        REG_PKT_SNR_VALUE	  			0x19
#define        REG_AFC_FEI	  					0x1A
#define        REG_PKT_RSSI_VALUE	  			0x1A
#define        REG_AFC_MSB	  					0x1B
#define        REG_RSSI_VALUE_LORA	  			0x1B
#define        REG_AFC_LSB	  					0x1C
#define        REG_HOP_CHANNEL	  				0x1C
#define        REG_FEI_MSB	  					0x1D
#define        REG_MODEM_CONFIG1	 		 	0x1D
#define        REG_FEI_LSB	  					0x1E
#define        REG_MODEM_CONFIG2	  			0x1E
#define        REG_PREAMBLE_DETECT  			0x1F
#define        REG_SYMB_TIMEOUT_LSB  			0x1F
#define        REG_RX_TIMEOUT1	  				0x20
#define        REG_PREAMBLE_MSB_LORA  			0x20
#define        REG_RX_TIMEOUT2	  				0x21
#define        REG_PREAMBLE_LSB_LORA  			0x21
#define        REG_RX_TIMEOUT3	 				0x22
#define        REG_PAYLOAD_LENGTH_LORA		 	0x22
#define        REG_RX_DELAY	 					0x23
#define        REG_MAX_PAYLOAD_LENGTH 			0x23
#define        REG_OSC		 					0x24
#define        REG_HOP_PERIOD	  				0x24
#define        REG_PREAMBLE_MSB_FSK 			0x25
#define        REG_FIFO_RX_uint8_t_ADDR 		0x25
#define        REG_PREAMBLE_LSB_FSK 			0x26
#define        REG_SYNC_CONFIG	  				0x27
#define        REG_SYNC_VALUE1	 				0x28
#define        REG_SYNC_VALUE2	  				0x29
#define        REG_SYNC_VALUE3	  				0x2A
#define        REG_SYNC_VALUE4	  				0x2B
#define        REG_SYNC_VALUE5	  				0x2C
#define        REG_SYNC_VALUE6	  				0x2D
#define        REG_SYNC_VALUE7	  				0x2E
#define        REG_SYNC_VALUE8	  				0x2F
#define        REG_PACKET_CONFIG1	  			0x30
#define        REG_PACKET_CONFIG2	  			0x31
#define        REG_DETECT_OPTIMIZE	  			0x31
#define        REG_PAYLOAD_LENGTH_FSK			0x32
#define        REG_NODE_ADRS	  				0x33
#define        REG_BROADCAST_ADRS	 		 	0x34
#define        REG_FIFO_THRESH	  				0x35
#define        REG_SEQ_CONFIG1	  				0x36
#define        REG_SEQ_CONFIG2	  				0x37
#define        REG_DETECTION_THRESHOLD 			0x37
#define        REG_TIMER_RESOL	  				0x38
#define        REG_TIMER1_COEF	  				0x39
#define        REG_TIMER2_COEF	  				0x3A
#define        REG_IMAGE_CAL	  				0x3B
#define        REG_TEMP		  					0x3C
#define        REG_LOW_BAT	  					0x3D
#define        REG_IRQ_FLAGS1	  				0x3E
#define        REG_IRQ_FLAGS2	  				0x3F
#define        REG_DIO_MAPPING1	  				0x40
#define        REG_DIO_MAPPING2	  				0x41
#define        REG_VERSION	  					0x42
#define        REG_AGC_REF	  					0x43
#define        REG_AGC_THRESH1	  				0x44
#define        REG_AGC_THRESH2	  				0x45
#define        REG_AGC_THRESH3	  				0x46
#define        REG_PLL_HOP	  					0x4B
#define        REG_TCXO		  					0x58
#define        REG_PA_DAC		  				0x5A
#define        REG_PLL		  					0x5C
#define        REG_PLL_LOW_PN	  				0x5E
#define        REG_FORMER_TEMP	  				0x6C
#define        REG_BIT_RATE_FRAC	  			0x70

//FREQUENCY CHANNELS:
//the frequency frf depends on the 32 bit register FRF according to the relation:
//frf = 32e6*FRF/2^19
#define CH_868v1  0xD90666 // central freq  868.10MHz
#define CH_868v3  0xD91333 // central freq  868.30MHz
#define CH_868v5  0xD92000 // central freq  868.50MHz
// Other channels:
#define CH_10_868  0xD84CCC // channel 10, central freq  865.20MHz
#define CH_11_868  0xD86000 // channel 11, central freq  865.50MHz
#define CH_12_868  0xD87333 // channel 12, central freq  865.80MHz
#define CH_13_868  0xD88666 // channel 13, central freq  866.10MHz
#define CH_14_868  0xD89999 // channel 14, central freq  866.40MHz
#define CH_15_868  0xD8ACCC // channel 15, central freq  866.70MHz
#define CH_16_868  0xD8C000 // channel 16, central freq  867.00MHz
#define CH_17_868  0xD90000 // channel 17, central freq  868.00MHz
#define CH_00_900  0xE1C51E // channel 00, central freq  903.08MHz
#define CH_01_900  0xE24F5C // channel 01, central freq  905.24MHz
#define CH_02_900  0xE2D999 // channel 02, central freq  907.40MHz
#define CH_03_900  0xE363D7 // channel 03, central freq  909.56MHz
#define CH_04_900  0xE3EE14 // channel 04, central freq  911.72MHz
#define CH_05_900  0xE47851 // channel 05, central freq  913.88MHz
#define CH_06_900  0xE5028F // channel 06, central freq  916.04MHz
#define CH_07_900  0xE58CCC // channel 07, central freq  918.20MHz
#define CH_08_900  0xE6170A // channel 08, central freq  920.36MHz
#define CH_09_900  0xE6A147 // channel 09, central freq  922.52MHz
#define CH_10_900  0xE72B85 // channel 10, central freq  924.68MHz
#define CH_11_900  0xE7B5C2 // channel 11, central freq  926.84MHz
#define CH_12_900  0xE4C000 // default channel 915MHz

//LORA BANDWIDTH:
#define BW_125  0x00
#define BW_250  0x01
#define BW_500  0x02
/*double SignalBwLog[] =
{
    5.0969100130080564143587833158265,
    5.397940008672037609572522210551,
    5.6989700043360188047862611052755
};*/

//LORA CODING RATE:
#define CR_5  0x01	// CR  4/5
#define CR_6  0x02	// CR  4/6
#define CR_7  0x03	// CR  4/7
#define CR_8  0x04	// CR  4/8

//LORA SPREADING FACTOR:
#define SF_6  0x06
#define SF_7  0x07
#define SF_8  0x08
#define SF_9  0x09
#define SF_10  0x0A
#define SF_11  0x0B
#define SF_12  0x0C

//OUTPUT POWER (from -1 dBm up to +14 dBm, step  + 1 dB)
#define POW_m1  0x0
#define POW_0  0x1
#define POW_1  0x2
#define POW_2  0x3
#define POW_3  0x4
#define POW_4  0x5
#define POW_5  0x6
#define POW_6  0x7
#define POW_7  0x8
#define POW_8  0x9
#define POW_9  0xA
#define POW_10  0xB
#define POW_11  0xC
#define POW_12  0xD
#define POW_13  0xE
#define POW_14  0xF

//LORA MODES:
#define LORA_SLEEP_MODE  0x80
#define LORA_STANDBY_MODE  0x81
#define LORA_TX_MODE  0x83
#define LORA_RX_MODE  0x85
#define LORA_STANDBY_FSK_REGS_MODE  0xC1

//FSK MODES:
#define FSK_SLEEP_MODE  0x00
#define FSK_STANDBY_MODE  0x01
#define FSK_TX_MODE  0x03
#define FSK_RX_MODE  0x05

//OTHER CONSTANTS:

#define st_SD_ON  1
#define st_SD_OFF  0
#define HEADER_ON  0
#define HEADER_OFF  1
#define CRC_ON  1
#define CRC_OFF  0
#define LORA  1
#define FSK  0
#define BROADCAST_0  0x00
#define MAX_LENGTH  255
//#define MAX_PAYLOAD  251
#define MAX_PAYLOAD 251
#define MAX_LENGTH_FSK  64
#define MAX_PAYLOAD_FSK  60
#define ACK_LENGTH	5
#define OFFSET_PAYLOADLENGTH  5
#define OFFSET_RSSI  137
#define NOISE_FIGURE  6.0
#define NOISE_ABSOLUTE_ZERO  174.0
#define MAX_TIMEOUT  10000		//10000 msec  10.0 sec
#define MAX_WAIT  12000		//12000 msec  12.0 sec
#define MESH_TIMEOUT  3600000  //3600000 msec  3600 sec  1 hour
#define MAX_RETRIES  5
#define CORRECT_PACKET  0
#define INCORRECT_PACKET  1
#define BROADCAST_ADDR  0

//AJOUT:

#define FDA5kHz  0x52
#define FDA40kHz  0x28F
#define FDA125kHz  0x7FF
#define BR1200bauds  0x682B
#define BR4800bauds  0x1A0B
#define BR38p4kbauds  0x30D
#define BR62p5kbauds  0x200

// Structure pack :
typedef struct
{
	// Structure Variable : Packet destination
	uint8_t dst;
	// Structure Variable : Packet source
	uint8_t src;
	// Structure Variable : Packet number
	uint8_t packnum;
	// Structure Variable : Packet length
	uint8_t length;
	// Structure Variable : Packet payload
	uint8_t data[MAX_PAYLOAD];
	// Structure Variable : Retry number
	uint8_t retry;
}pack;

// Structure SX1272status :
typedef struct
{
/// Variables /////////////////////////////////////////////////////////////

// Variable : SD state.
//    st_SD = 00  --> SD_OFF
//    st_SD = 01  --> SD_ON
uint8_t st_SD;

// Variable : bandwidth configured in LoRa mode.
//    bandwidth = 00  --> BW = 125KHz
//    bandwidth = 01  --> BW = 250KHz
//    bandwidth = 10  --> BW = 500KHz
uint8_t _bandwidth;

// Variable : coding rate configured in LoRa mode.
//    codingRate = 001  --> CR = 4/5
//    codingRate = 010  --> CR = 4/6
//    codingRate = 011  --> CR = 4/7
//    codingRate = 100  --> CR = 4/8
uint8_t _codingRate;

// Variable : spreading factor configured in LoRa mode.
//    spreadingFactor = 6   --> SF = 6, 64 chips/symbol
//    spreadingFactor = 7   --> SF = 7, 128 chips/symbol
//    spreadingFactor = 8   --> SF = 8, 256 chips/symbol
//    spreadingFactor = 9   --> SF = 9, 512 chips/symbol
//    spreadingFactor = 10  --> SF = 10, 1024 chips/symbol
//    spreadingFactor = 11  --> SF = 11, 2048 chips/symbol
//    spreadingFactor = 12  --> SF = 12, 4096 chips/symbol
uint8_t _spreadingFactor;

// Variable : frequency channel.
//		channel= 0xD90666  --> CH = 868.10MHz
//		channel= 0xD91333  --> CH = 868.30MHz
//		channel= 0xD92000  --> CH = 868.50MHz
// Other channels:
//    channel = 0xD84CCC  --> CH = 10_868, 865.20MHz
//    channel = 0xD86000  --> CH = 11_868, 865.50MHz
//    channel = 0xD87333  --> CH = 12_868, 865.80MHz
//    channel = 0xD88666  --> CH = 13_868, 866.10MHz
//    channel = 0xD89999  --> CH = 14_868, 866.40MHz
//    channel = 0xD8ACCC  --> CH = 15_868, 866.70MHz
//    channel = 0xD8C000  --> CH = 16_868, 867.00MHz
//    channel = 0xE1C51E  --> CH = 00_900, 903.08MHz
//    channel = 0xE24F5C  --> CH = 01_900, 905.24MHz
//    channel = 0xE2D999  --> CH = 02_900, 907.40MHz
//    channel = 0xE363D7  --> CH = 03_900, 909.56MHz
//    channel = 0xE3EE14  --> CH = 04_900, 911.72MHz
//    channel = 0xE47851  --> CH = 05_900, 913.88MHz
//    channel = 0xE5028F  --> CH = 06_900, 916.04MHz
//    channel = 0xE58CCC  --> CH = 07_900, 918.20MHz
//    channel = 0xE6170A  --> CH = 08_900, 920.36MHz
//    channel = 0xE6A147  --> CH = 09_900, 922.52MHz
//    channel = 0xE72B85  --> CH = 10_900, 924.68MHz
//    channel = 0xE7B5C2  --> CH = 11_900, 926.84MHz
uint32_t _channel;

// Variable : output power.
uint8_t _power;

// Variable : SNR from the last packet received in LoRa mode.
int8_t _SNR;

// Variable : RSSI current value.
int8_t _RSSI;

// Variable : RSSI from the last packet received in LoRa mode.
int16_t _RSSIpacket;

// Variable : preamble length sent/received.
uint16_t _preamblelength;

// Variable : payload length sent/received.
uint16_t _payloadlength;

// Variable : node address.
uint8_t _nodeAddress;

// Variable : implicit or explicit header in LoRa mode.
uint8_t _header;

// Variable : header received while waiting a packet to arrive.
uint8_t _hreceived;

// Variable : presence or absence of CRC calculation.
uint8_t _CRC;

// Variable : packet destination.
uint8_t _destination;

// Variable : packet number.
uint8_t _packetNumber;

// Variable : indicates if received packet is correct or incorrect.
uint8_t _reception;

// Variable : number of current retry.
uint8_t _retries;

// Variable : maximum number of retries.
uint8_t _maxRetries;

// Variable : maximum current supply.
uint8_t _maxCurrent;

// Variable : indicates FSK or LoRa modem.
uint8_t _modem;

// Variable : array with all the information about a sent packet.
pack packet_sent;

// Variable : array with all the information about a received packet.
pack packet_received;

// Variable : array with all the information about a sent/received ack.
pack ACK;

// Variable : temperature module.
int _temp;

// Variable : current timeout to send a packet.
uint16_t _sendTime;

}SX1272status;



// It puts the module ON
uint8_t BSP_SX1272_ON(int type_mod);
// It puts the module OFF
void BSP_SX1272_OFF(int type_mod);
// It clears the interruption flags.
void BSP_SX1272_clearFlags();

// It sets the LoRa mode on.
/* It stores in global '_LORA' variable '1' when success
\return '0' on success, '1' otherwise */
uint8_t BSP_SX1272_setLORA();

// It sets the FSK mode on.
/* It stores in global '_FSK' variable '1' when success
 * \return '0' on success, '1' otherwise */
uint8_t BSP_SX1272_setFSK();

// It gets the BW, SF and CR of the module.
/* It stores in global '_bandwidth' variable the BW
It stores in global '_codingRate' variable the CR
It stores in global '_spreadingFactor' variable the SF
\return '0' on success, '1' otherwise  */
uint8_t BSP_SX1272_getMode();

// It sets the BW, SF and CR of the module.
/* It stores in global '_bandwidth' variable the BW
It stores in global '_codingRate' variable the CR
It stores in global '_spreadingFactor' variable the SF
\param uint8_t mode : there is a mode number to different values of
the	configured parameters with this function.
\return '0' on success, '1' otherwise  */
int8_t BSP_SX1272_setMode(uint8_t mode);

// It gets the header mode configured.
/* It stores in global '_header' variable '0' when header is sent
(explicit header mode) or '1' when is not sent (implicit header
mode).
\return '0' on success, '1' otherwise */
uint8_t	BSP_SX1272_getHeader();

// It sets explicit header mode.
/* It stores in global '_header' variable '1' when success
\return '0' on success, '1' otherwise  */
int8_t	BSP_SX1272_setHeaderON();

// It sets implicit header mode.
/* It stores in global '_header' variable '0' when success
\return '0' on success, '1' otherwise  */
int8_t	BSP_SX1272_setHeaderOFF();

// It gets the CRC configured.
/* It stores in global '_CRC' variable '1' enabling CRC generation on
payload, or '0' disabling the CRC.
\return '0' on success, '1' otherwise  */
uint8_t	BSP_SX1272_getCRC();

// It sets CRC on.
/* It stores in global '_CRC' variable '1' when success
\return '0' on success, '1' otherwise  */
uint8_t	BSP_SX1272_setCRC_ON();

// It sets CRC off.
/* It stores in global '_CRC' variable '0' when success
\return '0' on success, '1' otherwise  */
uint8_t	BSP_SX1272_setCRC_OFF();

// It is true if the SF selected exists.
/* \param uint8_t spr : spreading factor value to check.
\return 'true' on success, 'false' otherwise*/
uint8_t	BSP_SX1272_isSF(uint8_t spr);

// It gets the SF configured.
/* It stores in global '_spreadingFactor' variable the current value of SF
\return '0' on success, '1' otherwise */
int8_t	BSP_SX1272_getSF();

// It sets the SF.
/* It stores in global '_spreadingFactor' variable the current value of SF
\param uint8_t spr : spreading factor value to set in the configuration.
\return '0' on success, '1' otherwise  */
uint8_t	BSP_SX1272_setSF(uint8_t spr);

// It is true if the BW selected exists.
/* \param uint16_t band : bandwidth value to check.
\return 'true' on success, 'false' otherwise */
uint8_t	BSP_SX1272_isBW(uint16_t band);

// It gets the BW configured.
/* It stores in global '_bandwidth' variable the BW selected
in the configuration
\return '0' on success, '1' otherwise */
int8_t	BSP_SX1272_getBW();

// It sets the BW.
/* It stores in global '_bandwidth' variable the BW selected
in the configuration
\param uint16_t band : bandwidth value to set in the configuration.
\return '0' on success, '1' otherwise */
int8_t BSP_SX1272_setBW(uint16_t band);

// It is true if the CR selected exists.
/* \param uint8_t cod : the coding rate value to check.
\return 'true' on success, 'false' otherwise  */
uint8_t	BSP_SX1272_isCR(uint8_t cod);

// It gets the CR configured.
/* It stores in global '_codingRate' variable the CR selected
in the configuration
\return '0' on success, '1' otherwise */
int8_t	BSP_SX1272_getCR();

// It sets the CR.
/* It stores in global '_codingRate' variable the CR selected
in the configuration
\param uint8_t cod : coding rate value to set in the configuration.
\return '0' on success, '1' otherwise  */
int8_t	BSP_SX1272_setCR(uint8_t cod);

// It is true if the channel selected exists.
/* \param uint32_t ch : frequency channel value to check.
\return 'true' on success, 'false' otherwise  */
uint8_t BSP_SX1272_isChannel(uint32_t ch);

// It gets frequency channel the module is using.
/* It stores in global '_channel' variable the frequency channel
\return '0' on success, '1' otherwise */
uint8_t BSP_SX1272_getChannel();

// It sets frequency channel the module is using.
/* It stores in global '_channel' variable the frequency channel
\param uint32_t ch : frequency channel value to set in the configuration.
\return '0' on success, '1' otherwise */
int8_t BSP_SX1272_setChannel(uint32_t ch);

// It gets the output power of the signal.
/* It stores in global '_power' variable the output power of the signal
\return '0' on success, '1' otherwise */
uint8_t BSP_SX1272_getPower();

// It sets the output power of the signal.
/* It stores in global '_power' variable the output power of the signal
\param uint8_t pow : value to set as output power.
\return '0' on success, '1' otherwise */
int8_t BSP_SX1272_setPower(uint8_t pow);

// It gets the preamble length configured.
/* It stores in global '_preamblelength' variable the preamble length
\return '0' on success, '1' otherwise */
uint8_t BSP_SX1272_getPreambleLength();

// It sets the preamble length.
/* It stores in global '_preamblelength' variable the preamble length
\param uint16_t l : preamble length to set in the configuration.
\return '0' on success, '1' otherwise  */
uint8_t BSP_SX1272_setPreambleLength(uint16_t l);

// It gets the payload length of the last packet to send/receive.
/* It stores in global '_payloadlength' variable the payload length of
the last packet to send/receive.
\return '0' on success, '1' otherwise  */
uint8_t BSP_SX1272_getPayloadLength();

// It sets the packet length to send/receive.
/* It stores in global '_payloadlength' variable the payload length of
the last packet to send/receive.
\param uint8_t l : payload length to set in the configuration.
\return '0' on success, '1' otherwise  */
int8_t BSP_SX1272_setPacketLength(uint8_t l);

// It gets the node address of the mote.
/* It stores in global '_nodeAddress' variable the node address
\return '0' on success, '1' otherwise */
uint8_t BSP_SX1272_getNodeAddress();

// It sets the node address of the mote.
/*
It stores in global '_nodeAddress' variable the node address
\param uint8_t addr : address value to set as node address.
\return '0' on success, '1' otherwise  */
int8_t BSP_SX1272_setNodeAddress(uint8_t addr);

// It gets the SNR of the latest received packet.
/* It stores in global '_SNR' variable the SNR
\return '0' on success, '1' otherwise */
int8_t BSP_SX1272_getSNR();

// It gets the current value of RSSI.
/* It stores in global '_RSSI' variable the current value of RSSI
\return '0' on success, '1' otherwise  */
uint8_t BSP_SX1272_getRSSI();

// It gets the RSSI of the latest received packet.
/* It stores in global '_RSSIpacket' variable the RSSI of the latest
packet received.
\return '0' on success, '1' otherwise  */
int16_t BSP_SX1272_getRSSIpacket();

// It sets the total of retries when a packet is not correctly received.
/* It stores in global '_maxRetries' variable the number of retries.
\param uint8_t ret : number of retries.
\return '0' on success, '1' otherwise */
uint8_t BSP_SX1272_setRetries(uint8_t ret);

// It gets the maximum current supply by the module.
/* *
\return '0' on success, '1' otherwise */
uint8_t BSP_SX1272_getMaxCurrent();

// It sets the maximum current supply by the module.
/* It stores in global '_maxCurrent' variable the maximum current supply.
\param uint8_t rate : maximum current supply.
\return '0' on success, '1' otherwise  */
int8_t BSP_SX1272_setMaxCurrent(uint8_t rate);

// It gets the content of the main configuration registers.
/* It stores in global '_bandwidth' variable the BW.
It stores in global '_codingRate' variable the CR.
It stores in global '_spreadingFactor' variable the SF.
It stores in global '_power' variable the output power of the signal.
It stores in global '_channel' variable the frequency channel.
It stores in global '_CRC' variable '1' enabling CRC generation on
payload, or '0' disabling the CRC.
It stores in global '_header' variable '0' when header is sent
(explicit header mode) or '1' when is not sent (implicit header
mode).
It stores in global '_preamblelength' variable the preamble length.
It stores in global '_payloadlength' variable the payload length of
the last packet to send/receive.
It stores in global '_nodeAddress' variable the node address.
It stores in global '_temp' variable the module temperature.
\return '0' on success, '1' otherwise */
uint8_t BSP_SX1272_getRegs();

// It sets the maximum number of uint8_ts from a frame that fit in a packet
// structure.
/* It stores in global '_payloadlength' variable the maximum number of uint8_ts.
\param uint16_t length16 : total frame length.
\return '0' on success, '1' otherwise */
uint8_t BSP_SX1272_truncPayload(uint16_t length16);

// It writes an ACK in FIFO to send it.
/* *
\return '0' on success, '1' otherwise */
uint8_t BSP_SX1272_setACK();

// It puts the module in reception mode.
/* *
\return '0' on success, '1' otherwise  */
uint8_t BSP_SX1272_receive();

// It receives a packet before MAX_TIMEOUT.
/*  *
\return '0' on success, '1' otherwise  */
uint8_t BSP_SX1272_receivePacketMAXTimeout();

// It receives a packet before a timeout.
/* \param uint32_t wait : time to wait to receive something.
\return '0' on success, '1' otherwise  */
uint8_t BSP_SX1272_receivePacketTimeout(uint32_t wait);

// It receives a packet before MAX_TIMEOUT and reply with an ACK.
/* *
\return '0' on success, '1' otherwise  */
uint8_t BSP_SX1272_receivePacketMAXTimeoutACK();

// It receives a packet before a timeout and reply with an ACK.
/* \param uint32_t wait : time to wait to receive something.
\return '0' on success, '1' otherwise  */
uint8_t BSP_SX1272_receivePacketTimeoutACK(uint32_t wait);

// It puts the module in 'promiscuous' reception mode with a timeout.
/* \param uint32_t wait : time to wait to receive something.
\return '0' on success, '1' otherwise  */
uint8_t BSP_SX1272_receiveAll(uint32_t wait);

// It checks if there is an available packet and its destination before a
// timeout.
/* *
\param uint32_t wait : time to wait while there is no a valid header
received.
\return 'true' on success, 'false' otherwise  */
uint8_t	BSP_SX1272_availableData(uint32_t wait);

// It writes a packet in FIFO in order to send it.
/* \param uint8_t dest : packet destination.
\param char *payload : packet payload.
\return '0' on success, '1' otherwise */
uint8_t BSP_SX1272_setPacket(uint8_t dest, char *payload);

// It writes a packet in FIFO in order to send it.
/* \param uint8_t dest : packet destination.
\param uint8_t *payload: packet payload.
\return '0' on success, '1' otherwise */
//uint8_t BSP_SX1272_setPacket(uint8_t dest, uint8_t *payload);

// It reads a received packet from the FIFO, if it arrives before ending
// MAX_TIMEOUT time.
/*  *
\return '0' on success, '1' otherwise */
uint8_t BSP_SX1272_getPacketMAXTimeout();

// It receives and gets a packet from FIFO, if it arrives before ending
// 'wait' time.
/*  *
\param uint32_t wait : time to wait while there is not a complete packet
received.
\return '0' on success, '1' otherwise */
int8_t BSP_SX1272_getPacket(uint32_t wait);

// It sends the packet stored in FIFO before ending MAX_TIMEOUT.
/* *
\return '0' on success, '1' otherwise */
uint8_t BSP_SX1272_sendWithMAXTimeout();

// It tries to send the packet stored in FIFO before ending 'wait' time.
/* \param uint32_t wait : time to wait to send the packet.
\return '0' on success, '1' otherwise */
uint8_t BSP_SX1272_sendWithTimeout(uint32_t wait);

// It tries to send the packet which payload is a parameter before ending
// MAX_TIMEOUT.
/* \param uint8_t dest : packet destination.
\param char *payload : packet payload.
\return '0' on success, '1' otherwise */
uint8_t BSP_SX1272_sendPacketMAXTimeout(	uint8_t dest,char *payload);

// It tries to send the packet which payload is a parameter before ending
// MAX_TIMEOUT.
/* \param uint8_t dest : packet destination.
\param uint8_t *payload : packet payload.
\param uint16_t length : payload buffer length.
\return '0' on success, '1' otherwise */
//uint8_t sendPacketMAXTimeout(	uint8_t dest,
//								uint8_t *payload,
//								uint16_t length);

// It sends the packet which payload is a parameter before ending
// MAX_TIMEOUT.
/* \param uint8_t dest : packet destination.
\param uint8_t *payload: packet payload.
\param uint16_t length : payload buffer length.
\return '0' on success, '1' otherwise */
//uint8_t sendPacketTimeout(	uint8_t dest,
//							uint8_t *payload,
//							uint16_t length);

// It sends the packet which payload is a parameter before ending 'wait'
// time.
/* \param uint8_t dest : packet destination.
\param char *payload : packet payload.
\param uint32_t wait : time to wait.
\return '0' on success, '1' otherwise */
uint8_t BSP_SX1272_sendPacketTimeout(	uint8_t dest,
							char *payload,
							uint32_t wait);

// It sends the packet which payload is a parameter before ending 'wait'
// time.
/* \param uint8_t dest : packet destination.
\param uint8_t *payload : packet payload.
\param uint16_t length : payload buffer length.
\param uint32_t wait : time to wait.
\return '0' on success, '1' otherwise */
//uint8_t sendPacketTimeout(	uint8_t dest,
//							uint8_t *payload,
//							uint16_t length,
//							uint32_t wait);

// It sends the packet which payload is a parameter before MAX_TIMEOUT,
// and replies with ACK.
/* \param uint8_t dest : packet destination.
\param char *payload : packet payload.
\return '9'  --> The ACK lost (no data available)
		'8'  --> The ACK lost
		'7'  --> The ACK destination incorrectly received
		'6'  --> The ACK source incorrectly received
		'5'  --> The ACK number incorrectly received
		'4'  --> The ACK length incorrectly received
		'3'  --> N-ACK received
		'2'  --> The ACK has not been received
		'1'  --> not used (reserved)
		'0'  --> The ACK has been received with no errors */
uint8_t sendPacketMAXTimeoutACK(uint8_t dest,
								char *payload);

// It sends the packet which payload is a parameter before MAX_TIMEOUT,
// and replies with ACK.
/* \param uint8_t dest : packet destination.
\param uint8_t payload: packet payload.
\param uint16_t length : payload buffer length.
\return '9'  --> The ACK lost (no data available)
		'8'  --> The ACK lost
		'7'  --> The ACK destination incorrectly received
		'6'  --> The ACK source incorrectly received
		'5'  --> The ACK number incorrectly received
		'4'  --> The ACK length incorrectly received
		'3'  --> N-ACK received
		'2'  --> The ACK has not been received
		'1'  --> not used (reserved)
		'0'  --> The ACK has been received with no errors */
//uint8_t BSP_SX1272_sendPacketMAXTimeoutACK(uint8_t dest,
//								uint8_t *payload,
//								uint16_t length);

// It sends the packet which payload is a parameter before a timeout,
// and replies with ACK.
/*\param uint8_t dest : packet destination.
\param uint8_t payload: packet payload.
\param uint16_t length : payload buffer length.
\return '9'  --> The ACK lost (no data available)
		'8'  --> The ACK lost
		'7'  --> The ACK destination incorrectly received
		'6'  --> The ACK source incorrectly received
		'5'  --> The ACK number incorrectly received
		'4'  --> The ACK length incorrectly received
		'3'  --> N-ACK received
		'2'  --> The ACK has not been received
		'1'  --> not used (reserved)
		'0'  --> The ACK has been received with no errors*/
//uint8_t sendPacketTimeoutACK(	uint8_t dest,
//								uint8_t *payload,
//								uint16_t length);

// It sends the packet which payload is a parameter before 'wait' time,
// and replies with ACK.
/* \param uint8_t dest : packet destination.
\param char *payload : packet payload.
\param uint32_t wait : time to wait to send the packet.
\return '9'  --> The ACK lost (no data available)
		'8'  --> The ACK lost
		'7'  --> The ACK destination incorrectly received
		'6'  --> The ACK source incorrectly received
		'5'  --> The ACK number incorrectly received
		'4'  --> The ACK length incorrectly received
		'3'  --> N-ACK received
		'2'  --> The ACK has not been received
		'1'  --> not used (reserved)
		'0'  --> The ACK has been received with no errors */
uint8_t BSP_SX1272_sendPacketTimeoutACK(	uint8_t dest,
								char *payload,
								uint32_t wait);

// It sends the packet which payload is a parameter before 'wait' time,
// and replies with ACK.
/* \param uint8_t dest : packet destination.
\param uint8_t payload: packet payload.
\param uint16_t length : payload buffer length.
\param uint32_t wait : time to wait to send the packet.
\return '9'  --> The ACK lost (no data available)
		'8'  --> The ACK lost
		'7'  --> The ACK destination incorrectly received
		'6'  --> The ACK source incorrectly received
		'5'  --> The ACK number incorrectly received
		'4'  --> The ACK length incorrectly received
		'3'  --> N-ACK received
		'2'  --> The ACK has not been received
		'1'  --> not used (reserved)
		'0'  --> The ACK has been received with no errors */
/*uint8_t BSP_SX1272_sendPacketTimeoutACK(uint8_t dest,
								uint8_t *payload,
								uint16_t length,
								uint32_t wait);*/

// It sets the destination of a packet.
/* \param uint8_t dest : value to set as destination address.
\return '0' on success, '1' otherwise */
int8_t BSP_SX1272_setDestination(uint8_t dest);

// It sets the waiting time to send a packet.
/*It stores in global '_sendTime' variable the time for each mode.
\return '0' on success, '1' otherwise */
uint8_t BSP_SX1272_setTimeout();

// It gets the theoretical value of the time-on-air of the packet
/* \remarks http://www.semtech.com/images/datasheet/sx1272.pdf
It stores in global '_sendTime' variable the time for each mode.
\return float: time on air depending on module settings and packet length */
float BSP_SX1272_timeOnAir( uint16_t payloadlength );

// It sets the payload of the packet that is going to be sent.
/* \param char *payload : packet payload.
\return '0' on success, '1' otherwise */
uint8_t BSP_SX1272_setPayload(char *payload);

// It sets the payload of the packet that is going to be sent.
/*\param uint8_t payload: packet payload.
\return '0' on success, '1' otherwise */
//uint8_t BSP_SX1272_setPayload(uint8_t *payload);

// It receives and gets an ACK from FIFO, if it arrives before ending
// 'wait' time.
/* *
\param uint32_t wait : time to wait while there is no an ACK received.
\return '8'  --> The ACK lost
		'7'  --> The ACK destination incorrectly received
		'6'  --> The ACK source incorrectly received
		'5'  --> The ACK number incorrectly received
		'4'  --> The ACK length incorrectly received
		'3'  --> N-ACK received
		'2'  --> The ACK has not been received
		'1'  --> not used (reserved)
		'0'  --> The ACK has been received with no errors */
uint8_t BSP_SX1272_getACK(uint32_t wait);

// It sends a packet, waits to receive an ACK and updates the _retries
// value, before ending MAX_TIMEOUT time.
/* \param uint8_t dest : packet destination.
\param uint8_t *payload : packet payload.
\param uint16_t length : payload buffer length.
\return '0' on success, '1' otherwise */
//uint8_t BSP_SX1272_sendPacketMAXTimeoutACKRetries(	uint8_t dest,
//										uint8_t *payload,
//										uint16_t length);

// It sends a packet, waits to receive an ACK and updates the _retries
// value, before ending 'wait' time.
/* \param uint8_t dest : packet destination.
\param char *payload : packet payload.
\param uint32_t wait : time to wait while trying to send the packet.
\return '0' on success, '1' otherwise */
uint8_t BSP_SX1272_sendPacketTimeoutACKRetries(uint8_t dest,
									char *payload,
									uint32_t wait);

// It sends a packet, waits to receive an ACK and updates the _retries
// value, before ending 'wait' time.
/* \param uint8_t dest : packet destination.
\param uint8_t *payload : packet payload.
\param uint16_t length : payload buffer length.
\param uint32_t wait : time to wait while trying to send the packet.
\return '0' on success, '1' otherwise*/
//uint8_t BSP_SX1272_sendPacketTimeoutACKRetries(uint8_t dest,
//									uint8_t *payload,
//									uint16_t length,
//									uint32_t wait);

// It gets the internal temperature of the module.
/* It stores in global '_temp' variable the module temperature.
\return '0' on success, '1' otherwise */
uint8_t BSP_SX1272_getTemp();


/*
Function: It sets the CAD mode to search Channel Activity Detection
Returns: Integer that determines if there has been any error
state = true   --> Channel Activity Detected
state = false  --> Channel Activity NOT Detected*/
uint8_t BSP_SX1272_cadDetected();

#endif /* BSP_INC_SX1272_H_ */
