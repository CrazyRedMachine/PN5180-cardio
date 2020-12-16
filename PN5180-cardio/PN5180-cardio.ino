/* === USER CONFIGURABLE OPTIONS === */

/* General usage on boards without USB MCU requires WITH_SPICEAPI instead */
#define WITH_USBHID 0

/* Keypad on boards without USB MCU requires WITH_SPICEAPI */
#define WITH_KEYPAD 1

/* Launch game with "-api 1337 -apipass changeme -apiserial COM1 -apiserialbaud 57600" or similar */
#define WITH_SPICEAPI 1
/* Adjust your serial port here(Serial, Serial1, Serial2, etc.) - WiFi/Network support is possible, but out of scope for this project */
#define SPICEAPI_INTERFACE Serial
#define SPICEAPI_BAUD 57600
#define SPICEAPI_PASS "changeme"
/* For games with multiple readers */
#define SPICEAPI_PLAYERNUM 0

//sneaky ISO14443 support (reader will pretend it was a FeliCa)
#define WITH_ISO14443 0

/* === END OF USER CONFIGURABLE OPTIONS === */

/* DO NOT MESS WITH THE LINES BELOW UNLESS YOU KNOW WHAT YOU'RE DOING */
#include "src/PN5180/PN5180.h"
#include "src/PN5180/PN5180FeliCa.h"
#include "src/PN5180/PN5180ISO15693.h"
#if WITH_ISO14443 == 1
  #include "src/PN5180/PN5180ISO14443.h"
#endif

#if WITH_USBHID == 1
  #if !defined(USBCON)
    #if WITH_SPICEAPI == 0
      #error WITH_SPICEAPI option is mandatory for non-USB MCU 
    #endif
    #warning The USBHID mode can only be used with a USB MCU (e.g. Arduino Leonardo, Arduino Micro, etc.).
    #define USBHID 0
  #else
    #define USBHID 1
    #include "src/Cardio.h"
  #endif
#endif

#if WITH_KEYPAD == 1
  #if !defined(USBCON)
    #if WITH_SPICEAPI == 0
      #error WITH_SPICEAPI option is mandatory to use keypads on non-USB MCU 
    #else
      #define USBKEYPAD 0
      #define SPICEKEYPAD 1
    #endif
  #else
    #define USBKEYPAD 1
    #define SPICEKEYPAD 0
    #include <Keyboard.h>
  #endif
  #include <Keypad.h>
#endif

#if WITH_SPICEAPI == 1
  /* Wrapper Buffer Sizes - Should be tuned up or down depending on available memory. These are tuned for an Arduino UNO. */
  /* If your code hangs when sending a request, try adjusting it down or up */
  #define SPICEAPI_WRAPPER_BUFFER_SIZE 128
  #define SPICEAPI_WRAPPER_BUFFER_SIZE_STR 128

  #include "src/spiceapi/connection.h"
  #include "src/spiceapi/wrappers.h"

  char uidBuf[18];
  spiceapi::Connection spiceCon(256, SPICEAPI_PASS);
#endif

#define PN5180_NSS  10
#define PN5180_BUSY 9
#define PN5180_RST  7

PN5180FeliCa nfcFeliCa(PN5180_NSS, PN5180_BUSY, PN5180_RST);
PN5180ISO15693 nfc15693(PN5180_NSS, PN5180_BUSY, PN5180_RST);
#if WITH_ISO14443 == 1
  PN5180ISO14443 nfc14443(PN5180_NSS, PN5180_BUSY, PN5180_RST);
#endif

#if USBHID == 1
  Cardio_ Cardio;
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

  byte rowPins[ROWS] = {1, 6, 5, 3}; //connect to the row pinouts of the keypad
  byte colPins[COLS] = {2, 0, 4}; //connect to the column pinouts of the keypad
  Keypad kpd = Keypad( makeKeymap(numpad), rowPins, colPins, ROWS, COLS );
#endif
 
void setup() {

#if USBKEYPAD == 1
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

#if USBHID == 1
  Cardio.begin(false);
#endif

#if WITH_SPICEAPI == 1
  SPICEAPI_INTERFACE.begin(SPICEAPI_BAUD);
  while (!SPICEAPI_INTERFACE);
#endif
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
    if (uidLengthMF > 0) {
      for (int i=0; i<7; i++) {
        uid[i] = uid[3+i];
      }
      uid[8] = 0;
      
#if USBHID == 1     
      Cardio.setUID(2, uid);
      Cardio.sendState();
#endif

#if WITH_SPICEAPI == 1
      formatUid(uid, uidBuf);
      spiceapi::card_insert(spiceCon, SPICEAPI_PLAYERNUM, uidBuf);
#endif

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
#if USBHID == 1
      Cardio.setUID(2, uid);
      Cardio.sendState();
#endif

#if WITH_SPICEAPI == 1
      formatUid(uid, uidBuf);
      spiceapi::card_insert(spiceCon, SPICEAPI_PLAYERNUM, uidBuf);
#endif
      
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
    
#if USBHID == 1
    Cardio.setUID(1, hid_data);
    Cardio.sendState();
#endif

#if WITH_SPICEAPI == 1
    formatUid(hid_data, uidBuf);
    spiceapi::card_insert(spiceCon, SPICEAPI_PLAYERNUM, uidBuf);
#endif

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
#if USBKEYPAD == 1
            Keyboard.press(kpd.key[i].kchar);
#endif
            break;
          case HOLD:
            break;
          case RELEASED:
#if USBKEYPAD == 1
            Keyboard.release(kpd.key[i].kchar);
#endif

#if SPICEKEYPAD == 1
            spiceapi::keypads_write(spiceCon, SPICEAPI_PLAYERNUM, &kpd.key[i].kchar);
#endif
            break;
          case IDLE:
            break;
        }

      }
    }
  }
}
#endif

#if WITH_SPICEAPI == 1
void formatUid(uint8_t* ary, char* buf) {
  sprintf(buf, "%02X%02X%02X%02X%02X%02X%02X%02X", ary[0], ary[1], ary[2], ary[3], ary[4], ary[5], ary[6], ary[7]);
}
#endif
