//Set to 0 to disable keypad matrix
#define WITH_KEYPAD 1

#include "src/Cardio.h"

#if WITH_KEYPAD == 1
  #include <Keypad.h>
  #include <Keyboard.h>
#endif

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

/* For mini keypad */
byte rowPins[ROWS] = {4, 9, 8, 6}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {5, 3, 7}; //connect to the column pinouts of the keypad
Keypad kpd = Keypad( makeKeymap(numpad), rowPins, colPins, ROWS, COLS );
#endif

/* For NFC */
//SL015M out on pin 2
#define R1_DET 2
//Serial on pins 0(RX), 1(TX)
#define R1_SER Serial1
#define RFID_BAUD 115200
int rfSENSOR;
HardwareSerial* rfSerial;

void setup() {

#if WITH_KEYPAD == 1
  /* Keypad */
    kpd.setDebounceTime(10);
    Keyboard.begin();
#endif

/* NFC */
rfSENSOR = R1_DET;
rfSerial = &R1_SER;

pinMode(rfSENSOR,INPUT);                            // Rfid sensor
rfSerial->begin(RFID_BAUD);

/* HID */
  Cardio.begin(false);
}

uint32_t loopCnt = 0;
unsigned long lastReport = 0;
unsigned long timesent = 0;
int cardBusy = 0;
bool rfcmdsent = false;
// read cards loop
void loop() {
#if WITH_KEYPAD == 1
  /* KEYPAD */
  keypadCheck();
#endif
  
  /* NFC (check for ISO-15693 card) */
  if (millis()-lastReport < cardBusy) return;
  
  cardBusy = 0;
  uint8_t uid[8] = {0,0,0,0,0,0,0,0};

  if (digitalRead(rfSENSOR)==LOW)            // card presence
  {
          byte rfidp[256];            // rfid response buffer
      byte rf_i;         //response buffer size
    if (!rfcmdsent)
    {
      rfSerial->write(0xBA);                  // pream
      rfSerial->write(0x02);                  // size
      rfSerial->write(0x31);                  // rfidcommand: get tag info
      rfSerial->write(0xBA ^ 0x02 ^ 0x31);    // checksum

      rf_i = 0;
      rfcmdsent= true;
      timesent = millis();
    }
    else // if command has been sent, let's read answer
    {
      if (rfSerial->available()>0)
      {
        rfidp[rf_i] = rfSerial->read();
        rf_i++;
        if (rfidp[0] != 0xBD)                // restart filling buffer from begining until received byte is 0xBD (preamb)
          rf_i = 0;

        if (rf_i>2)                          // if we know paquet length
        {
          byte length = rfidp[1];
          if (rf_i >= length + 2)            // if paquet complete
          {
            // checksum check
            byte chk = 0x00;
            for (int i=0;i< length+1;i++)
              chk ^= rfidp[i];

            if (chk != rfidp[length+1])
            {
              rfcmdsent = false;             // checksum error, send cmd again
            }
            else
            {
              if (rfidp[3]!=0x00)            // Operation not succeeded
              {
                rfcmdsent = false;           // error, send again
              }
              else
              {
              //success, let's copy the uid
                for (int i=0;i<8;i++)
                {
                  uid[i] = rfidp[11-i];
                }
                /*
                if (uid[0] == 0xE0 && uid[1] == 0x04) // if correct konami card
                  card = 1;
                else
                  card = 0;
                */
                Cardio.setUID(1, uid);
                Cardio.sendState();
                lastReport = millis();
                cardBusy = 3000;
                return;
              }
            }
          }
        }
      }
      else
      {
      // command sent, but nothing to receive
        if (millis() - timesent >500)        // if last command was sent more than 500ms ago, send a new one
          rfcmdsent = false;
      }
    }
  }

  keypadCheck();

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

