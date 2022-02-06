#include "Config.h"
#include "CARDIOHID.h"

static const uint8_t PROGMEM hid_report[] = {
0x06, 0xCA, 0xFF,  // Usage Page (Vendor Defined 0xFFCA)
0x09, 0x01,        // Usage (0x01)
0xA1, 0x01,        // Collection (Application)
0x85, 0x01,        //   Report ID (1)
0x06, 0xCA, 0xFF,  //   Usage Page (Vendor Defined 0xFFCA)
0x09, 0x41,        //   Usage (0x41)
0x15, 0x00,        //   Logical Minimum (0)
0x25, 0xFF,        //   Logical Maximum (-1)
0x75, 0x08,        //   Report Size (8)
0x95, 0x08,        //   Report Count (8)
0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x85, 0x02,        //   Report ID (2)
0x06, 0xCA, 0xFF,  //   Usage Page (Vendor Defined 0xFFCA)
0x09, 0x42,        //   Usage (0x42)
0x15, 0x00,        //   Logical Minimum (0)
0x25, 0xFF,        //   Logical Maximum (-1)
0x75, 0x08,        //   Report Size (8)
0x95, 0x08,        //   Report Count (8)
0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0xC0,              // End Collection
// 42 bytes
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
#if CUSTOM_VIDPID == 1
const DeviceDescriptor PROGMEM USB_DeviceDescriptorIAD =
  D_DEVICE(0xEF,0x02,0x01,64,0x1ccf,0x5252,0x100,IMANUFACTURER,IPRODUCT,ISERIAL,1);
#endif
const char* const PROGMEM String_Manufacturer = "CrazyRedMachine";
const char* const PROGMEM String_Product = "CardIO";
#if CARDIO_ID == 1
const char* const PROGMEM String_Serial = "CARDIOP1";
#else
const char* const PROGMEM String_Serial = "CARDIOP2";	
#endif

CARDIOHID_::CARDIOHID_(void) : PluggableUSBModule(1, 1, epType) {	
    epType[0] = EP_TYPE_INTERRUPT_IN;
    PluggableUSB().plug(this);
}

int CARDIOHID_::getInterface(byte* interface_count) {
    *interface_count += 1;
    HIDDescriptor hid_interface = {
        D_INTERFACE(pluggedInterface, 1, USB_DEVICE_CLASS_HUMAN_INTERFACE, HID_SUBCLASS_NONE, HID_PROTOCOL_NONE),
        D_HIDREPORT(sizeof(hid_report)),
        D_ENDPOINT(USB_ENDPOINT_IN(pluggedEndpoint), USB_ENDPOINT_TYPE_INTERRUPT, USB_EP_SIZE, 0x01)
    };
    return USB_SendControl(0, &hid_interface, sizeof(hid_interface));
}

int CARDIOHID_::getDescriptor(USBSetup& setup) {
#if CUSTOM_VIDPID == 1
	if(setup.wValueH == USB_DEVICE_DESCRIPTOR_TYPE) {
        return USB_SendControl(TRANSFER_PGM, (const uint8_t*)&USB_DeviceDescriptorIAD, sizeof(USB_DeviceDescriptorIAD));
    }
#endif
    if (setup.wValueH == USB_STRING_DESCRIPTOR_TYPE) { 
        if (setup.wValueL == IPRODUCT) {
            return USB_SendStringDescriptor(String_Product, strlen(String_Product), 0);
        } 
        else if (setup.wValueL == IMANUFACTURER) {
            return USB_SendStringDescriptor(String_Manufacturer, strlen(String_Manufacturer), 0);
        } 
        else if (setup.wValueL == ISERIAL) {
            return USB_SendStringDescriptor(String_Serial, strlen(String_Serial), 0);
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

bool CARDIOHID_::setup(USBSetup& setup) {
    if (pluggedInterface != setup.wIndex) {
        return false;
    }

    uint8_t request = setup.bRequest;
    uint8_t request_type = setup.bmRequestType;

    if (request_type == REQUEST_DEVICETOHOST_CLASS_INTERFACE) {
/* todo get feature report */    
        return true;
    }

    if (request_type == REQUEST_HOSTTODEVICE_CLASS_INTERFACE) {
        if (request == HID_SET_REPORT) {
            if (setup.wValueH == HID_REPORT_TYPE_OUTPUT) {
                return true;
            }
            else if (setup.wValueH == HID_REPORT_TYPE_FEATURE ) {
/* todo set feature report */ 
				return true;
			}
        }
    }

    return false;
}

int CARDIOHID_::sendState(uint8_t type, uint8_t *value) {
    uint8_t data[9];

    data[0] = type;
	memcpy(data+1, value, 8);

    return USB_Send(pluggedEndpoint | TRANSFER_RELEASE, data, 9);
}
