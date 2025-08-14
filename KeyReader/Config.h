/*
*	Config.h, Copyright Jonathan Mackey 2025
*
*	GNU license:
*	This program is free software: you can redistribute it and/or modify
*	it under the terms of the GNU General Public License as published by
*	the Free Software Foundation, either version 3 of the License, or
*	(at your option) any later version.
*
*	This program is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*	Please maintain this license information along with authorship and copyright
*	notices in any redistribution of this code.
*
*/
#ifndef Config_h
#define Config_h

#include <inttypes.h>
#include "PlatformDefs.h"

// This is coded for a STM32F429Vx

#define BAUD_RATE		19200
#define DEBOUNCE_DELAY	20		// ms, for buttons

#define DISPLAY_CONTROLLER	TFT_ILI9488
namespace Config
{
	const pin_t		kExtInt0Pin			= PA0;	// Available as Ext Int 0
	const pin_t		kExtInt1Pin			= PA1;	// Available as Ext Int 1
	const pin_t		kTx2Pin				= PA2;
	const pin_t		kRx2Pin				= PA3;
	const pin_t		kDCMI_HSyncPin		= PA4;
	const pin_t		kPA5Pin				= PA5;	// Unused
	const pin_t		kDCMI_PixClkPin		= PA6;
	const pin_t		kCameraXClkPin		= PA7;
	const pin_t		kPA8Pin				= PA8;	// Status LED Yellow
	const pin_t		kYellowLEDPin		= PA8;	// Status LED Yellow
	const pin_t		kSDDetectPin		= PA9;
	const pin_t		kSDSelectPin		= PA10;
	const pin_t		kUSB_DNPin			= PA11;
	const pin_t		kUSB_DPPin			= PA12;
	const pin_t		kSWDIOPin			= PA13;
	const pin_t		kSWCLKPin			= PA14;
	const pin_t		kBacklightPin		= PA15;

	const pin_t		kCameraPwrDnPin		= PB0;
	const pin_t		kCameraResetPin		= PB1;
	const pin_t		kBoot1Pin			= PB2;	// Externally pulled low
	const pin_t		kSCK1Pin			= PB3;
	const pin_t		kMISO1Pin			= PB4;
	const pin_t		kMOSI1Pin			= PB5;
	const pin_t		kSCL1Pin			= PB6;
	const pin_t		kDCMI_VSyncPin		= PB7;
	const pin_t		kTouchIRQPin		= PB8;
	const pin_t		kSDA1Pin			= PB9;
	const pin_t		kSCL2Pin			= PB10;
	const pin_t		kSDA2Pin			= PB11;
	const pin_t		kPB12Pin			= PB12;	// Unused
	const pin_t		kPB13Pin			= PB13;	// Unused
	const pin_t		kLCD_TRPin			= PB14;
	const pin_t		kPB15Pin			= PB15;	// Unused

	const pin_t		kPC0Pin				= PC0;	// Unused
	const pin_t		kPC1Pin				= PC1;	// Unused
	const pin_t		kPC2Pin				= PC2;	// Unused
	const pin_t		kPC3Pin				= PC3;	// Unused
	const pin_t		kExtInt4Pin			= PC4;	// Available as Ext Int 4
	const pin_t		kExtInt5Pin			= PC5;	// Available as Ext Int 5
	const pin_t		kDCMI_D0Pin			= PC6;
	const pin_t		kDCMI_D1Pin			= PC7;
	const pin_t		kDCMI_D2Pin			= PC8;
	const pin_t		kDCMI_D3Pin			= PC9;
	const pin_t		kPC10Pin			= PC10;	// Status LED Blue
	const pin_t		kPC11Pin			= PC11;	// Status LED Green
	const pin_t		kPC12Pin			= PC12;	// Status LED Red
	const pin_t		kBlueLEDPin			= PC10;	// Status LED Blue
	const pin_t		kGreenLEDPin		= PC11;	// Status LED Green
	const pin_t		kRedLEDPin			= PC12;	// Status LED Red
	const pin_t		kPC13Pin			= PC13;	// Unused
	const pin_t		kOsc32InPin			= PC14;	// RTC OSC
	const pin_t		kOsc32OutPin		= PC15;	// RTC OSC

	const pin_t		kFMC_D2Pin			= PD0;
	const pin_t		kFMC_D3Pin			= PD1;
	const pin_t		kKRBacklightPin		= PD2;
	const pin_t		kDCMI_D5Pin			= PD3;
	const pin_t		kFMC_NOEPin			= PD4;
	const pin_t		kFMC_NWEPin			= PD5;
	const pin_t		kPD6Pin				= PD6;	// Unused
	const pin_t		kFMC_NE1Pin			= PD7;
	const pin_t		kFMC_D13Pin			= PD8;
	const pin_t		kFMC_D14Pin			= PD9;
	const pin_t		kFMC_D15Pin			= PD10;
	const pin_t		kFMC_A16Pin			= PD11;
	const pin_t		kPD12Pin			= PD12;	// Unused
	const pin_t		kPD13Pin			= PD13;	// Unused
	const pin_t		kFMC_D0Pin			= PD14;
	const pin_t		kFMC_D1Pin			= PD15;

