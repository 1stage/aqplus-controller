/*

Aq+ Gamepad
--------------------------------------------------------------------------------------------
by Sean P. Harrington
sph@1stage.com
aquarius.1stage.com
--------------------------------------------------------------------------------------------

Updated 26 Feb 2025

--------------------------------------------------------------------------------------------
Overview:
--------------------------------------------------------------------------------------------

The Aq+ Gamepad is a modern game controller for Aquarius and Aquarius+ 8-bit Z80-based 
computers. It features either wired (Mini/Micro/MX expander required for original Aquarius)
or wireless (Aquarius+) connectivity, and emulates the original Aquarius hand controller in
a form-factor that is more comfortable and fun to use.

The system was developed using an ESP32-S3-DevKitC module as the microcontroller and BLE 
(Bluetooth Low Energy) as the wireless solution. It is configured using the Arduino Core, 
and uses the "ESP32-S3-Box" board profile. Additionally, BLE connectivity is coded using 
the ESP32-BLE-Gamepad library, which itself relies on the NimBLE-Arduino library. The RGB 
LED serves as a system status indicator and uses the Adafruit NeoPixel library. All libraries 
can be loaded through the Arduino Library Manager. The system also features a LiPo 
(lithium polymer) battery, with smart charging.

The system uses the Preferences library to save in non-volatile storage (NVS) the settings
specific to the individual device, such as X, Y, & battery trim settings, serial number,
unit name, etc. These settings will get removed if the Tools > Erase All Flash Before Sketch
Upload option is enabled. So ensure that it is shown as DISABLED.

--------------------------------------------------------------------------------------------
Features:
--------------------------------------------------------------------------------------------

Quick pairing with Aquarius+ systems:
  0.  You should be on Aquarius+ OS V1.2 or later
  1.  Turn on Aquarius+ and activate Bluetooth
    - CTRL+TAB to bring up Settings overlay
    - Arrow key down to ESP Settings and hit ENTER
    - Arrow key to Bluetooth and hit ENTER
    - Arrow key to ON/OFF and toggle to ON with ENTER
  2.  Turn on Aq+ Gamepad OR plug into power via USB Micro port:
    - The main LED on the Aq+ Gamepad should be on and have a CYAN (aqua) color
    - The charging light (RED) may also be on
  3.  In the Aquarius+ Settings overlay:
    - Arrow down to "Aq+ Gamepad" in the Found Devices section
    - Hit ENTER, and give it a unique name to identify it, and hit ENTER again
    - The main LED on the Aq+ Gamepad should turn to BLUE color
    - The new Aq+ Gamepad should be listed by this unique name in the Connected Devices section
  4.  Hit ESCAPE key until the Settings overlay goes away
  5.  Your Aq+ Gamepad is now mapped to Hand Controller 1
    - If you add another Aq+ Gamepad after, it will map to Hand Controller 2

Ability to toggle DPAD (8 direction thumbstick) or Analog mode (16 direction thumbstick):
  Note that this function will temporarily interrupt Aq+ Gamepad output and reset the device.
  1.  With the Aq+ Gamepad turned on (CYAN or BLUE LED), press and hold (in this order):
    - Thumbstick button (push in on thumbstick until it clicks)
    - Button 6 (upper right button, regardless of button row mapping)
    - Guide button (button at center of Aq+ Gamepad)
    - Let go of all buttons
  2.  Main LED will flash YELLOW (switching to DPAD mode) or MAGENTA (switching to Analog mode) 3 times
    - Aq+ will reset (takes about 2-3 seconds)
    - The main light will turn CYAN (aqua) and then BLUE (if paired to Aquarius+)

    - Analog mode:                      U
                                    UUL   UUR
                                  UL         UR
                                LUL           RUR
                               L        +        R
                                LDL           RDR
                                  DL         DR
                                    DDL   DDR
                                        D
                              
    - DPAD mode (default):              U
                                    
                                  UL         UR
                                       
                               L        +        R
                                       
                                  DL         DR
                                    
                                        D
                              

Ability to swap button rows (1 2 3) and (4 5 6):
  Note that this function will temporarily interrupt Aq+ Gamepad output and reset the device.
  0.  By default, button row (4 5 6) is closest to the lower right "wing" of the Aq+ Gamepad
  1.  With the Aq+ Gamepad turned on (CYAN or BLUE LED), press and hold (in this order):
    - Thumbstick button (push in on thumbstick until it clicks)
    - Button 5 (upper middle button, regardless of mapping)
    - Guide button (button at center of Aq+ Gamepad)
    - Let go of all buttons
  2.  Main LED will flash GREEN (switching to NOT SWAPPED mode) or RED (switching to SWAPPED mode) 4 times
    - Aq+ will reset (takes about 2-3 seconds)
    - The main light will turn CYAN (aqua) and then BLUE (if paired to Aquarius+)

    - SWAPPED mode (default):    (1)  (2)  (3)
                                  (4) (5) (6)

    - NOT SWAPPED mode:          (4)  (5)  (6)
                                  (1) (2) (3)

Ability to show current DPAD and Button Row Swap settings:
  Note that this function will temporarily interrupt Aq+ Gamepad output.
  1.  With the Aq+ Gamepad turned on (CYAN or BLUE LED), press and hold (in this order):
    - Thumbstick button (push in on thumbstick until it clicks)
    - Button 4 (upper left button, regardless of mapping)
    - Guide button (button at center of Aq+ Gamepad)
    - Let go of all buttons
  2.  Main LED will display two colors, one after the other, each for about 2 seconds:
    - DPAD setting
      -  YELLOW: DPAD   (8 direction mode)
      - MAGENTA: Analog (16 direction mode)
    - Button Row Swap setting
      -   GREEN: NOT SWAPPED mode
      -     RED: SWAPPED mode

Easy-to-Use Menu System, via USB serial terminal:
  1.  Download and install Arduino IDE (see Links & References section below)
  2.  Download and activate ESP32 boards, drivers, and supporting tools:
    - From within the Arduino IDE, go to the Boards Manager:
      - (SHFT+CTRL+B on Windows) 
      - Tools > Board: [current board name] > Boards Manager
    - In the "Filter your search..." box, type "esp32", then from the list below,
      select "esp32 by Espressif System", choosing version 3.1.1 or later from the
      dropdown.
    - Click on the INSTALL button
      - The system will download and install the ESP32 boards definitions
      - Depending on your connection, this could take a few minutes
  3.  Once installed, set the correct Board for this project:
    - From the menu, select Tools > Board: > esp32 > ESP32-S3-Box
  4.  Install the Arduino libraries for this project:
    - From within the Arduino IDE, go to the Library Manager:
      - (SHFT+CTRL+I on Windows) 
      - Tools > Manage Libraries...
    - In the "Filter your search..." box, type "ESP32-BLE", then from the list below,
      select "ESP32-BLE-Gamepad by lemmingDev", choosing version 0.7.3 or later from the
      dropdown.
    - Click on the INSTALL button
      - The system will download and install the library
      - It will also install "NimBLE-Arduino by h2zero",  2.2.2 or later
      - Depending on your connection, this could take a few minutes
    - Clear the "Filter your search..." box, and type "NeoPixel", then from the list below,
      select "Adafruit NeoPixel by Adafruit", choosing 1.12.4 or later or later from the
      dropdown.
    - Click on the INSTALL button
      - The system will download and install the library
      - Depending on your connection, this could take a few minutes
  5.  Connect the Aq+ Gamepad to your system using a USB micro cable.
    - You may need to use a system utility (i.e. Device Manager on Windows) to determine 
      what communication port the Aq+ Gamepad has been assigned, needed in the next steps.
  6.  Set the proper Board Settings for the project:
    - Tools > Port: > COMxx (whatever port your Aq+ Gamepad has been assigned)
    - Tools > Core Debug Level: > None
    - Tools > USB DFU On Boot: > Disabled
    - Tools > Erase All Flash Before Sketch Upload: > Disabled
    - Tools > USB Firmware MSC On Boot: > Disabled
    - Tools > Partition Scheme: > 16MB Flash (3MB App/9.9MB FATFS)
    - Tools > USB Mode: > Hardware CDC and JTAG
    - Tools > Programmer: > Esptool (ensure it is CHECKED)
  7.  Configure the Serial Monitor:
    - Tools > Serial Monitor
    - Within the Serial Monitor window or tab, from the drop-down, select New Line
    - Within the Arduino IDE, the Baud rate, stop, and parity bits are automatically set, 
      but for other terminal programs, they are 115200 baud, 8 bits, no parity
  Once all of this is set up, when the Aq+ Gamepad is attached, you can interact with the
  Aq+ Gamepad terminal menu by entering commands in the top box of the Serial Monitor and
  then typing ENTER. Below is the HELP menu, showing all available commands:

   A  - Toggle DPAD <> Analog state
   B  - Toggle button rows (1 2 3) <> (4 5 6)
   C  - Show configuration data
   D  - Show debug data x10, once every 1/4 second
   L  - Show current DPAD & Button Row settings
   M  - Auto-set max battery level (only when charge light is off)
   P  - Show preferences data
   R  - Reset X & Y trim to 0
   S  - Show BTLE statistics
   T  - Auto-trim X & Y axis
  H/? - Show Help (this) screen

--------------------------------------------------------------------------------------------
Troubleshooting:
--------------------------------------------------------------------------------------------

Aq+ Gamepad Won't Reconnect via Bluetooth: 
  If the Aquarius+ computer is power cycled (turned off and then on), or if it's ESP32 is reset (CTRL+SHIFT+ESC)
  while an Aq+ Gamepad is connected, the Aq+ Gamepad's main LED will remain BLUE, thinking it is still connected, 
  even though the connection has been broken. This leaves the Aq+ Gamepad in a state where the unit can't be 
  reconnected to. To fix:
  - Make sure the USB micro charging cable is not attached to the Aq+ Gamepad
  - Power cycle the Aq+ Gamepad using the switch on the back while the Aquarius+ computer is on and Bluetooth is enabled
  - The unit's main LED should start out CYAN and then turn BLUE
  - It should show up in the Overlay setting menu as a Connected device

Aq+ Gamepad Doesn't Control the Overlay Menu:
  The first Aq+ Gamepad that is connected via Bluetooth to the Aquarius+ at start up is assigned to 
  Hand Controller slot 1, and can be used to navigate the Overlay settings menu:
  - First, enable the Allow Gamepad navigation setting option in the Overlay menu
  - Second, the Aq+ Gamepad must be turned on, paired, and connected to the Aquarius+ as the FIRST
    controller attached
  - Third, if you've swapped button rows (1 2 3) <> (4 5 6), your Activate (Button 4) and Back (Button 5)
    will now be on the other row of buttons
  - Finally, the Aq+ Gamepad must be in DPAD mode rather than Analog mode to navigate up and down
    the Overlay menu list

Aq+ Gamepad Thumbstick is Generating a Direction Even When Centered:
  The Aq+ Gamepad uses a standard thumbstick component with two potentiometers that register 0 - ~5226 on both the
  X and Y axis, usually with a value of ~2613 as the middle range. While a small "dead zone" has been created in the
  center to compensate for this and create stability when at rest, it doesn't always block out values that are
  just beyond it. This is because while the thumbstick components are MOSTLY the same, they are not rigorously
  tested or calibrated, as they are commodity items. As such, the X and Y values have to be trimmed to better center
  the stick. To reset or calibrate the Aq+ Gamepad:
  - Connect the Aq+ Gamepad to a computer configured to communicate via serial terminal (i.e. Arduino IDE Serial Monitor)
  - Within the terminal window, type ? and ENTER to see the available menu of commands
  - Run a diagnostic by typing D and ENTER. If in any of the 10 rows the following conditions are true, 
    an auto-trim is needed:
    - The second column (dataOut) shows anything other than 0b100000000
    - Either the fourth (rawX) or seventh (rawY) columns show values that are below 2560 or above 2660
  - Run auto-trim by typing T and ENTER. The system will auto-trim the X & Y values.

Aq+ Gamepad Battery Never Shows as Fully Charged OR Over-Charged:
  The battery management circuitry on the Aq+ Gamepad ensures that an approved LiPo battery pack will charge safely
  and consistently. As such, while the reporting of the battery level may be a bit off, the circuit itself is
  managing it correctly. Simlar to the way in which the X and Y values for the thumbstick are reported based on
  electrical values on those sense pins, the battery is subject to the same fluctuations. Two preferences are
  saved within the Aq+ Gamepad, a maximum battery value that saves the top end range of the analog value output from the
  battery (managed by the battery management circuit), and a battery trim value that can be used to better update 
  the "perceived" top end of the battery to normalize to 100% when charging has stopped (no RED charging light).
  - For proper calibration of the battery level, the battery must be fully charged:
    - A battery that is not fully charged will have a red light glowing when it is plugged in via USB micro to
      either a computer or a power source
    - A battery that is fully charged will NOT have a red light glowing when it is plugged in
  - Connect the Aq+ Gamepad to a computer configured to communicate via serial terminal (i.e. Arduino IDE Serial Monitor)
  - Within the terminal window, type ? and ENTER to see the available menu of commands
  - Run a diagnostic by typing D and ENTER. If the tenth column (rawBatt) in any of the 10 rows show values that are 
    below 2560 or above 2660, a calibration is needed.
  - Run auto-set max battery level by typing M and ENTER. The system will auto-adjust the max battery value.

Terminal Isn't Working:
  - Ensure the Aq+ Gamepad and Arduino IDE are set-up correctly, per the instructions above
  - If you're using a different terminal program, your settings must match the ones indicated
    in the set-up section, particularly line terminations MUST ONLY be New Line, not NL + CR, etc.

--------------------------------------------------------------------------------------------
To-Do List:
--------------------------------------------------------------------------------------------

 - Web/GUI interface for prefs & settings
 - Test for & recover from Bluetooth "slave" disconnections (Aq+ power cycle, Aq+ ESP32 reset)

--------------------------------------------------------------------------------------------
Revisions:
--------------------------------------------------------------------------------------------

0.5.0, 26 FEB 2025 -  Added DPAD & Button Row Swap status check
0.4.0, 24 FEB 2025 -  Added toggle to SWAP button rows (1 2 3) <> (4 5 6)
                      Created terminal menu via USB serial for stats and prefs changes
0.3.0, 22 FEB 2025 -  Incorporated Preferences for NVS data, and removed SettingsManagerESP32 
                      Added toggle between Analog Thumb and DPAD/HAT
0.2.2, 21 FEB 2025 -  Fixed XY dead zone issues
                      Commented legacy code
                      Turned all main loop processes into funtions
                      Created TRIM variables for thumb stick
0.2.1, 18 FEB 2025 -  Fixed NeoPixel error (logging was too verbose)
0.2.0, 12 FEB 2025 -  Added 1+3+5 wired chord to AQ button
0.1.9, 03 DEC 2024 -  Created state machine for LED/Battery
0.1.8, 30 NOV 2024 -  Remapped GPIO pin assignments after PCB rework
0.1.7, 19 NOV 2024 -  Added SettingsManagerESP32 to save persistent variables
0.1.6, 17 NOV 2024 -  Added battery divider sense pin
0.1.5, 17 NOV 2024 -  Switched to GPIO rather than Shift Register
0.1.4, 16 NOV 2024 -  Reworked analog angle calculations from Aq+ ESP32 code
                      Created buttonByte and thumbByte to combine into dataByte
0.1.3, 11 NOV 2024 -  Shift register implemented for wired connections
                      Turn button components into arrays and loop
0.1.2, 09 NOV 2024 -  Button alignment with Xbox mappings
                      Thumbstick scaling fixed
                      Buttons transitioned to single-pin (from matrix)
0.1.1, 08 NOV 2024 -  Button matrix (unsuccessful)
0.1.0, 07 NOV 2024 -  Initial version, using Gamepad example (four buttons)
                      Installed Arduino bootloader
                      Identified compatible Board profile (ESP32-S3-Box)

--------------------------------------------------------------------------------------------
Links & References:
--------------------------------------------------------------------------------------------

AQP Gamepad GitHub: https://github.com/1stage/aqplus-controller
Arduino IDE: https://www.arduino.cc/en/software
ESP32-BLE-Gamepad: https://github.com/lemmingDev/ESP32-BLE-Gamepad
Adafruit NeoPixel: https://github.com/adafruit/Adafruit_NeoPixel
NimBLE Arduino: https://github.com/h2zero/NimBLE-Arduino
ESP32-S3_DevKitC: https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32s3/esp32-s3-devkitc-1/index.html

*/