/*
  Any number of DS18B20 temperature sensor

  
  Inspired by 
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp-now-esp8266-nodemcu-arduino-ide/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/


#include <Wire.h>
#include "Adafruit_MCP9808.h"
#include <HS300xlib.h>

//Create HS300x object
HS300xlib hs300x; 


// Create the MCP9808 temperature sensor object
Adafruit_MCP9808 tempsensor = Adafruit_MCP9808();

#define TEMP_POWER D5
#define ADC_GND D6
#define TEMP_DATA D7

#define DEBUG 1

#if DEBUG == 2
  #define debugln(x) Serial1.println(x)
  #define debug(x) Serial1.print(x)
  #define debugBegin(x) Serial1.begin(x)
  #define serial Serial1
  #define LED_PIN      99 
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
#elif DEBUG == 4
  #define debugln(x) Serial.println(x) 
  #define debug(x) Serial.print(x) 
  #define debugBegin(x) Serial.begin(x)
  #define serial Serial
  #define LED_PIN       LED_BUILTIN 
  //Deep Sleep time in microseconds
  #define sleepTime 30e6  //30 sec
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
  //#define BOARD "ESP12WineCellar"
  #if  DEBUG == 4
    #define BOARD "ESP12BatteryTester"
  #elif  DEBUG == 5
    #define BOARD "ESP12BatteryTester"
  #elif  DEBUG == 2
    #define BOARD "ESP12MultiSensorTest2"
  #elif  DEBUG == 1
    #define BOARD "MST1"
  #else
    #define BOARD "ESP12MultiSensor"
  #endif
#endif
#include <home_wifi_multi.h> 

#include <OneWire.h>
#include <DallasTemperature.h>

// GPIO where the DS18B20 is connected to
const int oneWireBus = TEMP_DATA;  

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

// REPLACE WITH RECEIVER MAC Address
uint8_t broadcastAddress[] = {0x2C,0xF4,0x32,0x20,0x5D,0x1C}; //MAC1;
uint8_t tempDeviceAddress[8];
String strMAC;

//DS18B20 count
int devCount = 0;

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
  debug("On Data Sent Time :");
  debugln(millis());
}
 
void setup() {
  // Init Serial Monitor
  debugBegin(115200);

  while (!serial);
  debugln();
  debugln("Set Up");

  pinMode(LED_PIN, OUTPUT);
  pinMode(TEMP_POWER, OUTPUT);
  pinMode(ADC_GND, OUTPUT);
  
  digitalWrite(TEMP_POWER, HIGH);
  delay(100);
 

  // Start the DS18B20 sensor
  sensors.begin();
  devCount = sensors.getDeviceCount();
  debug("Sensors count: ");
  debugln(devCount);  
  
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
    
   debugln("Found MCP9808!");

  tempsensor.setResolution(3); // sets the resolution mode of reading, the modes are defined in the table bellow:
  // Mode Resolution SampleTime
  //  0    0.5°C       30 ms
  //  1    0.25°C      65 ms
  //  2    0.125°C     130 ms
  //  3    0.0625°C    250 ms

  
  debugln("setup completed");
}
 
void loop() {
  // if ((millis() - lastTime) > timerDelay) {
    
  float temperatureC;
  float temperatureF;
  float temperatureAir;
  float humidity;
  long ellapsedTime = millis();
  float batVolt;
  digitalWrite(ADC_GND, LOW);
  debug("Loop Enter Time: ");
  debugln(ellapsedTime);

  sensors.setResolution(12);

  // Set values to send
  strcpy(myData.a,BOARD);
  char result[8];

  sensors.requestTemperatures();
  for (int i = 0;  i < devCount;  i++)
  {
    temperatureC = sensors.getTempCByIndex(i);
    temperatureF = temperatureC * 1.8 + 32; 
    sensors.getAddress(tempDeviceAddress, i);
    strMAC = mac2string();
    debug("Sensor Address :");
    debugln(strMAC);

    strcat(myData.a,"|");
    strcat(myData.a,strMAC.c_str());
    strcat(myData.a,"|TempC|");
    dtostrf(temperatureC, 6, 2, result);
    strcat(myData.a,result);
    strcat(myData.a,"|TempF|");
    dtostrf(temperatureF, 6, 2, result);
    strcat(myData.a,result);
    
    debug("DS18B20 Temp Air: ");
    debug(temperatureC);  debug("*C\t and "); 
    debug(temperatureF); debugln("*F.");
    //printAddress( tempDeviceAddress);
  }
  ellapsedTime = millis();
  debug("Temp collected Time: ");
  debugln(ellapsedTime); 
  digitalWrite(TEMP_POWER, LOW);   
  digitalWrite(LED_PIN, LOW);

  tempsensor.wake();   // wake up, ready to read!
  temperatureC = tempsensor.readTempC();
  temperatureF = tempsensor.readTempF();
  tempsensor.shutdown_wake(1); // shutdown MSP9808 - power consumption ~0.1 mikro Ampere, stops temperature sampling
  strcat(myData.a,"|MCP980x");
  strcat(myData.a,"|TempC|");
  dtostrf(temperatureC, 6, 2, result);
  strcat(myData.a,result);
  strcat(myData.a,"|TempF|");
  dtostrf(temperatureF, 6, 2, result);
  strcat(myData.a,result);

  ellapsedTime = millis();
  debug("Temp MCP980x collected Time: ");
  debugln(ellapsedTime); 

  int8_t respon = hs300x.MeasurementReq();
  if(respon){
    temperatureC = hs300x.getTemperatureC();
    temperatureF = temperatureC * 1.8 + 32;
    humidity = hs300x.getHumidity();
    strcat(myData.a,"|Hs300x");
    strcat(myData.a,"|TempC|");
    dtostrf(temperatureC, 6, 2, result);
    strcat(myData.a,result);
    strcat(myData.a,"|TempF|");
    dtostrf(temperatureF, 6, 2, result);
    strcat(myData.a,result);
    strcat(myData.a,"|Humid|");
    dtostrf(humidity, 6, 2, result);
    strcat(myData.a,result);
  }


  batVolt = analogRead(A0);
  digitalWrite(ADC_GND, HIGH);
  batVolt = batVolt * 5 / 1000;
  debug("Battery Voltage :");
  debugln(batVolt);
  strcat(myData.a,"|BatV|");
  dtostrf(batVolt, 6, 2, result);
  strcat(myData.a,result);
  ellapsedTime = millis();
  debug("Battery Voltage collected Time: ");
  debugln(ellapsedTime);   
  debugln(myData.a); 

  WiFi.mode(WIFI_STA);
  debug("WiFi.status(): ");
  debugln(WiFi.status());

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    debugln("Error initializing ESP-NOW");
    return;
  }
  debugln("WiFi and ESP-NOW initialized");
  debug(" Time: ");
  debugln(millis());

  
  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
  
  // Send message via ESP-NOW
  esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
  delay(50);
    
  ellapsedTime = millis();
  debug("Loop Exit Time: ");
  debugln(ellapsedTime);
    
  ESP.deepSleep(sleepTime);  //, WAKE_RF_DISABLED);
}

String mac2string() {
  char str[2];
  String strMAC = "MAC";
  for (int i = 0; i < sizeof(tempDeviceAddress); i++) {
    itoa(tempDeviceAddress[i], str, 16);
    strMAC.concat(':');
    if (str[1] == '\0') {
      strMAC.concat("0");
      strMAC.concat(str[0]);
    } else {
      strMAC.concat(str[0]);
      strMAC.concat(str[1]);
    }
  }
  return strMAC;
}
