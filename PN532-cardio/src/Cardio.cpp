/*
  Cardio.cpp

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
#include "Cardio.h"

#if defined(_USING_DYNAMIC_HID)

#define CARDIO_REPORT_ID_INDEX 7

Cardio_::Cardio_()
{

	// Build Cardio HID Report Description
		
	uint8_t tempHidReportDescriptor[150];
	int hidReportDescriptorSize = 0;

	// USAGE_PAGE (Card Reader)
	tempHidReportDescriptor[hidReportDescriptorSize++] = 0x04 | 0x02;
	tempHidReportDescriptor[hidReportDescriptorSize++] = 0xCA;
	tempHidReportDescriptor[hidReportDescriptorSize++] = 0xFF;

	// USAGE (0x01)
	tempHidReportDescriptor[hidReportDescriptorSize++] = 0x08 | 0x01;
	tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;

	// COLLECTION (Application)
	tempHidReportDescriptor[hidReportDescriptorSize++] = 0xa0 | 0x01;
	tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;

	// REPORT_ID (1 - ISO15693 tags)
	tempHidReportDescriptor[hidReportDescriptorSize++] = 0x84 | 0x01;
	tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;
	
	// USAGE_PAGE (Card Reader)
	tempHidReportDescriptor[hidReportDescriptorSize++] = 0x04 | 0x02;
	tempHidReportDescriptor[hidReportDescriptorSize++] = 0xCA;
	tempHidReportDescriptor[hidReportDescriptorSize++] = 0xFF;

	// USAGE (65)
	tempHidReportDescriptor[hidReportDescriptorSize++] = 0x08 | 0x01;
	tempHidReportDescriptor[hidReportDescriptorSize++] = 0x41;
	
	// LOGICAL_MINIMUM (0)
	tempHidReportDescriptor[hidReportDescriptorSize++] = 0x14 | 0x01;
	tempHidReportDescriptor[hidReportDescriptorSize++] = 0x00;

	// LOGICAL_MAXIMUM (255)
	tempHidReportDescriptor[hidReportDescriptorSize++] = 0x24 | 0x01;
	tempHidReportDescriptor[hidReportDescriptorSize++] = 0xFF;

	// REPORT_SIZE (# of bits in byte)
	tempHidReportDescriptor[hidReportDescriptorSize++] = 0x74 | 0x01;
	tempHidReportDescriptor[hidReportDescriptorSize++] = 0x08;

	// REPORT_COUNT (# of bytes in UID)
	tempHidReportDescriptor[hidReportDescriptorSize++] = 0x94 | 0x01;
	tempHidReportDescriptor[hidReportDescriptorSize++] = 0x08;
	
	// INPUT (Data,Var,Abs)
	tempHidReportDescriptor[hidReportDescriptorSize++] = 0x80 | 0x01;
	tempHidReportDescriptor[hidReportDescriptorSize++] = 0x02;
   
	// REPORT_ID (2 - FeliCa tags)
	tempHidReportDescriptor[hidReportDescriptorSize++] = 0x84 | 0x01;
	tempHidReportDescriptor[hidReportDescriptorSize++] = 0x02;
	
	// USAGE_PAGE (Card Reader)
	tempHidReportDescriptor[hidReportDescriptorSize++] = 0x04 | 0x02;
	tempHidReportDescriptor[hidReportDescriptorSize++] = 0xCA;
	tempHidReportDescriptor[hidReportDescriptorSize++] = 0xFF;

	// USAGE (65)
	tempHidReportDescriptor[hidReportDescriptorSize++] = 0x08 | 0x01;
	tempHidReportDescriptor[hidReportDescriptorSize++] = 0x42;
	
	// LOGICAL_MINIMUM (0)
	tempHidReportDescriptor[hidReportDescriptorSize++] = 0x14 | 0x01;
	tempHidReportDescriptor[hidReportDescriptorSize++] = 0x00;

	// LOGICAL_MAXIMUM (255)
	tempHidReportDescriptor[hidReportDescriptorSize++] = 0x24 | 0x01;
	tempHidReportDescriptor[hidReportDescriptorSize++] = 0xFF;

	// REPORT_SIZE (# of bits in byte)
	tempHidReportDescriptor[hidReportDescriptorSize++] = 0x74 | 0x01;
	tempHidReportDescriptor[hidReportDescriptorSize++] = 0x08;

	// REPORT_COUNT (# of bytes in UID)
	tempHidReportDescriptor[hidReportDescriptorSize++] = 0x94 | 0x01;
	tempHidReportDescriptor[hidReportDescriptorSize++] = 0x08;
	
	// INPUT (Data,Var,Abs)
	tempHidReportDescriptor[hidReportDescriptorSize++] = 0x80 | 0x01;
	tempHidReportDescriptor[hidReportDescriptorSize++] = 0x02;   

	// END_COLLECTION
	tempHidReportDescriptor[hidReportDescriptorSize++] = 0xc0;

	// Create a copy of the HID Report Descriptor template that is just the right size
	uint8_t *customHidReportDescriptor = new uint8_t[hidReportDescriptorSize];
	memcpy(customHidReportDescriptor, tempHidReportDescriptor, hidReportDescriptorSize);
	
	// Register HID Report Description
	DynamicHIDSubDescriptor *node = new DynamicHIDSubDescriptor(customHidReportDescriptor, hidReportDescriptorSize, false);
	DynamicHID().AppendDescriptor(node);
	
	// Setup Cardio State
	_uid = new uint8_t[8];
	
	// Calculate HID Report Size
	_hidReportSize = 8;
	
	// Initalize Cardio State
	_type = 0;
	
	for (int index = 0; index < 8; index++)
	{
		_uid[index] = 0;
	}
}

void Cardio_::begin(bool initAutoSendState)
{
	_autoSendState = initAutoSendState;
	if (initAutoSendState) sendState();
}

void Cardio_::end()
{
}

void Cardio_::setUID(uint8_t type, uint8_t *value)
{
	_type = type;
	for (int i=0; i<8; i++)
	   _uid[i] = value[i];
	if (_autoSendState) sendState();
}

void Cardio_::sendState()
{
	uint8_t data[_hidReportSize];
	int index = 0;
	// Load Button State
	for (; index < _hidReportSize; index++)
	{
		data[index] = _uid[index];
	}

	DynamicHID().SendReport(_type, data, _hidReportSize);
}

#endif
