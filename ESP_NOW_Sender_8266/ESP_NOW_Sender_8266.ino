/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp-now-esp8266-nodemcu-arduino-ide/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/


#ifdef ESP32
  #include <WiFi.h>
  #include <esp_now.h>
  #define BOARD "ESP32"
#else
  #include <ESP8266WiFi.h>
  #include <espnow.h>
  #define BOARD "ESP12"
#endif
#include <home_wifi_multi.h> 

// REPLACE WITH RECEIVER MAC Address
uint8_t broadcastAddress[] = MAC1;

#define LED_PIN       LED_BUILTIN 

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  char a[240];
} struct_message;

// Create a struct_message called myData
struct_message myData;

unsigned long lastTime = 0;  
unsigned long timerDelay = 2000;  // send readings timer

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0){
    Serial.println("Delivery success");
  }
  else{
    Serial.println("Delivery fail");
    digitalWrite(LED_PIN, HIGH);
  }
}
 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
 
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
}
 
void loop() {
  if ((millis() - lastTime) > timerDelay) {
    // Set values to send
    strcpy(myData.a, BOARD);

   digitalWrite(LED_PIN, LOW);

    // Send message via ESP-NOW
    esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    
    lastTime = millis();
  }
}
