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

#define DEBUG 0

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
  #define BOARD "ESP12MultiSensor"
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
  pinMode(D6, OUTPUT);
 

  // Start the DS18B20 sensor
  sensors.begin();
  devCount = sensors.getDeviceCount();
  debug("Sensors count: ");
  debugln(devCount);  
  debugln("setup completed");
}
 
void loop() {
  // if ((millis() - lastTime) > timerDelay) {
    
  float temperatureC;
  float temperatureF;
  float temperatureAir;
  long ellapsedTime = millis();
  float batVolt;
  digitalWrite(D6, LOW);
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
  digitalWrite(LED_PIN, LOW);

  batVolt = analogRead(A0);
  digitalWrite(D6, HIGH);
  batVolt = batVolt * 5 / 1000;
  debug("Battery Voltage :");
  debugln(batVolt);
  strcat(myData.a,"|ButV|");
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
