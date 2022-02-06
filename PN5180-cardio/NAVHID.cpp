#include "NAVHID.h"

#define BUTTON_PADDING (8 - (NUMBER_OF_BUTTONS % 8))+8
#define LED_PADDING 5

uint8_t usb_data[128];
uint16_t lamp_hid_state = 0;
uint8_t extern led_pins[]; // start, then left-right, then up-down
uint8_t extern button_pins[]; // start, then left-right, then up-down

static const uint8_t PROGMEM hid_report[] = {
    0x05, 0x01,                      // USAGE_PAGE (Generic Desktop)
    0x09, 0x04,                      // USAGE (Joystick)
    0xa1, 0x01,                      // COLLECTION (Application)

    0x85, 0x03,                      //   REPORT_ID (3)

    /* Buttons */
    0x05, 0x09,                      //   USAGE_PAGE (Button)
    0x19, 0x01,                      //   USAGE_MINIMUM (Button 1)
    0x29, 0x05,         //   USAGE_MAXIMUM (Button NUMBER_OF_BUTTONS)
    0x15, 0x00,                      //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                      //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,                      //   REPORT_SIZE (1)
    0x95, 0x05,         //   REPORT_COUNT (NUMBER_OF_BUTTONS)
    0x55, 0x00,                      //   UNIT_EXPONENT (0)
    0x65, 0x00,                      //   UNIT (None)
    0x81, 0x02,                      //   INPUT (Data,Var,Abs)
    /* Buttons END */

    /* Buttons padding */
    0x75, 0x01,                      //   REPORT_SIZE (1)
    0x95, 0x03,            //   REPORT_COUNT (BUTTON_PADDING)
    0x81, 0x03,                      //   INPUT (Cnst,Var,Abs)
    /* Buttons padding END */

    /* 2 knobs as analog axis */
  0x05, 0x01,        //   Usage Page (Generic Desktop Ctrls)
  0x09, 0x01,        //   Usage (Pointer)
  0x15, 0x00,        //   Logical Minimum (0)
  0x26, 0xFF, 0x00,  //   Logical Maximum (255)
  0x95, 0x02,        //   Report Count (2)
  0x75, 0x08,        //   Report Size (8)
  0xA1, 0x00,        //   Collection (Physical)
    0x09, 0x30,        //     Usage (X)
    0x09, 0x31,        //     Usage (Y)
    0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
  0xC0,              //   End Collection (analog axis)

       /*Lights */
    0x85, 0x04,                    /*   REPORT_ID 4*/ 
    0x15, 0x00,                    /*     LOGICAL_MINIMUM (0) */ 
    0x25, 0x01,                    /*     LOGICAL_MAXIMUM (1) */ 
    /*Led Up/Down */ 
    0x05, 0x0a,                    /*     USAGE_PAGE (Ordinals) */ 
    0x09, 0x01,                    /*     USAGE (Instance 1) */ 
    0xa1, 0x02,                    /*     COLLECTION (Logical) */ 
    0x05, 0x08,                    /*       USAGE_PAGE (LEDs) */ 
    0x09, 0x4b,                    /*       USAGE (Generic Indicator 1) */ 
    0x79, 0x04,                    /*       STRING INDEX (4) */
    0x75, 0x01,                    /*       REPORT_SIZE (1) */ 
    0x95, 0x01,                    /*       REPORT_COUNT (1) */ 
    0x91, 0x02,                    /*       OUTPUT (Data,Var,Abs) */ 
    0xc0,                          /*     END_COLLECTION */ 
    /*Led Left/Right */ 
    0x05, 0x0a,                    /*     USAGE_PAGE (Ordinals) */ 
    0x09, 0x02,                    /*     USAGE (Instance 2) */ 
    0xa1, 0x02,                    /*     COLLECTION (Logical) */ 
    0x05, 0x08,                    /*       USAGE_PAGE (LEDs) */ 
    0x09, 0x4b,                    /*       USAGE (Generic Indicator 1) */ 
    0x79, 0x05,
    0x75, 0x01,                    /*       REPORT_SIZE (1) */ 
    0x95, 0x01,                    /*       REPORT_COUNT (1) */ 
    0x91, 0x02,                    /*       OUTPUT (Data,Var,Abs) */ 
    0xc0,                          /*     END_COLLECTION */ 
    /*Led Start */ 
    0x05, 0x0a,                    /*     USAGE_PAGE (Ordinals) */ 
    0x09, 0x03,                    /*     USAGE (Instance 3) */ 
    0xa1, 0x02,                    /*     COLLECTION (Logical) */ 
    0x05, 0x08,                    /*       USAGE_PAGE (LEDs) */ 
    0x09, 0x4b,                    /*       USAGE (Generic Indicator 1) */ 
    0x79, 0x06,
    0x75, 0x01,                    /*       REPORT_SIZE (1) */ 
    0x95, 0x01,                    /*       REPORT_COUNT (1) */ 
    0x91, 0x02,                    /*       OUTPUT (Data,Var,Abs) */ 
    0xc0,                          /*     END_COLLECTION */ 
    /* LEDs END */

    /* LEDs padding */
    0x75, 0x01,                      //   REPORT_SIZE (1)
    0x95, LED_PADDING,               //   REPORT_COUNT (BUTTON_PADDING)
    0x91, 0x03,                      //   OUTPUT (Cnst,Var,Abs)
    /* LEDs padding END */

    0xc0                             // END_COLLECTION
};

