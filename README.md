# PN5180-cardio
PN5180 eAmusement wavepass USB HID card reader (cardio)

# Acknowledgments

This work is based on zyp's cardio (obviously).

ISO15693 code is based on [ATrappmann/PN5180-Library](https://github.com/ATrappmann/PN5180-Library).

FeliCa code is inspired by [tueddy/PN5180-Library/ISO14443](https://github.com/tueddy/PN5180-Library/tree/ISO14443) ISO14443 implementation.

HID layer code is based on Matthew Heironimus' <Joystick.h> library.

# Supported devices

This code has been tested on Arduino Due and Leonardo.

Arduino Due is recommended because it is 3.3v logic.
According to the PN5180 datasheet, a 5v<->3.3v level shifter is required
for use an Arduino Leonardo. However, I tested without it and everything
was working properly, and my PN5180 is not fried. I still recommend a Due
just in case.

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
SS,10   | HV4 - LV4      | NSS (=Not SS -> active LOW)
BUSY,9  |       <---     |        BUSY
Reset,7 | HV2 - LV2      | RST

Please refer to your Arduino pinout to find SCK, MISO and MOSI pins
(on Due and Leonardo they are on the SPI / ICSP header)

Unlike the other pins, NSS BUSY and RST can be moved to other gpios
if you need to.

# How to use

Download zip, connect the PN5180 to the Arduino, flash the firmware.

Congratulations, your device should work just like a real cardio, use 
your favorite tools instructions to play (e.g. "spicetools -cardio" ).
