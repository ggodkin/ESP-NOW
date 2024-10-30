#include <ESP8266WiFi.h>
#include <espnow.h>

#define GPIO13 13
#define GPIO12 12

typedef struct struct_message {
  char a[240];
} struct_message;

struct_message myData;

void setup() {
  Serial.begin(9600);
  delay(1000); // Ensure this matches your serial monitor setting

  pinMode(GPIO13, OUTPUT);
  pinMode(GPIO12, INPUT);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    delay(1000);
    return;
  }

  uint8_t broadcastAddress[] = {0x2C,0xF4,0x32,0x20,0x5D,0x1C}; // Replace with your receiver's MAC address
  if (esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_CONTROLLER, 0, NULL, 0) != 0) {
    Serial.println("Failed to add peer");
    delay(1000);
    return;
  } else {
    Serial.println("Added peer");
  }

  digitalWrite(GPIO13, HIGH);

  unsigned long startTime = millis();

  int gpio12_status = digitalRead(GPIO12);
  int adc_reading = analogRead(A0);

  // Debugging output
  Serial.print("GPIO12 Status: ");
  Serial.println(gpio12_status);
  Serial.print("ADC Reading: ");
  Serial.println(adc_reading);
  delay(1000);

  snprintf(myData.a, sizeof(myData.a), "%d|%d", gpio12_status, adc_reading);

  // Debugging output
  Serial.print("Data to send: ");
  Serial.println(myData.a);
  delay(1000);

  esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
  delay(1000);

  if (millis() - startTime < 5000) {
    digitalWrite(GPIO13, LOW);
  } else {
    digitalWrite(GPIO13, LOW);
  }
}

void loop() {
  Serial.println("Loop running...");
  delay(1000); // Add a delay to avoid flooding the serial monitor
}

// 0x2C,0xF4,0x32,0x20,0x5D,0x1C