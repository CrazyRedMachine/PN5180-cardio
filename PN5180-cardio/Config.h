#ifndef CONFIG_h
#define CONFIG_h

/* Pinout for the PN5180 free pins */
#define PN5180_PIN_NSS  10
#define PN5180_PIN_BUSY 9
#define PN5180_PIN_RST  7

/* Use a matrix keypad */
#define WITH_KEYPAD 0
  #define PIN_ROW1 1
  #define PIN_ROW2 6
  #define PIN_ROW3 5
  #define PIN_ROW4 3
  #define PIN_COL1 2
  #define PIN_COL2 0
  #define PIN_COL3 4

/* Launch game with "-api 1337 -apipass changeme -apiserial COM1 -apiserialbaud 57600" or similar */
/* Adjust your serial port here(Serial, Serial1, Serial2, etc.) - WiFi/Network support is possible, but out of scope for this project */
#define SPICEAPI_INTERFACE Serial
#define SPICEAPI_BAUD 57600
#define SPICEAPI_PASS "changeme"
/* For games with multiple readers */
#define SPICEAPI_PLAYERNUM 0

/* ISO14443 support (for older Aime/Nesica/BANAPASSPORT cards... reader will pretend it was a FeliCa for maximum cardio compatibility) */
#define WITH_ISO14443 1

#endif
