#ifndef CONFIG_h
#define CONFIG_h

/* ISO14443 support (for older Aime/Nesica/BANAPASSPORT cards... reader will pretend it was a FeliCa for maximum cardio compatibility) */
#define WITH_ISO14443 1

/* Use a matrix keypad */
#define WITH_KEYPAD 1
  #define PIN_ROW1 1
  #define PIN_ROW2 0
  #define PIN_ROW3 4
  #define PIN_ROW4 5
  #define PIN_COL1 6
  #define PIN_COL2 7
  #define PIN_COL3 8
  #define PIN_COL4 9

/* Player ID (1 or 2) */
#define CARDIO_ID 1
/* Enable custom VID/PID (will require to hold reset button to reflash firmware) */
#define CUSTOM_VIDPID 1

#endif