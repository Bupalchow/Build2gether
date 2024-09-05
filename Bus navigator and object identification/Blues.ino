#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>  
#include <Notecard.h>
#include <cstring>

#define NOTE_PRODUCT_UID "com.gmail.bupalchowdary06:build2gether"

char ssid[] = "bupal";
char password[] = "12345678";

Notecard notecard;
 
WiFiUDP udp;
const int udpPort = 4212; 
char incomingPacket[255]; 

void setup() {
  Serial.begin(115200);
  
  myServo.attach(servoPin);
  myServo.write(180);
  
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  
  Wire.begin();  
  
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
    if (millis() - startTime > 15000) { 
      Serial.println("\nFailed to connect to WiFi.");
      return;
    }
  }
  
  Serial.println("\nConnected to WiFi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  
  // Start UDP communication
  udp.begin(udpPort);
  Serial.printf("Listening on UDP port %d\n", udpPort);
  notecard.begin(); 
  notecard.setDebugOutputStream(Serial);

  // Setup WiFi on Notecard
  J *req1 = notecard.newRequest("card.wifi");
  if (req1 != NULL) {
    JAddStringToObject(req1, "ssid", ssid);
    JAddStringToObject(req1, "password", password);
    notecard.sendRequest(req1);
    Serial.println("WiFi connected via Notecard.");
  }

  // Set up the hub connection
  J *req = notecard.newRequest("hub.set");
  if (req != NULL) {
    JAddStringToObject(req, "product", NOTE_PRODUCT_UID);
    JAddStringToObject(req, "mode", "continuous");
    notecard.sendRequest(req);
  }
}

void loop() {
  int packetSize = udp.parsePacket(); 
  
  if (packetSize) { 
    int len = udp.read(incomingPacket, 255); 
    if (len > 0) {
      incomingPacket[len] = '\0'; 
    }
    
    Serial.printf("Received packet: %s\n", incomingPacket);
    
   if (strcmp(incomingPacket, "FD") == 0) {
      // Send fall detection alert via Notecard
      J *req3 = notecard.newRequest("note.add");
      if (req3 != NULL) {
        JAddStringToObject(req3, "file", "FD.qo");
        JAddBoolToObject(req3, "sync", true);
        J *body = JAddObjectToObject(req3, "body");
        if (body) {
          JAddNumberToObject(body, "Fall detected", 0);
        }
        notecard.sendRequest(req3);
        Serial.println("Fall detection signal sent.");
      }
    } 
    else {
      Serial.println("Unknown command received.");
    }
  }

  delay(10); 
}