/*

Aq+ Gamepad
--------------------------------------------------------------------------------------------
by Sean P. Harrington
sph@1stage.com
aquarius.1stage.com
--------------------------------------------------------------------------------------------

--------------------------------------------------------------------------------------------
Overview:
--------------------------------------------------------------------------------------------

The Aq+ Gamepad is a modern game controller for Aquarius and Aquarius+ 8-bit Z80-based 
computers. It features either wired (Mini/Micro/MX expander required for original Aquarius)
or wireless (Aquarius+) connectivity, and emulates the original Aquarius hand controller in
a form-factor that is more comfortable and fun to use.

--------------------------------------------------------------------------------------------

For more information, see the included Docs.ino file.

*/

// Libraries
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <BleGamepad.h>
#include <BleGamepadConfiguration.h>
#include <math.h>
#include <Preferences.h>

// Supporting project files
#include <Unit_Settings.ino>
#include <Docs.ino>

#define BUTTONPIN_1 5   // ESP32 GPIO pin Aq+ physical button 1 is attached to
#define BUTTONPIN_2 6   // ESP32 GPIO pin Aq+ physicalbutton 2 is attached to
#define BUTTONPIN_3 7   // ESP32 GPIO pin Aq+ physicalbutton 3 is attached to
#define BUTTONPIN_4 8   // ESP32 GPIO pin Aq+ physicalbutton 4 is attached to
#define BUTTONPIN_5 9   // ESP32 GPIO pin Aq+ physicalbutton 5 is attached to
#define BUTTONPIN_6 10  // ESP32 GPIO pin Aq+ physicalbutton 6 is attached to
#define BUTTONPIN_T 4   // ESP32 GPIO pin Aq+ thumbstick button is attached to
#define BUTTONPIN_G 11  // ESP32 GPIO pin Aq+ guide button is attached to

#define XPIN 15         // ESP32 GPIO pin thumbstick X is attached to
#define YPIN 16         // ESP32 GPIO pin thumbstick Y is attached to

#define BATTDIV 13      // Pin to read battery voltage level

#define RGBPIN 48       // ESP32 GPIO pin RGB LED data is attached to
#define NUMPIX 1        // Number of NeoPixel units

#define DATAPIN_0 39    // Wired data out pin 0
#define DATAPIN_1 38    // Wired data out pin 1
#define DATAPIN_2 21    // Wired data out pin 2
#define DATAPIN_3 18    // Wired data out pin 3
#define DATAPIN_4 40    // Wired data out pin 4
#define DATAPIN_5 41    // Wired data out pin 5
#define DATAPIN_6 42    // Wired data out pin 6
#define DATAPIN_7 17    // Wired data out pin 7
#define DATAPIN_8 47    // Wired data out pin 8

// Create a NeoPixel object to control RGB LED
Adafruit_NeoPixel pixels(NUMPIX, RGBPIN, NEO_GRB + NEO_KHZ800);

// Create a BleGamepad object to control
BleGamepad aqpGamepad(AQGP_NAME, AQGP_MNFR, AQGP_BATT);

// Create a BleGamepadConfiguration object to store all of the options
BleGamepadConfiguration aqpGamepadConfig;

// Create a Preferences object to access stored NVS values
Preferences aqgp_prefs;

// Create a state change history for all 8 physical buttons.
// buttonState[button_num][previous, current], HIGH = OFF
int buttonState[8][2] = { { HIGH, HIGH },
                          { HIGH, HIGH },
                          { HIGH, HIGH },
                          { HIGH, HIGH },
                          { HIGH, HIGH },
                          { HIGH, HIGH },
                          { HIGH, HIGH },
                          { HIGH, HIGH } };

// Create an LED RGB color data structure
struct ledColor {
  int ledR;        /* RED value   */
  int ledG;        /* GREEN value */
  int ledB;        /* BLUE value  */
};

// Set of standard colors to use for easy reference within the code
ledColor RED     = { 32,  0,  0 };
ledColor GREEN   = {  0, 32,  0 };
ledColor BLUE    = {  0,  0, 32 };
ledColor CYAN    = {  0, 32, 32 };
ledColor YELLOW  = { 32, 32,  0 };
ledColor MAGENTA = { 32,  0, 32 };
ledColor BLACK   = {  0,  0,  0 };
ledColor WHITE   = { 32, 32, 32 };

// Variables for serial command line input
String inputString  = "";     // A String to hold incoming data
String inputLast    = "";     // A string to hold the last attempted command
bool stringComplete = false;  // whether the string is complete

// Create state memory for battery level: batteryState[previous, current]
int batteryState[2] = { 1, 1 };

// Create state memory for connectivity: connectState[previous, current]
int connectState[2] = { 1, 1 };

// Combined state memory for battery & connectivity: battConState[previous, current]
int battConState[2] = { 1, 1 };

// Create a state change history for the thumbstick: thumbState[previous, current]
int thumbState[2]   = { 0, 0 };

// Create a data pin array for wired output.
int dataPins[9] = { DATAPIN_0,
                    DATAPIN_1,
                    DATAPIN_2,
                    DATAPIN_3,
                    DATAPIN_4,
                    DATAPIN_5,
                    DATAPIN_6,
                    DATAPIN_7,
                    DATAPIN_8 };

// Create an array mapping between Aquarius buttons and BLE Gamepad buttons in a SWAPPED configuration.
// buttonPinMap[button_num][Aq+ Button Pin, BLE Gamepad button]
uint8_t buttonPinMap[8][2] =   { { BUTTONPIN_1, BUTTON_1 },
                                 { BUTTONPIN_2, BUTTON_2 },
                                 { BUTTONPIN_3, BUTTON_4 },
                                 { BUTTONPIN_4, BUTTON_5 },
                                 { BUTTONPIN_5, BUTTON_7 },
                                 { BUTTONPIN_6, BUTTON_8 },
                                 { BUTTONPIN_T, BUTTON_11 },
                                 { BUTTONPIN_G, BUTTON_13 } };

