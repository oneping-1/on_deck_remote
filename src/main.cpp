#include <Arduino.h>
#include <ArduinoJson.h>

#include <LedController.hpp>
#include <cmath>
#define DIN_PIN 12
#define CLK_PIN 14
#define CS_PIN 13
#define SEGMENT_COUNT 1
LedController lc(DIN_PIN, CLK_PIN, CS_PIN, SEGMENT_COUNT, false);

#include <ESP32Encoder.h>
#define ENCODER_A 47
#define ENCODER_B 21
#define ENCODER_S 48
ESP32Encoder encoder;

#define BUTTON_A 38
#define BUTTON_B 35
#define BUTTON_C 2
#define BUTTON_D 36
#define BUTTON_E 1
#define BUTTON_F 37

#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiManager.h>

byte numberArray[10] = {
    B01111110,  // 0
    B00110000,  // 1
    B01101101,  // 2
    B01111001,  // 3
    B00110011,  // 4
    B01011011,  // 5
    B01011111,  // 6
    B01110000,  // 7
    B01111111,  // 8
    B01111011   // 9
};

uint8_t activeButton = 4;
uint8_t activeValue = 0;
uint8_t minValue = 255;
uint8_t maxValue = 0;
uint8_t newValue = -1;
String url = "http://192.168.1.90/settings";

void printNumber(uint8_t num) {
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
    }
    if (digit != 0) {
      leadingZero = true;
    }
  }
}

int retrieveSetting(String setting) {

  HTTPClient http;
  http.begin(url);
  int httpCode = http.GET();

  if (httpCode == 200) {
    String payload = http.getString();
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (!error) {
      if (doc.containsKey(setting)) {
        String value = doc[setting].as<String>();

        // Example: convert known string values to ints
        if (setting == "mode") {
          if (value == "overview") return 0;
          if (value == "gamecast") return 1;
        }

        // For other settings that are already numeric, just return them
        if (doc[setting].is<int>()) {
          return doc[setting].as<int>();
        }
      }
    } else {
      Serial.print("JSON parse failed: ");
      Serial.println(error.f_str());
    }
  } else {
    Serial.print("HTTP GET failed with code: ");
    Serial.println(httpCode);
  }

  http.end();
  return -1;  // default/error fallback
}

void setSetting(){
  if (activeButton == 4){
    return;
  }

  HTTPClient http;
  String option = "";
  if (activeButton == 0){
    option = "?mode=";
  } else if (activeButton == 1){
    option = "?brightness=";
  } else if (activeButton == 2) {
    option = "?gamecast_id=";
  } else if (activeButton == 3) {
    option = "?delay=";
  }
  String finalUrl = url + option + String(activeValue);
  Serial.println(finalUrl);
  http.begin(finalUrl);
  int httpCode = http.GET();
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  lc.init(DIN_PIN, CLK_PIN, CS_PIN, SEGMENT_COUNT, false);
  lc.setIntensity(15);
  lc.clearMatrix();
  for (int i = 0; i < 4; i++){
    lc.setRow(0, i, B10000000);
  }

  WiFiManager wm;
  bool res = wm.autoConnect("on_deck remote");

  if (!res){
    Serial.println("falied to connect to wifi");
    delay(3000);
    ESP.restart();
    delay(5000);
  }

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected");

  // Make sure pins are defined as inputs
  pinMode(ENCODER_A, INPUT_PULLUP);
  pinMode(ENCODER_B, INPUT_PULLUP);
  pinMode(ENCODER_S, INPUT);

  pinMode(BUTTON_A, INPUT);
  pinMode(BUTTON_B, INPUT);
  pinMode(BUTTON_C, INPUT);
  pinMode(BUTTON_D, INPUT);
  pinMode(BUTTON_E, INPUT);
  pinMode(BUTTON_F, INPUT);

  encoder.attachFullQuad(ENCODER_B, ENCODER_A);
  encoder.setCount(0);

  for (int i = 0; i < 3; i++){
    lc.setRow(0, i, 0);
  }
}

void loop() {
  static int lastVal = 0;
  int raw = encoder.getCount();
  int val = raw / 2;
  newValue = val;

  if ((activeValue != newValue) and (newValue >= minValue) and (newValue <= maxValue)){
    activeValue = newValue;
    printNumber(activeValue);
  }
  else {
    encoder.setCount(activeValue * 2);
    Serial.println(newValue);
    Serial.println(minValue);
    Serial.println(maxValue);
    Serial.println();
  }


  int a = digitalRead(ENCODER_A);
  int b = digitalRead(ENCODER_B);
  int s = digitalRead(ENCODER_S);

  int A = digitalRead(BUTTON_A);
  int B = digitalRead(BUTTON_B);
  int C = digitalRead(BUTTON_C);
  int D = digitalRead(BUTTON_D);
  int E = digitalRead(BUTTON_E);
  int F = digitalRead(BUTTON_F);

  if (A == 0) {
    activeButton = 0;
    activeValue = retrieveSetting("mode");
    encoder.setCount(activeValue * 2);
    printNumber(activeValue);
    minValue = 0;
    maxValue = 1;
  }

  if (B == 0) {
    activeButton = 1;
    activeValue = retrieveSetting("brightness");
    encoder.setCount(activeValue * 2);
    printNumber(activeValue);
    minValue = 0;
    maxValue = 3;
  }

  if (C == 0) {
    activeButton = 2;
    activeValue = retrieveSetting("gamecast_id");
    encoder.setCount(activeValue * 2);
    printNumber(activeValue);
    minValue = 0;
    maxValue = retrieveSetting("num_games")-1;
  }

  if (D == 0) {
    activeButton = 3;
    activeValue = retrieveSetting("delay");
    encoder.setCount(activeValue * 2);
    printNumber(activeValue);
    minValue = 0;
    maxValue = 254;
  }

  if (E == 0) {
    activeButton = 4;
    Serial.println("Button E");
    for (int i = 0; i < 4; i++){
      lc.setRow(0, i, B00000000);
    }
    lc.setRow(0, 3, B10000000);
    minValue = 255;
    maxValue = 0;
  }

  if (F == 0) {
    // activateButton = 5;
    setSetting();
  }

  // Serial.println();
  delay(50);
}