# ~~PN5180~~ SL015M-cardio (keypad-matrix version)
~~PN5180~~ SL015M eAmusement wavepass USB HID card reader (cardio) with integrated keypad

This is a quick and dirty adaptation to SL015M as it was used in many ACRealIO devices, to allow owners to use it as cardio as well.

# Supported devices

Leonardo, Pro Micro

Note that the SL015M doesn't support FeliCa cards.

# Pinout

SL015M out on pin 2
SL015M Serial interface on pins 0(Arduino RX, ex 19), 1(Arduino TX)

If you're coming from an arduino MEGA with your ACRealIO:

- wire that goes to MEGA pin 20 now goes to Leonardo/ProMicro pin 2
- wire that goes to MEGA pin 19 now goes to Leonardo/ProMicro pin 0
- wire that goes to MEGA pin 18 now goes to Leonardo/ProMicro pin 1

# Keypad Pinout

Connect the row pins to gpio 4 9 8 6.
Connect the column pins to gpio 5 3 7.

# How to use

Download zip, install Keypad library by Stanley and Brevig directly from Arduino IDE, flash the firmware, unplug, connect the SL015M and keypad to the Arduino.

Congratulations, your device should work just like a real cardio, use 
your favorite tools instructions to play (e.g. "spicetools -cardio" ),
and the keypad should be recognized as an additional USB peripheral.
