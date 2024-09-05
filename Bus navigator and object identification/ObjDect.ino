#include <WiFi.h>
#include <WiFiUdp.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "esp_camera.h"

// Wi-Fi credentials
const char* ssid = "bupal";
const char* password = "12345678";

// Google Vision API key
const char* apiKey = "YOUR_GOOGLE_VISION_API_KEY";


const char* unihikerIP = "192.168.1.100";  
const unsigned int unihikerPort = 12345;   

const int buttonPin = 2;  

WiFiUDP udp;

void setup() {
  Serial.begin(115200);

  pinMode(buttonPin, INPUT_PULLUP);

  if (!initCamera()) {
    Serial.println("Camera initialization failed!");
    return;
  }

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  udp.begin(unihikerPort); 
  Serial.println("UDP initialized.");
}

void loop() {

  if (digitalRead(buttonPin) == LOW) {
    delay(50);  

    Serial.println("Button pressed, taking a picture...");
    takePictureAndIdentify();

    while (digitalRead(buttonPin) == LOW) {
      delay(10);
    }
  }
}

bool initCamera() {

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = 35; 
  config.pin_d1 = 34;
  config.pin_d2 = 39;
  config.pin_d3 = 36;
  config.pin_d4 = 19;
  config.pin_d5 = 18;
  config.pin_d6 = 5;
  config.pin_d7 = 4;
  config.pin_xclk = 15;
  config.pin_pclk = 12;
  config.pin_vsync = 27;
  config.pin_href = 23;
  config.pin_sccb_sda = 13;
  config.pin_sccb_scl = 14;
  config.pin_pwdn = -1;  
  config.pin_reset = -1;  
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_QVGA;  
  config.jpeg_quality = 12;  
  config.fb_count = 1;

  esp_err_t err = esp_camera_init(&config);
  return (err == ESP_OK);
}

void takePictureAndIdentify() {
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  Serial.println("Image captured, sending to Google Vision API...");

  if (sendToGoogleVisionAPI(fb->buf, fb->len)) {
    Serial.println("Image sent successfully");
  }

  esp_camera_fb_return(fb);
}

bool sendToGoogleVisionAPI(uint8_t* imageData, size_t imageLength) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected");
    return false;
  }

  HTTPClient http;
  String endpoint = "https://vision.googleapis.com/v1/images:annotate?key=" + String(apiKey);
  String base64Image = base64::encode(imageData, imageLength);
  String requestBody = "{\"requests\":[{\"image\":{\"content\":\"" + base64Image + "\"},\"features\":[{\"type\":\"LABEL_DETECTION\",\"maxResults\":1}]}]}";

  http.begin(endpoint);
  http.addHeader("Content-Type", "application/json");

  int httpResponseCode = http.POST(requestBody);
  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println("Response: " + response);


    DynamicJsonDocument doc(2048);
    deserializeJson(doc, response);

    const char* description = doc["responses"][0]["labelAnnotations"][0]["description"];
    if (description) {
      Serial.println("Detected Object: " + String(description));
      sendToUnihiker(description); 
    } else {
      Serial.println("No object detected.");
    }

    http.end();
    return true;
  } else {
    Serial.println("Error on sending POST: " + String(httpResponseCode));
    http.end();
    return false;
  }
}

void sendToUnihiker(const char* message) {
  udp.beginPacket(unihikerIP, unihikerPort);
  udp.write(message);
  udp.endPacket();
  Serial.println("Sent to Unihiker: " + String(message));
}
