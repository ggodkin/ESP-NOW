/**
 * 
 * Author: Andreas Spiess, 2017
 * 
  This sketch receives ESP-Now message and sends it as an MQTT messge
  It is heavily based on of Anthony's gateway setch sketch

https://github.com/HarringayMakerSpace/ESP-Now
Anthony Elder
 */
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <home_wifi_multi.h>  //#include <credentials.h>

extern "C" {
  #include "user_interface.h"
  #include <espnow.h>
}

/* Set a private Mac Address
 *  http://serverfault.com/questions/40712/what-range-of-mac-addresses-can-i-safely-use-for-my-virtual-machines
 * Note: the point of setting a specific MAC is so you can replace this Gateway ESP8266 device with a new one
 * and the new gateway will still pick up the remote sensors which are still sending to the old MAC 
 */
//uint8_t mac[] =  MAC1;  //{0x36, 0x33, 0x33, 0x33, 0x33, 0x33};
void initVariant() {
  //WiFi.mode(WIFI_AP);
  //wifi_set_macaddr(SOFTAP_IF, &mac[0]);
}

char *ssid      = SSID1;               // Set you WiFi SSID
char *password  = PWD1;               // Set you WiFi password

WiFiClient wifiClient;

String deviceMac;

// keep in sync with ESP_NOW sensor struct
struct __attribute__((packed)) SENSOR_DATA {
   char testdata[240];
} sensorData;

volatile boolean haveReading = false;

/* Presently it doesn't seem posible to use both WiFi and ESP-NOW at the
 * same time. This gateway gets around that be starting just ESP-NOW and
 * when a message is received switching on WiFi to sending the MQTT message
 * to Watson, and then restarting the ESP. The restart is necessary as 
 * ESP-NOW doesn't seem to work again even after WiFi is disabled.
 * Hopefully this will get fixed in the ESP SDK at some point.
 */

int heartBeat;


void setup() {
  //Serial.begin(115200); 
  //Serial.println();
  //Serial.println("ESP_Now Controller");
  //Serial.println();

  WiFi.mode(WIFI_STA);
//  Serial.print("This node AP mac: "); Serial.println(WiFi.softAPmacAddress());
  //Serial.print("This node STA mac: "); Serial.println(WiFi.macAddress());
  Wire.begin(1, 2); /* join i2c bus with SDA=D1 and SCL=D2 of NodeMCU */

  initEspNow();  
  //Serial.println("Setup done");
}


void loop() {
  if (millis()-heartBeat > 30000) {
    //Serial.println("Waiting for ESP-NOW messages...");
    //Serial.println();
    //Serial.print("Loop - ESP Board MAC Address:  ");
    //Serial.println(WiFi.macAddress());
    heartBeat = millis();
  }

  if (haveReading == 1) {
    //Serial.println("Have Reading");
    haveReading = false;
    //Send I2C
    Wire.beginTransmission(2);
    Wire.write(sensorData.testdata);
    Wire.endTransmission();
  }
}

void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&sensorData, incomingData, sizeof(sensorData));
  //Serial.print("Bytes received: ");
  //Serial.println(len);
  //Serial.print("testdata: ");
  //Serial.println(sensorData.testdata);
  haveReading = true;
  //Serial.print("haveReading: ");
  //Serial.println(haveReading);

}

void initEspNow() {
  if (esp_now_init()!=0) {
    //Serial.println("*** ESP_Now init failed");
    ESP.restart();
  }

  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(OnDataRecv);
/*  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);

  esp_now_register_recv_cb([](uint8_t *mac, uint8_t *data, uint8_t len) {

    deviceMac = "";
    deviceMac += String(mac[0], HEX);
    deviceMac += String(mac[1], HEX);
    deviceMac += String(mac[2], HEX);
    deviceMac += String(mac[3], HEX);
    deviceMac += String(mac[4], HEX);
    deviceMac += String(mac[5], HEX);
    
    memcpy(&sensorData, data, sizeof(sensorData));

    Serial.print("Message received from device: "); Serial.print(deviceMac);
    Serial.println(sensorData.testdata);
    Serial.println();
//    Serial.printf(" Temp=%0.1f, Hum=%0.0f%%, pressure=%0.0fmb\n", 
//       sensorData.temp, sensorData.humidity, sensorData.pressure);    

    haveReading = true;
  });
  */
}
