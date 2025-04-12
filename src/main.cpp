#include <Arduino.h>
#include "LedController.hpp"
#include <cmath>

#define DIN_PIN 12
#define CLK_PIN 14
#define CS_PIN 13
#define SEGMENT_COUNT 1   // Change if you have multiple chained MAX7219 chips

LedController lc(DIN_PIN, CLK_PIN, CS_PIN, SEGMENT_COUNT, false);  // software SPI

byte numberArray[10] = {
  B01111110, // 0 
  B00110000, // 1
  B01101101, // 2
  B01111001, // 3
  B00110011, // 4
  B01011011, // 5
  B01011111, // 6
  B01110000, // 7
  B01111111, // 8
  B01111011  // 9
};

void setup() {
  Serial.begin(115200);
  delay(1000);  // let the serial console catch up

  Serial.println("Starting segment scan test...");

  lc.init(DIN_PIN, CLK_PIN, CS_PIN, SEGMENT_COUNT, false);
  lc.setIntensity(5);  // max brightness
  lc.clearMatrix();
}

void print_number(int8_t num){
  if (num < 0){
    lc.setRow(0, 0, B00000001);
    num = num * -1;
  }
  else {
    lc.setRow(0, 0, B00000000);
  }

  bool leadingZero = false;
  for (int i = 2; i >= 0; i--){
    int divisor = pow(10, i);
    int8_t digit = num / divisor;
    num %= divisor;
    // lc.setDigit(0, 4-i, numberArray[digit], false);
    Serial.println(i, digit);
    if ((digit == 0) and (!leadingZero)){
      lc.setRow(0, 3-i, B00000000);
    }
    else if ((digit != 0) and (!leadingZero)){
      lc.setRow(0, 3-i, numberArray[digit]);
      leadingZero = true;
    }
    else{
      lc.setRow(0, 3-i, numberArray[digit]);
    }
  }
  Serial.println();

  // lc.setRow(0, 3, numberArray[num]);
}

void loop() {

    // Apply pattern to all 4 digits
    for (int i = -125; i < 125; i++){
      print_number(i);
      Serial.println(i);
      delay(50);
    }

}