// Create an array mapping between Aquarius buttons and BLE Gamepad buttons in a NOT SWAPPED configuration.
// buttonPinMapNS[button_num][Aq+ Button Pin, BLE Gamepad button]
uint8_t buttonPinMapNS[8][2] = { { BUTTONPIN_1, BUTTON_5 },
                                 { BUTTONPIN_2, BUTTON_7 },
                                 { BUTTONPIN_3, BUTTON_8 },
                                 { BUTTONPIN_4, BUTTON_1 },
                                 { BUTTONPIN_5, BUTTON_2 },
                                 { BUTTONPIN_6, BUTTON_4 },
                                 { BUTTONPIN_T, BUTTON_11 },
                                 { BUTTONPIN_G, BUTTON_13 } };

// Create an array for the data mappings for a SWAPPED configuration for
// the wired output (byte mapping) for the 8 buttons.
uint8_t buttonByte[8]   = { 0b01000000,               /* Button 1 (BLE Gamepad)                                        */
                            0b10000100,               /* Button 2 (BLE Gamepad)                                        */
                            0b10100000,               /* Button 3 (BLE Gamepad)                                        */
                            0b00100000,               /* Button 4 (BLE Gamepad)                                        */
                            0b10000010,               /* Button 5 (BLE Gamepad)                                        */
                            0b10000001,               /* Button 6 (BLE Gamepad)                                        */
                            0b11111111,               /* Thumb stick press: all 1's serves as mask of other buttons    */
                            0b11100010 };             /* Guide: Was 0b00000000, now a chord of 1+3+5                   */

// Create an array for the data mappings for a NOT SWAPPED configuration for
// the wired output (byte mapping) for the 8 buttons.
uint8_t buttonByteNS[8] = { 0b00100000,               /* Button 4 (BLE Gamepad)                                        */
                            0b10000010,               /* Button 5 (BLE Gamepad)                                        */
                            0b10000001,               /* Button 6 (BLE Gamepad)                                        */
                            0b01000000,               /* Button 1 (BLE Gamepad)                                        */
                            0b10000100,               /* Button 2 (BLE Gamepad)                                        */
                            0b10100000,               /* Button 3 (BLE Gamepad)                                        */
                            0b11111111,               /* Thumb stick press: all 1's serves as mask of other buttons    */
                            0b11100010 };             /* Guide: Was 0b00000000, now a chord of 1+3+5                   */

// Create an array for the data mappings for
// the wired output (byte mappings for the 16 thumbstick values)
uint8_t thumbByte[17] = { 0b00000000,               /* CENTERED         */
                          0b00000010,               /* RIGHT            */
                          0b00010010,               /* RIGHT_DOWN_RIGHT */
                          0b00010011,               /* DOWN_RIGHT       */
                          0b00000011,               /* DOWN_DOWN_RIGHT  */
                          0b00000001,               /* DOWN             */
                          0b00010001,               /* DOWN_DOWN_LEFT   */
                          0b00011001,               /* DOWN_LEFT        */
                          0b00001001,               /* LEFT_DOWN_LEFT   */
                          0b00001000,               /* LEFT             */
                          0b00011000,               /* LEFT_UP_LEFT     */
                          0b00011100,               /* UP_LEFT          */
                          0b00001100,               /* UP_UP_LEFT       */
                          0b00000100,               /* UP               */
                          0b00010100,               /* UP_UP_RIGHT      */
                          0b00010110,               /* UP_RIGHT         */
                          0b00000110 };             /* RIGHT_UP_RIGHT   */


// Create an alternate set of X & Y value pairs
// for DPAD mapping in DPAD Mode
uint16_t dpadThumb[17][2] = { { 16384, 16384 },     /* DPAD_CENTERED    */
                              { 32767, 16384 },     /* DPAD_RIGHT       */
                              { 32767, 16384 },     /* DPAD_RIGHT       */
                              { 32767, 32767 },     /* DPAD_DOWN_RIGHT  */
                              { 16384, 32767 },     /* DPAD_DOWN        */
                              { 16384, 32767 },     /* DPAD_DOWN        */
                              { 16384, 32767 },     /* DPAD_DOWN        */
                              {     0, 32767 },     /* DPAD_DOWN_LEFT   */
                              {     0, 16384 },     /* DPAD_LEFT        */
                              {     0, 16384 },     /* DPAD_LEFT        */
                              {     0, 16384 },     /* DPAD_LEFT        */
                              {     0,     0 },     /* DPAD_UP_LEFT     */
                              { 16384,     0 },     /* DPAD_UP          */
                              { 16384,     0 },     /* DPAD_UP          */
                              { 16384,     0 },     /* DPAD_UP          */
                              { 32767,     0 },     /* DPAD_UP_RIGHT    */
                              { 32767, 16384 } };   /* DPAD_RIGHT       */

// Create an array for the data mappings for
// the wired output (byte mappings for the 8 DPAD values)
uint8_t dpadByte[17] =  { 0b00000000,               /* DPAD_CENTERED    */
                          0b00000010,               /* DPAD_RIGHT       */
                          0b00000010,               /* DPAD_RIGHT       */
                          0b00010011,               /* DPAD_DOWN_RIGHT  */
                          0b00000001,               /* DPAD_DOWN        */
                          0b00000001,               /* DPAD_DOWN        */
                          0b00000001,               /* DPAD_DOWN        */
                          0b00011001,               /* DPAD_DOWN_LEFT   */
                          0b00001000,               /* DPAD_LEFT        */
                          0b00001000,               /* DPAD_LEFT        */
                          0b00001000,               /* DPAD_LEFT        */
                          0b00011100,               /* DPAD_UP_LEFT     */ 
                          0b00000100,               /* DPAD_UP          */
                          0b00000100,               /* DPAD_UP          */
                          0b00000100,               /* DPAD_UP          */
                          0b00010110,               /* DPAD_UP_RIGHT    */
                          0b00000010 };             /* DPAD_RIGHT       */

  // Create an array for the DPAD instructions to Gamepad BLE
  //   Possible DPAD/HAT switch position values are:
  //   DPAD_CENTERED, DPAD_UP, DPAD_UP_RIGHT, DPAD_RIGHT, DPAD_DOWN_RIGHT, DPAD_DOWN, DPAD_DOWN_LEFT, DPAD_LEFT, DPAD_UP_LEFT
  //   (or HAT_CENTERED, HAT_UP etc)
  signed char dpadSwitch[17] = { DPAD_CENTERED,
                                 DPAD_RIGHT,
                                 DPAD_RIGHT,
                                 DPAD_DOWN_RIGHT,
                                 DPAD_DOWN,
                                 DPAD_DOWN,
                                 DPAD_DOWN,
                                 DPAD_DOWN_LEFT,
                                 DPAD_LEFT,
                                 DPAD_LEFT,
                                 DPAD_LEFT,
                                 DPAD_UP_LEFT,
                                 DPAD_UP,
                                 DPAD_UP,
                                 DPAD_UP,
                                 DPAD_UP_RIGHT,
                                 DPAD_RIGHT };

