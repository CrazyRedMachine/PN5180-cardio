#include "Config.h"
#include "src/PN5180/PN5180.h"
#include "src/PN5180/PN5180FeliCa.h"
#include "src/PN5180/PN5180ISO15693.h"
#if WITH_ISO14443 == 1
  #include "src/PN5180/PN5180ISO14443.h"
#endif
#include "CARDIOHID.h"
#include <Keyboard.h>
#include <Keypad.h>

PN5180FeliCa nfcFeliCa(PN5180_PIN_NSS, PN5180_PIN_BUSY, PN5180_PIN_RST);
PN5180ISO15693 nfc15693(PN5180_PIN_NSS, PN5180_PIN_BUSY, PN5180_PIN_RST);
#if WITH_ISO14443 == 1
  PN5180ISO14443 nfc14443(PN5180_PIN_NSS, PN5180_PIN_BUSY, PN5180_PIN_RST);
#endif
CARDIOHID_ Cardio;

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

  byte rowPins[ROWS] = {PIN_ROW1, PIN_ROW2, PIN_ROW3, PIN_ROW4};
  byte colPins[COLS] = {PIN_COL1, PIN_COL2, PIN_COL3};
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
}

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
  
#if WITH_ISO14443 == 1
// check for ISO14443 card
  nfc14443.reset();
#if WITH_KEYPAD == 1
  keypadCheck();
#endif
  nfc14443.setupRF();
  uint8_t uidLengthMF = nfc14443.readCardSerial(uid);

  if (uidLengthMF > 0) 
  {
      uid[0] &= 0x0F; //some games won't accept FeliCa cards with a first byte higher than 0x0F  
      Cardio.sendState(2, uid);
      lastReport = millis();
      cardBusy = 3000;
      uidLengthMF = 0;
      return;
    }
#endif /* ISO14443 */

  // check for FeliCa card
  nfcFeliCa.reset();
#if WITH_KEYPAD == 1
  keypadCheck();
#endif
  nfcFeliCa.setupRF();
  uint8_t uidLength = nfcFeliCa.readCardSerial(uid);
    if (uidLength > 0) {
      Cardio.sendState(2, uid);      
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
    Cardio.sendState(1, hid_data);
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
