// An example demonstrating the multiceiver capability of the NRF24L01+
// in a star network with one PRX hub and up to six PTX nodes

//This sketch is a modification from a video on the ForceTronics YouTube Channel,

//which code was leveraged from http://maniacbug.github.io/RF24/starping_8pde-example.html

//This sketch is free to the public to use and modify at your own risk


#include <SPI.h> //Call SPI library so you can communicate with the nRF24L01+
#include <nRF24L01.h> //nRF2401 libarary found at https://github.com/tmrh20/RF24/
#include <RF24.h> //nRF2401 libarary found at https://github.com/tmrh20/RF24/
#include "LowPower.h"

const int pinCE = 7; //This pin is used to set the nRF24 to standby (0) or active mode (1)
const int pinCSN = 8; //This pin is used to tell the nRF24 whether the SPI communication is a command or message to send out

RF24 radio(pinCE, pinCSN); // Create your nRF24 object or wireless SPI connection
#define WHICH_NODE 2     // must be a number from 1 - 6 identifying the PTX node

const uint64_t wAddress[] = {0x7878787878LL, 0xB3B4B5B6F1LL, 0xB3B4B5B6CDLL, 0xB3B4B5B6A3LL, 0xB3B4B5B60FLL, 0xB3B4B5B605LL};
const uint64_t PTXpipe = wAddress[ WHICH_NODE - 1 ];   // Pulls the address from the above array for this node's pipe
byte counter = 1; //used to count the packets sent
bool done = false; //used to know when to stop sending packets

/////////MQ-2///////////////////////
#define adc_pin   A1
////////////////////////////////////

void NRF_transmit_value(char*, int);

void setup()  
{
  Serial.begin(9600);   //start serial to communicate process
  pinMode(adc_pin,INPUT);
  radio.begin();            //Start the nRF24 module
  radio.setPALevel(RF24_PA_LOW);  // "short range setting" - increase if you want more range AND have a good power supply
  radio.setRetries(1,5);
  radio.setChannel(108);          // the higher channels tend to be more "open"
  radio.openReadingPipe(0,PTXpipe);  //open reading or receive pipe
  radio.stopListening(); //go into transmit mode
}
void loop()  
{
 radio.powerUp();
 int analogSensor = analogRead(adc_pin);
 Serial.println(analogSensor);
 NRF_transmit_value(analogSensor);
 delay(10000);
 
 
}

void NRF_transmit_value(int MQ2_value)
{
  radio.openWritingPipe(PTXpipe);        //open writing or transmit pipe
  if (!radio.write( &MQ2_value, sizeof(MQ2_value) )){  //if the write fails let the user know over serial monitor
     Serial.println("MQ2_value delivery failed");      
  }
  else { //if the write was successful 
      Serial.print("Success sending MQ2_value!");
      radio.powerDown();
  }  
}
