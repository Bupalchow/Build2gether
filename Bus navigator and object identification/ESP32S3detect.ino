#include <WiFi.h>
#include <WiFiUdp.h>
#include <Seeed_Arduino_SSCMA.h>

// Change to your WiFi SSID and password
const char* ssid = "bupal";
const char* password = "12345678";

const char* unihikerIP = "192.168.137.56"; // Replace with UniHiker's IP address
const int unihikerPort = 12345; 

SSCMA AI;
WiFiUDP udp;

void setup() {
    AI.begin();
    Serial.begin(9600);
    WiFi.begin(ssid, password); 
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi.");
}

void loop() {
    if (!AI.invoke()) {
        for (int i = 0; i < AI.boxes().size(); i++) {
            if (AI.boxes()[i].score > 70) {
                Serial.println("Bus detected");
                if (WiFi.status() == WL_CONNECTED) {
                    sendUDPMessage("busDetected");
                    delay(1000);
                }
            }
        }
    }
}

void sendUDPMessage(const char* message) {
    udp.beginPacket(unihikerIP, unihikerPort);
    udp.write(message);
    udp.endPacket();
    Serial.println("UDP message sent to UniHiker: " + String(message));
}
