/* Touch sensor code based on ESP32 example
   ESP-NOW Broadcast Master
   Lucas Saavedra Vaz - 2024 */

#include "ESP32_NOW.h"
#include "WiFi.h"
#include <esp_mac.h>  // For the MAC2STR and MACSTR macros

/* Definitions */
#define ESPNOW_WIFI_CHANNEL 1
#define TOUCH_THRESHOLD 40
#define DEBOUNCE_DELAY 100 // 100 milliseconds

/* Classes */
// Creating a new class that inherits from the ESP_NOW_Peer class is required.
class ESP_NOW_Broadcast_Peer : public ESP_NOW_Peer {
  public:
    // Constructor of the class using the broadcast address
    ESP_NOW_Broadcast_Peer(uint8_t channel, wifi_interface_t iface, const uint8_t *lmk) : ESP_NOW_Peer(ESP_NOW.BROADCAST_ADDR, channel, iface, lmk) {}
    // Destructor of the class
    ~ESP_NOW_Broadcast_Peer() {
      remove();
    }
    // Function to properly initialize the ESP-NOW and register the broadcast peer
    bool begin() {
      if (!ESP_NOW.begin() || !add()) {
        log_e("Failed to initialize ESP-NOW or register the broadcast peer");
        return false;
      }
      return true;
    }
    // Function to send a message to all devices within the network
    bool send_message(const uint8_t *data, size_t len) {
      if (!send(data, len)) {
        log_e("Failed to broadcast message");
        return false;
      }
      return true;
    }
};

/* Global Variables */
uint32_t msg_count = 0;
// Create a broadcast peer object
ESP_NOW_Broadcast_Peer broadcast_peer(ESPNOW_WIFI_CHANNEL, WIFI_IF_STA, NULL);
volatile bool touch1detected = false;
volatile bool touch2detected = false;
unsigned long lastDebounceTime_T2 = 0;
unsigned long lastDebounceTime_T3 = 0;

/* Function Prototypes */
void sendESPNowData();
void handleTouchInput();
void debounceTouch1();
void debounceTouch2();

/* Interrupt Service Routines */
void IRAM_ATTR gotTouch1() {
  touch1detected = true;
  lastDebounceTime_T2 = millis();
}

void IRAM_ATTR gotTouch2() {
  touch2detected = true;
  lastDebounceTime_T3 = millis();
}

/* Main */
void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }

  touchAttachInterrupt(T2, gotTouch1, TOUCH_THRESHOLD);
  touchAttachInterrupt(T3, gotTouch2, TOUCH_THRESHOLD);

  // Initialize the Wi-Fi module
  WiFi.mode(WIFI_STA);
  WiFi.setChannel(ESPNOW_WIFI_CHANNEL);
  while (!WiFi.STA.started()) {
    delay(100);
  }

  Serial.println("ESP-NOW Example - Broadcast Master");
  Serial.println("Wi-Fi parameters:");
  Serial.println("  Mode: STA");
  Serial.println("  MAC Address: " + WiFi.macAddress());
  Serial.printf("  Channel: %d\n", ESPNOW_WIFI_CHANNEL);

  // Register the broadcast peer
  if (!broadcast_peer.begin()) {
    Serial.println("Failed to initialize broadcast peer");
    Serial.println("Rebooting in 5 seconds...");
    delay(5000);
    ESP.restart();
  }

  Serial.println("Setup complete. Broadcasting messages every 5 seconds.");
}

void loop() {
  debounceTouch1();
  debounceTouch2();
  handleTouchInput();
  delay(100);
}

void debounceTouch1() {
  if (touch1detected && (millis() - lastDebounceTime_T2) > DEBOUNCE_DELAY) {
    touch1detected = false;  // Reset detected flag
    Serial.println("Touch 1 detected");
    msg_count = 2;
    sendESPNowData();
  }
}

void debounceTouch2() {
  if (touch2detected && (millis() - lastDebounceTime_T3) > DEBOUNCE_DELAY) {
    touch2detected = false;  // Reset detected flag
    Serial.println("Touch 2 detected");
    msg_count = 3;
    sendESPNowData();
  }
}

void handleTouchInput() {
  // Nothing needed here since debounce functions now handle sending data
}

void sendESPNowData() {
  char data[32];
  snprintf(data, sizeof(data), "bbl|%lu", msg_count);
  Serial.printf("Broadcasting message: %s\n", data);

  if (!broadcast_peer.send_message((uint8_t *)data, sizeof(data))) {
    Serial.println("Failed to broadcast message");
  }

  // Delay for 0.5 seconds
  delay(500);

  // Clear touch interrupt status by reading the touch pins
  touchRead(T2);
  touchRead(T3);
}


// #include "ESP32_NOW.h"
// #include "WiFi.h"

// #include <esp_mac.h>  // For the MAC2STR and MACSTR macros

