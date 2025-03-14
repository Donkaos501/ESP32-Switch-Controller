#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <ezButton.h>

WiFiMulti WiFiMulti;

const char* UUID = "bd4a6037-0864-4b0e-80e1-8d1c87bf9aa0";
const char* FRIENDLY_NAME = "Küche hinten";
const char* SERVER_URL = "http://192.168.101.184:8080/esp";

#define SWITCH_1_PIN 19
#define SWITCH_2_PIN 21

const int LONG_PRESS_DURATION = 2000; 

ezButton switch1(SWITCH_1_PIN);
ezButton switch2(SWITCH_2_PIN);

unsigned long switch1PressTime = 0;
unsigned long switch2PressTime = 0;
bool switch1LongPressTriggered = false;
bool switch2LongPressTriggered = false;

void setup() {
  Serial.begin(115200);

  switch1.setDebounceTime(50);
  switch2.setDebounceTime(50);

  WiFiMulti.addAP("WIFINAME", "WIFIPASSWORD");
  Serial.print("Connecting to WiFi... ");

  while (WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("\nWiFi connected!");
  Serial.println("IP address: " + WiFi.localIP().toString());
}

void loop() {
  switch1.loop();
  switch2.loop();

  unsigned long currentTime = millis();

  if (switch1.isPressed()) {
    if (switch1PressTime == 0) {
      switch1PressTime = currentTime; 
      switch1LongPressTriggered = false;
    }
  }

  if (switch1PressTime > 0 && (currentTime - switch1PressTime >= LONG_PRESS_DURATION) && switch1.getState() == LOW && !switch1LongPressTriggered) {
    Serial.println("Switch 1 - Long Press Triggered!");
    sendPostRequest("switch_1_long");
    switch1LongPressTriggered = true; 

  if (switch1.isReleased()) {
    if (!switch1LongPressTriggered && (currentTime - switch1PressTime < LONG_PRESS_DURATION)) {
      Serial.println("Switch 1 - Short Press Triggered!");
      sendPostRequest("switch_1_short");
    }
    switch1PressTime = 0; 
  }

  if (switch2.isPressed()) {
    if (switch2PressTime == 0) {
      switch2PressTime = currentTime; 
      switch2LongPressTriggered = false;
    }
  }

  if (switch2PressTime > 0 && (currentTime - switch2PressTime >= LONG_PRESS_DURATION) && switch2.getState() == LOW && !switch2LongPressTriggered) {
    Serial.println("Switch 2 - Long Press Triggered!");
    sendPostRequest("switch_2_long");
    switch2LongPressTriggered = true; 
  }

  if (switch2.isReleased()) {
    if (!switch2LongPressTriggered && (currentTime - switch2PressTime < LONG_PRESS_DURATION)) {
      Serial.println("Switch 2 - Short Press Triggered!");
      sendPostRequest("switch_2_short");
    }
    switch2PressTime = 0; 
  }
}

void sendPostRequest(String action) {
  if (WiFiMulti.run() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(SERVER_URL);
    http.addHeader("Content-Type", "application/json");

    String postData = "{";
    postData += "\"uuid\": \"" + String(UUID) + "\",";
    postData += "\"friendly_name\": \"" + String(FRIENDLY_NAME) + "\",";
    postData += "\"ip\": \"" + WiFi.localIP().toString() + "\",";
    postData += "\"action\": \"" + action + "\"";
    postData += "}";

    Serial.println("Sending POST request:");
    Serial.println(postData);

    int httpResponseCode = http.POST(postData);
    Serial.print("Response code: ");
    Serial.println(httpResponseCode);

    http.end();
  } else {
    Serial.println("WiFi not connected. Cannot send POST request.");
  }
}