// Number of thumbstick samples to take per cycle
const int thumbSamples = 5;       // Number of thumbstick samples to take per cycle
const int delaySamples = 4;       // Delay in milliseconds between samples

// Battery level variables
const int battSamples  = 5;       // Number of battery samples to take per cycle
int battValues[battSamples];      // Holder array for battery samples
int rawBatt;                      // Holder for raw battery level reading on ESP32 analog GPIO
uint8_t battLevel = 100;          // Starting battery level, updated from BATTDIV reading in main loop

// Persistent holder for new button data
uint8_t buttonOut = 0b00000000;
// Persistent holder for new thumbstick data
uint8_t thumbOut  = 0b00000000;
// Per-cycle holder for combined button and thumbstick data sent to data out pins
uint8_t dataOut   = 0b00000000;

// Default multiplier for milliseconds
int timeScaler = 2;
// Wait time in millis() between LED updates
int cycleLEDWait = battLevel * timeScaler;
// Holder for LED pulse animation update timer
unsigned long cycleLED = millis() + cycleLEDWait;
// LED animation holder
int counterLED = 1;

// Holder for LED color, set to BLACK / OFF
ledColor theLED = BLACK;

// DPAD on toggle
bool dpadON;

// Button row swap toggle
bool bRowSwapON;

// Additional thumbstick variables
int XTrim;                  // Trim adjustment (per device) for rawX value
int YTrim;                  // Trim adjustment (per device) for rawY value
int battMax;                // Trim adjustment (per device) for max battery value when 100% (no charge light shown)
int XValues[thumbSamples];  // Holder array for X samples
int YValues[thumbSamples];  // Holder array for Y samples
int XValue;                 // Normalized X value from samples
int YValue;                 // Normalized Y value from samples
int adjXValue;              // Scaled X value for Gamepad value range
int adjYValue;              // Scaled Y value for Gamepad value range
int dirXValue;              // Scaled X value for trigonometric value range
int dirYValue;              // Scaled Y value for trigonometric value range
float x;                    // X value / 180 for finding 16 direction range
float y;                    // Y value / 180 for finding 16 direction range
float deadZone;             // Holder for mapping the center dead zone to reduce jitters
float dzFactor = 1.0;       // The acceptable area for thumbstick jitter: 1.0 is standard; 1.1 is less sensitive, etc.
float angle;                // Holder for angle value for 16 direction mapping

// Set up the system
void setup() {

  // Turn on the serial port.
  Serial.begin(115200);

  // reserve 200 bytes for the inputString & inputLast:
  inputString.reserve(200);
  inputLast.reserve(200);

  // Initialize the Preferences object in RW mode..
  aqgp_prefs.begin("aqgp", false, 0);

  // ...or Initialize the Preferences object in READ ONLY mode
  // aqgp_prefs.begin("aqgp", true, 0);

  // Remove all preferences under the aqgp namespace...
  // aqgp_prefs.clear();

  // ...or remove individual keys...
  // aqgp_prefs.remove("xtrim");
  // aqgp_prefs.remove("ytrim");
  // aqgp_prefs.remove("battmax");
  // aqgp_prefs.remove("dpadon");
  // aqgp_prefs.remove("browswapon");

  // Put the X & Y trim and battmax uint values
  // X & Y raw values should be ~2613
  // aqgp_prefs.putUInt("xtrim", 70);         /* XTrim value to align X pot to ~2613      */
  // aqgp_prefs.putUInt("ytrim", 97);         /* YTrim value to align Y pot to ~2613      */
  // aqgp_prefs.putUInt("battmax", 3273);     /* Value to align battery value to 100      */
  // aqgp_prefs.putUInt("dpadon", 1);         /* Toggle for saved DPAD state              */
  // aqgp_prefs.putUInt("browswapon", 1);     /* Toggle for saved row swap state          */

  // Get the X & Y trim and battmax uint values; if the key does not exist, return a default value (second parameter)
  XTrim      = aqgp_prefs.getUInt("xtrim", 0);
  YTrim      = aqgp_prefs.getUInt("ytrim", 0);
  battMax    = aqgp_prefs.getUInt("battmax", 3250);
  dpadON     = aqgp_prefs.getUInt("dpadon", 1);
  bRowSwapON = aqgp_prefs.getUInt("browswapon", 1);

  // Publish AQP Gamepad config.
  aqpGamepadConfig.setModelNumber(AQGP_MODL);
  aqpGamepadConfig.setSoftwareRevision(AQGP_SWFM);
  aqpGamepadConfig.setFirmwareRevision(AQGP_SWFM);
  aqpGamepadConfig.setSerialNumber(AQGP_SRLN);
  aqpGamepadConfig.setHardwareRevision(AQGP_HWRV);

  // Set all GPIO button pins [x][0] to INPUT_PULLUP
  for (int i = 0; i < 8; i++) {
    if (bRowSwapON) {                             /* Row Swap ON */
      pinMode(buttonPinMap[i][0], INPUT_PULLUP);
    } else {                                      /* Row Swap not on */
      pinMode(buttonPinMapNS[i][0], INPUT_PULLUP);
    }
  }

  // Set all wired output dataPins[x] to OUTPUT.
  for (int i = 0; i < 8; i++) {
    pinMode(dataPins[i], OUTPUT);
  }

  // Set datapin[9] as OUTPUT and LOW (GND)
  pinMode(dataPins[9], OUTPUT);
  digitalWrite(dataPins[9], LOW);

  // Set the battery level sensor to input (analog).
  pinMode(BATTDIV, INPUT);

  // Set the X and Y pots on the thumbstick to input (analog).
  pinMode(XPIN, INPUT);
  pinMode(YPIN, INPUT);

  // Start the BLE Gamepad using the configs.
  aqpGamepad.begin(&aqpGamepadConfig);

  // Send data (blank, at start) to the wired connection.
  shiftOut(dataOut);

  // Set the initial battery level (100%).
  // It will be read and updated in the main loop.
  aqpGamepad.setBatteryLevel(battLevel);

  // Setup the LED cycle timer.
  cycleLED = millis() + cycleLEDWait;

  // Perform initial battery and BTLE checks
  checkBattery();
  checkConnect();
  calcRGB();

  // Start up the RGB LED and update it
  pixels.begin();
  pixels.clear();
  theLED = CYAN;
  updateNeoPixel();

  // Close the Preferences
  aqgp_prefs.end();

  // Serial terminal start-up message
  Serial.println();
  Serial.println("Welcome to the Aq+ Gamepad!");
  Serial.println("Use ? or H for HELP.");
  commandPrompt();
}

