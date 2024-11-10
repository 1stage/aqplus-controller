/*

Aq+ Gamepad
--------------------------------------------------------------------------------------------
by Sean P. Harrington
sph@1stage.com
aquarius.1stage.com
--------------------------------------------------------------------------------------------

Abstract -
The Aq+ Gamepad is a modern game controller for Aquarius and Aquarius+ 8-bit Z80-based 
computers. It features either wired (Mini/Micro/MX expander required for original Aquarius)
or wireless (Aquarius+) connectivity, and emulates the original Aquarius hand controller in
a form-factor that is more comfortable and fun to use.

The system uses an ESP32-S3 module with BLE (Bluetooth Low Energy) as the microcontroller.
It is configured for use using the Arduino IDE, and uses the "ESP32-S3-Box" board profile.
Additionally, BLE connectivity is coded using the ESP32-BLE-Gamepad library, which itself
relies on the NimBLE-Arduino library. The RGB LED serves as a system status indicator and 
uses the Adafruit NeoPixel library. All libraries can be loaded through the Arduino Library
Manager. The system also features a LiPo (lithium polymer) battery, with smart charging.

--------------------------------------------------------------------------------------------

Revisions:
0.1.3, 10 NOV 2024 -  Shift register for wired connections
0.1.2, 09 NOV 2024 -  Button alignment with Xbox mappings
                      Thumbstick scaling fixed
                      Buttons transitioned to single-pin (from matrix)
0.1.1, 08 NOV 2024 -  Button matrix (unsuccessful)
0.1.0, 07 NOV 2024 -  Initial version, using Gamepad example (four buttons)
                      Installed Arduino bootloader
                      Identified compatible Board profile (ESP32-S3-Box)

Links & References -
AQP Gamepad GitHub: https://github.com/1stage/aqplus-controller
ESP32-S3_DevKitC: https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32s3/esp32-s3-devkitc-1/index.html
Arduino Core for ESP32: https://github.com/espressif/arduino-esp32
ESP32-BLE-Gamepad: https://github.com/lemmingDev/ESP32-BLE-Gamepad
Adafruit NeoPixel: https://github.com/adafruit/Adafruit_NeoPixel
NimBLE Arduino: https://github.com/h2zero/NimBLE-Arduino

*/

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <BleGamepad.h>
#include <BleGamepadConfiguration.h>

#define BUTTONPIN_1 5   // ESP32 GPIO pin Aq+ button 1 is attached to
#define BUTTONPIN_2 6   // ESP32 GPIO pin Aq+ button 2 is attached to
#define BUTTONPIN_3 7   // ESP32 GPIO pin Aq+ button 3 is attached to
#define BUTTONPIN_4 8   // ESP32 GPIO pin Aq+ button 4 is attached to
#define BUTTONPIN_5 9   // ESP32 GPIO pin Aq+ button 5 is attached to
#define BUTTONPIN_6 10  // ESP32 GPIO pin Aq+ button 6 is attached to
#define BUTTONPIN_T 4   // ESP32 GPIO pin Aq+ thumbstick button is attached to
#define BUTTONPIN_G 11  // ESP32 GPIO pin Aq+ guide button is attached to

#define XPIN 15  // ESP32 GPIO pin thumbstick X is attached to
#define YPIN 16  // ESP32 GPIO pin thumbstick Y is attached to

#define RGBPIN 48  // ESP32 GPIO pin RGB LED data is attached to
#define NUMPIX 1   // Number of NeoPixel units

#define AQGP_NAME "Aq+ Gamepad"
#define AQGP_MNFR "1STAGE"
#define AQGP_BATT 100
#define AQGP_MODL "1.0"
#define AQGP_SWFM "0.1.3"
#define AQGP_HWRV "rev0"
#define AQGP_SRLN "AGP-xx-SPH"

Adafruit_NeoPixel pixels(NUMPIX, RGBPIN, NEO_GRB + NEO_KHZ800);  // Create a NeoPixel object to control

BleGamepad bleGamepad(AQGP_NAME, AQGP_MNFR, AQGP_BATT);  // Create a BleGamepad object to control

BleGamepadConfiguration bleGamepadConfig;  // Create a BleGamepadConfiguration object to store all of the options

int previousButton1State = HIGH;
int previousButton2State = HIGH;
int previousButton3State = HIGH;
int previousButton4State = HIGH;
int previousButton5State = HIGH;
int previousButton6State = HIGH;
int previousButtonTState = HIGH;
int previousButtonGState = HIGH;

const int thumbSamples = 5;  // Number of thumbstick samples to take
const int delaySamples = 4;  // Delay in milliseconds between samples

unsigned long cycleLED;

uint8_t ledR = 0;
uint8_t ledG = 0;
uint8_t ledB = 0;

int scaler = 1;

