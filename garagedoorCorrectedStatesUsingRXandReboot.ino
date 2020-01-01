#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <functional>
#include "switch.h"
#include "UpnpBroadcastResponder.h"
#include "CallbackFunction.h"

// prototypes
boolean connectWifi();

//on/off callbacks 
bool garageDoorOpenOn();
bool garageDoorOpenOff();
bool garageDoorCloseOn();
bool garageDoorCloseOff();
bool garageDoorRebootOn();
bool garageDoorRebootOff();


// Change this before you flash
const char* ssid = "HomeAutomation";
const char* password = "homeautomation1";

//const char* ssid = "OldRouter";
//const char* password = "OldRouter";
boolean wifiConnected = false;

UpnpBroadcastResponder upnpBroadcastResponder;

Switch *garagedooropen = NULL;
Switch *garagedoorclose = NULL;
Switch *garagedoorboot = NULL;

bool isgarageDoorOpenOn = false;
bool isgarageDoorCloseOn = false;
bool isgarageDoorBootOn = false;
bool GarageDoorState = false;
bool PrevGarageDoorState = false;

const int garageDoorRelay = 0;  // GPIO0 pin.
const int GPIO2 = 2;  // GPIO2 pin.
const int RXPin = 3;  // RX pin.

void setup()
{
  pinMode(garageDoorRelay, OUTPUT);
  pinMode(GPIO2, OUTPUT); // to start with, it changes later...
     
  Serial.begin(9600);

  Serial.println("Booting...");
  delay(2000);

  //flash fast a few times to indicate CPU is booting
  digitalWrite(GPIO2, LOW); 
  delay(100);    
  digitalWrite(GPIO2, HIGH);
  delay(100); 
  digitalWrite(GPIO2, LOW); 
  delay(100);    
  digitalWrite(GPIO2, HIGH);
  delay(100);   
  digitalWrite(GPIO2, LOW); 
  delay(100);   
  digitalWrite(GPIO2, HIGH);
  
  Serial.println("Delaying a bit...");
  delay(2000);   
  
  // Initialise wifi connection
  wifiConnected = connectWifi();
     
  if(wifiConnected){

  //flash slow a few times to indicate wifi connected OK
  digitalWrite(GPIO2, LOW); 
  delay(1000);    
  digitalWrite(GPIO2, HIGH);
  delay(1000); 
  digitalWrite(GPIO2, LOW); 
  delay(1000);    
  digitalWrite(GPIO2, HIGH);
   delay(1000); 
  digitalWrite(GPIO2, LOW); 
  delay(1000);    
  digitalWrite(GPIO2, HIGH);
    
    upnpBroadcastResponder.beginUdpMulticast();
    
    // Define your switches here. Max 10
    // Format: Alexa invocation name, local port no, on callback, off callback
    garagedooropen = new Switch("garage door open", 80, garageDoorOpenOn, garageDoorOpenOff);
    garagedoorclose = new Switch("garage door close", 81, garageDoorCloseOn, garageDoorCloseOff);
    garagedoorboot = new Switch("garage door controller reboot", 82, garageDoorBootOn, garageDoorBootOff);

    Serial.println("Adding switches upnp broadcast responder");
    upnpBroadcastResponder.addDevice(*garagedooropen);
    upnpBroadcastResponder.addDevice(*garagedoorclose);
    upnpBroadcastResponder.addDevice(*garagedoorboot);
    
  }

    digitalWrite(garageDoorRelay, LOW); // turn off relay 
    digitalWrite(GPIO2, HIGH); // turn off LED 


   Serial.println("Making GPIO2 into an INPUT"); // used to detect garage door current state
   pinMode(GPIO2, INPUT); 

   Serial.println("Making RX into an INPUT"); // used to detect garage door current state
   //GPIO 3 (RX) swap the pin to a GPIO.

   pinMode(RXPin, FUNCTION_3);
   pinMode(RXPin, INPUT);
   

    PrevGarageDoorState = GarageDoorState; // edge detection of garage door state
}
     