// Main cycle loop
void loop() {

  dataOut = 0b00000000;     // Reset outbound data register each cycle
                            // Note that buttonOut and thumbOut persist through cycle loops

  // Check for DPAD switch button chord (THUMB + GUIDE + 6)
  if ((!digitalRead(BUTTONPIN_T)) && (!digitalRead(BUTTONPIN_G)) && (!digitalRead(BUTTONPIN_6))) {
    DPSwitch();
  }
  
  // Check for Button row swap chord (THUMB + GUIDE + 5)
  if ((!digitalRead(BUTTONPIN_T)) && (!digitalRead(BUTTONPIN_G)) && (!digitalRead(BUTTONPIN_5))) {
    BRSwap();            
  }

  if (bRowSwapON) {
    checkButtons();         // Read buttons for SWAPPED and update wireless connection
  } else {
    checkButtonsNS();       // Read buttons for NOT SWAPPED and update wireless connection
  }

  checkThumbs();            // Read thumbstick and update wireless connection
  shiftOut(dataOut);        // Write data for buttons and thumbsticks to wired connection

  // Check battery level every 1 second at the .9 second mark
  if (millis() % 1000 > 900) {
    checkBattery();
  }

  // Check BTLE connection every 1 second at the .8 second mark
  if (((millis() % 1000) > 800) && ((millis() % 1000) <= 900)) {
    checkConnect();
  }

  setState();               // Set system state for battery and wireless
  calcRGB();                // Calculate the RGB LED color and pulse rate from system state
  updateNeoPixel();         // Update NeoPixel RGB LED

  // Check Serial for input
  serialEvent();

  // Process a completed command, if it's been entered in the terminal
  if (stringComplete) {
    // Parse the entered command
    if (parseCommand(inputString)) {
      // Clear the string
      inputString = "";
      inputLast   = "";
    } else {
      inputString = inputLast;
    }
    stringComplete = false;
    commandPrompt();
  }
}


// The serialEvent() occurs whenever a new data comes in the hardware serial RX. This
// routine is run between each time loop() runs, so using delay inside loop can
// delay response. Multiple bytes of data may be available.
void serialEvent() {
  while (Serial.available()) {
    // Get the new byte
    char inChar = (char)Serial.read();
    // Add it to the inputString
    inputString += inChar;
    // Echo it to the serial output (may not be necessary)
    Serial.print(inChar);
    // If the incoming character is a newline, set a flag so the main loop can
    // process the command
    if (inChar == '\n') {
      stringComplete = true;
      Serial.println();
    }
  }
}

bool parseCommand(String thisCommand) {
  // Normalize entered commands to upper case for easier processing
  thisCommand.toUpperCase();
  // Help
  if ((thisCommand == "?\n") | (thisCommand == "H\n")) {
    Serial.println("Help:");
    printHelp();
    return true;
  }
  // Toggle DPAD <> Analog
  if (thisCommand == "A\n") {
    DPSwitch();
    return true;
  }
  // Toggle button rows (1 2 3) <> (4 5 6)
  if (thisCommand == "B\n") {
    BRSwap();
    return true;
  }
  // Config output
  if (thisCommand == "C\n") {
    Serial.println("Config Data:");
    configOut();
    return true;
  }
  // Debug output
  if (thisCommand == "D\n") {
    Serial.println("Debug data x10:");
    for (int i = 0; i < 10; i++) {
      debugOut();
      delay(250);
    }
    Serial.println();
    return true;
  }
  // Show DPAD & Button Swap Settings
  if (thisCommand == "L\n") {
    Serial.println("Layout Settings:");
    Serial.print("     dpadON = ");
    Serial.println(dpadON);
    Serial.print(" bRowSwapON = ");
    Serial.println(bRowSwapON);
    Serial.println();
    showLayout();
    return true;
  }
  // Auto-set Max Battery level
  if (thisCommand == "M\n") {
    Serial.println("For best results, ONLY PERFORM when charge light is not on.");
    Serial.println("Auto-setting maximum battery level...");
    aqgp_prefs.begin("aqgp", false, 0);
    aqgp_prefs.putUInt("battmax", rawBatt);
    battMax = aqgp_prefs.getUInt("battmax", rawBatt);
    aqgp_prefs.end();
    Serial.println();
    Serial.print("Max battery level set to ");
    Serial.println(battMax);
    Serial.println();
    return true;
  }
  // Prefs output
  if (thisCommand == "P\n") {
    Serial.println("Prefs data:");
    prefsOut();
    return true;
  }
  // Reset X & Y trim
  if (thisCommand == "R\n") {
    Serial.println("Resetting X & Y trim values to 0...");
    resetXYTrim();
    Serial.println();
    Serial.print("X & Y trim values reset to 0");
    Serial.println();
    return true;
  }
  // Stats output
  if (thisCommand == "S\n") {
    Serial.println("BTLE Stats:");
    statsOut();
    return true;
  }
  // Auto-set X & Y trim
  if (thisCommand == "T\n") {
    Serial.println("For best results, do NOT move thumbstick!");
    Serial.println("Auto-setting X & Y trim values to middle range...");
    autoTrimXY();
    Serial.println();
    Serial.print("XTrim updated to ");
    Serial.println(XTrim);
    Serial.print("YTrim updated to ");
    Serial.println(YTrim);
    Serial.println();
    return true;
  }
  /* Else bad command  */
  Serial.println("Command not recognized. Use ? or H for HELP.");
  Serial.println();
  return false;
}

