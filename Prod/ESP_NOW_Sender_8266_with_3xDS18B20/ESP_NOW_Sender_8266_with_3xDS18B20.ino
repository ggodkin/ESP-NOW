/*
  3 DS18B20 temperature sensor. 1 air 2 waterproof underground
  287DF4F857200186
  air

  287D8E6F622001DA
  underground

  2871D688622001C3
  underground
  
  Inspired by 
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp-now-esp8266-nodemcu-arduino-ide/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

#define DEBUG 1

#if DEBUG == 2
  #define debugln(x) Serial1.println(x)
  #define debug(x) Serial1.print(x)
  #define debugBegin(x) Serial1.begin(x)
  #define serial Serial1
  #define LED_PIN      11 
  //Deep Sleep time in microseconds
  #define sleepTime 120e6  //2min
#elif DEBUG == 1
  #define debugln(x) Serial.println(x) 
  #define debug(x) Serial.print(x) 
  #define debugBegin(x) Serial.begin(x)
  #define serial Serial
  #define LED_PIN       LED_BUILTIN 
  //Deep Sleep time in microseconds
  #define sleepTime 120e6  //2min
#else
  #define serial true
  #define debugln(x)
  #define debug(x)
  #define debugBegin(x)
  #define LED_PIN       99
  //Deep Sleep time in microseconds
  #define sleepTime 900e6  //15min
#endif
  

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



//#include <Wire.h>              // Wire library (required for I2C devices)

#include <OneWire.h>
#include <DallasTemperature.h>


// GPIO where the DS18B20 is connected to
const int oneWireBus = D7;  

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

uint8_t sensorAir[8] = { 0x28, 0x7D, 0xF4, 0xF8, 0x57, 0x20, 0x01, 0x86 };
uint8_t sensorEarth2[8] = { 0x28, 0x7D, 0x8E, 0x6F, 0x62, 0x20, 0x01, 0xDA };
uint8_t sensorEarth6[8] = { 0x28, 0x71, 0xD6, 0x88, 0x62, 0x20, 0x01, 0xC3 };

DeviceAddress tempDeviceAddress;

// REPLACE WITH RECEIVER MAC Address
uint8_t broadcastAddress[] = {0x2C,0xF4,0x32,0x20,0x5D,0x1C}; //MAC1;


// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  char a[240];
} struct_message;

// Create a struct_message called myData
struct_message myData;
String sendMsg;

unsigned long lastTime = 0;  
#define unsigned long timerDelay = 10000;  // send readings timer

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  debug("Last Packet Send Status: ");
  debugln(sendStatus);
  if (sendStatus == 0){
    debugln("Delivery success");
  }
  else{
    debugln("Delivery fail");
    digitalWrite(LED_PIN, HIGH);
  }
}
 
void setup() {
  // Init Serial Monitor
  debugBegin(115200);

  while (!serial);
  debugln();
  debugln("Set Up");

  pinMode(LED_PIN, OUTPUT);
 
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    debugln("Error initializing ESP-NOW");
    return;
  }
  debugln("WiFi initialized");
  
  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);


  // Start the DS18B20 sensor
  sensors.begin();
  
  debugln("setup completed");
}
 
void loop() {
  // if ((millis() - lastTime) > timerDelay) {
    
    float temperatureC;
    float temperatureF;
    float temperatureAir, temperatureGround2, temperatureGround6;

    sensors.setResolution(12);
    //sensors.getAddress(tempDeviceAddress, 0);
     

    // Set values to send
    strcpy(myData.a,BOARD);
    char result[8];

    sensors.requestTemperatures();
    temperatureC = sensors.getTempC(sensorEarth6);
    temperatureF = sensors.getTempF(sensorEarth6);

    strcat(myData.a,"|Earth6C|");
    dtostrf(temperatureC, 6, 2, result);
    strcat(myData.a,result);
    strcat(myData.a,"|Earth6F|");
    dtostrf(temperatureF, 6, 2, result);
    strcat(myData.a,result);

    debug("DS18B20 Temp Earth 6: ");
    debug(temperatureC);  debug("*C\t and "); 
    debug(temperatureF); debugln("*F.");
    //printAddress( tempDeviceAddress);
    //strAddress(tempDeviceAddress);
 
    sensors.requestTemperatures();
    temperatureC = sensors.getTempC(sensorEarth2);
    temperatureF = sensors.getTempF(sensorEarth2);
    strcat(myData.a,"|Earth2C|");
    dtostrf(temperatureC, 6, 2, result);
    strcat(myData.a,result);
    strcat(myData.a,"|Earth2F|");
    dtostrf(temperatureF, 6, 2, result);
    strcat(myData.a,result);

    debug("DS18B20 Temp Earth 2: ");
    debug(temperatureC);  debug("*C\t and "); 
    debug(temperatureF); debugln("*F.");
    //printAddress( tempDeviceAddress);
    //strAddress(tempDeviceAddress);

    sensors.requestTemperatures();
    temperatureC = sensors.getTempC(sensorAir);
    temperatureF = sensors.getTempF(sensorAir);
    strcat(myData.a,"|AirC|");
    dtostrf(temperatureC, 6, 2, result);
    strcat(myData.a,result);
    strcat(myData.a,"|AirF|");
    dtostrf(temperatureF, 6, 2, result);
    strcat(myData.a,result);
    
    debug("DS18B20 Temp Air: ");
    debug(temperatureC);  debug("*C\t and "); 
    debug(temperatureF); debugln("*F.");
    //printAddress( tempDeviceAddress);

    
    digitalWrite(LED_PIN, LOW);

    // Send message via ESP-NOW
    esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

    ESP.deepSleep(sleepTime);
    
    lastTime = millis();
  //}
}

// function to print a device address
void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++) {
    if (deviceAddress[i] < 16) debug("0");
      Serial.print(deviceAddress[i], HEX);
  }
}

void strAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++) {
    if (deviceAddress[i] < 16) debug("0");
      debugln();
      Serial.print(deviceAddress[i], HEX);
  }
  
}
