#include "Config.h"
#include <Wire.h>
#include "PN532_I2C.h"
#include "PN532.h"


#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);



bool displayscrii = 0;
uint64_t sysledtime = 0;
bool ledstat = 0;
uint32_t idcnt = 0;
#include <Adafruit_NeoPixel.h>


#define LED_PIN PA6
#define LED_Brightness 32
#define NUMPIXELS 10
Adafruit_NeoPixel pixels(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

char uidBuf[18];

PN532_I2C pn532i2c(Wire);
PN532 nfc(pn532i2c);





void displayTopDisp(char stat[], uint16_t color) {
  display.fillRect(88, 0, 128, 8, color);
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(F("[Spice Reader]"));
  display.setTextColor(!color);
  display.setCursor(91, 0);
  display.print(stat);
  display.display();
  display.setTextColor(SSD1306_WHITE);
}

void clear_disp_down() {
  display.stopscroll();
  display.fillRect(0, 10, 128, 32, SSD1306_BLACK);

  displayscrii = 0;
}

void scroll_disp(int xx1, char text1[], int xx2, char text2[]) {
  clear_disp_down();
  displayscrii = 1;
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(xx1, 14);
  display.print(text1);
  display.setCursor(xx2, 24);
  display.print(text2);
  display.display();
  display.startscrollleft(0x03, 0x03);
}

void static_disp(int xx1, char text1[], int xx2, char text2[]) {
  clear_disp_down();
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(xx1, 14);
  display.print(text1);
  display.setCursor(xx2, 24);
  display.print(text2);
  display.display();
}





void setup() {
  Serial.begin(115200);
  /* NFC */
  nfc.begin();


  nfc.setPassiveActivationRetries(0x20);
  nfc.SAMConfig();


  pinMode(PC13, OUTPUT);
  //
  //while(1){
  //  digitalWrite(PC13,1);
  //  delay(200);
  //  digitalWrite(PC13,0);
  //  delay(200);
  //
  //  }

  display.begin(SCREEN_ADDRESS);
  display.clearDisplay();

  displayTopDisp("Init", SSD1306_WHITE);



  pixels.begin();
  pixels.setBrightness(LED_Brightness);



  uint32_t versiondata ;

  pixels.fill( pixels.Color(50, 30, 40));
  pixels.show();

  delay(100);
  while (!(versiondata = nfc.getFirmwareVersion()))
  {
    delay(500);
    Serial.print("Didn't find PN53x board");

    pixels.fill( pixels.Color(255, 0, 0));
    pixels.show();
    delay(500);

    pixels.fill( pixels.Color(0, 0, 0));
    pixels.show();
  }



  Serial.dtr(0);
  delay(400);

  displayTopDisp("Ready", SSD1306_BLACK);

  scroll_disp(15, "Waiting for card", 0, "AmusIC Aime Banapass");

  pixels.fill( pixels.Color(255, 255, 255));
  pixels.show();
}


void apiSendBuf() {
  Serial.print("{\"id\":");
  Serial.print(++idcnt);
  Serial.print(",\"module\":\"card\",\"function\":\"insert\",\"params\":[0,\"");
//  Serial.print(uidBuf);
  for(int i = 0;i<16;i++)
  Serial.print(uidBuf[i]);
  Serial.print("\"]}");
  Serial.write('\0');
  delay(100);
  for (int i = 1; i <= 8; i++)
    Serial.write('\0');

}


unsigned long lastReport = 0;
int cardBusy = 0;
// read cards loop
void loop() {
  if (millis() - sysledtime > 500) {
    sysledtime = millis();
    ledstat = !ledstat;
    digitalWrite(PC13, ledstat);

  }
  if (millis() - lastReport < cardBusy) return;

  cardBusy = 0;
  static uint8_t uid[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  static uint8_t uid_t[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  static uint8_t uidLength;
  // check for FeliCa card
  static uint8_t ret;
  static uint16_t systemCode = 0xFFFF;
  static uint8_t requestCode = 0x01;       // System Code request
  static uint8_t idm[8];
  static uint8_t pmm[8];
  static uint16_t systemCodeResponse;
  ret = nfc.felica_Polling(systemCode, requestCode, idm, pmm, &systemCodeResponse, 500);
  if (ret == 1) {
    uidBuf[0] &= 0x0F;
    formatUid(idm, uidBuf);
    apiSendBuf();
    lastReport = millis();
    cardBusy = 2000;
    uidLength = 0;
    static_disp(2, "Readed Card", 22, "AmusIC Felica");
    pixels.fill( pixels.Color(0, 255, 0));
    pixels.show();
    delay(500);

    scroll_disp(15, "Waiting for card", 0, "AmusIC Aime Banapass");
    pixels.fill( pixels.Color(255, 255, 255));
    pixels.show();
    return;
  }

  //14443
  if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength)) {
    for (int i = 0; i < 8; i++) {
      uid_t[i] = uid[i % uidLength];
    }
    uid_t[0] &= 0x0F;
    formatUid(uid_t, uidBuf);
    apiSendBuf();
    lastReport = millis();
    cardBusy = 2000;
    uidLength = 0;
    static_disp(2, "Readed Card", 22, "Aime Bana Mifare");
    pixels.fill( pixels.Color(0, 255, 0));
    pixels.show();
    delay(500);
    scroll_disp(15, "Waiting for card", 0, "AmusIC Aime Banapass");
    pixels.fill( pixels.Color(255, 255, 255));
    pixels.show();
    return;
  }
  // no card detected
  lastReport = millis();
  cardBusy = 200;
}


void formatUid(uint8_t* ary, char* buf) {
  sprintf(buf, "%02X%02X%02X%02X%02X%02X%02X%02X", ary[0], ary[1], ary[2], ary[3], ary[4], ary[5], ary[6], ary[7]);
}
