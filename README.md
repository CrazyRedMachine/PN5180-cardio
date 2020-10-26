# PN5180-cardio (keypad-matrix version)
PN5180 eAmusement wavepass USB HID card reader (cardio) with integrated keypad

# Acknowledgments

This work is based on zyp's cardio (obviously).

ISO15693 code is based on [ATrappmann/PN5180-Library](https://github.com/ATrappmann/PN5180-Library).

FeliCa code is inspired by [tueddy/PN5180-Library/ISO14443](https://github.com/tueddy/PN5180-Library/tree/ISO14443) ISO14443 implementation.

HID layer code is based on Matthew Heironimus' <Joystick.h> library.

The keypad code uses the Keypad library by Mark Stanley and Alexander Brevig.

# Supported devices

This code has been tested on Arduino Due, Leonardo, and Pro Micro.

Arduino Due is recommended because it is 3.3v logic (or Pro Micro in 3.3v mode).
According to the PN5180 datasheet, a 5v<->3.3v level shifter is required
for use an Arduino Leonardo. However, I tested without it and everything
was working properly, and my PN5180 is not fried. I still recommend a Due
or a 3.3V pro micro just in case.

# Pinout

Because PN5180 is an SPI device, some pins are constrained,

Arduino | Level Converter | PN5180 pin mapping:
--- | --- | ---
5V      |      <-->       |      5V
3.3V    |       <-->      |       3.3V
GND    |        <-->      |       GND
5V      | HV | .
GND     | GND (HV) | .
 .        |   LV              | 3.3V
  .      |    GND (LV)        | GND
SCLK    | HV1 - LV1       | SCLK
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

# How to use

Download zip, install Keypad library by Stanley and Brevig directly from Arduino IDE, flash the firmware, unplug, connect the PN5180 and keypad to the Arduino.

Congratulations, your device should work just like a real cardio, use 
your favorite tools instructions to play (e.g. "spicetools -cardio" ),
and the keypad should be recognized as an additional USB peripheral.