	const pin_t		kDispResetPin		= PE0;
	const pin_t		kTouchCSPin			= PE1;
	const pin_t		kEnterBtnPin		= PE2;	// Ext Int 2
	const pin_t		kCancelBtnPin		= PE3;	// Ext Int 3
	const pin_t		kDCMI_D4Pin			= PE4;
	const pin_t		kDCMI_D6Pin			= PE5;
	const pin_t		kDCMI_D7Pin			= PE6;
	const pin_t		kFMC_D4Pin			= PE7;
	const pin_t		kFMC_D5Pin			= PE8;
	const pin_t		kFMC_D6Pin			= PE9;
	const pin_t		kFMC_D7Pin			= PE10;
	const pin_t		kFMC_D8Pin			= PE11;
	const pin_t		kFMC_D9Pin			= PE12;
	const pin_t		kFMC_D10Pin			= PE13;
	const pin_t		kFMC_D11Pin			= PE14;
	const pin_t		kFMC_D12Pin			= PE15;

	const pin_t		kOscInPin			= PH0;
	const pin_t		kOscOutPin			= PH1;

	const uint8_t	kDisplayRotation	= 3;	// 0, 1, 2, 3 = 0, 90, 180, 270

	const uint32_t	kEnterBtnMask 		= _BV(2); //digitalPinToBitMask(PE2);
	const uint32_t	kCancelBtnMask		= _BV(3); //digitalPinToBitMask(PE3);
//	const uint32_t	kSDDetectMask 		= _BV(11); //digitalPinToBitMask(PA11);
//	const uint32_t	kTouchIRQMask 		= _BV(8); //digitalPinToBitMask(PA8);

	const uint32_t	kPINBtnMask = (kEnterBtnMask | kCancelBtnMask);

	const uint8_t	kTextInset			= 3; // Makes room for drawing the selection frame
	const uint8_t	kTextVOffset		= 6; // Makes room for drawing the selection frame
	// To make room for the selection frame the actual font height in the font
	// file is reduced.  The actual height is kFontHeight.
	const uint8_t	kFontHeight			= 43;

#if 0
	// Touchscreen min/max for 4" ILI9488 display
	const uint16_t	kDisplayWidth		= 320;
	const uint16_t	kDisplayHeight		= 480;
	const bool		kInvertTouchX		= false;
	const bool		kInvertTouchY		= false;
#else
	// Touchscreen min/max for 3.5" ILI9488 display
	const uint16_t	kDisplayWidth		= 320;
	const uint16_t	kDisplayHeight		= 480;
	const bool		kInvertTouchX		= true;
	const bool		kInvertTouchY		= true;
#endif

	/*
	*	The OV5640 camera I2C address is the camera SCCB address shifted right
	*	one bit. (0x78 >> 1 = 0x3C)
	*
	*	The OV2640 I2C address is 0x30
	*/
	const uint8_t kOV5640DeviceAddr = 0x3C;	// Camera I2C address

	const uint8_t kAT24CDeviceAddr = 0x50;	// Serial EEPROM I2C address
	const uint8_t kAT24CDeviceCapacity = 8;	// Value at end of AT24Cxxx xxx/8

	/*
	*	The SKRreferences occupy the first N bytes of the AT24C64 EEPROM
	*	The AT24C64 EEPROM has 8KB of storage.
	*/
	const uint16_t	kKRPreferencesAddr		= 0;		// EEPROM Page 0
	// struct SKRreferences is defined in KRSettings.h

	const uint16_t	kUtilitiesDialogPrefsAddr		= 64;	// EEPROM Page 2
	// struct SUtilitiesDialogPrefs is defined in KRSettings.h

	const uint16_t	kSKeyViewPrefsAddr		= 128;	// EEPROM Page 3
	// struct SKeyViewPrefs is defined in KRSettings.h

	const uint16_t	kMainViewPrefsAddr		= 192;	// EEPROM Page 4
	// struct SMainViewPrefs is defined in KRSettings.h

	/*
	*	TFT ILI9488 Addressing:
	*	The address line connects to the D/C pin on the ILI9488 controller. 
	*	This board uses A16, kFMC_A16Pin, physical pin PD11.  For other boards
	*	this pin can be set to A0 through A24, whatever is available.  See the
	*	FMC pin definition table in the MCU doc.
	*	
	*	PSRAM uses Memory Bank 1.  Commands are written to the base address.
	*	Data is written to the base address + the address line mask.  The mask
	*	value may need to be shifted by 1 bit depending on the memory width. 
	*	
	*	PSRAM Bank 1 = 0x60000000
	*	Command write address = 0x60000000
	*	
	*	A16 data address = 0x60000000 + (1 << (16+1)) =
	*										0x60000000 + 0x20000 = 0x60020000
	*/

	#define FMC_CmdAddr ((volatile uint16_t*)0x60000000U)
	#define FMC_DataAddr ((volatile uint16_t*)0x60020000U)
}

#endif // Config_h

