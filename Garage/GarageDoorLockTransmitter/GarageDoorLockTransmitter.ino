/*
ESP-NOW test
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
  #define BOARD "ESP12"
#endif
// #include <home_wifi_multi.h> 

//#include <Wire.h>              // Wire library (required for I2C devices)

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
}
 
void setup() {
  // Init Serial Monitor
  debugBegin(115200);

  while (!serial);
  debugln();
  debugln("Set Up");

  pinMode(LED_PIN, OUTPUT);
  pinMode(14, OUTPUT);  // Ground ADC resistive divider
  pinMode(12, INPUT);

  digitalWrite(14, HIGH); // will be set to low to provide ADC input
  debugln("setup completed");
}
 
void loop() {
    long ellapsedTime = millis();
    float batVolt;
    debug("Loop Enter Time: ");
    debugln(ellapsedTime);
     

    // Set values to send
    strcpy(myData.a,BOARD);
    char result[8];


    ellapsedTime = millis();
    debug("Temp collected Time: ");
    debugln(ellapsedTime);    
    digitalWrite(LED_PIN, LOW);
    digitalWrite(14, LOW);

    batVolt = analogRead(A0);

    batVolt = batVolt * 5 / 1000;
    debug("Battery Voltage :");
    debugln(batVolt);
    // strcat(myData.a,"|BatV|");
    // dtostrf(batVolt, 6, 2, result);
    // strcat(myData.a,result);
    // ellapsedTime = millis();
    // debug("Battery Voltage collected Time: ");
    // debugln(ellapsedTime);    

    int gpio12_status = digitalRead(12);
  // Debugging output
  Serial.print("GPIO12 Status: ");
  Serial.println(gpio12_status);
  digitalWrite(14, HIGH);

  snprintf(myData.a, sizeof(myData.a), "%S|%d|%f", "glock", gpio12_status, batVolt);

  // Debugging output
  Serial.print("Data to send: ");
  Serial.println(myData.a);
    
  WiFi.mode(WIFI_STA);
//  while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
//    //debugln("Connecting to WiFi...");
//    //TODO if counter grreater than threshold go to deep sleep
//  }
  debug("WiFi.status(): ");
  debugln(WiFi.status());

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    debugln("Error initializing ESP-NOW");
    // digitalWrite(13, LOW); //put to sleep driving MCU
    // return;
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
    
    ellapsedTime = millis();
    debug("Loop Exit Time: ");
    debugln(ellapsedTime);
    // digitalWrite(13, LOW); //put to sleep driving MCU
    ESP.deepSleep(sleepTime);
    
    lastTime = millis();
  //}
}

