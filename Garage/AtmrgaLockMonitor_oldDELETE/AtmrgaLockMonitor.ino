#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

#define REED_SWITCH_PIN 2
#define TRANSMITTER_ENABLE_PIN 3
// #define TRANSMISSION_COMPLETE_PIN 4

volatile bool reedSwitchTriggered = false;

void setup() {
  //Reduce power consumpion
  setAllPinsLow();
  disableADC();
  disableAnalogComparator();
  disableInternalVoltageReference();
  disableBrownOutDetector();
  // Disable SPI
  power_spi_disable();

  // Disable UART
  power_usart0_disable();

  // Disable TWI (I2C)
  power_twi_disable();

  // Disable Watchdog Timer
  wdt_disable();
  pinMode(REED_SWITCH_PIN, INPUT_PULLUP);
  pinMode(TRANSMITTER_ENABLE_PIN, OUTPUT);
  // pinMode(TRANSMISSION_COMPLETE_PIN, INPUT_PULLUP);

  // Attach interrupt to reed switch pin
  attachInterrupt(digitalPinToInterrupt(REED_SWITCH_PIN), reedSwitchISR, CHANGE);

  // Ensure transmitter is off initially
  digitalWrite(TRANSMITTER_ENABLE_PIN, LOW);
  digitalWrite(TRANSMITTER_ENABLE_PIN, HIGH);

  // Enable sleep mode
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
}

void loop() {
  if (reedSwitchTriggered) {
    // Enable the external transmitter
    digitalWrite(TRANSMITTER_ENABLE_PIN, HIGH);
    digitalWrite(TRANSMITTER_ENABLE_PIN, LOW);
 
    // Wait for transmission MCU to initialize
    delay(30);
    // Enable the external transmitter
   digitalWrite(TRANSMITTER_ENABLE_PIN, HIGH);
 

    // Reset the flag
    reedSwitchTriggered = false;

    // Clear any pending interrupts
    EIFR |= (1 << INTF0);

    // Re-attach the interrupt
    attachInterrupt(digitalPinToInterrupt(REED_SWITCH_PIN), reedSwitchISR, CHANGE);
  }

  // Enter sleep mode
  sleep_enable();
  sei(); // Ensure global interrupts are enabled
  sleep_cpu();

  // MCU will continue from here after waking up
  sleep_disable();
}

// Interrupt Service Routine for reed switch
void reedSwitchISR() {
  reedSwitchTriggered = true;
  // Wake up the MCU
  sleep_disable();
  detachInterrupt(digitalPinToInterrupt(REED_SWITCH_PIN));
}


void setAllPinsLow() {
  // Loop through all digital pins
  for (uint8_t pin = 0; pin < 22; pin++) {
    pinMode(pin, OUTPUT); // Set pin as output
    digitalWrite(pin, LOW); // Set pin to LOW
  }
}

void disableADC() {
  ADCSRA &= ~(1 << ADEN); // Clear the ADEN bit to disable the ADC
}

void disableAnalogComparator() {
  ACSR |= (1 << ACD); // Set the ACD bit in the ACSR register to disable the analog comparator
}

void disableInternalVoltageReference() {
  ADMUX &= ~((1 << REFS1) | (1 << REFS0)); // Clear REFS1 and REFS0 bits to disable internal voltage reference
}

void disableBrownOutDetector() {
  // Disable interrupts
  cli();
  
  // Temporarily enable change of BOD settings
  MCUCR |= (1 << BODSE);
  
  // Disable BOD
  MCUCR |= (1 << BODS);
  
  // Re-enable interrupts
  sei();
}

