/*
  Cardio.h

  Copyright (c) 2020, CrazyRedMachine
  
  Based on <Joystick.h> library
  Copyright (c) 2015-2017, Matthew Heironimus

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef CARDIO_h
#define CARDIO_h

#include "DynamicHID/DynamicHID.h"

#if ARDUINO < 10606
#error The Cardio library requires Arduino IDE 1.6.6 or greater. Please update your IDE.
#endif // ARDUINO < 10606

#if ARDUINO > 10606
#if !defined(USBCON)
#error The Cardio library can only be used with a USB MCU (e.g. Arduino Leonardo, Arduino Micro, etc.).
#endif // !defined(USBCON)
#endif // ARDUINO > 10606

#if !defined(_USING_DYNAMIC_HID)

#warning "Using legacy HID core (non pluggable)"

#else // !defined(_USING_DYNAMIC_HID)

//================================================================================
//  Cardio

class Cardio_
{
private:

    // Cardio State
	uint8_t                 _type; //1 iso15693 or 2 felica
    uint8_t                 *_uid = NULL;
    bool                    _autoSendState = true;
	uint8_t                  _hidReportSize; 

public:
	Cardio_();

	void begin(bool initAutoSendState = true);
	void end();

	void setUID(uint8_t type, uint8_t *value);
	void clearHID();

	void sendState();
};

#endif // !defined(_USING_DYNAMIC_HID)
#endif // CARDIO_h