void loop()
{
  
  GarageDoorState = digitalRead(RXPin); //either GPIO 2 or RX Pin depending on variable used...
  delay(100);  
  if (GarageDoorState == LOW) {
      if (PrevGarageDoorState == HIGH){ 
        Serial.println("GarageDoor State has just opened (Logic LOW)");
      }
  }       
      
  if (GarageDoorState == HIGH) {
       if (PrevGarageDoorState == LOW){ 
        Serial.println("GarageDoor State has just closed (Logic HIGH)");
      }
  }      

   PrevGarageDoorState = GarageDoorState;   // remember prev state for next pass
   
  if(wifiConnected){
     // digitalWrite(GPIO2, LOW); // turn on LED with voltage Low
      upnpBroadcastResponder.serverLoop();

      garagedoorboot->serverLoop();
      garagedoorclose->serverLoop();
      garagedooropen->serverLoop();
   }
}

bool garageDoorOpenOn() {
    Serial.println("Request to Open door received SW #1 On...");

      if (GarageDoorState == HIGH) { // only pulse relay if door is currently closed
          Serial.println("Door is closed - pulsing relay to open it");

          Serial.println("XXX Pulsing Relay on ...");
          digitalWrite(garageDoorRelay, HIGH); // turn on relay 
          delay(2000);    ;
          Serial.println("XXX Pulsing Relay off again ...");
          digitalWrite(garageDoorRelay, LOW); // turn off relay 
          }
      else {
          Serial.println("Door is already open - not pulsing relay!");
      }
         
    isgarageDoorOpenOn = false;    
    return GarageDoorState;
}

bool garageDoorOpenOff() { 
    
    Serial.println("Request to Open door received SW#1 Off ...");

      if (GarageDoorState == HIGH) { // only pulse relay if door is currently closed
          Serial.println("Door is closed - pulsing relay to open it");

          Serial.println("XXX Pulsing Relay on ...");
          digitalWrite(garageDoorRelay, HIGH); // turn on relay 
          delay(2000);    ;
          Serial.println("XXX Pulsing Relay off again ...");
          digitalWrite(garageDoorRelay, LOW); // turn off relay 
          }
      else {
          Serial.println("Door is already open - not pulsing relay!");
      }
            
    isgarageDoorOpenOn = false;
    return GarageDoorState;
}

bool garageDoorCloseOn() {
    Serial.println("Request to Close door received SW#2 On");

      if (GarageDoorState == LOW) { // only pulse relay if door is currently open
          Serial.println("Door is open - pulsing relay to close it");

          Serial.println("XXX Pulsing Relay on ...");
          digitalWrite(garageDoorRelay, HIGH); // turn on relay 
          delay(2000);    ;
          Serial.println("XXX Pulsing Relay off again ...");
          digitalWrite(garageDoorRelay, LOW); // turn off relay
      }
      else {
          Serial.println("Door is already closed, not pulsing relay...");
      }
      
    isgarageDoorCloseOn = false;
    return GarageDoorState;
}

bool garageDoorCloseOff() {  

    Serial.println("Request to Close door received (SW#2 Off)");

      if (GarageDoorState == LOW) { // only pulse relay if door is currently open
          Serial.println("Door is open - pulsing relay to close it");

          Serial.println("XXX Pulsing Relay on ...");
          digitalWrite(garageDoorRelay, HIGH); // turn on relay 
          delay(2000);    ;
          Serial.println("XXX Pulsing Relay off again ...");
          digitalWrite(garageDoorRelay, LOW); // turn off relay
      }
      else {
          Serial.println("Door is already closed, not pulsing relay...");
      }
  
  isgarageDoorCloseOn = false;
  return GarageDoorState;
}




bool garageDoorBootOn() {
    Serial.println("Request to reboot door controller received SW#3 On");

 //ESP.restart(); hangs..
 ESP.reset();
      
    isgarageDoorBootOn = false;
    return isgarageDoorBootOn;
}

bool garageDoorBootOff() {  

    Serial.println("Request to reboot garage door controller received (SW#3 Off)");


  
  isgarageDoorBootOn = false;
  return GarageDoorState;
}

// connect to wifi – returns true if successful or false if not
boolean connectWifi(){
  boolean state = true;
  int i = 0;
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.println("Connecting to WiFi Network");

  // Wait for connection
  Serial.print("Connecting ...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(5000);
    Serial.print(".");
    if (i > 10){
      state = false;
      break;
    }
    i++;
  }
  
  if (state){
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
  else {
    Serial.println("");
    Serial.println("Connection failed. Bugger");
  }
  
  return state;
}

