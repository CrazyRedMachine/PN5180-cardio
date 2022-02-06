#include "HID.h"

#define STRING_ID_Base 4

class CARDIOHID_ : public PluggableUSBModule {
    public:
        CARDIOHID_(void);
        int sendState(uint8_t type, uint8_t *value);
    protected:
        uint8_t epType[1];
        unsigned long lastHidUpdate = 0;
        int getInterface(uint8_t* interface_count);
        int getDescriptor(USBSetup& setup);
        bool setup(USBSetup& setup);
};

extern CARDIOHID_ CARDIOHID;
