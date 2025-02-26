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
To-Do List:
--------------------------------------------------------------------------------------------

 - OTA updates?
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