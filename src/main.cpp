#include <Arduino.h>
#include <cmath>
#include <LedController.hpp>
#include <ESP32Encoder.h>

#define DIN_PIN 12
#define CLK_PIN 14
#define CS_PIN 13
#define SEGMENT_COUNT 1

#define ENCODER_A 47
#define ENCODER_B 21
#define ENCODER_S 48

LedController lc(DIN_PIN, CLK_PIN, CS_PIN, SEGMENT_COUNT, false);
ESP32Encoder encoder;

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
  delay(1000);

  Serial.println("Starting encoder debug...");

  lc.init(DIN_PIN, CLK_PIN, CS_PIN, SEGMENT_COUNT, false);
  lc.setIntensity(5);
  lc.clearMatrix();

  // Make sure pins are defined as inputs
  pinMode(ENCODER_A, INPUT_PULLUP);
  pinMode(ENCODER_B, INPUT_PULLUP);

  encoder.attachFullQuad(ENCODER_A, ENCODER_B);
  encoder.setCount(0);
}

void print_number(int8_t num) {
  if (num < 0) {
    lc.setRow(0, 0, B00000001);
    num = -num;
  } else {
    lc.setRow(0, 0, B00000000);
  }

  bool leadingZero = false;
  for (int i = 2; i >= 0; i--) {
    int divisor = pow(10, i);
    int8_t digit = num / divisor;
    num %= divisor;
    byte digitArray = numberArray[digit];

    if (i == 0) {
      digitArray |= B10000000;  // Add decimal point
    }

    if ((digit == 0) && (!leadingZero) && (i != 0)) {
      lc.setRow(0, 3 - i, B00000000);
    } else {
      lc.setRow(0, 3 - i, digitArray);
      if (digit != 0) leadingZero = true;
    }
  }
}

void loop() {
  static int lastVal = 0;
  int raw = encoder.getCount();
  int val = raw / 2;

  int a = digitalRead(ENCODER_A);
  int b = digitalRead(ENCODER_B);

  Serial.printf("Encoder A: %d | B: %d | Raw: %d | Detent: %d\n", a, b, raw, val);

  if (val != lastVal) {
    print_number(val);
    lastVal = val;
  }

  delay(10);
}
