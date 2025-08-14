/*
*	OV5640.h, Copyright Jonathan Mackey 2025
*
*	Modified version of initialization example from the OV5640 Camera Module
*	Software Application Notes, rev 1.3, OmniVision Technologies, Inc. 2011
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
#ifndef OV5640_h
#define OV5640_h

namespace OV5640
{
	/*
	*	kBWThreshold is the luminance Y B&W transition threshold.  Any Y value
	*	above kBWThreshold is considered white, and anything below is black.
	*/
	//const uint16_t	kBWThreshold	= 0x96; Moved to DCMI_OV5640
	
	/*	The TotalWidth param determines how much time there is between lines.
	*	If you're processing line by line and you need more time, increase the
	*	TotalWidth.
	*
	*	TotalWidth must be much larger than the XOutputSize depending on the PLL
	*	clock speed.  With a logic analyzer you can see the effect TotalWidth
	*	has on the timing.  TotalWidth has nothing to do with the number of
	*	pixels per line, that is strictly controlled by XOutputSize.
	*/
	
	/*
	*	Hi Res Settings
	*
	*	No scaling, cropped to 1000 pixels at full resolution
	*/
	const uint16_t	kHRXAddrStart	= 784;	// [0x3800, 0x3801]
	const uint16_t	kHRYAddrStart	= 14;	// [0x3802, 0x3803]
	const uint16_t	kHRXAddrEnd		= 1816;	// [0x3804, 0x3805]
	const uint16_t	kHRYAddrEnd		= 1944;	// [0x3806, 0x3807]
	const uint32_t	kHRXOutputSize	= 1000;	// [0x3808, 0x3809]
	const uint32_t	kHRYOutputSize	= 1918;	// [0x380A, 0x380B]
	const uint16_t	kHRTotalWidth	= 2500;	// [0x380C, 0x380D]
	const uint16_t	kHRTotalHeight	= 1976;	// [0x380E, 0x380F]
	const uint16_t	kHRXInset		= 16;	// [0x3810, 0x3811]
	const uint16_t	kHRYInset		= 6;	// [0x3812, 0x3813]

	const uint16_t	kHiResInit[] =
	{
		0x3820, 0x06,	// Sensor vflip
		0x3821, 0x00,	// Mirror, also bit 5, when set, enables JPEG <<<<
		0x3800, kHRXAddrStart >> 8,		// X Origin D[15:8]
		0x3801, kHRXAddrStart & 0xFF,	// X Origin D[7:0]
		0x3802, kHRYAddrStart >> 8,		// Y Origin D[15:8]
		0x3803, kHRYAddrStart & 0xFF,	// Y Origin D[7:0]
		0x3804, kHRXAddrEnd >> 8,		// X End D[11:8]
		0x3805, kHRXAddrEnd & 0xFF,		// X End D[7:0]
		0x3806, kHRYAddrEnd >> 8,		// Y End D[10:8]
		0x3807, kHRYAddrEnd & 0xFF,		// Y End D[7:0]
		0x3808, kHRXOutputSize >> 8,	// Output Width D[11:8]
		0x3809, kHRXOutputSize & 0xFF,	// Output Width D[7:0]
		0x380A, kHRYOutputSize >> 8,	// Output Height D[10:8]
		0x380B, kHRYOutputSize & 0xFF,	// Output Height D[7:0]
		0x380C, kHRTotalWidth >> 8,		// Total Width D[11:8] 
		0x380D, kHRTotalWidth & 0xFF,	// Total Width D[7:0]
		0x380E, kHRTotalHeight >> 8,	// Total Height D[15:8]
		0x380F, kHRTotalHeight & 0xFF,	// Total Height D[7:0]
		0x3810, kHRXInset >> 8,			// X Offset D[11:8]
		0x3811, kHRXInset & 0xFF,		// X Offset D[7:0]
		0x3812, kHRYInset >> 8,			// Y Offset D[10:8]
		0x3813, kHRYInset & 0xFF,		// Y Offset D[7:0]
#if 0
		
		0x5000, 0x27,	// ISP Control
		0x460C, 0x22,	// VFIFO control, DVP PCLK divider control by 0x3824[4:0]
		0x3824, 0x02,	// DVP PCLK divider D[4:0] (varies)
//		0x460C, 0x20,	// VFIFO control, DVP PCLK divider control by 0x3824[4:0]
//		0x3824, 0x01,	// DVP PCLK divider D[4:0] (varies)
		0x3035, 0x21,	// 24MHz system clock div = 2, MIPI scale div = 1
		0x3036, 0x30,	// PLL Multiplier (fps)
		0x3008, 0x02,
//		0x3035, 0x21,
#else
		0x3814, 0x11,	// Timing X increment, D[7:4] odd, D[3:0] even
		0x3815, 0x11,	// Timing Y increment, D[7:4] odd, D[3:0] even
//0x3618, 0x04,	// Undocumented
//0x3612 ,0x4B,	// Undocumented
//0x3708, 0x21,	// Undocumented
//0x3709, 0x12,	// Undocumented
//0x370C, 0x00,	// Undocumented
//0x3A02, 0x07,	// Undocumented
//0x3A03, 0xB0,	// Undocumented

//0x3A0E, 0x06,	// 50Hz Max Bands in One Frame D[5:0]
//0x3A0D, 0x08,	// 60Hz Max Bands in One Frame D[5:0]
//0x3A14, 0x07,	// 50Hz Maximum Exposure Output Limit D[15:8]
//0x3A15, 0xB0,	// 50Hz Maximum Exposure Output Limit D[7:0]
		
		0x4004, 0x06,	// BLC line number D[7:0]
		0x5000, 0x07,	// ISP Control
0x5181, 0x52,	// Advanced White Balance, varies
	// Same as common init	0x5182, 0x00,	// Advanced White Balance, varies
	// Same as common init	0x5197, 0x01,	// AWB local limit
	// Same as common init	0x519E, 0x38,	// AWB Control/Debug Mode
		0x3035, 0x21,	// 24MHz system clock div = 2, MIPI scale div = 1
		0x5000, 0x27,	// ISP Control
		0x5001, 0x83,	// ISP Control
		0x3035, 0x71,	// 24MHz system clock div = 4, MIPI scale div = 1
		0x4713, 0x02,	// JPEG mode select, 3 = mode 3 ... N/A, not using JPEG
//0x3036, 0x69,	// PLL Multiplier (fps)  << Fucks up image.  Probably too fast.
		0x3036, 0x30,	// PLL Multiplier (fps)
		0x4407, 0x0C,	// JPEG Control, quantization scale ... N/A, not using JPEG
		0x460B, 0x37,	// Debug mode?
		0x460C, 0x20,	// VFIFO control, DVP PCLK divider control by 0x3824[4:0]
		0x3824, 0x01,	// DVP PCLK divider D[4:0] (varies)
		0x4005, 0x1A,	// BLC always update
		0x4300, 0x30,	// Set YUV422 Format
		0x501F, 0x00,	// Format 0=YUV, 1 = RGB
		0x3008, 0x02,	// Software resume
#endif
		0
	};
	#include "OV5640_1080P.h"
	
	/*
	*	Preview Settings
	*/
	const uint16_t	kXAddrStart		= 800;	// [0x3800, 0x3801]
	const uint16_t	kYAddrStart		= 14;	// [0x3802, 0x3803]
	const uint16_t	kXAddrEnd		= 1800;	// [0x3804, 0x3805]
	const uint16_t	kYAddrEnd		= 1944;	// [0x3806, 0x3807]
	const uint32_t	kXOutputSize	= 124;	// [0x3808, 0x3809]
	const uint32_t	kYOutputSize	= 240;	// [0x380A, 0x380B]
	const uint16_t	kTotalWidth		= 1000;	// [0x380C, 0x380D]
	const uint16_t	kTotalHeight	= 1930;	// [0x380E, 0x380F]
	const uint16_t	kXInset			= 0;	// [0x3810, 0x3811]
	const uint16_t	kYInset			= 0;	// [0x3812, 0x3813]

	const uint16_t	kPreviewInit[] =
	{
		/*
		*	Geometry
		*/
		0x3820, 0x06,	// Sensor vflip
		0x3821, 0x00,	// Mirror, also bit 5, when set, enables JPEG <<<<
		0x3800, kXAddrStart >> 8,		// X Origin D[15:8]
		0x3801, kXAddrStart & 0xFF,		// X Origin D[7:0]
		0x3802, kYAddrStart >> 8,		// Y Origin D[15:8]
		0x3803, kYAddrStart & 0xFF,		// Y Origin D[7:0]
		0x3804, kXAddrEnd >> 8,			// X End D[11:8]
		0x3805, kXAddrEnd & 0xFF,		// X End D[7:0]
		0x3806, kYAddrEnd >> 8,			// Y End D[10:8]
		0x3807, kYAddrEnd & 0xFF,		// Y End D[7:0]
		0x3808, kXOutputSize >> 8,		// Output Width D[11:8]
		0x3809, kXOutputSize & 0xFF,	// Output Width D[7:0]
		0x380A, kYOutputSize >> 8,		// Output Height D[10:8]
		0x380B, kYOutputSize & 0xFF,	// Output Height D[7:0]
		0x380C, kTotalWidth >> 8,		// Total Width D[11:8] 
		0x380D, kTotalWidth & 0xFF,		// Total Width D[7:0]
		0x380E, kTotalHeight >> 8,		// Total Height D[15:8]
		0x380F, kTotalHeight & 0xFF,	// Total Height D[7:0]
		0x3810, kXInset >> 8,			// X Offset D[11:8]
		0x3811, kXInset & 0xFF,			// X Offset D[7:0]
		0x3812, kYInset >> 8,			// Y Offset D[10:8]
		0x3813, kYInset & 0xFF,			// Y Offset D[7:0]

		0x3814, 0x31,	// Timing X increment, D[7:4] odd, D[3:0] even
		0x3815, 0x31,	// Timing Y increment, D[7:4] odd, D[3:0] even
		// Digital Effects bit in 0x5001 is needed for hue/saturation control, brightness, contrast, etc.
		//0x5001, 0xA3,	// ISP Control Digital Effects, Scaling, Color Matrix, AWB
		//0x5001, 0x21,	// ISP Control Digital Effects, Scaling, Color Matrix, AWB
		//0x5001, 0x7F,	// ISP Control Digital Effects, Scaling, Color Matrix, AWB
		0x5001, 0x7E,	// ISP Control Digital Effects, Scaling, Color Matrix, AWB
	#if 0
		0x5580, 0x01,	// Enable hue
		0x5581, 0x6F,	// Hue cos coefficient
		0x5582, 0x40,	// Hue sin coefficient
		0x5588, 0x31,	// Hue sin & cos signs (I have no clue)
	#endif
		0x3035, 0x21,	// 24MHz system clock div = 2, MIPI scale div = 1
		0x3036, 0x30,	// PLL Multiplier (fps)
		0x3008, 0x02,	// Software resume
		0
	};
	/*
	*	Common initialzation settings.
	*/
	#include "OV5640_ComInitJ12.h"
	//#include "OV5640_ComInitJ4.h"
	enum EPixelFormat
	{
		e8BitGrayscale,
		eRGB888	= 2,
		eYUV422 = 4,
		//eJPEG	= 6,
		eRGB565 = 6
	};
	
	const uint8_t kPixelFormat[] =
	{
		0x10, 0x00,	// 8-Bit Grayscale
		0x23, 0x01,	// RGB888
		0x30, 0x00,	// YUV422
		//0x30, 0x00,	// JPEG
		0x6F, 0x01	// RGB565
	};
	
	enum ETestPattern
	{
		eHideTestPattern,
		eShowGradientPattern = 2,
		eShowColorBarPattern = 4
	};
	const uint8_t kTestPatternCmd[] =
	{
		0x10, 0x00,	// Hide Color Bar
		0x40, 0x8C,	// Show gradient color bar
		0x40, 0x80	// Show standard color bar
	};
	
}
#endif // OV5640_h
