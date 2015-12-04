#include <RFM69.h>
#include <SPI.h> // the RFM69 library uses SPI

RFM69 radio;

#define FREQUENCY       RF69_915MHZ // or RF69_433MHZ (check your radio)
#define NETWORK_ID      100
#define BROADCASTER_ID  0
#define LCM_ID          1 
#define MCM_ID          2
#define LED             9

typedef struct {
  int           nodeId; //store this nodeId
  unsigned long uptime; //uptime in ms
  float         xPos;   //NEW X POSITION
  float         yPos;   //NEW Y POSITION
  int           active;
} mcmPayload;

typedef struct {
  int           nodeId; //store this nodeId
  unsigned long uptime; //uptime in ms
  int           active; // BINARY VALUE DETERMINING WHETHER LED MUST BE ON OR OFF
  int           red;
  int           green;
  int           blue;
} lcmPayload;

lcmPayload lightData; //DECLARE THE LIGHT DATA PAYLOAD
mcmPayload cncData;   //DECLARE THE MOTOR DATA PAYLOAD

//---------------------------------- S E T U P -------------------------------

void setup() {
  Serial.begin(9600);
  
  // setup the radio
  radio.initialize(FREQUENCY, BROADCASTER_ID, NETWORK_ID);
  
  Serial.println("RADIO INITIALIZED\n\n");
  Serial.println("Enter '1' to send red");
  Serial.println("Enter '2' to send green");
  Serial.println("Enter '3' to send blue");
}

//---------------------------------- L O O P --------------------------------

void loop() {
    
  if(Serial.available()>0) {
      
    char input = Serial.read();

    //FIRST CHECK FOR RECEIVED PACKETS
    if (radio.receiveDone())
    {
      Serial.print('[');Serial.print(radio.SENDERID, DEC);Serial.print("] ");
      for (byte i = 0; i < radio.DATALEN; i++)
        Serial.print((char)radio.DATA[i]);
      Serial.print("   [RX_RSSI:");Serial.print(radio.readRSSI());Serial.print("]");
  
      if (radio.ACKRequested())
      {
        radio.sendACK(); //SEND ACKNOWLEDGMENT TO THE RADIO THAT SENT THE PACKET
        Serial.print(" - ACK sent");
        Serial.println();
        delay(10);
      }

    }

    //SEND THE DATA TO THE LIGHT CONTROL MODULE
    //sendToLCM(input);
    if (input == '5') sendToMCM();

    }
}


//--------------------------- S E N D  T O  L C M --------------------------

void sendToLCM(char _input)
{
  
  int retries = 5;

  if (_input == '1')
  {
    lightData.nodeId = BROADCASTER_ID;
    lightData.uptime = millis();
    lightData.active = 1;
    lightData.red = 255;
    lightData.green = 0;
    lightData.blue = 0;
    if (radio.sendWithRetry(LCM_ID, (const void*)(&lightData), sizeof(lightData)), retries)
      Serial.print(" ok!");
    else Serial.print(" nothing...");
    Serial.println("sending red");
  }

      
  if (_input == '2')
  {
    lightData.nodeId = BROADCASTER_ID;
    lightData.uptime = millis();
    lightData.active = 1;
    lightData.red = 0;
    lightData.green = 255;
    lightData.blue = 0;
    if (radio.sendWithRetry(LCM_ID, (const void*)(&lightData), sizeof(lightData)),retries)
      Serial.print(" ok!");
    else Serial.print(" nothing...");
    Serial.println("sending green");
    
  }
      
  if (_input == '3')
  {
    lightData.nodeId = BROADCASTER_ID;
    lightData.uptime = millis();
    lightData.active = 1;
    lightData.red = 0;
    lightData.green = 0;
    lightData.blue = 255;
    if (radio.sendWithRetry(LCM_ID, (const void*)(&lightData), sizeof(lightData)),retries)
      Serial.print(" ok!");
    else Serial.print(" nothing...");
        Serial.println("sending blue");
  }

  for (int i = 0; i < 50; i++) Blink(LED, 20);
}

//--------------------------- S E N D  T O  M C M --------------------------

void sendToMCM()
{
  Serial.println("Sending to MCM");
  
  cncData.nodeId = BROADCASTER_ID;
  cncData.uptime = millis();
  cncData.xPos = 1;
  cncData.yPos = 1;
  cncData.active = 1;
  
int retries = 3; 

    if (radio.sendWithRetry(MCM_ID, (const void*)(&cncData), sizeof(cncData)),retries)
      Serial.print("Sending Motor Data");
    else Serial.print(" nothing...");

  for (int i = 0; i < 50; i++) Blink(LED, 20);
}
//----------------------------------- B L I N K ----------------------------

void Blink(byte PIN, int DELAY_MS)
{
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN,HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN,LOW);
}
