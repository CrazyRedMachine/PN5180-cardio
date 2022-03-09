#include "Config.h"
#include "CARDIOHID.h"
#include <Keyboard.h>
#include <Keypad.h>

#if WITH_PN5180 == 1
  #include "src/PN5180/PN5180.h"
  #include "src/PN5180/PN5180FeliCa.h"
  #include "src/PN5180/PN5180ISO15693.h"
  #if WITH_ISO14443 == 1
    #include "src/PN5180/PN5180ISO14443.h"
  #endif
  PN5180FeliCa nfcFeliCa(PN5180_PIN_NSS, PN5180_PIN_BUSY, PN5180_PIN_RST);
  PN5180ISO15693 nfc15693(PN5180_PIN_NSS, PN5180_PIN_BUSY, PN5180_PIN_RST);
  #if WITH_ISO14443 == 1
    PN5180ISO14443 nfc14443(PN5180_PIN_NSS, PN5180_PIN_BUSY, PN5180_PIN_RST);
  #endif
#endif

CARDIOHID_ Cardio;

#if WITH_NAVIGATION == 1
#include "NAVHID.h"
NAVHID_ NAVHID;

// Pins where the LEDs are connected to
uint8_t led_pins[] = {
    PIN_LED_UP,
    PIN_LED_DOWN,
    PIN_LED_LEFT,
    PIN_LED_RIGHT,
    PIN_LED_START,
};

// Pins where the buttons are connected to
uint8_t button_pins[] = {
    PIN_BUT_UP,
    PIN_BUT_DOWN,
    PIN_BUT_LEFT,
    PIN_BUT_RIGHT,
    PIN_BUT_START,
};
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

#if WITH_NAVIGATION == 1
    for (int i = 0; i < NUMBER_OF_BUTTONS; i++) {
        pinMode(button_pins[i], INPUT_PULLUP);
    }

    for (int i = 0; i < NUMBER_OF_LEDS; i++) {
        pinMode(led_pins[i], OUTPUT);
        digitalWrite(led_pins[i], HIGH);
    }
    delay(200);
    for (int i = 0; i < NUMBER_OF_LEDS; i++) {
        digitalWrite(led_pins[i], LOW);
    }
#endif

#if WITH_KEYPAD == 1
/* Keypad */
    kpd.setDebounceTime(10);
    Keyboard.begin();
#endif

#if WITH_PN5180 == 1
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
#endif

}

// read cards loop
void loop() {
  
#if WITH_KEYPAD == 1
  /* KEYPAD */
  keypadCheck();
#endif

#if WITH_NAVIGATION == 1
  inputCheck();
#endif

#if WITH_PN5180 == 1
static unsigned long lastReport = 0;
static int cardBusy = 0;
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
#if WITH_NAVIGATION == 1
  inputCheck();
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
#if WITH_NAVIGATION == 1
  inputCheck();
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
#if WITH_NAVIGATION == 1
  inputCheck();
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
#endif /* PN5180 */
}

#if WITH_NAVIGATION == 1
void inputCheck(){
    static uint32_t last_report = 0;
    static bool reactive = false;
    static bool hid_lights = false;
    uint8_t buttons_state = 0;
    
    /* autoswitch between HID and reactive modes */
    if ((millis()-NAVHID.getLastHidUpdate()) > 3000)
    {
      reactive = true;
      hid_lights = false;
    }
    else
    {
      reactive = false;
      hid_lights = true;
    }
      
    /* read buttons */
    for (int i = 0; i < NUMBER_OF_BUTTONS; i++) {
        int button_value = digitalRead(button_pins[i]);
        // Put button states into the buttons_state variable via bitwise operations
        if (button_value == LOW) {
            buttons_state |= 1 << i;
        } else {
            buttons_state &= ~(1 << i);
        }
    }

    /* write lights */
    NAVHID.write_lights(buttons_state, hid_lights, reactive);

    /* Send button state every 1000 µs */
    if (((micros() - last_report) >= REPORT_DELAY)) {
        NAVHID.send_state(buttons_state);
        last_report = micros();
    }
}
#endif

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
