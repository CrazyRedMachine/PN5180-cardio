#include "Config.h"
#include <Wire.h>
#include "src/PN532/PN532_I2C.h"
#include "src/PN532/PN532.h"
#include "src/Cardio.h"
#include <Keyboard.h>
#include <Keypad.h>

PN532_I2C pn532i2c(Wire);
PN532 nfc(pn532i2c);

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

  byte rowPins[ROWS] = {PIN_ROW1, PIN_ROW2, PIN_ROW3, PIN_ROW4}; //connect to the row pinouts of the keypad
  byte colPins[COLS] = {PIN_COL1, PIN_COL2, PIN_COL3}; //connect to the column pinouts of the keypad
  Keypad kpd = Keypad( makeKeymap(numpad), rowPins, colPins, ROWS, COLS );
#endif
 
void setup() {

#if WITH_KEYPAD == 1
  /* Keypad */
    kpd.setDebounceTime(10);
    Keyboard.begin();
#endif

/* NFC */
  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata)
  {
Serial.begin(115200);
Serial.print("Didn't find PN53x board");
    while (1) {delay(10);};      // halt
  }

  // Got ok data, print it out!
/*  Serial.print("Found chip PN5"); Serial.println((versiondata >> 24) & 0xFF, HEX);
  Serial.print("Firmware ver. "); Serial.print((versiondata >> 16) & 0xFF, DEC);
  Serial.print('.'); Serial.println((versiondata >> 8) & 0xFF, DEC);
*/
  // Set the max number of retry attempts to read from a card
  // This prevents us from waiting forever for a card, which is
  // the default behaviour of the PN532.
  nfc.setPassiveActivationRetries(0xFF);
  nfc.SAMConfig();

//  memset(_prevIDm, 0, 8);

  Cardio.begin(false);
}

unsigned long lastReport = 0;
uint16_t cardBusy = 0;

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
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)

  // check for FeliCa card
  uint8_t ret;
  uint16_t systemCode = 0xFFFF;
  uint8_t requestCode = 0x01;       // System Code request
  uint8_t idm[8];
  uint8_t pmm[8];
  uint16_t systemCodeResponse;

  // Wait for an FeliCa type cards.
  // When one is found, some basic information such as IDm, PMm, and System Code are retrieved.
  ret = nfc.felica_Polling(systemCode, requestCode, idm, pmm, &systemCodeResponse, 500);

#if WITH_KEYPAD == 1
  keypadCheck();
#endif
    if (ret == 1) {
      Cardio.setUID(2, idm);
      Cardio.sendState();      
      lastReport = millis();
      cardBusy = 3000;
      uidLength = 0;
      return;
    }

   // check for ISO14443 card

#if WITH_KEYPAD == 1
  keypadCheck();
#endif
    if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength)) {
    for (int i=0; i<8; i++) {
      hid_data[i] = uid[i%uidLength];
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
