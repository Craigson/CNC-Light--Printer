/*-------------------------------------------------------------------
 * THIS VERSION USES ONLY 1 cnc AS A TEST FOR RECEIVING DATA VIA WIRELESS 
 * 
 * 
 * 
 * 
 * LOOK INTO USING ACCELSTEPPER LIBRARY
 -------------------------------------------------------------------*/

#include <RFM69.h>

//DEFINES FOR RADIO
#define FREQUENCY   RF69_915MHZ // 
#define NETWORK_ID  100 // radios must share the same network (0-255)
#define NODE_ID     2 // THIS NODES ID - radios should be given unique ID's (0-254, 255 = BROADCAST)

#define LED   9

//DEFINES FOR STEPPERS
#define STEP  2
#define DIR   3
#define MS1   4
#define MS2   5
#define EN    6

RFM69 radio;

typedef struct {
  int           nodeId; //store this nodeId
  unsigned long uptime; //uptime in ms
  float         xPos;   //NEW X POSITION
  float         yPos;   //NEW Y POSITION
  int           active;
} mcmPayload;

typedef struct {
  float   x;
  float   y;
} vec2;

mcmPayload cncData; //DECLARE THE LIGHT DATA PAYLOAD

vec2 currentPosition;

//---------------------------------- S E T U P -------------------------------

void setup() {
 
  Serial.begin(9600);
  
  //SETUP THE RADIO
  radio.initialize(FREQUENCY, NODE_ID, NETWORK_ID);

  pinMode(STEP, OUTPUT);
  pinMode(DIR, OUTPUT);
  pinMode(MS1, OUTPUT);
  pinMode(MS2, OUTPUT);
  pinMode(EN, OUTPUT);
  //resetEDPins(); //Set step, direction, microstep and enable pins to default
  
  Serial.println("RADIO INITIALIZED");
  Serial.println("Listening...");

    digitalWrite(MS1, LOW); //WRITE BOTH LOW FOR FULL STEP
  digitalWrite(MS2, LOW); //WRITE BOTH LOW FOR FULL STEP
    digitalWrite(DIR, LOW); //Pull direction pin low to move "forward"
  
}

byte ackCount=0;

//---------------------------------- L O O P --------------------------------

void loop() {
  if (Serial.available() > 0)
  {
    char input = Serial.read();

    if (input == '1') movecnc();
  }
  
  if (radio.receiveDone())
  {
    Serial.print('[');Serial.print(radio.SENDERID, DEC);Serial.print("] ");
    Serial.print(" [RX_RSSI:");Serial.print(radio.readRSSI());Serial.print("]");

    if (radio.DATALEN != sizeof(mcmPayload))
      Serial.print("Invalid payload received, not matching Payload struct!");
    else
    {
      cncData = *(mcmPayload*)radio.DATA; //assume radio.DATA actually contains our struct and not something else
      Serial.print(" nodeId=");
      Serial.print(cncData.nodeId);
      Serial.print(" uptime=");
      Serial.print(cncData.uptime);
      if (cncData.active == 1) movecnc();
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
}

// ------------------------------- M O V E  M O T O R --------------------
void movecnc()
{
  digitalWrite(EN, LOW); //Pull enable pin low to allow cnc control
  
  Serial.println("Moving forward at default step mode.");


  for (int x= 1; x<10; x++)  //Loop the forward stepping enough times for motion to be visible
  {
    digitalWrite(STEP,HIGH); //Trigger one step forward
    delay(1000);
    digitalWrite(STEP,LOW); //Pull step pin low so it can be triggered again
    delay(1000);
  }

  Blink(LED, 20);
}


//----------------------------------- B L I N K ----------------------------

void Blink(byte PIN, int DELAY_MS)
{
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN,HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN,LOW);
}