void commandPrompt() {
    Serial.print(AQGP_NAME);
    Serial.print(" > ");
    Serial.print(inputString);
}

void printHelp() {
  Serial.println("  A  - Toggle DPAD <> Analog state");
  Serial.println("  B  - Toggle button rows (1 2 3) <> (4 5 6)");
  Serial.println("  C  - Show configuration data");
  Serial.println("  D  - Show debug data x10, once every 1/4 second");
  Serial.println("  L  - Show current DPAD & Button Row settings");
  Serial.println("  M  - Auto-set max battery level (only when charge light is off)");
  Serial.println("  P  - Show preferences data");
  Serial.println("  R  - Reset X & Y trim to 0");
  Serial.println("  S  - Show BTLE statistics");
  Serial.println("  T  - Auto-trim X & Y axis");
  Serial.println(" H/? - Show Help (this) screen");
  Serial.println();
}

void resetXYTrim() {
  aqgp_prefs.begin("aqgp", false, 0);
  XTrim = 0;
  YTrim = 0;
  aqgp_prefs.putUInt("xtrim", XTrim);
  aqgp_prefs.putUInt("ytrim", YTrim);
  aqgp_prefs.end();
}

void autoTrimXY() {
  aqgp_prefs.begin("aqgp", false, 0);
  XTrim = 0;
  YTrim = 0;
  XTrim = 2600 - XValue;
  YTrim = 2600 - YValue;
  aqgp_prefs.putUInt("xtrim", XTrim);
  aqgp_prefs.putUInt("ytrim", YTrim);
  aqgp_prefs.end();
}

void prefsOut() {
  Serial.print("XTrim | YTrim = ");           // Saved XTrim and YTrim values from aqgp_prefs
  Serial.print(XTrim);
  Serial.print(" | ");
  Serial.println(YTrim);
  Serial.print("      battMax = ");           // Saved battmax value from aqgp_prefs
  Serial.println(battMax);
  Serial.print("       dpadON = ");           // Saved dpadON mode from aqgp_prefs
  Serial.println(dpadON);
  Serial.print("   bRowSwapON = ");           // Saved bRowSwapON mode from aqgp_prefs
  Serial.println(bRowSwapON);
  Serial.print("         BTLE = ");           // Bluetooth LE connection status (0 disconnected, 1 connected)
  Serial.println(aqpGamepad.isConnected());
  Serial.println();
}

void configOut() {
  if (aqpGamepad.isConnected()) {
    Serial.print("       Device Name : ");
    Serial.println(aqpGamepad.getDeviceName());
    Serial.print("        Device MFR : ");
    Serial.println(aqpGamepad.getDeviceManufacturer());
    Serial.print("   Device ModelNum : ");
    Serial.println(aqpGamepad.configuration.getModelNumber());
    Serial.print("         Device SN : ");
    Serial.println(aqpGamepad.configuration.getSerialNumber());
    Serial.print("     Device SW Rev : ");
    Serial.println(aqpGamepad.configuration.getSoftwareRevision());
    Serial.print("     Device FW Rev : ");
    Serial.println(aqpGamepad.configuration.getFirmwareRevision());
    Serial.print("     Device HW Rev : ");
    Serial.println(aqpGamepad.configuration.getHardwareRevision());
    Serial.print("        Device VID : ");
    Serial.println(aqpGamepad.configuration.getVid(), HEX);
    Serial.print("        Device PID : ");
    Serial.println(aqpGamepad.configuration.getPid(), HEX);
    Serial.print("   Device GUID Ver : ");
    Serial.println(aqpGamepad.configuration.getGuidVersion());
    Serial.print("     Device TX Lvl : ");
    Serial.println(aqpGamepad.configuration.getTXPowerLevel());
  } else {
    Serial.println("No device connected");
  }
  Serial.println();
}

void statsOut() {
  if (aqpGamepad.isConnected()) {

    // Get the HEX address as a string;
    Serial.print("     StringAddress : ");
    Serial.println(aqpGamepad.getStringAddress());

    // Get the HEX address as an NimBLEAddress instance
    Serial.print("     NimBLEAddress : ");

    NimBLEAddress bleAddress = aqpGamepad.getAddress();
    Serial.println(bleAddress.toString().c_str());

    // Get values directly from an NimBLEConnInfo instance
    NimBLEConnInfo peerInfo = aqpGamepad.getPeerInfo();

    Serial.print("      Peer Address : ");
    Serial.println(peerInfo.getAddress().toString().c_str());      // NimBLEAddress
    Serial.print("   Peer ID Address : ");
    Serial.println(peerInfo.getIdAddress().toString().c_str());    // NimBLEAddress
    Serial.print("   Peer ConnHandle : ");
    Serial.println(peerInfo.getConnHandle());                      // uint16_t
    Serial.print(" Peer ConnInterval : ");
    Serial.println(peerInfo.getConnInterval());                    // uint16_t
    Serial.print("  Peer ConnTimeOut : ");
    Serial.println(peerInfo.getConnTimeout());                     // uint16_t
    Serial.print("  Peer ConnLatency : ");
    Serial.println(peerInfo.getConnLatency());                     // uint16_t
    Serial.print("          Peer MTU : ");
    Serial.println(peerInfo.getMTU());                             // uint16_t
    Serial.print("     Peer isMaster : ");
    Serial.println(peerInfo.isMaster());                           // bool
    Serial.print("      Peer isSlave : ");
    Serial.println(peerInfo.isSlave());                            // bool
    Serial.print("     Peer isBonded : ");
    Serial.println(peerInfo.isBonded());                           // bool
    Serial.print("  Peer isEncrypted : ");
    Serial.println(peerInfo.isEncrypted());                        // bool
    Serial.print("     Peer isAuth'd : ");
    Serial.println(peerInfo.isAuthenticated());                    // bool
    Serial.print("   Peer SecKeySize : ");
    Serial.println(peerInfo.getSecKeySize());                      // uint8_t
  } else {
    Serial.println("No device connected");
  }
  Serial.println();
}

