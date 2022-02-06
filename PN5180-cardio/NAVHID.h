#include "HID.h"

#define NUMBER_OF_LEDS 5
#define NUMBER_OF_BUTTONS 5

#define REPORT_DELAY 3990
#define MS_DEBOUNCE 5

#define STRING_ID_Base 4

class NAVHID_ : public PluggableUSBModule {
    public:
        NAVHID_(void);
        int send_state(uint8_t button_state);
        void write_lights(uint8_t button_state, bool hid, bool reactive);
        unsigned long getLastHidUpdate();
    protected:
        uint8_t epType[1];
        unsigned long lastHidUpdate = 0;

        int getInterface(uint8_t* interface_count);
        int getDescriptor(USBSetup& setup);
        bool setup(USBSetup& setup);
};

extern NAVHID_ NAVHID;
