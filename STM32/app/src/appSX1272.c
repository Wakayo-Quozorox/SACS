/*
 * appSX1272.c
 *
 *  Created on: 25 ao�t 2020
 *      Author: Arnaud
 */

#include "main.h"
#include "appSX1272.h"
#include "SX1272.h"
#include "comSX1272.h"
#include "string.h"
#include "delay.h"

extern SX1272status currentstate;

///////////////////////////////////////////////////////////////
// D�claration variables globales
///////////////////////////////////////////////////////////////
static uint16_t LgMsg = 0;
//static char Message[] = "Salut Francis, comment vas-tu prout ?";

static uint8_t Message[] = {0b10101010,0b00010001,0b01100110,0b01110010,0b01100001,0b01101101,0b01100010,0b01101111,0b01101001,0b01110011,0b01100101,0b00000000};


static float waitPeriod = 0; //en ms
static int cp = 0;  //compteur de paquets transmis
static int type_modulation=TypeModulation;
static uint16_t RegBitRate = BitRate;
static uint16_t RegFdev = Fdev;

// status variables
static int8_t e;
static uint8_t ConfigOK = 1;

///////////////////////////////////////////////////////////////
// Setup function
///////////////////////////////////////////////////////////////
void APP_SX1272_setup()
{
  // Power ON the module
  e = BSP_SX1272_ON(type_modulation);
  if (e == 0)
  {
    my_printf("SX1272 Module on\r\n");
  }
  else
  {
    //my_printfln(F("Problem of activation of SX1272 Module !"));
    ConfigOK = 0;
  }
  // Select frequency channel
  e = BSP_SX1272_setChannel(freq_centrale);
  my_printf("Frequency channel ");
  my_printf("%d",freq_centrale);
  if (e == 0)
  {
    my_printf(" has been successfully set.\r\n");
  }
  else
  {
    my_printf(" has not been set !\r\n");
    ConfigOK = 0;
  }
  // Select output power
  e = BSP_SX1272_setPower(OutPower);
  my_printf("Output power ");
  my_printf("%d",OutPower);
  if (e == 0)
  {
    my_printf(" has been successfully set.\r\n");
  }
  else
  {
    my_printf(" has not been set !\r\n");
    ConfigOK = 0;
  }

  if (ConfigOK == 1)
  {
	//////////////////////////////////////////////////////////////////////
  //config suppl�mentaire mode LORA
	//////////////////////////////////////////////////////////////////////
    if(type_modulation==0)
    {
      // Set header
      e = BSP_SX1272_setHeaderON();
      // Set transmission mode
      e = BSP_SX1272_setCR(paramCR);    // CR_5 : CR = 4/5
      e = BSP_SX1272_setSF(paramSF);   // SF = 12
      e = BSP_SX1272_setBW(paramBW);    // BW = 125 KHz
      // Set CRC
      e = BSP_SX1272_setCRC_ON();
      // Set the node address
      e = BSP_SX1272_setNodeAddress(RX_Addr);
      // Set the length of preamble
      e = BSP_SX1272_setPreambleLength(PreambLong);
      // Set the number of transmission retries
      currentstate._maxRetries = MaxNbRetries;
    }
	//////////////////////////////////////////////////////////////////////
	//config suppl�mentaire mode FSK
	//////////////////////////////////////////////////////////////////////
    else
    {
      // Set CRC
      e = BSP_SX1272_setCRC_ON();
      // Set the node address
      e = BSP_SX1272_setNodeAddress(RX_Addr);
      // Set the length of preamble
      e = BSP_SX1272_setPreambleLength(PreambLong);
      // Set the number of transmission retries
      currentstate._maxRetries = MaxNbRetries;

      BSP_SX1272_Write(REG_SYNC_VALUE1,0x05);
      BSP_SX1272_Write(REG_SYNC_VALUE2,0x05);
      BSP_SX1272_Write(REG_SYNC_VALUE3,0x05);
      BSP_SX1272_Write(REG_SYNC_VALUE4,0x05);

	  //Set the frequency deviation an bit rate parameters
	  BSP_SX1272_Write(REG_FDEV_MSB,(RegFdev>>8)&0x00FF);// FDA = Fstep*FDEV = 61Hz*Fdev : ex: 0x7FF*61 = 125kHz ex2: 0X52*61=5kHz
	  BSP_SX1272_Write(REG_FDEV_LSB,RegFdev&0x00FF);//...
      BSP_SX1272_Write(REG_BITRATE_MSB,(RegBitRate>>8)&0x00FF);//FXOSC=32Mz, BR = FXOSC/(Bitrate + BitrateFrac/16), ex: FXOSC/0x682B = 1200 bauds, ex2: FXOSC/0x200=62.5 kbauds
      BSP_SX1272_Write(REG_BITRATE_LSB,RegBitRate&0x00FF);//...

    }
    my_printf("SX1272 successfully configured !\r\n");
  }
  else
  {
    my_printf("SX1272 initialization failed !\r\n");
  }

  waitPeriod = PeriodTransmission;

  BSP_DELAY_ms(1000);
}

