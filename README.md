[![Donate](https://img.shields.io/badge/Donate-PayPal-green.svg)](https://www.paypal.com/donate?hosted_button_id=WT735CX4UMZ9U)

# PN5180-cardio

PN5180 eAmusement wavepass USB HID card reader (cardio) with optional integrated keypad, ISO14443 to FeliCa spoof (for Aime, Nesica, BANAPASSPORT compatibility)

## Alternate branches

Be sure to check out alternate branches for variants using other backends or hardware.

# Acknowledgments

This work is based on zyp's cardio (obviously).

ISO15693 code is based on [ATrappmann/PN5180-Library](https://github.com/ATrappmann/PN5180-Library).

ISO14443 implementation taken from [tueddy/PN5180-Library/ISO14443](https://github.com/tueddy/PN5180-Library/tree/ISO14443).

The keypad code uses the Keypad library by Mark Stanley and Alexander Brevig.

Spiceapi version provided by [goat](https://github.com/goaaats) (thanks! :)), refer to SpiceAPI branch for more information.

# Supported devices

Code has been tested on Arduino Due, Leonardo, and Pro Micro.

Arduino Due is recommended because it is 3.3v logic (or Pro Micro in 3.3v mode).
According to the PN5180 datasheet, a 5v<->3.3v level shifter is required
for use an Arduino Leonardo. However, I tested without it and everything
was working properly, and my PN5180 is not fried. I still recommend a Due
or a 3.3V pro micro just in case.

# Pinout

Note that the 3.3v pin from PN5180 is not connected to the arduino.

Because PN5180 is an SPI device, some pins are constrained,

Arduino | Level Converter | PN5180 pin mapping:
--- | --- | ---
5V      |      <-->       |      5V
GND    |        <-->      |       GND
5V      | HV | .
GND     | GND (HV) | .
 .        |   LV              | 3.3V
  .      |    GND (LV)        | GND
SCK    | HV1 - LV1       | SCK
MISO      |     <---        | MISO
MOSI    | HV3 - LV3       | MOSI
10   | HV4 - LV4      | NSS (=Not SS -> active LOW)
9  |       <---     |        BUSY
7 | HV2 - LV2      | RST

Please refer to your Arduino pinout to find SCK, MISO and MOSI pins
(on Due and Leonardo they are on the SPI / ICSP header, on Pro Micro
they are pins 15 (SCK),14 (MISO),16 (MOSI))

Unlike the other pins, NSS BUSY and RST can be moved to other gpios
if you need to.

# Keypad Pinout

Connect the row pins to gpio 1 6 5 3.
Connect the column pins to gpio 2 0 4.

If you want to use the keypad, you have to set `#define WITH_KEYPAD 1` in Config.h

You should also install Keypad library by Stanley and Brevig (can be installed directly from Arduino IDE library manager).

# How to use

- Download zip
- (optional) set `#define WITH_KEYPAD 1` (in PN5180-cardio.ino user configurable options) and install Keypad library by Stanley and Brevig directly from Arduino IDE
- flash the firmware
- unplug the arduino
- connect the PN5180 and keypad to the Arduino.

Congratulations, your device should work just like a real cardio, use 
your favorite tools instructions to play (e.g. `spicetools -cardio`).

The keypad should be recognized as an additional USB device.

# SPICEAPI

If you insist on using an arduino without HID support (ie. atmega328p based boards like Uno, Nano, Mega...), then you can still use this code through spiceapi. Refer to SpiceAPI branch for more information.

## Donation

If this project helps you and you want to give back, you can help me with my future projects.

While not necessary, donations are much appreciated and will only go towards funding future github projects (arcade hardware ain't cheap :( ).

Of course you'll also receive my gratitude and I'll remember you if you post a feature request ;)

[![paypal](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/donate?hosted_button_id=WT735CX4UMZ9U)
