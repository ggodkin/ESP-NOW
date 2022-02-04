/*
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
  #define LED_PIN      11 
#elif DEBUG == 1
  #define debugln(x) Serial.println(x) 
  #define debug(x) Serial.print(x) 
  #define LED_PIN       LED_BUILTIN 
#else
  #define debugln(x)
  #define debug(x)
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
#define sleepTime 10e6

#include <Wire.h>              // Wire library (required for I2C devices)
#include "Adafruit_MCP9808.h"
#include <Adafruit_BME280.h>
#include <Adafruit_BMP280.h>   // Adafruit BMP280 sensor library
#define BMP280_I2C_ADDRESS  0x76
#define BME280_I2C_ADDRESS  0x76
#define SEALEVELPRESSURE_HPA (1013.25)

#include <OneWire.h>
#include <DallasTemperature.h>


// GPIO where the DS18B20 is connected to
const int oneWireBus = D7;  

// Create the MCP9808 temperature sensor object
Adafruit_MCP9808 tempsensor = Adafruit_MCP9808();

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

Adafruit_BMP280 bmp280;
Adafruit_BME280 bme280;

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

float temperature, humidity, pressure, altitude;

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  debug("Last Packet Send Status: ");
  if (sendStatus == 0){
    debugln("Delivery success S1");
  }
  else{
    debugln("Delivery fail");
    debugln("Delivery fail S1");
    digitalWrite(LED_PIN, HIGH);
  }
}
 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
  Serial1.begin(115200);

  while (!Serial);
  while (!Serial);  
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

  // Make sure the sensor is found, you can also pass in a different i2c
  // address with tempsensor.begin(0x19) for example, also can be left in blank for default address use
  // Also there is a table with all addres possible for this sensor, you can connect multiple sensors
  // to the same i2c bus, just configure each sensor with a different address and define multiple objects for that
  //  A2 A1 A0 address
  //  0  0  0   0x18  this is the default address
  //  0  0  1   0x19
  //  0  1  0   0x1A
  //  0  1  1   0x1B
  //  1  0  0   0x1C
  //  1  0  1   0x1D
  //  1  1  0   0x1E
  //  1  1  1   0x1F
  if (!tempsensor.begin(0x18)) {
    debugln("Couldn't find MCP9808! Check your connections and verify the address is correct.");
    while (1);
  }

   Serial.println("Found MCP9808!");

  tempsensor.setResolution(3); // sets the resolution mode of reading, the modes are defined in the table bellow:
  // Mode Resolution SampleTime
  //  0    0.5°C       30 ms
  //  1    0.25°C      65 ms
  //  2    0.125°C     130 ms
  //  3    0.0625°C    250 ms

  // Start the DS18B20 sensor
  sensors.begin();
  
  bmp280.begin(BMP280_I2C_ADDRESS);
  bme280.begin(BME280_I2C_ADDRESS);
  debugln("setup completed");
}
 
void loop() {
 // if ((millis() - lastTime) > timerDelay) {
    
    float temp = bmp280.readTemperature();   // get temperature
    float pres = bmp280.readPressure();      // get pressure

    sensors.setResolution(12);
    sensors.requestTemperatures(); 
    float temperatureC = sensors.getTempCByIndex(0);
    float temperatureF = sensors.getTempFByIndex(0);

    temperature = bme280.readTemperature();
    humidity = bme280.readHumidity();
    pressure = bme280.readPressure() / 100.0F;
    altitude = bme280.readAltitude(SEALEVELPRESSURE_HPA);

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

    strcat(myData.a,"|temp77|");
    dtostrf(temperature, 6, 2, result);
    strcat(myData.a,result);
    strcat(myData.a,"|hum77|");
    dtostrf(humidity, 6, 2, result);
    strcat(myData.a,result);
    strcat(myData.a,"|pres77|");
    dtostrf(pressure, 6, 2, result);
    strcat(myData.a,result);
    strcat(myData.a,"|alt77|");
    dtostrf(altitude, 6, 2, result);
    strcat(myData.a,result);

    tempsensor.wake();   // wake up, ready to read!

    // Read and print out the temperature, also shows the resolution mode used for reading.
    debug("Resolution in mode: ");
    debugln (tempsensor.getResolution());
    float c = tempsensor.readTempC();
    float f = tempsensor.readTempF();
    debug("Temp 9808: "); 
    debug(c); debug("*C\t and "); 
    debug(f); debugln("*F.");
  
    debugln("Shutdown MCP9808.... ");
    tempsensor.shutdown_wake(1); // shutdown MSP9808 - power consumption ~0.1 mikro Ampere, stops temperature sampling

    debug("BMP Temp: ");
    debugln(temp);
    
    debug("BME Temp: ");
    debugln(temperature);
    
    debug("DS18B20 Temp: ");
    debugln(temperatureC);  debug("*C\t and "); 
    debug(temperatureF); debugln("*F.");
    
    digitalWrite(LED_PIN, LOW);

    // Send message via ESP-NOW
    esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

    ESP.deepSleep(sleepTime);
    
    lastTime = millis();
  //}
}