// Send Gamepad diagnostics out serial connection
void debugOut() {
  Serial.print("dataOut=");                 // Bit value being sent out the wired connection (1 = ON, LOW ; 0 = OFF, HIGH)
  Serial.print(dataOut+256, BIN);
  Serial.print(" : XTrim=");                // Saved? xtrim value from aqgp_prefs
  Serial.print(XTrim);
  Serial.print(" : rawX=");                 // Raw X value (0-5119) as read by the ESP32 analog GPIO
  Serial.print(XValue);
  Serial.print(" : adjX=");                 // Adjusted X value (0-32730) in the range needed by Gamepad BLE
  Serial.print(adjXValue);
  Serial.print(" : YTrim=");                // Saved? ytrim value from aqgp_prefs
  Serial.print(YTrim);
  Serial.print(" : rawY=");                 // Raw Y value (0-5119) as read by the ESP32 analog GPIO
  Serial.print(YValue);
  Serial.print(" : adjY=");                 // Adjusted Y value (0-32730) in the range needed by Gamepad BLE
  Serial.print(adjYValue);
  Serial.print(" : battMax=");              // Saved? battmax value from aqgp_prefs
  Serial.print(battMax);
  Serial.print(" : rawBatt=");              // Raw battery level as read by the ESP32 analog GPIO
  Serial.print(rawBatt);
  Serial.print(" : bLevel=");               // Scaled battery level (0-100)
  Serial.print(battLevel);
  Serial.print(" : BTLE=");                 // Bluetooth LE connection status (0 disconnected, 1 connected)
  Serial.print(aqpGamepad.isConnected());
  Serial.print(" : dpadON=");               // DPAD mode ON?
  Serial.print(dpadON);
  Serial.print(" : bRowSwapON=");           // Button row swap mode ON?
  Serial.print(bRowSwapON);
  Serial.print(" : bcState=");              // Combined battery + connection status integer: (battery state * 10) + connection state
  Serial.println(battConState[1]);
}

// Display current configuration of DPAD and Button Row Swap on RGB LED
void showLayout() {
  if (dpadON) {
    theLED = YELLOW;
    updateNeoPixel();
  } else {
    theLED = MAGENTA;
    updateNeoPixel();
  }
  delay(1500);
  if (bRowSwapON) {
    theLED = GREEN;
    updateNeoPixel();
  } else {
    theLED = RED;
    updateNeoPixel();
  }
  delay(1500);
  theLED = BLACK;
  updateNeoPixel();

}

// Toggle DPAD <> Analog state
void DPSwitch() {

  // Flip the dpadON value
  dpadON = !dpadON;

  // Save the new dpadON value to NVS
  aqgp_prefs.begin("aqgp", false, 0);

  // Blink the RGB LED
  if (dpadON) {
    Serial.println("Switching to DPAD mode...");
    Serial.println("  (system will reset)");
    aqgp_prefs.putUInt("dpadon", 1);
    for (int i = 0; i < 3; i++) {
      theLED = YELLOW;
      updateNeoPixel();
      delay(500);
      theLED = BLACK;
      updateNeoPixel();
      delay(500);
    }
  } else {
    Serial.println("Switching to Analog mode...");
    Serial.println("  (system will reset)");
    aqgp_prefs.putUInt("dpadon", 0);
    for (int i = 0; i < 3; i++) {
      theLED = MAGENTA;
      updateNeoPixel();
      delay(500);
      theLED = BLACK;
      updateNeoPixel();
      delay(500);
    }
  }
  Serial.println();
  aqpGamepad.resetButtons();
  aqgp_prefs.end();
  ESP.restart();
}

// Toggle button rows (1 2 3) <> (4 5 6)
void BRSwap() {

  // Flip the dpadON value
  bRowSwapON = !bRowSwapON;

  // Save the new bRowSwapON value to NVS
  aqgp_prefs.begin("aqgp", false, 0);

  // Blink the RGB LED
  if (bRowSwapON) {
    Serial.println("Switching to button  >   (1)  (2)  (3)");
    Serial.println("  row configuration  >     (4)(5)(6)  ");
    Serial.println("(system will reset)  >");
    aqgp_prefs.putUInt("browswapon", 1);
    for (int i = 0; i < 4; i++) {
      theLED = GREEN;
      updateNeoPixel();
      delay(500);
      theLED = BLACK;
      updateNeoPixel();
      delay(500);
    }
  } else {
    Serial.println("Switching to button  >   (4)  (5)  (6)");
    Serial.println("  row configuration  >     (1)(2)(3)  ");
    Serial.println("(system will reset)  >");
    aqgp_prefs.putUInt("browswapon", 0);
    for (int i = 0; i < 4; i++) {
      theLED = RED;
      updateNeoPixel();
      delay(500);
      theLED = BLACK;
      updateNeoPixel();
      delay(500);
    }
  }
  Serial.println();
  aqpGamepad.resetButtons();
  aqgp_prefs.end();
  ESP.restart();
}

// Loop through the 8 buttons in SWAPPED mode, check for changes, and apply updates
void checkButtons() {
  // buttonState[button_num][0 prev, 1 curr]
  // buttonPinMap[button_num][0 GPIO, 1 GP Map]
  for (int i = 0; i < 8; i++) {
    // Set button's current logical state to its current physical state
    buttonState[i][1] = digitalRead(buttonPinMap[i][0]);
    // If the current state is different from its last state...
    if (buttonState[i][1] != buttonState[i][0]) {
      // ...if its current state is PRESSED...
      if (buttonState[i][1] == LOW) {
        // ...if there is a BLE connection, update that...
        if (aqpGamepad.isConnected()) { aqpGamepad.press(buttonPinMap[i][1]); };
        // ...and combine the previous button data with this button's data.
        buttonOut |= buttonByte[i];
      } else {
        // Otherwise its current state is RELEASED, so
        // if there is a BLE connection, update that...
        if (aqpGamepad.isConnected()) { aqpGamepad.release(buttonPinMap[i][1]); };
        // ...and remove this button's data from the previous button data.
        buttonOut &= ~buttonByte[i];
      }
    }
    // Set the previous button state to the current one.
    buttonState[i][0] = buttonState[i][1];
  }
  
  // Combine button data into wired data output.
  // (Wired data was blanked at the top of the main loop.)
  dataOut |= buttonOut;
}

