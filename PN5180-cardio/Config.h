#ifndef CONFIG_h
#define CONFIG_h

/* PN5180 support (disable to test only the navigation buttons/keypad) */
#define WITH_PN5180 1

/* ISO14443 support (for older Aime/Nesica/BANAPASSPORT cards... reader will pretend it was a FeliCa for maximum cardio compatibility) */
#define WITH_ISO14443 1

/* Pinout for the PN5180 free pins */
#define PN5180_PIN_NSS  10
#define PN5180_PIN_BUSY 9
#define PN5180_PIN_RST  7
#define PN5180_PIN_IRQ  30 //additional gpio

/* Use a matrix keypad */
#define WITH_KEYPAD 1
  #define PIN_ROW1 1
  #define PIN_ROW2 6
  #define PIN_ROW3 5
  #define PIN_ROW4 3
  #define PIN_COL1 2
  #define PIN_COL2 0
  #define PIN_COL3 4

/* Player ID (1 or 2) */
#define CARDIO_ID 1
/* Enable custom VID/PID (will require to hold reset button to reflash firmware) */
#define CUSTOM_VIDPID 1

/* NAVIGATION */
#define WITH_NAVIGATION 1
  #define PIN_BUT_UP    A0
  #define PIN_BUT_DOWN  A1
  #define PIN_BUT_LEFT  A2
  #define PIN_BUT_RIGHT A3
  #define PIN_BUT_START A4  
  #define PIN_LED_UP    8
  #define PIN_LED_DOWN  11
  #define PIN_LED_LEFT  12
  #define PIN_LED_RIGHT 13
  #define PIN_LED_START A5

#endif /* CONFIG_H */
