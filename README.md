# ~~PN5180-cardio~~ PN532-cardio

PN532 eAmusement/Aime/Nesica/BANAPASSPORT NFC USB HID card reader (cardio) with optional integrated keypad, Spiceapi integration. Supports ISO14443 and FeliCa.

This is a mod of my PN5180-cardio project made to work with a PN532 module instead, over its I2C communication mode.

# Acknowledgments

This work is based on zyp's cardio (obviously).

PN532 code is taken from [elechouse](https://github.com/elechouse/PN532) library.

HID layer code is inspired from Matthew Heironimus' <Joystick.h> library.

The keypad code uses the Keypad library by Mark Stanley and Alexander Brevig.

Spiceapi version provided by [goat](https://github.com/goaaats) (thanks! :))

# Supported devices

USBHID code has been tested on Arduino Due, Leonardo, and Pro Micro.

SPICEAPI code has been tested on Arduino UNO.
It should support any SPI-capable arduino without native USB HID capabilities but might require fine-tuning
`SPICEAPI_WRAPPER_BUFFER_SIZE` and `SPICEAPI_WRAPPER_BUFFER_SIZE_STR` parameters in `PN532-cardio.ino`.

# Pinout

# Keypad Pinout

Connect the row pins to gpio 1 6 5 3.
Connect the column pins to gpio 2 0 4.

If you want to use the keypad, you have to set `#define WITH_KEYPAD 1` in PN532-cardio.ino in the top header options.

If you're using the USBHID mode, then you should also install Keypad library by Stanley and Brevig (can be installed directly from Arduino IDE library manager).

In SPICEAPI mode, it will work through Spiceapi instead.

# How to use

## USBHID

- Download zip
- (optional) set `#define WITH_KEYPAD 1` (in PN532-cardio.ino user configurable options) and install Keypad library by Stanley and Brevig directly from Arduino IDE
- flash the firmware
- unplug the arduino
- connect the PN532 and keypad to the Arduino.

Congratulations, your device should work just like a real cardio, use 
your favorite tools instructions to play (e.g. `spicetools -cardio`).

The keypad should be recognized as an additional USB device.

## SPICEAPI

- Download zip
- set `#define WITH_SPICEAPI 1` (in PN532-cardio.ino user configurable options)
- set `SPICEAPI_PASS` accordingly (in PN532-cardio.ino user configurable options)
- (optional) set `#define WITH_KEYPAD 1` (in PN532-cardio.ino user configurable options) and install Keypad library by Stanley and Brevig directly from Arduino IDE
- flash the firmware
- unplug the arduino
- connect the PN532 and keypad to the Arduino.

Congratulations, your device should work, run `spicetools -apiserial COMx` (with `x` your arduino COM port number).
