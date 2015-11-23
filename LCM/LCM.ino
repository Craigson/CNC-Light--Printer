#include <Adafruit_NeoPixel.h>
#include <RFM69.h>
#include <SPI.h>


#define NEO         7 //LED IS ON PIN 7
#define FREQUENCY   RF69_915MHZ // 
#define NETWORK_ID  100 // radios must share the same network (0-255)
#define NODE_ID     1 // THIS NODES ID - radios should be given unique ID's (0-254, 255 = BROADCAST)
#define LED         9

Adafruit_NeoPixel pixel = Adafruit_NeoPixel(1, NEO, NEO_GRB + NEO_KHZ800);

RFM69 radio;

typedef struct {
  int           nodeId; //store this nodeId
  unsigned long uptime; //uptime in ms
  int           active; // BINARY VALUE DETERMINING WHETHER LED MUST BE ON OR OFF
  int           red;
  int           green;
  int           blue;
} lcmPayload;

lcmPayload lightData; //DECLARE THE LIGHT DATA PAYLOAD

bool ledActive = false;


//---------------------------------- S E T U P -------------------------------

void setup() {
 
  Serial.begin(9600);
  
  pixel.begin();

  //TURN OFF THE LED TO START
  pixel.setPixelColor(0, pixel.Color(0,0,0));
  
  //SETUP THE RADIO
  radio.initialize(FREQUENCY, NODE_ID, NETWORK_ID);
  
  Serial.println("RADIO INITIALIZED");
  Serial.println("Listening...");
  
}

byte ackCount=0;

//---------------------------------- L O O P --------------------------------

void loop() {
  
  if (radio.receiveDone())
  {
    Serial.print('[');Serial.print(radio.SENDERID, DEC);Serial.print("] ");
    Serial.print(" [RX_RSSI:");Serial.print(radio.readRSSI());Serial.print("]");

    if (radio.DATALEN != sizeof(lcmPayload))
      Serial.print("Invalid payload received, not matching Payload struct!");
    else
    {
      lightData = *(lcmPayload*)radio.DATA; //assume radio.DATA actually contains our struct and not something else
      Serial.print(" nodeId=");
      Serial.print(lightData.nodeId);
      Serial.print(" uptime=");
      Serial.print(lightData.uptime);
      if (lightData.active == 1) ledActive = true;
      {
        Serial.println("LED IS ACTIVE");
        pixel.setPixelColor(0, pixel.Color(lightData.red,lightData.green,lightData.blue));
      }
    }
    
    if (radio.ACKRequested())
    {
      byte theNodeID = radio.SENDERID;
      radio.sendACK();
      Serial.print(" - ACK sent.");

      // When a node requests an ACK, respond to the ACK
      // and also send a packet requesting an ACK (every 3rd one only)
      // This way both TX/RX NODE functions are tested on 1 end at the GATEWAY
      if (ackCount++%3==0)
      {
        Serial.print(" Pinging node ");
        Serial.print(theNodeID);
        Serial.print(" - ACK...");
        delay(3); //need this when sending right after reception .. ?
        if (radio.sendWithRetry(theNodeID, "ACK TEST", 8, 0))  // 0 = only 1 attempt, no retries
          Serial.print("ok!");
        else Serial.print("nothing");
      }
    }
    Blink(LED, 5);
  }
  if (ledActive) pixel.show();
}


//----------------------------------- B L I N K ----------------------------

void Blink(byte PIN, int DELAY_MS)
{
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN,HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN,LOW);
}
