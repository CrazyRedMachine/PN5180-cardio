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

uint16_t sleepTimeMS = 0x3FF;
void setup() {
  //Serial.begin(115200);
   //Serial.println(F("PN5180 LPCD Demo Sketch"));

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
  pinMode(PN5180_PIN_IRQ, INPUT);
  
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

  uint8_t irqPin[1];
  nfcFeliCa.readEEprom(IRQ_PIN_CONFIG, irqPin, sizeof(irqPin));

  //Serial.println(F("----------------------------------"));
  //Serial.println(F("Reading IRQ_ENABLE register..."));
  uint32_t irqEnable;
  nfcFeliCa.readRegister(IRQ_ENABLE, &irqEnable);
  //Serial.print(F("IRQ_ENABLE=0x"));
  //Serial.println(irqEnable, HEX);
  
/* SETUP LPCD */
  uint8_t data[255];  
  uint8_t response[256];  
  // LPCD_FIELD_ON_TIME (0x36)  
  uint8_t fieldOn = 0xF0; 
  data[0] = fieldOn;  
  nfcFeliCa.writeEEprom(0x36, data, 1); 
  nfcFeliCa.readEEprom(0x36, response, 1);  
  fieldOn = response[0];  

  // LPCD_THRESHOLD (0x37)  
  uint8_t threshold = 0x04; 
  data[0] = threshold;  
  nfcFeliCa.writeEEprom(0x37, data, 1); 
  nfcFeliCa.readEEprom(0x37, response, 1);  
  threshold = response[0];
  
  // LPCD_REFVAL_GPO_CONTROL (0x38) 
  uint8_t lpcdMode = 0x01; // 1 = LPCD SELF CALIBRATION 
  data[0] = lpcdMode; 
  nfcFeliCa.writeEEprom(0x38, data, 1); 
  nfcFeliCa.readEEprom(0x38, response, 1);  
  lpcdMode = response[0];
    
  // LPCD_GPO_TOGGLE_BEFORE_FIELD_ON (0x39) 
  uint8_t beforeFieldOn = 0xF0;   
  data[0] = beforeFieldOn;  
  nfcFeliCa.writeEEprom(0x39, data, 1); 
  nfcFeliCa.readEEprom(0x39, response, 1);  
  beforeFieldOn = response[0];
    
  // LPCD_GPO_TOGGLE_AFTER_FIELD_ON (0x3A)  
  uint8_t afterFieldOn = 0xF0;  
  data[0] = afterFieldOn; 
  nfcFeliCa.writeEEprom(0x3A, data, 1); 
  nfcFeliCa.readEEprom(0x3A, response, 1);  
  afterFieldOn = response[0]; 
  delay(100); 
        
  // turn on LPCD in self calibration mode  
  nfcFeliCa.switchToLPCD(sleepTimeMS);
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
  if (digitalRead(PN5180_PIN_IRQ) == HIGH) {
    //Serial.println("IRQ Triggered!");
    scanCard();
    nfcFeliCa.switchToLPCD(sleepTimeMS);
  }
#endif
}

#if WITH_PN5180 == 1
void scanCard(){

  /* NFC */
  uint8_t uid[8] = {0,0,0,0,0,0,0,0};
  uint8_t hid_data[8] = {0,0,0,0,0,0,0,0};

  // manage IRQ
  (void) nfcFeliCa.getIRQStatus();
  uint32_t u; 
  nfcFeliCa.readRegister(0x26, &u); 
    if (u == 0x10195C)
    {
    // turn on LPCD
    nfcFeliCa.begin();
    nfcFeliCa.reset();
    if (nfcFeliCa.switchToLPCD(sleepTimeMS)) {
      //Serial.println("switchToLPCD success");
    } else {
      //Serial.println("switchToLPCD failed");
    }
     return; 
    }

 // check for FeliCa card
 nfcFeliCa.reset();  
#if WITH_KEYPAD == 1
  keypadCheck();
#endif
#if WITH_NAVIGATION == 1
  inputCheck();
#endif
  nfcFeliCa.setupRF();
  if (nfcFeliCa.isCardPresent()) {
  uint8_t uidLength = nfcFeliCa.readCardSerial(uid);
    if (uidLength > 0) {

        //Serial.print(F("FeliCa card found, UID="));
        for (int i=0; i<uidLength; i++) {
          //Serial.print(uid[i] < 0x10 ? " 0" : " ");
          //Serial.print(uid[i], HEX);
        }
        //Serial.println();
        //Serial.println(F("----------------------------------"));
        
      Cardio.sendState(2, uid);  
      uidLength = 0;
      return;
    }
  } 
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
            //Serial.print(F("ISO14443 card found, UID="));
        for (int i=0; i<uidLengthMF; i++) {
          //Serial.print(uid[i] < 0x10 ? " 0" : " ");
          //Serial.print(uid[i], HEX);
        }
        //Serial.println();
        //Serial.println(F("----------------------------------"));
        
      uid[0] &= 0x0F; //some games won't accept FeliCa cards with a first byte higher than 0x0F  
      Cardio.sendState(2, uid);
      uidLengthMF = 0;
      return;
    }
#endif /* ISO14443 */

   // check for ISO-15693 card
  nfc15693.reset();
#if WITH_KEYPAD == 1
  keypadCheck();
#endif
#if WITH_NAVIGATION == 1
  inputCheck();
#endif
  nfc15693.setupRF();
      // check for ICODE-SLIX2 password protected tag
    uint8_t password[] = {0x5B, 0x6E, 0xFD, 0x7F};
    ISO15693ErrorCode myrc = nfc15693.disablePrivacyMode(password);
    if (ISO15693_EC_OK == myrc) {
      //Serial.println("disablePrivacyMode successful");
    }
  // try to read ISO15693 inventory
  ISO15693ErrorCode rc = nfc15693.getInventory(uid);
  if (rc == ISO15693_EC_OK ) {

        //Serial.print(F("ISO15693 card found, UID="));
        for (int i=0; i<8; i++) {
          //Serial.print(uid[7-i] < 0x10 ? " 0" : " ");
          //Serial.print(uid[7-i], HEX);
        }
        // lock password  
      ISO15693ErrorCode myrc = nfc15693.enablePrivacyMode(password);
      if (ISO15693_EC_OK == myrc) {
        //Serial.println("enablePrivacyMode successful");
      }
      
        //Serial.println();
        //Serial.println(F("----------------------------------"));
    
    for (int i=0; i<8; i++) {
      hid_data[i] = uid[7-i];
    }
    Cardio.sendState(1, hid_data);
    return;
  }
  // no card detected
  
}
#endif /* PN5180 */

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