void APP_SX1272_runTransmit(adrr,msg)
{
  uint8_t dest_address = adrr;

  //////////////////////////////////////////////////////////////////////////////////
  // Transmit a packet continuously with a pause of "waitPeriod"
  if (ConfigOK == 1)
  {

    LgMsg=sizeof(Message) / sizeof(Message[0]);
    e = BSP_SX1272_sendPacketTimeout(dest_address,Message,LgMsg,WaitTxMax);

    if(type_modulation)
    {
      BSP_SX1272_Write(REG_OP_MODE, FSK_STANDBY_MODE); // FSK standby mode to switch off the RF field
    }

    if (e == 0)
    {
      my_printf("\n Packet number ");
      my_printf("%d",cp);
	  my_printf(" ;Rx node address ");
	  my_printf("%d\r\n",dest_address);
      cp++;
    }
    else
    {
      my_printf("\n Trasmission problem !\r\n");
    }
    BSP_DELAY_ms(waitPeriod); //delay to send packet every PeriodTransmission
  }
}

void APP_SX1272_runReceive()
{
  char StatusRXMessage='0';
  uint8_t a[]={0};
  //////////////////////////////////////////////////////////////////////////////////
  // Receive packets continuously
  if (ConfigOK == 1)
  {
	    //affichage ent�te
	    //statut (correct = 1 or bad = 0 or non received = 2)
	  my_printf("\n \r\n");
	  my_printf("Packet status ; Packet number ; Received Lg ; Received data ; RSSI packet (dBm) ; source address; PER (%); BER (%)\r\n");
	  my_printf("\n \r\n");

    e = BSP_SX1272_receivePacketTimeout(WaitRxMax);
    //paquet re�u, correct ou non
    if (e == 0)
    {
      StatusRXMessage = '0';
      if (currentstate._reception == CORRECT_PACKET)
      {
       // Check if the received packet is correct
       // The length and the content of the packet is checked
       // if it is valid, the cpok counter is incremented
       LgMsg=sizeof(Message) / sizeof(Message[0]);
       if(currentstate.packet_received.length>=LgMsg)//check the length
       {
        if(memcmp(Message,currentstate.packet_received.data,LgMsg)==0)//check the content
        {
          StatusRXMessage = '1';
        }
       }
      }
    }
    // RX Timeout !! No packet received
    else
    {
      StatusRXMessage = '2';
    }

    //////////////////////////////////////////////////////////////////////////////////
    // Plot receive packets in the serial monitor
    my_printf("%d",StatusRXMessage);
    my_printf(" ; ");
    my_printf("%d",currentstate.packet_received.packnum);
    my_printf(" ; ");
    my_printf("%d",currentstate.packet_received.length);
    my_printf(" ; ");
    for (uint8_t i =0; i < currentstate.packet_received.length-OFFSET_PAYLOADLENGTH; i++)
    {
      a[i]=currentstate.packet_received.data[i];
      my_printf("%c",currentstate.packet_received.data[i]);
      my_printf(" ");
    }
    ///////////////////////////////////////////////////////////////////////////////////
    // Plot RSSI
    my_printf(" ; ");
    // LORA mode
    if(TypeModulation == 0)
    {
      e = BSP_SX1272_getRSSIpacket();
      my_printf("%d\r\n",currentstate._RSSIpacket);
    }
    // FSK mode
    else
    {
      //e = BSP_SX1272_getRSSI() done during RX, no packet RSSI available in FSK mode;
      //my_printf("%d\r\n",currentstate._RSSI);
    }
  }
  BSP_DELAY_ms(1000);
  return a,StatusRXMessage;
}