// /* Definitions */

// #define ESPNOW_WIFI_CHANNEL 1

// /* Classes */

// // Creating a new class that inherits from the ESP_NOW_Peer class is required.

// class ESP_NOW_Broadcast_Peer : public ESP_NOW_Peer {
// public:
//   // Constructor of the class using the broadcast address
//   ESP_NOW_Broadcast_Peer(uint8_t channel, wifi_interface_t iface, const uint8_t *lmk) : ESP_NOW_Peer(ESP_NOW.BROADCAST_ADDR, channel, iface, lmk) {}

//   // Destructor of the class
//   ~ESP_NOW_Broadcast_Peer() {
//     remove();
//   }

//   // Function to properly initialize the ESP-NOW and register the broadcast peer
//   bool begin() {
//     if (!ESP_NOW.begin() || !add()) {
//       log_e("Failed to initialize ESP-NOW or register the broadcast peer");
//       return false;
//     }
//     return true;
//   }

//   // Function to send a message to all devices within the network
//   bool send_message(const uint8_t *data, size_t len) {
//     if (!send(data, len)) {
//       log_e("Failed to broadcast message");
//       return false;
//     }
//     return true;
//   }
// };

// /* Global Variables */

// uint32_t msg_count = 0;

// // Create a broadcast peer object
// ESP_NOW_Broadcast_Peer broadcast_peer(ESPNOW_WIFI_CHANNEL, WIFI_IF_STA, NULL);

// int threshold = 30;
// bool touch1detected = false;
// bool touch2detected = false;

// void gotTouch1() {
//   touch1detected = true;
// }

// void gotTouch2() {
//   touch2detected = true;
// }

// /* Main */

// void setup() {
//   Serial.begin(115200);
//   while (!Serial) {
//     delay(10);
//   }

//   touchAttachInterrupt(T2, gotTouch1, threshold);
//   touchAttachInterrupt(T3, gotTouch2, threshold);


//   // Initialize the Wi-Fi module
//   WiFi.mode(WIFI_STA);
//   WiFi.setChannel(ESPNOW_WIFI_CHANNEL);
//   while (!WiFi.STA.started()) {
//     delay(100);
//   }

//   Serial.println("ESP-NOW Example - Broadcast Master");
//   Serial.println("Wi-Fi parameters:");
//   Serial.println("  Mode: STA");
//   Serial.println("  MAC Address: " + WiFi.macAddress());
//   Serial.printf("  Channel: %d\n", ESPNOW_WIFI_CHANNEL);

//   // Register the broadcast peer
//   if (!broadcast_peer.begin()) {
//     Serial.println("Failed to initialize broadcast peer");
//     Serial.println("Reebooting in 5 seconds...");
//     delay(5000);
//     ESP.restart();
//   }

//   Serial.println("Setup complete. Broadcasting messages every 5 seconds.");
// }

// void loop() {
//   if ((touch1detected) || (touch2detected)) {
//     // Disable interrupts 
//     noInterrupts(); 

//     if (touch1detected) {
//       touch1detected = false;
//       Serial.println("Touch 1 detected");
//       msg_count = 2;
//     } else {
//       touch2detected = false;
//       Serial.println("Touch 2 detected");
//       msg_count = 3;
//     }
//     char data[32];
//     snprintf(data, sizeof(data), "bbl|%lu", msg_count);

//     Serial.printf("Broadcasting message: %s\n", data);

//     if (!broadcast_peer.send_message((uint8_t *)data, sizeof(data))) {
//       Serial.println("Failed to broadcast message");
//     }
    
//     // Delay for 0.5 seconds 
//     delay(500); 
  
//     // Clear touch interrupt status by reading the touch pins 
//     touchRead(T2); 
//     touchRead(T3);

//     // Enable interrupts 
//     interrupts();
    
//   }
// }
/////////////////////////////////////////////////////////////////////////////////
// #define DEBUG 1

// #if DEBUG == 2
//   #define debugln(x) Serial1.println(x)
//   #define debug(x) Serial1.print(x)
//   #define debugBegin(x) Serial1.begin(x)
//   #define serial Serial1
//   #define LED_PIN      99 
//   //Deep Sleep time in microseconds
//   #define sleepTime 120e6  //2min
// #elif DEBUG == 1
//   #define debugln(x) Serial.println(x) 
//   #define debug(x) Serial.print(x) 
//   #define debugBegin(x) Serial.begin(x)
//   #define serial Serial
//   #define LED_PIN       LED_BUILTIN 
//   //Deep Sleep time in microseconds
//   #define sleepTime 30e6  //30 sec
// #else
//   #define serial true
//   #define debugln(x)
//   #define debug(x)
//   #define debugBegin(x)
//   #define LED_PIN       99
//   //Deep Sleep time in microseconds
//   #define sleepTime 900e6  //15min
// #endif
  

