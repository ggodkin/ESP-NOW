/* Receive ESP-Now transmission and pass it to UART1
 
   Inspired by:

  Andreas Spiess
  https://github.com/HarringayMakerSpace/ESP-Now/blob/master/EspNowWatsonRestartingGateway/EspNowWatsonRestartingGateway.ino
  
  Anthony Elder
  https://github.com/HarringayMakerSpace/ESP-Now
  
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp-now-esp8266-nodemcu-arduino-ide/

  https://github.com/RalphBacon/223-ESP-NOW
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <home_wifi_multi.h>  //#include <credentials.h>

#define SENDTOPIC "ESPNow/key"
#define SERVICETOPIC "ESPNow/service"

#define DEBUG 1

#if DEBUG == 1
  #define debug(x) Serial1.print(x)
  #define debugln(x) Serial1.println(x)
#else
  #define debug(x)
  #define debugln(x)
#endif

char *ssid      = SSID1;               // Set you WiFi SSID
char *password  = PWD1;               // Set you WiFi password

IPAddress server(mqttServer1);

WiFiClient wifiClient;
PubSubClient client(server, mqttPort1, wifiClient);

volatile boolean haveReading = false;
String inData;
String msgMQTT;

int heartBeat;
int rcvIterations = 0;

void wifiConnect() {
  WiFi.mode(WIFI_AP);
  debugln();
  debug("Connecting to "); debug(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
     delay(250);
     debug(".");
  }  
  debug("\nWiFi connected, IP address: "); debugln(WiFi.localIP());
}

void reconnectMQTT() {
  debugln(" Loop until we're reconnected");
  while (!client.connected()) {
    debug("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("Mailbox",  mqttUser1, mqttPassword1)) {
      debugln("connected");
      // Once connected, publish an announcement...
      client.publish(SERVICETOPIC, "I am live");
      // ... and resubscribe
      //  client.subscribe("inTopic");
    } else {
      debug("failed, rc = ");
      debug(client.state());
      debugln(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void setup() {
  Serial.begin(115200); 
  Serial1.begin(115200); //Debug serial
  debugln();
  debugln("Setup ESPMOW_UART2MQTT");

  wifiConnect();
  if (WiFi.status() == WL_CONNECTED) {
    reconnectMQTT();
  }  
  debugln("Setup Completed");
  
}

void loop() {
  if (millis()-heartBeat > 30000) {
    debugln("Waiting for UART messages...");
    debugln();
    heartBeat = millis();
  }
  
  rcvIterations = 0;
  
  while (Serial.available() > 0){
    char recieved = Serial.read();
    // Process message when new line character is recieved
    if (recieved == '\n'){
      debug("ESP01 Received: ");
      debugln(inData);
      haveReading = true;
      msgMQTT = inData;
      msgMQTT.trim();
      inData = ""; // Clear recieved buffer
    } else {
      inData += recieved; 
      rcvIterations ++;
    }
    if (rcvIterations > 240) {
      debugln("Missing EOL");
      inData = ""; // Clear recieved buffer
      return;
    }
  }
  
  if (haveReading) {
    debug("Have reading: ");
    sendToNodeRed(msgMQTT);
    haveReading = false;
  }
}

void sendToNodeRed(String strData) {
  debugln(strData);
  debug("Length: "); debugln(strData.length());
  publishMQTT(SENDTOPIC,strData);
}

void publishMQTT(String topic, String message) {
  debugln("Publish");
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.publish(SENDTOPIC, message.c_str());
}
