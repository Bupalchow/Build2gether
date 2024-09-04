#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
const char* ssid = "bupal";
const char* password = "12345678";

// UDP configuration
WiFiUDP udp;
unsigned int localUdpPort = 4215; 

const int relay1Pin = 4; 
const int relay2Pin = 5; 

char incomingPacket[255];

void setup() {
  Serial.begin(115200);
  
  pinMode(relay1Pin, OUTPUT);
  pinMode(relay2Pin, OUTPUT);
  
  digitalWrite(relay1Pin, HIGH); // HIGH to turn off relay 
  digitalWrite(relay2Pin, HIGH); 
  
  Serial.printf("Connecting to %s ...\n", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  
  Serial.printf("\nConnected to %s\n", ssid);
  Serial.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());
  
  // Start UDP
  udp.begin(localUdpPort);
  Serial.printf("UDP server started at port %d\n", localUdpPort);
}

void loop() {

  int packetSize = udp.parsePacket();
  if (packetSize) {

    int len = udp.read(incomingPacket, 255);
    if (len > 0) {
      incomingPacket[len] = '\0';
    }
    
    Serial.printf("Received packet: %s\n", incomingPacket);
    
    if (strcmp(incomingPacket, "lon") == 0) {
      digitalWrite(relay1Pin, LOW); // Turn ON relay (LOW for active low relay)
      Serial.println("Light ON");
    } else if (strcmp(incomingPacket, "loff") == 0) {
      digitalWrite(relay1Pin, HIGH); 
      Serial.println("Light OFF");
    } else if (strcmp(incomingPacket, "fon") == 0) {
      digitalWrite(relay2Pin, LOW);
      Serial.println("Fan ON");
    } else if (strcmp(incomingPacket, "foff") == 0) {
      digitalWrite(relay2Pin, HIGH);
      Serial.println("Fan OFF");
    }
  }
}
