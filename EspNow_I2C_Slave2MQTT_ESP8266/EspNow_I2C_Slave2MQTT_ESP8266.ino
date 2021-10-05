/**
 * 
 * Author: Andreas Spiess, 2017
 * 
  This sketch receives ESP-Now message and sends it as an MQTT messge
  It is heavily based on of Anthony's gateway setch sketch

https://github.com/HarringayMakerSpace/ESP-Now
Anthony Elder
 */
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <home_wifi_multi.h>  //#include <credentials.h>
#include <Wire.h>

extern "C" {
  #include "user_interface.h"
  #include <espnow.h>
}


//-------- Customise the above values --------
#define SENDTOPIC "ESPNow/key"
#define COMMANDTOPIC "ESPNow/command"
#define SERVICETOPIC "ESPNow/service"

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


IPAddress server(mqttServer1);


// the X's get replaced with the remote sensor device mac address
const char deviceTopic[] = "ESPNOW/";

WiFiClient wifiClient;
PubSubClient client(server, 1883, wifiClient);

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
  Serial.begin(115200); 
  Serial.println();
  Serial.println("I2C Slave to MQTT");
  Serial.println();

  WiFi.mode(WIFI_STA);
  Wire.begin(2);                /* join i2c bus with address 2 */
  Wire.onReceive(receiveEvent); /* register receive event */
  wifiConnect();
  reconnectMQTT();
  Serial.println("Setup done");
}


void loop() {
  if (millis()-heartBeat > 30000) {
    Serial.println("Waiting for I2C messages...");
    Serial.println();
    heartBeat = millis();
  }

  if (haveReading == 1) {
    Serial.println("Have Reading");
    haveReading = false;
    sendToNodeRed();
    client.disconnect();
    Serial.println("Disconnected");
  }
}

void sendToNodeRed() {

  Serial.println(sensorData.testdata);
  String payload = "{";
  payload += "\"test\":\"" + String(sensorData.testdata);
  payload += "\"}";
  Serial.println(payload);
  publishMQTT(SENDTOPIC,payload);
}

// function that executes whenever data is received from master
void receiveEvent(int howMany) {
 while (0 <Wire.available()) {
    char c = Wire.read();      /* receive byte as a character */
    Serial.print(c);           /* print the character */
  }
 Serial.println();             /* to newline */
}

void wifiConnect() {
  WiFi.mode(WIFI_STA);
  Serial.println();
  Serial.print("Connecting to "); Serial.print(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
     delay(250);
     Serial.print(".");
  }  
  Serial.print("\nWiFi connected, IP address: "); Serial.println(WiFi.localIP());
}

void publishMQTT(String topic, String message) {
  Serial.println("Publish");
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.publish(SENDTOPIC, message.c_str());
}

void reconnectMQTT() {
  Serial.println(" Loop until we're reconnected");
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("Mailbox",  mqttUser1, mqttPassword1)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(SERVICETOPIC, "I am live");
      // ... and resubscribe
      //  client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc = ");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