// #ifdef ESP32
//   #include <WiFi.h>
//   #include <esp_now.h>
//   #define BOARD "ESP32"
// #else
//   #include <ESP8266WiFi.h>
//   #include <espnow.h>
//   #define BOARD "ESP12"
// #endif

// #define ESPNOW_WIFI_CHANNEL 6

// // Structure example to send data
// // Must match the receiver structure
// typedef struct struct_message {
//   char a[240];
// } struct_message;

// // Create a struct_message called myData
// struct_message myData;
// String sendMsg;

// unsigned long lastTime = 0;  
// #define unsigned long timerDelay = 10000;  // send readings timer

// /* Classes */

// // Creating a new class that inherits from the ESP_NOW_Peer class is required.

// class ESP_NOW_Broadcast_Peer : public ESP_NOW_Peer {
// public:
//   // Constructor of the class using the broadcast address
//   ESP_NOW_Broadcast_Peer(uint8_t channel, wifi_interface_t iface, const uint8_t *lmk) : ESP_NOW_Peer(ESP_NOW.BROADCAST_ADDR, channel, iface, lmk) {}

//   // Destructor of the class
//   ~ESP_NOW_Broadcast_Peer() {
//     remove();
//   }

//   // Function to properly initialize the ESP-NOW and register the broadcast peer
//   bool begin() {
//     if (!ESP_NOW.begin() || !add()) {
//       log_e("Failed to initialize ESP-NOW or register the broadcast peer");
//       return false;
//     }
//     return true;
//   }

//   // Function to send a message to all devices within the network
//   bool send_message(const uint8_t *data, size_t len) {
//     if (!send(data, len)) {
//       log_e("Failed to broadcast message");
//       return false;
//     }
//     return true;
//   }
// };

// /* Global Variables */

// uint32_t msg_count = 0;

// // Create a broadcast peer object
// ESP_NOW_Broadcast_Peer broadcast_peer(ESPNOW_WIFI_CHANNEL, WIFI_IF_STA, NULL);

// // Callback when data is sent
// void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
//   debug("Last Packet Send Status: ");
//   debugln(sendStatus);
//   if (sendStatus == 0){
//     debugln("Delivery success");
//   }
//   else{
//     debugln("Delivery fail");
//     // digitalWrite(LED_PIN, HIGH);
//   }
// }

// int threshold = 40;
// bool touch1detected = false;
// bool touch2detected = false;

// void gotTouch1() {
//   touch1detected = true;
// }

// void gotTouch2() {
//   touch2detected = true;
// }

// void setup() {
//   debugBegin(115200);
//    while (!serial);
//   debugln();
//   debugln("Set Up");
//   debugln("ESP32 Touch Interrupt Test");
//   touchAttachInterrupt(T2, gotTouch1, threshold);
//   touchAttachInterrupt(T3, gotTouch2, threshold);

//   // Initialize the Wi-Fi module
//   WiFi.mode(WIFI_STA);
//   WiFi.setChannel(ESPNOW_WIFI_CHANNEL);
//   while (!WiFi.STA.started()) {
//     delay(100);
//   }
//   // esp_now_register_send_cb(OnDataSent);

//   debugln("ESP-NOW Example - Broadcast Master");
//   debugln("Wi-Fi parameters:");
//   debugln("  Mode: STA");
//   debugln("  MAC Address: " + WiFi.macAddress());
//   debugf("  Channel: %d\n", ESPNOW_WIFI_CHANNEL);

//   // Register the broadcast peer
//   if (!broadcast_peer.begin()) {
//     debugln("Failed to initialize broadcast peer");
//     debugln("Reebooting in 5 seconds...");
//     delay(5000);
//     ESP.restart();
//   }

//   debugln("Setup complete. Broadcasting messages every 5 seconds.");

// }

// void sendESPNowData(int touchSensor) {
//   debugln("Sending ESP-NOW data");

//   // Set values to send
//   // strcpy(myData.a, BOARD);

//   // digitalWrite(LED_PIN, LOW);

//   snprintf(myData.a, sizeof(myData.a), "%s|%d", "bbl", touchSensor);
//   debug("Data to send: ");
//   debugln(myData.a);

//   esp_err_t sendResult = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

//   if (sendResult == ESP_OK) {
//     debugln("Sent with success");
//   } else {
//     debugln("Error sending the data");
//   }

// }


// void loop() {
//   // strcpy(myData.a,BOARD);
//   // char result[8];

//   if ((touch1detected) || (touch2detected)) {
//     if (touch1detected) {
//       touch1detected = false;
//       debugln("Touch 1 detected");
//       sendESPNowData(2);
//     } else {
//       touch2detected = false;
//       debugln("Touch 2 detected");
//       sendESPNowData(3);
//     }
//   }
// }
