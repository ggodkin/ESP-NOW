/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp-now-esp8266-nodemcu-arduino-ide/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

#define DEBUG 0

#if DEBUG == 2
  #define debugln(x) Serial1.println(x)
  #define LED_PIN      11 
#elif DEBUG == 1
  #define debugln(x) Serial.println(x) 
  #define LED_PIN       LED_BUILTIN 
#else
  #define debugln(x)
  #define LED_PIN       LED_BUILTIN 
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

//Deep Sleep time in microseconds
#define sleepTime 60e6

#include <Wire.h>              // Wire library (required for I2C devices)
#include <Adafruit_BMP280.h>   // Adafruit BMP280 sensor library
#define BMP280_I2C_ADDRESS  0x76

#include <OneWire.h>
#include <DallasTemperature.h>

// GPIO where the DS18B20 is connected to
const int oneWireBus = D7;  

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

Adafruit_BMP280  bmp280;

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
unsigned long timerDelay = 10000;  // send readings timer

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0){
    debugln("Delivery success S1");
  }
  else{
    Serial.println("Delivery fail");
    debugln("Delivery fail S1");
    digitalWrite(LED_PIN, HIGH);
  }
}
 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
  Serial1.begin(115200);

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

  // Start the DS18B20 sensor
  sensors.begin();
  
  bmp280.begin(BMP280_I2C_ADDRESS);
}
 
void loop() {
 // if ((millis() - lastTime) > timerDelay) {
    
    float temp = bmp280.readTemperature();   // get temperature
    float pres = bmp280.readPressure();      // get pressure

    sensors.requestTemperatures(); 
    float temperatureC = sensors.getTempCByIndex(0);
    float temperatureF = sensors.getTempFByIndex(0);

    // Set values to send
    strcpy(myData.a,BOARD);
    strcat(myData.a,"-2|temp|");
    char result[8];
    dtostrf(temp, 6, 2, result);
    strcat(myData.a,result);
    strcat(myData.a,"|pres|");
    dtostrf(pres, 6, 2, result);
    strcat(myData.a,result);
    
    strcat(myData.a,"|tempC|");
    dtostrf(temperatureC, 6, 2, result);
    strcat(myData.a,result);
    strcat(myData.a,"|tempF|");
    dtostrf(temperatureF, 6, 2, result);
    strcat(myData.a,result);
    

   digitalWrite(LED_PIN, LOW);

    // Send message via ESP-NOW
    esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

    ESP.deepSleep(sleepTime);
    
    lastTime = millis();
  //}
}
