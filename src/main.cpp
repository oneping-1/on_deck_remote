#include <Arduino.h>
#include "LedController.hpp"
#include <cmath>

#define DIN_PIN 12
#define CLK_PIN 14
#define CS_PIN 13
#define SEGMENT_COUNT 1   // Change if you have multiple chained MAX7219 chips

LedController lc(DIN_PIN, CLK_PIN, CS_PIN, SEGMENT_COUNT, false);  // software SPI

void setup() {
  Serial.begin(115200);
  delay(1000);  // let the serial console catch up

  Serial.println("Starting segment scan test...");

  lc.init(DIN_PIN, CLK_PIN, CS_PIN, SEGMENT_COUNT, false);
  lc.setIntensity(15);  // max brightness
  lc.clearMatrix();

  lc.
}

void loop() {

    // Apply pattern to all 4 digits
    for (int digit = 3; digit >= 0; digit--) {
      for (int bit = 7; bit >= 0; bit--){
        byte pattern = pow(2, bit) ;  // Create a pattern with the current bit set
        lc.setRow(0, digit, pattern);
        delay(1000);
      }
      lc.setRow(0, digit, 0);  // Clear the row after displaying the pattern
    }

}
