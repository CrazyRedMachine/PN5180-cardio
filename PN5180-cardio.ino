// NAME: PN5180-CardIO.ino
//
// DESC: Implementation of zyp's famous "cardio" using an Arduino and a PN5180-NFC Module
//       from NXP Semiconductors.
//
// Based on PN5180-Library Copyright (c) 2018 by Andreas Trappmann. All rights reserved.
// Copyright (c) 2020 by CrazyRedMachine.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public 
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Lesser General Public License for more details.
//
// BEWARE: SPI with an Arduino to a PN5180 module has to be at a level of 3.3V
// use of logic-level converters from 5V->3.3V is absolutly neccessary
// on most Arduinos for all input pins of PN5180!
// If used with an ESP-32, there is no need for a logic-level converter, since
// it operates on 3.3V already.
//
// ADDITIONAL NOTES BY CrazyRedMachine :
// I tried to plug directly one of my PN5180 module to an Arduino Leonardo and to
// an Arduino Mega without any issue. That being said, Arduino Due is probably a 
// better choice since it supports this code and operates on 3.3V.
//
// Arduino <-> Level Converter <-> PN5180 pin mapping:
// 5V             <-->             5V
// 3.3V           <-->             3.3V
// GND            <-->             GND
// 5V      <-> HV
// GND     <-> GND (HV)
//             LV              <-> 3.3V
//             GND (LV)        <-> GND
// SCLK,13 <-> HV1 - LV1       --> SCLK
// MISO,12        <---         <-- MISO
// MOSI,11 <-> HV3 - LV3       --> MOSI
// SS,10   <-> HV4 - LV4       --> NSS (=Not SS -> active LOW)
// BUSY,9         <---             BUSY
// Reset,7 <-> HV2 - LV2       --> RST
//

//Set to 1 to enable keypad matrix
#define WITH_KEYPAD 0

#include "src/PN5180/PN5180.h"
#include "src/PN5180/PN5180FeliCa.h"
#include "src/PN5180/PN5180ISO15693.h"
#include "src/Cardio.h"

#if WITH_KEYPAD == 1
  #include <Keypad.h>
  #include <Keyboard.h>
#endif

#define PN5180_NSS  10
#define PN5180_BUSY 9
#define PN5180_RST  7

PN5180FeliCa nfcFeliCa(PN5180_NSS, PN5180_BUSY, PN5180_RST);
PN5180ISO15693 nfc15693(PN5180_NSS, PN5180_BUSY, PN5180_RST);

Cardio_ Cardio;

#if WITH_KEYPAD == 1
  /* Keypad declarations */
  const byte ROWS = 4;
  const byte COLS = 3;
  /* This is to use the toprow keys */
  char numpad[ROWS][COLS] = {
   {'7', '8', '9'},
   {'4', '5', '6'},
   {'1', '2', '3'},
   {'0', ',', '\337'}
  };

  byte rowPins[ROWS] = {1, 6, 5, 3}; //connect to the row pinouts of the keypad
  byte colPins[COLS] = {2, 0, 4}; //connect to the column pinouts of the keypad
  Keypad kpd = Keypad( makeKeymap(numpad), rowPins, colPins, ROWS, COLS );
#endif
 
void setup() {

#if WITH_KEYPAD == 1
  /* Keypad */
    kpd.setDebounceTime(10);
    Keyboard.begin();
#endif

/* NFC */
  nfcFeliCa.begin();
  nfcFeliCa.reset();

  uint8_t productVersion[2];
  nfcFeliCa.readEEprom(PRODUCT_VERSION, productVersion, sizeof(productVersion));
  if (0xff == productVersion[1]) { // if product version 255, the initialization failed
    exit(-1); // halt
  }
  
  uint8_t firmwareVersion[2];
  nfcFeliCa.readEEprom(FIRMWARE_VERSION, firmwareVersion, sizeof(firmwareVersion));
  uint8_t eepromVersion[2];
  nfcFeliCa.readEEprom(EEPROM_VERSION, eepromVersion, sizeof(eepromVersion));
  nfcFeliCa.setupRF();

  Cardio.begin(false);
}

uint32_t loopCnt = 0;
unsigned long lastReport = 0;
int cardBusy = 0;

// read cards loop
void loop() {
#if WITH_KEYPAD == 1
  /* KEYPAD */
  keypadCheck();
#endif
  
  /* NFC */
  if (millis()-lastReport < cardBusy) return;
  
  cardBusy = 0;
  uint8_t uid[8] = {0,0,0,0,0,0,0,0};
  uint8_t hid_data[8] = {0,0,0,0,0,0,0,0};

  // check for FeliCa card
  nfcFeliCa.reset();
#if WITH_KEYPAD == 1
  keypadCheck();
#endif
  nfcFeliCa.setupRF();
  uint8_t uidLength = nfcFeliCa.readCardSerial(uid);
    if (uidLength > 0) {
      Cardio.setUID(2, uid);
      Cardio.sendState();
      lastReport = millis();
      cardBusy = 3000;
      uidLength = 0;
      return;
    }

   // check for ISO-15693 card
  nfc15693.reset();
#if WITH_KEYPAD == 1
  keypadCheck();
#endif
  nfc15693.setupRF();
  // try to read ISO15693 inventory
  ISO15693ErrorCode rc = nfc15693.getInventory(uid);
  if (rc == ISO15693_EC_OK ) {
    for (int i=0; i<8; i++) {
      hid_data[i] = uid[7-i];
    }
    Cardio.setUID(1, hid_data);
    Cardio.sendState();
      lastReport = millis();
      cardBusy = 3000;
    return;
  }
  // no card detected
  lastReport = millis();
  cardBusy = 200;
}

#if WITH_KEYPAD == 1
void keypadCheck(){
    if (kpd.getKeys())
  {
    for (int i = 0; i < LIST_MAX; i++) // Scan the whole key list.
    {
      if ( kpd.key[i].stateChanged )   // Only find keys that have changed state.
      {
        switch (kpd.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
          case PRESSED:
            Keyboard.press(kpd.key[i].kchar);
            break;
          case HOLD:
            break;
          case RELEASED:
            Keyboard.release(kpd.key[i].kchar);
            break;
          case IDLE:
            break;
        }

      }
    }
  }
  
}
#endif