static bool SendControl(uint8_t d)
{
  return USB_SendControl(0, &d, 1) == 1;
}

static bool USB_SendStringDescriptor(const char *string_P, uint8_t string_len, uint8_t flags) {
        SendControl(2 + string_len * 2);
        SendControl(3);
        bool pgm = flags & TRANSFER_PGM;
        for(uint8_t i = 0; i < string_len; i++) {
                bool r = SendControl(pgm ? pgm_read_byte(&string_P[i]) : string_P[i]);
                r &= SendControl(0); // high byte
                if(!r) {
                        return false;
                }
        }
        return true;
}

/* HID descriptor strings */
const char* const PROGMEM LEDString_02 = "Start";
const char* const PROGMEM LEDString_01 = "Menu Left-Right";
const char* const PROGMEM LEDString_00 = "Menu Up-Down";

const char* String_indiv[] = {LEDString_00,LEDString_01,LEDString_02};
uint8_t STRING_ID_Count = 3;

NAVHID_::NAVHID_(void) : PluggableUSBModule(1, 1, epType) {
    epType[0] = EP_TYPE_INTERRUPT_IN;
    PluggableUSB().plug(this);
}

int NAVHID_::getInterface(byte* interface_count) {
    *interface_count += 1;
    HIDDescriptor hid_interface = {
        D_INTERFACE(pluggedInterface, 1, USB_DEVICE_CLASS_HUMAN_INTERFACE, HID_SUBCLASS_NONE, HID_PROTOCOL_NONE),
        D_HIDREPORT(sizeof(hid_report)),
        D_ENDPOINT(USB_ENDPOINT_IN(pluggedEndpoint), USB_ENDPOINT_TYPE_INTERRUPT, USB_EP_SIZE, 0x04)
    };
    return USB_SendControl(0, &hid_interface, sizeof(hid_interface));
}

int NAVHID_::getDescriptor(USBSetup& setup) {
    if (setup.wValueH == USB_STRING_DESCRIPTOR_TYPE) { 
        if(setup.wValueL >= STRING_ID_Base && setup.wValueL < (STRING_ID_Base + STRING_ID_Count)) {
            return USB_SendStringDescriptor(String_indiv[setup.wValueL - STRING_ID_Base], strlen(String_indiv[setup.wValueL - STRING_ID_Base]), 0);
        }                       
    }
      
    if (setup.bmRequestType != REQUEST_DEVICETOHOST_STANDARD_INTERFACE) {
        return 0;
    }

    if (setup.wValueH != HID_REPORT_DESCRIPTOR_TYPE) {
        return 0;
    }

    if (setup.wIndex != pluggedInterface) {
        return 0;
    }

    return USB_SendControl(TRANSFER_PGM, hid_report, sizeof(hid_report));
}

bool NAVHID_::setup(USBSetup& setup) {
    if (pluggedInterface != setup.wIndex) {
        return false;
    }

    uint8_t request = setup.bRequest;
    uint8_t request_type = setup.bmRequestType;

    if (request_type == REQUEST_HOSTTODEVICE_CLASS_INTERFACE) {
        if (request == HID_SET_REPORT) {
            if (setup.wValueH == HID_REPORT_TYPE_OUTPUT) {
                USB_RecvControl(usb_data, setup.wLength);

                if (usb_data[0] == 4) {
                    lamp_hid_state = usb_data[2]<<8 | usb_data[1];
                    lastHidUpdate = millis();
                    // No need to write lights or update lightmode here (for autoswitch), main loop() will take care of it
                }            
                return true;
            }
        }
    }

    return false;
}

unsigned long NAVHID_::getLastHidUpdate() {
    return lastHidUpdate;
}

void NAVHID_::write_lights(uint8_t button_state, bool hid, bool reactive) {
  /* build ledstate bitfield according to hid/reactive modes */
  if (!reactive) button_state = 0;
  if (hid) 
  {
    int fixed_hid_state = 0;
  //hid is up-down, then left-right, then start
  //bitfield is up down left right start
  if ((lamp_hid_state>>0)&1) fixed_hid_state |= 3; // 00011
  if ((lamp_hid_state>>1)&1) fixed_hid_state |= 12;// 01100
  if ((lamp_hid_state>>2)&1) fixed_hid_state |= 16;// 10000
    
    button_state |= fixed_hid_state;
  }           
  
  /* actually writing leds */
  for (int i=0; i<NUMBER_OF_LEDS; i++)
  {
        digitalWrite(led_pins[i], (button_state>>i)&1);
  }
}

int NAVHID_::send_state(uint8_t button_state) {
    uint8_t data[4];

    data[0] = (uint8_t) 3;
    data[1] = (uint8_t) (button_state & 0xFF);
    data[2] = 0x7F;
    data[3] = 0x7F;

    return USB_Send(pluggedEndpoint | TRANSFER_RELEASE, data, 4);
}
