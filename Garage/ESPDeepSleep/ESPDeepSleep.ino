#include "esp_sleep.h"
#include "driver/rtc_io.h"

#define HALL_SENSOR_PIN GPIO_NUM_33  // Change this to the pin connected to your Hall sensor

void setup() {
  Serial.begin(115200);
  delay(1000); // Give some time to open the serial monitor

  // Configure the Hall sensor pin as an RTC GPIO
  rtc_gpio_deinit(HALL_SENSOR_PIN);
  rtc_gpio_init(HALL_SENSOR_PIN);
  rtc_gpio_set_direction(HALL_SENSOR_PIN, RTC_GPIO_MODE_INPUT_ONLY);

  // Configure the wake-up source
  esp_sleep_enable_ext0_wakeup(HALL_SENSOR_PIN, 0); // Wake up when the pin goes low

  Serial.println("Going to sleep now");
  delay(1000);
  esp_deep_sleep_start();
}

void loop() {
  // This will not be reached after deep sleep
}
