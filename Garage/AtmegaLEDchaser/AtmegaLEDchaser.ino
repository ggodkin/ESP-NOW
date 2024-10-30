#include <Adafruit_NeoPixel.h>

#define PIN 6 // Pin where the WS2812 strip is connected
#define NUM_LEDS 8 // Total number of LEDs in the strip
#define CHASE_SPEED 100 // Speed of the chase effect (in milliseconds)

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  setInitialColors();
}

void loop() {
  chaseEffect(CHASE_SPEED);
}

void setInitialColors() {
  for (int i = 0; i < NUM_LEDS / 2; i++) {
    strip.setPixelColor(i, strip.Color(255, 0, 0)); // Red
  }
  for (int i = NUM_LEDS / 2; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 255)); // Blue
  }
  strip.show();
}

void chaseEffect(int speed) {
  uint32_t colors[NUM_LEDS];
  for (int i = 0; i < NUM_LEDS / 2; i++) {
    colors[i] = strip.Color(255, 0, 0); // Red
  }
  for (int i = NUM_LEDS / 2; i < NUM_LEDS; i++) {
    colors[i] = strip.Color(0, 0, 255); // Blue
  }

  while (true) {
    for (int i = 0; i < NUM_LEDS; i++) {
      strip.setPixelColor(i, colors[i]);
    }
    strip.show();
    delay(speed);

    // Shift colors
    uint32_t lastColor = colors[NUM_LEDS - 1];
    for (int i = NUM_LEDS - 1; i > 0; i--) {
      colors[i] = colors[i - 1];
    }
    colors[0] = lastColor;
  }
}