// Loop through the 8 buttons in NOT SWAPPED mode, check for changes, and apply updates
void checkButtonsNS() {
  // buttonState[button_num][0 prev, 1 curr]
  // buttonPinMapNS[button_num][0 GPIO, 1 GP Map]
  for (int i = 0; i < 8; i++) {
    // Set button's current logical state to its current physical state
    buttonState[i][1] = digitalRead(buttonPinMapNS[i][0]);
    // If the current state is different from its last state...
    if (buttonState[i][1] != buttonState[i][0]) {
      // ...if its current state is PRESSED...
      if (buttonState[i][1] == LOW) {
        // ...if there is a BLE connection, update that...
        if (aqpGamepad.isConnected()) { aqpGamepad.press(buttonPinMapNS[i][1]); };
        // ...and combine the previous button data with this button's data.
        buttonOut |= buttonByteNS[i];
      } else {
        // Otherwise its current state is RELEASED, so
        // if there is a BLE connection, update that...
        if (aqpGamepad.isConnected()) { aqpGamepad.release(buttonPinMapNS[i][1]); };
        // ...and remove this button's data from the previous button data.
        buttonOut &= ~buttonByteNS[i];
      }
    }
    // Set the previous button state to the current one.
    buttonState[i][0] = buttonState[i][1];
  }
  
  // Combine button data into wired data output.
  // (Wired data was blanked at the top of the main loop.)
  dataOut |= buttonOut;
}

// Read the sample values from the thumbsticks and update
void checkThumbs() {

  // Loop to sample the thumb stick a number of times
  // and normalize the value by averaging the samples
  for (int i = 0; i < thumbSamples; i++) {
    XValues[i] = analogRead(XPIN) + XTrim;
    YValues[i] = analogRead(YPIN) + YTrim;
    XValue += XValues[i];
    YValue += YValues[i];
    delay(delaySamples);
  }

  // Average samples
  XValue = XValue / thumbSamples;
  YValue = YValue / thumbSamples;

  // Scale thumbstick values to Gamepad BLE range, taking into account trim values
  adjXValue = map(XValue, 0, 5112, 0, 32737);
  adjYValue = map(YValue, 0, 5112, 0, 32737);

  // Set aqpGamepad thumbstick values
  if (aqpGamepad.isConnected()) {
    if (!dpadON) {
      aqpGamepad.setX(adjXValue);
      aqpGamepad.setY(adjYValue);
    }
  };

  // Scale thumbstick X & Y values to a positive/negative
  // range for performing the trigonometric calculations
  dirXValue = map(XValue, 0, 5112, -2048, 2048);
  dirYValue = map(YValue, 0, 5112, -2048, 2048);

  // Further scale X & Y values to a range of 32 values
  // for determining which of the 16 direction segments
  // they fall within.
  x = dirXValue / 128.0f;
  y = dirYValue / 128.0f;

  // Update the "dead zone" the middle of the thumbstick
  // when no input is being provided.
  deadZone = sqrtf(x * x + y * y);
  // Update the degree holder (0-359) the angle.
  angle = 0;
  // Create a variable to hold the 16 segment sector the
  // thumbstick is positioned within. The default of 0
  // indicates it's in the "dead zone".
  int p = 0;

  // Check to make sure the thumbstick is outside the "dead zone".
  if (deadZone > dzFactor) {
    // Calculate the angle and scale it.
    angle = atan2f(y, x) / (float)M_PI * 180.0f + 180.0f;
    // Calculate the sector and scale it to 1-16 using the "middle" of the angle range
    p = ((int)((angle + 11.25) / 22.5f) + 8) % 16 + 1;
  }

  // Set the current thumbstick state to the sector
  thumbState[1] = p;
  // If this is different from the previous sector...
  if (thumbState[1] != thumbState[0]) {
    // Update DPAD if on
    if (dpadON) {
      aqpGamepad.setHat1(dpadSwitch[p]);
      thumbOut = dpadByte[thumbState[1]];
      // Update X & Y on Gamepad to DPAD optimized values
      // aqpGamepad.setX(dpadThumb[thumbState[1]], 0 );
      // aqpGamepad.setY(dpadThumb[thumbState[1]], 1 );
      aqpGamepad.setX(dpadThumb[p][0] );
      aqpGamepad.setY(dpadThumb[p][1] );
    } else {
      // ...update the thumbstick output. Note that there is
      // no blending of this data from the previous state
      thumbOut = thumbByte[thumbState[1]];
    }
    // Set the previous thumbstick sector to the current
    thumbState[0] = p;
  }

  // Combine thumbstick data with the already combined button data
  dataOut |= thumbOut;
}

// Checks the state of the battery
void checkBattery() {

  // Loop to sample the battery a number of times
  // and normalize the value by averaging the samples
  for (int i = 0; i < battSamples; i++) {
    battValues[i] = analogRead(BATTDIV);
    rawBatt += battValues[i];
    delay(delaySamples);
  }

  // Average the values for normalization...
  rawBatt = int (rawBatt / battSamples);
  // ...and scale it to an expected 1-100 range
  battLevel = map(rawBatt, 0, battMax, 1, 100);

  // Publish this to the BLE Gamepad.
  if (aqpGamepad.isConnected()) {
    aqpGamepad.setBatteryLevel(battLevel);
  }

  // If the battery is 50-100%...
  if (battLevel > 49) {
    batteryState[1] = 1;
  }
  // If the battery is between 25-49%...
  if ((battLevel < 50) && (battLevel > 24)) {
    batteryState[1] = 2;
  }
  // If the battery is between 10-24%...
  if ((battLevel < 25) && (battLevel > 9)) {
    batteryState[1] = 3;
  }
  // If the battery is below 10%...
  if (battLevel < 10) {
    batteryState[1] = 4;
  }

  // Update previous state to current state.
  batteryState[0] = batteryState[1];
}

