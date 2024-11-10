/*
 * Shows how to use a 4 x 4 keypad, commonly seen in Arduino starter kits, with the library
 * https://www.aliexpress.com/item/32879638645.html
 * It maps the 16 buttons to the first 16 buttons of the controller
 * Only certain combinations work for multiple presses over 2 buttons
 */

#include <Arduino.h>
#include <Keypad.h>     // https://github.com/Chris--A/Keypad
#include <Adafruit_NeoPixel.h>
#include <BleGamepad.h> // https://github.com/lemmingDev/ESP32-BLE-Gamepad

BleGamepad bleGamepad("Aq+ Gamepad", "1STAGE", 100); // Shows how you can customise the device name, manufacturer name and initial battery level

#define ROWS 3
#define COLS 3
uint8_t rowPins[ROWS] = {6, 7, 15};     // ESP32 pins used for rows      --> adjust to suit --> Pinout on board: R1, R2, R3, R4
uint8_t colPins[COLS] = {16, 17, 18};   // ESP32 pins used for columns   --> adjust to suit --> Pinout on board: Q1, Q2, Q3, Q4
//uint8_t rowPins[ROWS] = {6, 7, 8};     // ESP32 pins used for rows      --> adjust to suit --> Pinout on board: R1, R2, R3, R4
//uint8_t colPins[COLS] = {9, 10, 11};   // ESP32 pins used for columns   --> adjust to suit --> Pinout on board: Q1, Q2, Q3, Q4
uint8_t keymap[ROWS][COLS] =
    {
        {7, 8, 9},    // Buttons  7, 8, 9      --> Used for calulating the bitmask for sending to the library
        {4, 5, 6},    // Buttons  4, 5, 6      --> Adjust to suit which buttons you want the library to send
        {1, 2, 3}     // Buttons  1, 2, 3      --> Eg. The value 12 in the array refers to button 12
    };

Keypad customKeypad = Keypad(makeKeymap(keymap), rowPins, colPins, ROWS, COLS);

#define RGBPIN 48
#define NUMPIX  1

Adafruit_NeoPixel pixels(NUMPIX,RGBPIN,NEO_GRB + NEO_KHZ800);

uint8_t ledR = 0;
uint8_t ledG = 0;
uint8_t ledB = 32;

int scaler = 1;

void setup()
{
    BleGamepadConfiguration bleGamepadConfig;
    bleGamepadConfig.setAutoReport(false); // Disable auto reports --> You then need to force HID updates with bleGamepad.sendReport()
    bleGamepad.begin();                    // Begin library with default buttons/hats/axes

    // changing bleGamepadConfig after the begin function has no effect, unless you call the begin function again

    pixels.begin();
    pixels.clear();
    pixels.setPixelColor(0, pixels.Color(ledR,ledG,ledB));
    pixels.show();
}

void loop()
{
    KeypadUpdate();
    RGBUpdate();
    delay(10);
}

void RGBUpdate()
{
  if (bleGamepad.isConnected())
  {
    if (millis() % 100 == 0)
    {
      ledB = 32;
      pixels.setPixelColor(0, pixels.Color(ledR,ledG,ledB));
      pixels.show();
    }
  }
  else
  {
    if (millis() % 100 == 0)
    {
      if (ledB < 2)
      {
        scaler = 1;
      }
      if (ledB > 63)
      {
        scaler = -1;
      }

      ledB = ledB + scaler;
      pixels.setPixelColor(0, pixels.Color(ledR,ledG,ledB));
      pixels.show();
    }
  }
}

void KeypadUpdate()
{
    customKeypad.getKeys();

    for (int i = 0; i < LIST_MAX; i++) // Scan the whole key list.      //LIST_MAX is provided by the Keypad library and gives the number of buttons of the Keypad instance
    {
        if (customKeypad.key[i].stateChanged) // Only find keys that have changed state.
        {
            uint8_t keystate = customKeypad.key[i].kstate;

            if (bleGamepad.isConnected())
            {
                if (keystate == PRESSED)
                {
                    bleGamepad.press(customKeypad.key[i].kchar);
                    String myString = customKeypad.key[0].kchar
                } // Press or release button based on the current state
                if (keystate == RELEASED)
                {
                    bleGamepad.release(customKeypad.key[i].kchar);
                }

                bleGamepad.sendReport(); // Send the HID report after values for all button states are updated, and at least one button state had changed
            }
        }
    }
}