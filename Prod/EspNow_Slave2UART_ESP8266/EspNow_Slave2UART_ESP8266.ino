/* Receive ESP-Now transmission and pass it to UART1
 *  
 */

#include <ESP8266WiFi.h>
#include <home_wifi_multi.h>  //#include <credentials.h>

#define DEBUG 1

#if DEBUG == 1
  #define debug(x) Serial1.print(x)
  #define debugln(x) Serial1.println(x)
#else
  #define debug(x)
  #define debugln(x)
#endif
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

// keep in sync with ESP_NOW sensor struct
struct __attribute__((packed)) SENSOR_DATA {
   char testdata[240];
} sensorData;

volatile boolean haveReading = false;

int heartBeat;

// Callback function that will be executed when data is received
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&sensorData, incomingData, sizeof(sensorData));
  debug("Bytes received: ");
  debugln(len);
  debug("testdata: ");
  debugln(sensorData.testdata);
  haveReading = 1;
}

void setup() {
  Serial.begin(115200); 
  Serial1.begin(115200); //Debug serial
  debugln();
  debugln("ESPMOW_Slave2UART");

  WiFi.mode(WIFI_STA);
  // Init ESP-NOW
  if (esp_now_init() != 0) {
    debugln("Error initializing ESP-NOW");
    return;
  } 
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(OnDataRecv);
  debugln("Setup done");
}


void loop() {
  if (millis()-heartBeat > 30000) {
    debugln("Waiting for ESP-Now messages...");
    debugln();
    heartBeat = millis();
  }

  if (haveReading == 1) {
    debugln("Have Reading");
    haveReading = false;
    sendToUART0();
  }
}

void sendToUART0() {

  debugln(sensorData.testdata);
  String payload = sensorData.testdata;
  Serial.println(payload);
}
