
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <SPI.h> 
#include <nRF24L01.h> 
#include <RF24.h> 


void listenToNrF();

ESP8266WiFiMulti WiFiMulti;
RF24 radio(D4, D8); // Declare object from nRF24 library (Create your wireless SPI)
const uint64_t rAddress[] = {0x7878787878LL, 0xB3B4B5B6F1LL, 0xB3B4B5B6CDLL, 0xB3B4B5B6A3LL, 0xB3B4B5B60FLL, 0xB3B4B5B605LL };


typedef struct
{
  int humidity;
  char temperature;
}DHT11_Struct;      

DHT11_Struct dht;   // this is not used in this code because I only care about temperature.

////////////sensor's data///////////////////
int MQ_2;
int MQ_135;
int DHT_11_temp;
int motor_pos;
//////////////////////////////////////////
void setup()   
{

  ///////////////CONNECT TO WIFI///////////////////////
  Serial.begin(115200);
  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("s50-2.4", "100%Kakao?");
  /////////////CONNECT TO WIFI////////////////////////
  
  radio.begin();  //Start the nRF24 module

  radio.setPALevel(RF24_PA_LOW);  // "short range setting" - increase if you want more range AND have a good power supply
  radio.setChannel(108);          // the higher channels tend to be more "open"

  // Open up to six pipes for PRX to receive data
  radio.openReadingPipe(0,rAddress[0]);
  radio.openReadingPipe(1,rAddress[1]);
  radio.openReadingPipe(2,rAddress[2]);
  radio.openReadingPipe(3,rAddress[3]);
  radio.openReadingPipe(4,rAddress[4]);
  radio.openReadingPipe(5,rAddress[5]);
  
  radio.startListening();                 // Start listening for messages
}
int a = 25;
void loop()  
{   
       listenToNrF();
       delay(200);
       
}

void listenToNrF()
{
  byte pipeNum = 0; //variable to hold which reading pipe sent data
   
    while(radio.available(&pipeNum)){ //Check if data is received
      if(pipeNum+1 == 2){
        radio.read(&MQ_2, sizeof(MQ_2)); 
        Serial.print("Received MQ_2 value from transmitter: "); 
        Serial.println(pipeNum + 1); //print which pipe or transmitter this is from
        Serial.print("MQ_2 value= ");
        Serial.print(MQ_2);
        postRequest("mq2",MQ_2);
        Serial.println();
      }
     else if(pipeNum+1 == 3){
        radio.read( &MQ_135, sizeof(MQ_135) ); //read one byte of data and store it in gotByte variable
        Serial.print("Received MQ_135 value from transmitter: "); 
        Serial.println(pipeNum + 1); //print which pipe or transmitter this is from
        Serial.print("MQ_135= ");
        Serial.print(MQ_135);
        postRequest("mq135",MQ_135);
        Serial.println();
     }
     else if(pipeNum+1 == 4){
        radio.read( &DHT_11_temp, sizeof(DHT_11_temp) ); //read one byte of data and store it in gotByte variable
        Serial.print("Received DHT_struct from transmitter: ");
        Serial.println(pipeNum + 1);
        Serial.print("Temp = ");
        Serial.println(DHT_11_temp);
        postRequest("temperature",DHT_11_temp);
        Serial.println();
     }
     else if(pipeNum+1 == 5){
        radio.read( &motor_pos, sizeof(motor_pos) ); //read one byte of data and store it in gotByte variable
        Serial.print("Received Motor_pos from transmitter: ");
        Serial.println(pipeNum + 1);
        Serial.print("Position in angle = ");
        Serial.println(motor_pos);
        postRequest("servo",motor_pos);
        Serial.println();
     }
    }

   delay(200); 
}


void postRequest(char* type, int val)
{
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    WiFiClient client;

    HTTPClient http;

    Serial.print("[HTTP] begin...\n");
    if (http.begin(client, "http://192.168.0.105/HomeAutomation/api/database")) {  // HTTP

      StaticJsonBuffer<300> JSONbuffer;   //Declaring static JSON buffer
      JsonObject& JSONencoder = JSONbuffer.createObject(); 
 
      JSONencoder["sensorType"] = type;
      JSONencoder["value"] = val;
      
      //JsonArray& values = JSONencoder.createNestedArray("values"); //JSON array
      //values.add(20); //Add value to array
      //values.add(21); //Add value to array
      //values.add(23); //Add value to array
 
      //JsonArray& timestamps = JSONencoder.createNestedArray("timestamps"); //JSON array
      //timestamps.add("10:10"); //Add value to array
      //timestamps.add("10:20"); //Add value to array
      //timestamps.add("10:30"); //Add value to array
 
      char JSONmessageBuffer[300];
      JSONencoder.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
      Serial.println(JSONmessageBuffer);

      Serial.print("[HTTP] POST...\n");
      http.addHeader("Content-Type", "application/json");
      int httpCode = http.POST(JSONmessageBuffer);

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] POST... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = http.getString();
          Serial.println(payload);
        }
      } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }

      http.end();
    } else {
      Serial.printf("[HTTP} Unable to connect\n");
    }
  }
}

byte daNumber = 1;
bool sendCorrectNumber(byte xMitter) {
    bool worked;                                    //variable to track if write was successful
    radio.stopListening();                          //Stop listening, start receiving data.
    radio.openWritingPipe(rAddress[xMitter]);       //Open writing pipe to the nRF24 that guessed the right number
                                                    // note that this is the same pipe address that was just used for receiving
    if(!radio.write(&daNumber, 1))  worked = false; //write the correct number to the nRF24 module, and check that it was received
    else worked = true;                             //it was received
    radio.startListening();                         //Switch back to a receiver
    return worked;                                   //return whether write was successful
}
