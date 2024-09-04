#include <WiFi.h>
#include <WiFiUdp.h>

char* ssid = "bupal";           // Replace with your network SSID
char* password = "12345678";   // Replace with your network password

WiFiUDP udp;
const int udpPort = 4210;  // The same port as defined in UniHiker code
char incomingPacket[255]; 

// Motor control pins
#define IN1 1  
#define IN2 2  
#define IN3 3  
#define IN4 4  

void setup() {
  Serial.begin(115200);
  
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  stopMotors();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  udp.begin(udpPort);
  Serial.printf("Listening on UDP port %d\n", udpPort);
}

void loop() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
    int len = udp.read(incomingPacket, 255);
    if (len > 0) {
      incomingPacket[len] = '\0';
    }
    Serial.printf("Received packet: %s\n", incomingPacket);

    float accX = parseData(incomingPacket, "AX");
    float accY = parseData(incomingPacket, "AY");

    if (accX > 0.5) {
      moveForward();
    } else if (accX < -0.5) {
      moveBackward();
    } else if (accY > 0.5) {
      turnRight();
    } else if (accY < -0.5) {
      turnLeft();
    } else {
      stopMotors();
    }
  }
}
float parseData(char* data, const char* key) {
  char* ptr = strstr(data, key);
  if (ptr) {
    return atof(ptr + strlen(key) + 1);
  }
  return 0.0;
}

void moveForward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void moveBackward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void turnLeft() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void turnRight() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}