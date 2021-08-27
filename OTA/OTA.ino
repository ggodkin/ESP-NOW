//
// Change log:

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include <PubSubClient.h>

#if !defined(ESP32) && !defined(ESP8266)
  #error This sketch works only for the ESP8266 and ESP32
#endif

// File in your library path to hold setings not to be shared with public
// in the following format
// #define SSID1 "Enter your SSID"
// #define PWD1 "Enter your network pasword"
// it will be extended with MQTT credentials

#include <home_wifi_multi.h>
const char* ssid = SSID1;
const char* password = PWD1;

static const char SSID[] = SSID1;
static const char PASSWORD[] = PWD1;

const char* mqtt_server = mqttServer1;
const int mqttPort = mqttPort1;
const char* mqttUser = mqttUser1;
const char* mqttPassword = mqttPassword1;

#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
char msgOut[MSG_BUFFER_SIZE];
int value = 0;
String msgStr;

const int durationOTA = 0;

WiFiClient espClient;
PubSubClient client(espClient);

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqttUser1, mqttPassword1)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("ESP-NOWbridge", "Connected");
      // ... and resubscribe
      client.subscribe("cmnd/NeopixelClock/GarageDoorClosed");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {

  Serial.begin(115200);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    delay(5000);
    ESP.restart();
  }

  
  ArduinoOTA.begin();
  
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if ((millis() < durationOTA) || (durationOTA == 0)) {
      ArduinoOTA.handle();
  }

 
}



 
