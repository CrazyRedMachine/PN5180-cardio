#include "Config.h"
#include "src/PN5180/PN5180.h"
#include "src/PN5180/PN5180FeliCa.h"
#include "src/PN5180/PN5180ISO15693.h"
#if WITH_ISO14443 == 1
  #include "src/PN5180/PN5180ISO14443.h"
#endif

#if WITH_KEYPAD == 1
  #include <Keypad.h>
#endif

  /* Wrapper Buffer Sizes - Should be tuned up or down depending on available memory. These are tuned for an Arduino UNO. */
  /* If your code hangs when sending a request, try adjusting it down or up */
  #define SPICEAPI_WRAPPER_BUFFER_SIZE 128
  #define SPICEAPI_WRAPPER_BUFFER_SIZE_STR 128

  #include "src/spiceapi/connection.h"
  #include "src/spiceapi/wrappers.h"

  char uidBuf[18];
  spiceapi::Connection spiceCon(256, SPICEAPI_PASS);

PN5180FeliCa nfcFeliCa(PN5180_PIN_NSS, PN5180_PIN_BUSY, PN5180_PIN_RST);
PN5180ISO15693 nfc15693(PN5180_PIN_NSS, PN5180_PIN_BUSY, PN5180_PIN_RST);
#if WITH_ISO14443 == 1
  PN5180ISO14443 nfc14443(PN5180_PIN_NSS, PN5180_PIN_BUSY, PN5180_PIN_RST);
#endif

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

  SPICEAPI_INTERFACE.begin(SPICEAPI_BAUD);
  while (!SPICEAPI_INTERFACE);
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
      formatUid(uid, uidBuf);
      spiceapi::card_insert(spiceCon, SPICEAPI_PLAYERNUM, uidBuf);
      lastReport = millis();
      cardBusy = 3000;
      uidLengthMF = 0;
      return;
    }
#endif

  // check for FeliCa card
  nfcFeliCa.reset();
#if WITH_KEYPAD == 1
  keypadCheck();
#endif
  nfcFeliCa.setupRF();
  uint8_t uidLength = nfcFeliCa.readCardSerial(uid);
    if (uidLength > 0) {
      formatUid(uid, uidBuf);
      spiceapi::card_insert(spiceCon, SPICEAPI_PLAYERNUM, uidBuf);
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
    
    formatUid(hid_data, uidBuf);
    spiceapi::card_insert(spiceCon, SPICEAPI_PLAYERNUM, uidBuf);
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
            break;
          case HOLD:
            break;
          case RELEASED:
            spiceapi::keypads_write(spiceCon, SPICEAPI_PLAYERNUM, &kpd.key[i].kchar);
            break;
          case IDLE:
            break;
        }

      }
    }
  }
}
#endif

void formatUid(uint8_t* ary, char* buf) {
  sprintf(buf, "%02X%02X%02X%02X%02X%02X%02X%02X", ary[0], ary[1], ary[2], ary[3], ary[4], ary[5], ary[6], ary[7]);
}
