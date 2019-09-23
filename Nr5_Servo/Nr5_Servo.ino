#include <Servo.h>




#include <SPI.h> //Call SPI library so you can communicate with the nRF24L01+
#include <nRF24L01.h> //nRF2401 libarary found at https://github.com/tmrh20/RF24/
#include <RF24.h> //nRF2401 libarary found at https://github.com/tmrh20/RF24/
#include "LowPower.h"
#include <dht.h>
const int pinCE = 7; //This pin is used to set the nRF24 to standby (0) or active mode (1)
const int pinCSN = 8; //This pin is used to tell the nRF24 whether the SPI communication is a command or message to send out

RF24 radio(pinCE, pinCSN); // Create your nRF24 object or wireless SPI connection
#define WHICH_NODE 5    // must be a number from 1 - 6 identifying the PTX node

const uint64_t wAddress[] = {0x7878787878LL, 0xB3B4B5B6F1LL, 0xB3B4B5B6CDLL, 0xB3B4B5B6A3LL, 0xB3B4B5B60FLL, 0xB3B4B5B605LL};
const uint64_t PTXpipe = wAddress[ WHICH_NODE - 1 ];   // Pulls the address from the above array for this node's pipe
byte counter = 1; //used to count the packets sent
bool done = false; //used to know when to stop sending packets

/////////Servo///////////////////////
int servoPin = 9;
int angle;
int old_angle;
Servo Servo1;
////////////////////////////////////

void NRF_transmit_value(char*, int);

void setup()  
{
  Serial.begin(115200);   //start serial to communicate process
  Servo1.attach(servoPin); 
  pinMode(A1,INPUT);
  radio.begin();            //Start the nRF24 module
  radio.setPALevel(RF24_PA_LOW);  // "short range setting" - increase if you want more range AND have a good power supply
  radio.setRetries(1,5);
  radio.setChannel(108);          // the higher channels tend to be more "open"
  radio.openReadingPipe(0,PTXpipe);  //open reading or receive pipe
  radio.stopListening(); //go into transmit mode
}
void loop()  
{
  
  int adc = analogRead(A1);
  angle = map(adc,0,1023,0,180);
  Serial.print("Angle=");
  Serial.println(angle);
  if( (angle > old_angle+5) || (angle < old_angle-5) ){
    old_angle = angle;
    Serial.print("old_angle=");
    Serial.println(old_angle);
    delay(3000);
    adc = analogRead(A1);
    angle = map(adc,0,1023,0,180);
    NRF_transmit_value(angle);
  }
  Servo1.write(angle);
  
  delay(1000);
}

void NRF_transmit_value(int angleT)
{
  radio.powerUp();
  delay(5);
  radio.openWritingPipe(PTXpipe);        //open writing or transmit pipe
  if (!radio.write( &angleT, sizeof(angleT) )){  //if the write fails let the user know over serial monitor
     Serial.println("Delivery failed");      
  }
  else { //if the write was successful 
      Serial.println("Success sending Angle!");
      radio.powerDown();
  }  
}
