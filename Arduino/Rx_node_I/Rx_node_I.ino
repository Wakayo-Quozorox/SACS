 ///////////////////////////////////////////////////////////////////////////////////////////////////////////
 // LoRa Rx node program
 ///////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Wire.h>
#include "sx1272_INSAT.h"
#include <SPI.h>

///////////////////////////////////////////////////////////////
// Configuration du module en mode LoRA ou FSK
///////////////////////////////////////////////////////////////

//Main parameters (to be changed)
#define OutPower POW_14 //min -1, max 14
#define TypeModulation 1 //0: LORA, 1:FSK

//LORA configuration (to be changed)
#define BW BW_125 //BW_125, BW_250 or BW_500
#define CR CR_5 //CR_5
#define SF SF_12 //SF_7 to SF_12

//FSK configuration (to be changed)
#define BitRate BR1200bauds //BR1200bauds or BR4800bauds
#define Fdev FDA5kHz//FDA5kHz

//Other common parameters (do not change!!)
#define PreambLong 12//12
#define RX_Addr 8 //address of the Rx node
#define freq_centrale CH_868v3
#define MaxNbRetries 3
#define WaitRxMax 10000 //en ms

// SACS constants
#define preambSACS 2 // Number of bytes before data (header + sid + ack + size)
#define endingSACS 1 // Number of bytes after data (ending pattern)

///////////////////////////////////////////////////////////////
// Déclaration variables globales
///////////////////////////////////////////////////////////////
uint16_t  LgMsg = 0;
uint8_t Message[] = {0b10101010,0b00010001,0b01100110,0b01110010,0b01100001,0b01101101,0b01100010,0b01101111,0b01101001,0b01110011,0b01100101,0b00000000};
uint8_t rx_address = RX_Addr;
int type_modulation=TypeModulation;
uint16_t RegBitRate = BitRate;
uint16_t RegFdev = Fdev;
int cpok=0;
int cpall=0;
// status variables
int8_t e;
boolean ConfigOK = true;

///////////////////////////////////////////////////////////////
// Setup function
///////////////////////////////////////////////////////////////
void setup() { 

  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  // Print a start message
  Serial.println(F("SX1272 module configuration in Arduino"));
  // Power ON the module
  e = sx1272.ON(type_modulation);
  if (e == 0)
  {
    Serial.println(F("SX1272 Module on"));
  }
  else
  {
    Serial.println(F("Problem of activation of SX1272 Module !"));
    ConfigOK = false;
  }
  // Select frequency channel
  e = sx1272.setChannel(freq_centrale);
  Serial.print(F("Frequency channel "));
  Serial.print(freq_centrale,HEX);
  if (e == 0)
  {
    Serial.println(F(" has been successfully set."));
  }
  else
  {
    Serial.println(F(" has not been set !"));
    ConfigOK = false;
  }
  // Select output power
  e = sx1272.setPower(OutPower);
  Serial.print(F("Output power "));
  Serial.print(OutPower,HEX);
  if (e == 0)
  {
    Serial.println(F(" has been successfully set."));
  }
  else
  {
    Serial.println(F(" has not been set !"));
    ConfigOK = false;
  }
  if (ConfigOK == true) 
  {
    //////////////////////////////////////////////////////////////////////
    //config supplémentaire mode LORA
    //////////////////////////////////////////////////////////////////////
    if(type_modulation==0)
    {
      // Set header
      e = sx1272.setHeaderON();
      // Set transmission mode
      e = sx1272.setCR(CR);    // CR = 4/5
      e = sx1272.setSF(SF);   // SF = 12
      e = sx1272.setBW(BW);    // BW = 125 KHz
      // Set CRC
      e = sx1272.setCRC_ON();
      // Set the node address
      e = sx1272.setNodeAddress(rx_address);
      // Set the length of preamble
      e = sx1272.setPreambleLength(PreambLong);
      // Set the number of transmission retries
      sx1272._maxRetries = MaxNbRetries;
    }
    //////////////////////////////////////////////////////////////////////
    //config supplémentaire mode FSK
    //////////////////////////////////////////////////////////////////////
    else
    {
      // Set CRC
      e = sx1272.setCRC_ON();
      // Set the node address
      e = sx1272.setNodeAddress(rx_address);
      // Set the length of preamble
      e = sx1272.setPreambleLength(PreambLong);
      // Set the number of transmission retries
      sx1272._maxRetries = MaxNbRetries;

      //Serial.print("REG_SYNC_CONFIG: ");
      sx1272.readRegister(REG_SYNC_CONFIG);
      sx1272.readRegister(REG_SYNC_VALUE1);
      sx1272.readRegister(REG_SYNC_VALUE2);
      sx1272.readRegister(REG_SYNC_VALUE3);
      sx1272.readRegister(REG_SYNC_VALUE4);


      sx1272.writeRegister(REG_SYNC_VALUE1,0x05);
      sx1272.writeRegister(REG_SYNC_VALUE2,0x05);
      sx1272.writeRegister(REG_SYNC_VALUE3,0x05);
      sx1272.writeRegister(REG_SYNC_VALUE4,0x05);

      
      //Set the frequency deviation an bit rate parameters   
      sx1272.writeRegister(REG_FDEV_MSB,(RegFdev>>8)&0x00FF);// FDA = Fstep*FDEV = 61Hz*Fdev : ex: 0x7FF*61 = 125kHz ex2: 0X52*61=5kHz
      sx1272.writeRegister(REG_FDEV_LSB,RegFdev&0x00FF);//...
      sx1272.writeRegister(REG_BITRATE_MSB,(RegBitRate>>8)&0x00FF);//FXOSC=32Mz, BR = FXOSC/(Bitrate + BitrateFrac/16), ex: FXOSC/0x682B = 1200 bauds, ex2: FXOSC/0x200=62.5 kbauds
      sx1272.writeRegister(REG_BITRATE_LSB,RegBitRate&0x00FF);//...
      
    }
    Serial.println(F("SX1272 successfully configured !"));
  }
  else
  {
    Serial.println(F("SX1272 initialization failed !")); 
  }

  if (ConfigOK == true) {
    //affichage entête
    //statut (correct = 1 or bad = 0 or non received = 2) 
    Serial.println(F("\n "));
    Serial.println(F("Module ready for reception ! "));
    Serial.println(F("Packet status ; Packet number ; Received Lg ; TRAME ; DATA ; RSSI packet (dBm) ; source address; PER (%); BER (%)"));
    Serial.println(F("\n "));
  } 
}

