/* === USER CONFIGURABLE OPTIONS === */

/* General usage on boards without USB MCU requires WITH_SPICEAPI instead */
#define WITH_USBHID 1

/* Keypad on boards without USB MCU requires WITH_SPICEAPI */
#define WITH_KEYPAD 0

/* Launch game with "-api 1337 -apipass changeme -apiserial COM1 -apiserialbaud 57600" or similar */
#define WITH_SPICEAPI 0
/* Adjust your serial port here(Serial, Serial1, Serial2, etc.) - WiFi/Network support is possible, but out of scope for this project */
#define SPICEAPI_INTERFACE Serial
#define SPICEAPI_BAUD 57600
#define SPICEAPI_PASS "changeme"
/* For games with multiple readers */
#define SPICEAPI_PLAYERNUM 0

/* === END OF USER CONFIGURABLE OPTIONS === */

/* DO NOT MESS WITH THE LINES BELOW UNLESS YOU KNOW WHAT YOU'RE DOING */
#include <Wire.h>
#include "src/PN532/PN532_I2C.h"
#include "src/PN532/PN532.h"

PN532_I2C pn532i2c(Wire);
PN532 nfc(pn532i2c);

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

#if USBHID == 1
  Cardio.begin(false);
#endif

#if WITH_SPICEAPI == 1
  SPICEAPI_INTERFACE.begin(SPICEAPI_BAUD);
  while (!SPICEAPI_INTERFACE);
#endif
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
  ret = nfc.felica_Polling(systemCode, requestCode, idm, pmm, &systemCodeResponse, 5000);

#if WITH_KEYPAD == 1
  keypadCheck();
#endif
    if (ret == 1) {
#if USBHID == 1
      Cardio.setUID(2, idm);
      Cardio.sendState();
#endif

#if WITH_SPICEAPI == 1
      formatUid(idm, uidBuf);
      spiceapi::card_insert(spiceCon, SPICEAPI_PLAYERNUM, uidBuf);
#endif
      
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
