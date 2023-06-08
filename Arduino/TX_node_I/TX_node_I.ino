 ///////////////////////////////////////////////////////////////////////////////////////////////////////////
 // LoRa Tx node program
 ///////////////////////////////////////////////////////////////////////////////////////////////////////////
 
#include <Wire.h>
#include "sx1272_INSAT.h" // Include the SX1272 and SPI library:
#include <SPI.h>

///////////////////////////////////////////////////////////////
// Configuration du module en mode LoRA ou FSK
///////////////////////////////////////////////////////////////

//Main parameters (to be changed)
#define OutPower POW_14 //min -1, max 14
#define TypeModulation 1 //0: LORA, 1:FSK
#define PeriodTransmission 5000 //en ms

//LORA configuration (to be changed)
#define BW BW_125 //BW_125, BW_250 or BW_500
#define CR CR_5 //CR_5
#define SF SF_12 //SF_7 to SF_12

//FSK configuration (to be changed)
#define BitRate BR1200bauds //BR1200bauds or BR4800bauds
#define Fdev FDA5kHz//FDA5kHz

//Other common parameters (do not change!!)
#define AddrRxNode BROADCAST_ADDR
#define PreambLong 12//12
#define TX_Addr 1 //address of the Tx node
#define freq_centrale CH_868v3
#define MaxNbRetries 3
#define WaitTxMax 5000 //en ms

///////////////////////////////////////////////////////////////
// Déclaration variables globales
///////////////////////////////////////////////////////////////
char LgMsg = 0;
char Message[] = "Salut Francis, comment vas-tu ?";
uint8_t tx_address = TX_Addr;
float waitPeriod = 0; //en ms
float TOA = 0; //en ms
int cp = 0;  //compteur de paquets transmis
int type_modulation=TypeModulation;
uint16_t RegBitRate = BitRate;
uint16_t RegFdev = Fdev;
uint8_t newpower = OutPower;
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
      e = sx1272.setCR(CR);    // CR_5 : CR = 4/5
      e = sx1272.setSF(SF);   // SF = 12
      e = sx1272.setBW(BW);    // BW = 125 KHz
      // Set CRC
      e = sx1272.setCRC_ON();
      // Set the node address
      e = sx1272.setNodeAddress(tx_address);
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
      e = sx1272.setNodeAddress(tx_address);
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
    delay(1000);  
    Serial.println(F("  "));
    Serial.println(F("----------------------------------------"));
    Serial.println(F("Continuous transmission in broadcast mode.")); 
    Serial.println(F("----------------------------------------")); 
  } 

  TOA = sx1272.timeOnAir();
  waitPeriod = PeriodTransmission-TOA;
  if (waitPeriod < 0) {
    Serial.println(F("TOA longer than transmission period !"));
    waitPeriod = 5000;  
  }

  delay(waitPeriod);
  
}

///////////////////////////////////////////////////////////////
// main loop function
///////////////////////////////////////////////////////////////
void loop() {

  uint8_t dest_address = AddrRxNode;
  uint8_t inByte = 0;
  uint8_t flagchange=0;
  
  //////////////////////////////////////////////////////////////////////////////////
  // Check the serial port to know if there is an input character to be treated
  while (Serial.available()) 
  {
    char rxcar=Serial.read();

    if(rxcar=='+') //increase the output power
    {
      if(newpower<15)
      {
        newpower = newpower+1;
        flagchange=1;
      }
    }
    else if(rxcar=='-') //decrease the output power
    {
      if(newpower>0)
      {
        newpower = newpower-1;
        flagchange=1;
      }
    }
    //////////////////////////////////////////////////////////////////////////////////
    // Modify the output power if needed
    if(flagchange)
    {
      //sx1272.OFF(type_modulation);
      //delay(1000);
      //e = sx1272.ON(type_modulation);
      //delay(1000); 
      e = sx1272.setPower(newpower);
      flagchange=0;
    }
  }

  
  //////////////////////////////////////////////////////////////////////////////////
  // Transmit a packet continuously with a pause of "waitPeriod"
  if (ConfigOK == true) 
  {
    
    LgMsg=strlen(Message);
    e = sx1272.sendPacketTimeout(dest_address,Message,WaitTxMax);   
    
    if(type_modulation)
    {
      sx1272.writeRegister(REG_OP_MODE, FSK_STANDBY_MODE); // FSK standby mode to switch off the RF field
    }

    if (e == 0) 
    {
      Serial.print(F("\n Packet number "));
      Serial.print(cp,DEC);
	    Serial.print(F(" ;Rx node address "));
	    Serial.print(dest_address,DEC);
      Serial.print(F(" ; Output power (dBm)"));
      Serial.print(newpower-1,DEC);
      cp++;
    }
    else 
    {
      Serial.println(F("\n Trasmission problem !"));  
    }
    delay(waitPeriod); //delay to send packet every PeriodTransmission
  }
}