// Checks the state of connectivity
void checkConnect() {
  // If BLE is connected...
  if (aqpGamepad.isConnected()) {
    connectState[1] = 2;
  } else {
    connectState[1] = 1;
  }
  // Update previous state to current state.
  connectState[0] = connectState[1];
}

// Set battery and connectivity state
void setState() {
  // Combine battery and connect states into one number.
  battConState[1] = (batteryState[1] * 10) + connectState[1];

  // Update previous states with current states.
  battConState[0] = battConState[1];
}

// Calculates current color & pulse rate of LED
void calcRGB() {
  cycleLEDWait = battLevel * timeScaler;

  // Process every possible combination of states.
  switch (battConState[1]) {
    // Batt 50-100%, BLE NOT connected
    // Set color to a solid MAGENTA
    case 11:
      theLED = CYAN;
      break;
    // Batt 50-100%, BLE connected
    // Set color to a solid BLUE
    case 12:
      theLED = BLUE;
      break;

    // Batt 25-49%, BLE NOT connected
    // Pulse MAGENTA to BLACK to MAGENTA
    case 21:
      if (millis() >= cycleLED) {
        // See if we're at the "bottom" of the pulse cycle...
        if (theLED.ledB < 1) {
          theLED = BLACK;
          // ...and send it in the positive direction.
          counterLED = 1;
        }
        // Then see if we're at the "top" of the pulse cycle...
        if (theLED.ledB > 32) {
          theLED = CYAN;
          // ...and send it in the negative direction.
          counterLED = -1;
        }
        theLED.ledR += counterLED;
        theLED.ledG = 0;
        theLED.ledB += counterLED;
        cycleLED = millis() + cycleLEDWait;
      }
      break;

    // Batt 25-49%, BLE connected
    // Pulse BLUE to BLACK TO BLUE
    case 22:
      if (millis() >= cycleLED) {
        // See if we're at the "bottom" of the pulse cycle...
        if (theLED.ledB < 1) {
          theLED = BLACK;
          // ...and send it in the positive direction.
          counterLED = 1;
        }
        // Then see if we're at the "top" of the pulse cycle...
        if (theLED.ledB > 32) {
          theLED = BLUE;
          // ...and send it in the negative direction.
          counterLED = -1;
        }
        theLED.ledR = 0;
        theLED.ledG = 0;
        theLED.ledB += counterLED;
        cycleLED = millis() + cycleLEDWait;
      }
      break;

    // Batt 10-24%, BLE NOT connected
    // Pulse MAGENTA to BLACK to MAGENTA
    case 31:
      if (millis() >= cycleLED) {
        // See if we're at the "bottom" of the pulse cycle...
        if (theLED.ledB < 1) {
          theLED = BLACK;
          // ...and send it in the positive direction.
          counterLED = 1;
        }
        // Then see if we're at the "top" of the pulse cycle...
        if (theLED.ledB > 32) {
          theLED = CYAN;
          // ...and send it in the negative direction.
          counterLED = -1;
        }
        theLED.ledR += counterLED;
        theLED.ledG = 0;
        theLED.ledB += counterLED;
        cycleLED = millis() + cycleLEDWait;
      }
      break;

    // Batt 10-24%, BLE connected
    // Pulse BLUE to BLACK to BLUE
    case 32:
      if (millis() >= cycleLED) {
        // See if we're at the "bottom" of the pulse cycle...
        if (theLED.ledB < 1) {
          theLED = BLACK;
          // ...and send it in the positive direction.
          counterLED = 1;
        }
        // Then see if we're at the "top" of the pulse cycle...
        if (theLED.ledB > 32) {
          theLED = BLUE;
          // ...and send it in the negative direction.
          counterLED = -1;
        }
        theLED.ledR = 0;
        theLED.ledG = 0;
        theLED.ledB += counterLED;
        cycleLED = millis() + cycleLEDWait;
      }
      break;

    // Battery <10% or other untrapped errors
    // Set timer to pulse RED to BLACK to RED
    default:
      if (millis() >= cycleLED) {
        // See if we're at the "bottom" of the pulse cycle...
        if (theLED.ledR < 1) {
          theLED = BLACK;
          // ...and send it in the positive direction.
          counterLED = 1;
        }
        // Then see if we're at the "top" of the pulse cycle...
        if (theLED.ledR > 32) {
          theLED = RED;
          // ...and send it in the negative direction.
          counterLED = -1;
        }
        theLED.ledR += counterLED;
        theLED.ledG = 0;
        theLED.ledB = 0;
        cycleLED = millis() + cycleLEDWait;
      }
      break;
  }
}

// Update the NeoPixel RGB LED
void updateNeoPixel() {
  // Update the RGB LED values...
  pixels.setPixelColor(0, theLED.ledR, theLED.ledG, theLED.ledB);
  // ...and publish them to the device.
  pixels.show();
}

// Takes a byte and assigns its bits to individual GPIO pins for the wired connection.
void shiftOut(byte myDataOut) {
  // Start at the MSB/high bit, and work down.
  for (int i = 7; i >= 0; i--) {

    // This next section may seem contradictory, but the myDataOut bits
    // represent a byte in traditional binary format, where 1=ON/HIGH/TRUE
    // and 0=OFF/LOW/FALSE, but they are presented electronically on the
    // GPIO pins in the inverse, where 1=OFF/LOW/FALSE and 0=ON/HIGH/TRUE.

    // So, if the saved bit and current bit are 1...
    if (myDataOut & (1 << i)) {
      // ...set that GPIO pin/bit to LOW.
      digitalWrite(dataPins[i], LOW);
      // Otherwise...
    } else {
      // ...set that GPIO pin/bit to HIGH.
      digitalWrite(dataPins[i], HIGH);
    }
  }
}
