/*
 * ESP-Now code is based on following:

// REPLACE WITH YOUR RECEIVER MAC Address
uint8_t broadcastAddress[] = {0xb4, 0xe6, 0x2d, 0x6a, 0x83, 0xa7}; //ESP12

typedef struct struct_message {
  int devID;
  int sensorID;
  float sensorValue;
} struct_message;

*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp-now-esp8266-nodemcu-arduino-ide/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

#include <ESP8266WiFi.h>
#include <espnow.h>

//Required for DS18b20
#include <OneWire.h>
#include <DallasTemperature.h>
// Data wire is plugged into port 3 on the Arduino
#define ONE_WIRE_BUS D3
#define TEMPERATURE_PRECISION 12
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

unsigned long timeReset = millis();
unsigned int deviceCount = 0;
uint8_t* deviceAddress;

// REPLACE WITH RECEIVER MAC Address
uint8_t broadcastAddress[] = {0xb4, 0xe6, 0x2d, 0x6a, 0x83, 0xa7}; //ESP12

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  int devID;
  int sensorID;
  float sensorValue;
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
  }
}
 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);

  // Start up the DS18b20 library
  sensors.begin();

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
    myData.devID = 1;
    Serial.print("Counting sensors: ");
    deviceCount = sensors.getDeviceCount();
    Serial.println (deviceCount);
    //Do not disable Watchdog, only for testing 
    ESP.wdtDisable();
    yield();
    sensors.setResolution(TEMPERATURE_PRECISION);
    yield();

    // call sensors.requestTemperatures() to issue a global temperature 
    // request to all devices on the bus
    Serial.print("Requesting temperatures...");
    sensors.requestTemperatures(); // Send the command to get temperatures
    yield();
    Serial.println("DONE");

    for(int i=0;i<deviceCount;i++) {
      myData.sensorID = i;
      myData.sensorValue = sensors.getTempCByIndex(i);
      Serial.print("Temperature sensor " + String(i) + " address ");
      //printAddress(deviceAddress);
      Serial.println( " : " + String(myData.sensorValue));
      esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    }
    // Set values to send
    /*
    myData.sensorID = 1;
    myData.sensorValue = -17.34;
 
    // Send message via ESP-NOW
    esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    */

    lastTime = millis();
  }
}
