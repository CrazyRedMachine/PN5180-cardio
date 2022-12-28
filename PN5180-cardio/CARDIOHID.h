#include "HID.h"

#if defined(ARDUINO_ARCH_AVR)

#define EPTYPE_DESCRIPTOR_SIZE      uint8_t

#elif defined(ARDUINO_ARCH_SAM)

#define EPTYPE_DESCRIPTOR_SIZE      uint32_t
#define USB_EP_SIZE                 64
#define TRANSFER_PGM                0x80
#define USB_SendControl             USBD_SendControl
#define USB_RecvControl             USBD_RecvControl
#define USB_Recv                    USBD_Recv
#define USB_Send                    USBD_Send
#define USB_Flush                   USBD_Flush
#define HID_REPORT_TYPE_INPUT       1
#define HID_REPORT_TYPE_OUTPUT      2
#define HID_REPORT_TYPE_FEATURE     3

#else

#error "Unsupported architecture"

#endif

#define STRING_ID_Base 4

class CARDIOHID_ : public PluggableUSBModule {
    public:
        CARDIOHID_(void);
        int sendState(uint8_t type, uint8_t *value);
    protected:
        EPTYPE_DESCRIPTOR_SIZE epType[1];
        unsigned long lastHidUpdate = 0;
        int getInterface(uint8_t* interface_count);
        int getDescriptor(USBSetup& setup);
        bool setup(USBSetup& setup);
};

extern CARDIOHID_ CARDIOHID;