void setup() {

  bleGamepadConfig.setModelNumber(AQGP_MODL);
  bleGamepadConfig.setSoftwareRevision(AQGP_SWFM);
  bleGamepadConfig.setFirmwareRevision(AQGP_SWFM);
  bleGamepadConfig.setSerialNumber(AQGP_SRLN);
  bleGamepadConfig.setHardwareRevision(AQGP_HWRV);

  pinMode(BUTTONPIN_1, INPUT_PULLUP);
  pinMode(BUTTONPIN_2, INPUT_PULLUP);
  pinMode(BUTTONPIN_3, INPUT_PULLUP);
  pinMode(BUTTONPIN_4, INPUT_PULLUP);
  pinMode(BUTTONPIN_5, INPUT_PULLUP);
  pinMode(BUTTONPIN_6, INPUT_PULLUP);
  pinMode(BUTTONPIN_T, INPUT_PULLUP);
  pinMode(BUTTONPIN_G, INPUT_PULLUP);
  pinMode(XPIN, INPUT);
  pinMode(YPIN, INPUT);

  bleGamepad.begin();

  cycleLED = millis() + 50;
  pixels.begin();
  pixels.clear();
  pixels.setPixelColor(0, pixels.Color(ledR, ledG, ledB));
  pixels.show();
}

void loop() {

  updateLED();

  if (bleGamepad.isConnected()) {

    int currentButton1State = digitalRead(BUTTONPIN_1);
    int currentButton2State = digitalRead(BUTTONPIN_2);
    int currentButton3State = digitalRead(BUTTONPIN_3);
    int currentButton4State = digitalRead(BUTTONPIN_4);
    int currentButton5State = digitalRead(BUTTONPIN_5);
    int currentButton6State = digitalRead(BUTTONPIN_6);
    int currentButtonTState = digitalRead(BUTTONPIN_T);
    int currentButtonGState = digitalRead(BUTTONPIN_G);

    int XValues[thumbSamples];
    int YValues[thumbSamples];
    int XValue = 0;
    int YValue = 0;

    if (currentButton1State != previousButton1State) {
      if (currentButton1State == LOW) {
        bleGamepad.press(BUTTON_1);
      } else {
        bleGamepad.release(BUTTON_1);
      }
    }
    previousButton1State = currentButton1State;

    if (currentButton2State != previousButton2State) {
      if (currentButton2State == LOW) {
        bleGamepad.press(BUTTON_2);
      } else {
        bleGamepad.release(BUTTON_2);
      }
    }
    previousButton2State = currentButton2State;

    if (currentButton3State != previousButton3State) {
      if (currentButton3State == LOW) {
        bleGamepad.press(BUTTON_4);
      } else {
        bleGamepad.release(BUTTON_4);
      }
    }
    previousButton3State = currentButton3State;

    if (currentButton4State != previousButton4State) {
      if (currentButton4State == LOW) {
        bleGamepad.press(BUTTON_5);
      } else {
        bleGamepad.release(BUTTON_5);
      }
    }
    previousButton4State = currentButton4State;

    if (currentButton5State != previousButton5State) {
      if (currentButton5State == LOW) {
        bleGamepad.press(BUTTON_7);
      } else {
        bleGamepad.release(BUTTON_7);
      }
    }
    previousButton5State = currentButton5State;

    if (currentButton6State != previousButton6State) {
      if (currentButton6State == LOW) {
        bleGamepad.press(BUTTON_8);
      } else {
        bleGamepad.release(BUTTON_8);
      }
    }
    previousButton6State = currentButton6State;

    if (currentButtonTState != previousButtonTState) {
      if (currentButtonTState == LOW) {
        bleGamepad.press(BUTTON_11);
      } else {
        bleGamepad.release(BUTTON_11);
      }
    }
    previousButtonTState = currentButtonTState;

    if (currentButtonGState != previousButtonGState) {
      if (currentButtonGState == LOW) {
        bleGamepad.press(BUTTON_13);
      } else {
        bleGamepad.release(BUTTON_13);
      }
    }
    previousButtonGState = currentButtonGState;

    for (int i = 0; i < thumbSamples; i++) {
      XValues[i] = analogRead(XPIN);
      YValues[i] = analogRead(YPIN);
      XValue += XValues[i];
      YValue += YValues[i];
      delay(delaySamples);
    }

    XValue = XValue / thumbSamples;
    YValue = YValue / thumbSamples;

    int adjXValue = map(XValue, 0, 4095, 0, 32737);
    int adjYValue = map(YValue, 0, 4095, 0, 32737);

    bleGamepad.setX(adjXValue);
    bleGamepad.setY(adjYValue);
  }
}

void updateLED() {
  if (bleGamepad.isConnected()) {
    ledR = 0;
    ledB = 32;
  } else {
    if (millis() >= cycleLED) {
      if (ledR < 1) {
        scaler = 1;
      }
      if (ledR > 32) {
        scaler = -1;
      }
      ledB = 0;
      ledR = ledR + scaler;
      cycleLED = millis() + 50;
    }
  }
  pixels.setPixelColor(0, pixels.Color(ledR, ledG, ledB));
  pixels.show();
}
