// Complete Instructions to Get and Change ESP MAC Address:
// https://RandomNerdTutorials.com/get-change-esp32-esp8266-mac-address-arduino/

#ifdef ESP32
  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
#endif
#include <home_wifi_multi.h>
const char* ssid = SSID1;
const char* password = PWD1;

void setup(){
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.begin(115200);
  delay(3000);
  Serial.println();
  Serial.print("Setup - ESP Board MAC Address:  ");
  Serial.println(WiFi.macAddress());
}
 
void loop(){
  delay(3000);
  Serial.println();
  Serial.print("Loop - ESP Board MAC Address:  ");
  Serial.println(WiFi.macAddress());
}