///////////////////////////////////////////////////////////////
// Main loop function
///////////////////////////////////////////////////////////////
void loop()
{
  char StatusRXMessage;
  uint8_t inByte = 0;

  //////////////////////////////////////////////////////////////////////////////////
  // Check the serial port to know if there is an input character to be treated
  if (Serial.available()) 
  {
    inByte = Serial.read()-'0';
    Serial.println(F("reset PER"));  
    cpall=0;
    cpok=0;
  }
  
  //////////////////////////////////////////////////////////////////////////////////
  // Receive packets continuously
  if (ConfigOK == true) 
  {
    e = sx1272.receivePacketTimeout(WaitRxMax);
    cpall=cpall+1;//increment the total number of packet supposed to be received
    //paquet reçu, correct ou non
    if (e == 0) 
    {
      StatusRXMessage = '0';
      if (sx1272._reception == CORRECT_PACKET) 
      {
       // Check if the received packet is correct
       // The length and the content of the packet is checked
       // if it is valid, the cpok counter is incremented 
       LgMsg=sizeof(Message) / sizeof(Message[0]);
       if(sx1272.packet_received.length>=LgMsg)//check the length
       {
        if(memcmp(Message,sx1272.packet_received.data,LgMsg)==0)//check the content
        {
          cpok=cpok+1;
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
    // Calculation of the PER and the BER
    float PER = 100-(cpok*100)/cpall;
    PER = PER/100;
    double exponant = 1.0/(LgMsg*8.0);
    float BER = 1-1*pow((1-PER),exponant);

    //////////////////////////////////////////////////////////////////////////////////
    // Plot receive packets in the serial monitor
    Serial.print(StatusRXMessage);
    Serial.print(F(" ; "));
    Serial.print(sx1272.packet_received.packnum,DEC);
    Serial.print(F(" ; "));
    Serial.print(sx1272.packet_received.length,DEC);
    Serial.print(F(" ; "));
    for (uint8_t i = 0; i < sx1272.packet_received.length-OFFSET_PAYLOADLENGTH; i++)
    {
      // Serial.print(sx1272.packet_received.data[i],HEX);
      Serial.print(sx1272.packet_received.data[i],BIN);
      Serial.print(" ");
    }
    Serial.print(F(" ; "));
    for (uint8_t i = preambSACS; i < sx1272.packet_received.length-OFFSET_PAYLOADLENGTH-endingSACS; i++)
    {
      // Serial.print(sx1272.packet_received.data[i],HEX);
      Serial.print(char(sx1272.packet_received.data[i]));
    }
    ///////////////////////////////////////////////////////////////////////////////////
    // Plot RSSI
    Serial.print(F(" ; "));
    // LORA mode
    if(TypeModulation == 0)
    {
      e = sx1272.getRSSIpacket();
      Serial.print(sx1272._RSSIpacket, DEC);
    }
    // FSK mode
    else
    {
      //e = sx1272.getRSSI() done during RX, no packet RSSI available in FSK mode;
      Serial.print(sx1272._RSSI, DEC);
    }

    ///////////////////////////////////////////////////////////////////////////////////
    // Plot PER and BER
    Serial.print(F(" ; "));
    Serial.print(sx1272.packet_received.src,DEC);
    Serial.print(F(" ; "));
    Serial.print(PER*100);
    Serial.print(F(" ; "));
    Serial.println(BER*100);
  }
  delay(1000);
}
