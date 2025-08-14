	const uint16_t	kCommonInit[] = 
	{
		/*
		*	The defaults noted in the documentation don't appear to be accurate.
		*
		*	Many of the settings are not defined in the doc, and many of the
		*	values set are noted in the document as "Debug mode, changing this
		*	value is not allowed"
		*
		*/
		0x3031, 0x08,	// SC PWC Bit[3]: Bypass internal 1.5V regulator, use external
		0x4740, 0x22,	// Polarity control
		0x3008, 0x42,	// Software power down
		0x3103, 0x03,	// Select system input clock from PLL (D1=1)
		0x3017, 0xFF,	// Set Pins VSYNC, HREF, PCLK, D6 to D9 as output
		0x3018, 0xF3,	// Set Pins GPIO0, GPIO1, D2 to D5 as output (D0 & D1 are for 10 bit)
		0x3034, 0x18,	// (default) PLL Charge pump = 1, MIPI = 10-bit  (0x18 for 8-bit)
		0x3035, 0x41,	// 24MHz system clock div = 4, MIPI scale div = 1
		0x3037, 0x13,	// PLL root divider div/2, pre divider/3
		0x3108, 0x01,	// SCLK = pll_clki/2, SCLK2x = pll_clki, PCLK = pll_clki
		0x3630, 0x36,	// Undocumented
		0x3631, 0x0E,	// Undocumented
		0x3632, 0xE2,	// Undocumented
		0x3633, 0x12,	// Undocumented
		0x3621, 0xE0,	// Undocumented
		0x3704, 0xA0,	// Undocumented
		0x3703, 0x5A,	// Undocumented
		0x3715, 0x78,	// Undocumented
		0x3717, 0x01,	// Undocumented
		0x370B, 0x60,	// Undocumented
		0x3705, 0x1A,	// Undocumented
		0x3905, 0x02,	// Undocumented
		0x3906, 0x10,	// Undocumented
		0x3901, 0x0A,	// Undocumented
		0x3731, 0x12,	// Undocumented
	#if 0
		0x3600, 0x08,	// VCM control, debug Mode, undocumented
		0x3601, 0x33,	// VCM control, debug Mode, undocumented
		0x302D, 0x60,	// System control, undocumented
		0x3620, 0x52,	// Undocumented
		0x371B, 0x20,	// Undocumented
		0x471C, 0x50,	// Undocumented
	#endif
		0x3A13, 0x43,	// Pre-gain enabled, Pre-gain = ?
		0x3A18, 0x00,	// Gain Output Top Limit
		0x3A19, 0xF8,	// Gain Output Top Limit
		0x3635, 0x13,	// Undocumented
		0x3636, 0x03,	// Undocumented
		0x3634, 0x40,	// Undocumented
		0x3622, 0x01,	// Undocumented
		0x3C01, 0x34,	// 50/60Hz detector, counter threshold = 4, enabled, sum auto enabled
		0x3C04, 0x28,	// Threshold for low sum = 40
		0x3C05, 0x98,	// Threshold for high sum = 152
		0x3C06, 0x00,	// Lightmeter 1 threshold D15:8
		0x3C07, 0x00,	// Lightmeter 1 threshold D7:0
		0x3C08, 0x01,	// Lightmeter 2 threshold D15:8
		0x3C09, 0x2C,	// Lightmeter 1 threshold D7:0
		0x3C0A, 0x9C,	// Sample number D15:8
		0x3C0B, 0x40,	// Sample number D7:0 = 40000
		/*
		*	These undocmented settings appear to change based on the resolution.
		*/
		0x3618, 0x00,	// Undocumented 0x00 0x00 0x04 0x04 0x00 0x04
		0x3612, 0x29,	// Undocumented 0x29 0x49 0x4B 0x4B 0x49 0x2B
		0x3708, 0x64,	// Undocumented 0x64 0x66 0x21 0x64 0x64 0x64
		0x3709, 0x52,	// Undocumented 0x52 0x52 0x12 0x12 0x52 0x12
		0x370C, 0x03,	// Undocumented 0x03 0x03
		0x3A02, 0x03,	// Undocumented 0x03 0x01
		0x3A03, 0xD8,	// Undocumented 0xD8 0xF0 0xC4
		
		0x3A08, 0x01,	// 50Hz Band Width D[9:8]
		0x3A09, 0x27,	// 50Hz Band Width D[7:0]
		0x3A0A, 0x00,	// 60Hz Band Width D[13:8]
		0x3A0B, 0xF6,	// 60Hz Band Width D[7:0]
		0x3A0E, 0x03,	// 50Hz Max Bands in One Frame D[5:0]
		0x3A0D, 0x04,	// 60Hz Max Bands in One Frame D[5:0]
		0x3A14, 0x03,	// 50Hz Maximum Exposure Output Limit D[15:8]
		0x3A15, 0xD8,	// 50Hz Maximum Exposure Output Limit D[7:0]
		0x4001, 0x02,	// BLC start line D[5:0]
		0x4004, 0x02,	// BLC line number D[7:0]
		0x3000, 0x00,	// System Enable/Reset, 0 = enable all
		0x3002, 0x1C,	// System Enable/Reset, 1 = reset, 0x1C = reset JPG, SFIFO, JFIFO
		0x3004, 0xFF,	// Clock enable, enable all clocks
		0x3006, 0xC3,	// Clock enable, enable Average, MUX, FMT, PSRAM clocks
		0x300E, 0x58,	// MIPI/DVP Control, DVP enable, Power down MIPI RxTx
		0x302E, 0x00,	// Undocumented
		0x4713, 0x03,	// JPEG mode select, 3 = mode 3 ... N/A, not using JPEG
		0x4407, 0x04,	// JPEG Control, quantization scale ... N/A, not using JPEG
		0x440E, 0x00,	// Undocumented
		0x460B, 0x35,	// Debug mode?
		0x460C, 0x22,	// VFIFO control, DVP PCLK divider control by 0x3824[4:0]
		0x3824, 0x02,	// DVP PCLK divider D[4:0] (varies)
		0x5000, 0xA7,	// ISP Control
		0x5180, 0xFF,	// Advanced White Balance (default)
		0x5181, 0xF2,	// Advanced White Balance, varies
		0x5182, 0x00,	// Advanced White Balance, varies
		0x5183, 0x14,	// Advanced White Balance enabled
		0x5184, 0x25,	// Advanced White Balance
		0x5185, 0x24,	// Advanced White Balance
		0x5186, 0x09,	// AWB control register
		0x5187, 0x09,	// AWB control register
		0x5188, 0x09,	// AWB control register
		0x5189, 0x75,	// AWB control register
		0x518A, 0x54,	// AWB control register
		0x518B, 0xE0,	// AWB control register
		0x518C, 0xB2,	// AWB control register
		0x518D, 0x42,	// AWB control register
		0x518E, 0x3D,	// AWB control register
		0x518F, 0x56,	// AWB control register
		0x5190, 0x46,	// AWB control register
		0x5191, 0xF8,	// AWB top limit
		0x5192, 0x04,	// AWB bottom limit
		0x5193, 0x70,	// AWB Red limit
		0x5194, 0xF0,	// AWB Green limit
		0x5195, 0xF0,	// AWB Blue limit
		0x5196, 0x03,	// AWB control, fast enable, AWB bias stat
		0x5197, 0x01,	// AWB local limit
		0x5198, 0x04,	// Debug Mode
		0x5199, 0x12,	// Debug Mode
		0x519A, 0x04,	// Debug Mode
		0x519B, 0x00,	// Debug Mode
		0x519C, 0x06,	// Debug Mode
		0x519D, 0x82,	// Debug Mode
		0x519E, 0x38,	// AWB Control/Debug Mode
		0x5381, 0x1E,	// CMX control, CMX1 for Y
		0x5382, 0x5B,	// CMX control, CMX2 for Y
		0x5383, 0x08,	// CMX control, CMX3 for Y
		0x5384, 0x0A,	// CMX control, CMX4
		0x5385, 0x7E,	// CMX control, CMX5
		0x5386, 0x88,	// CMX control, CMX6
		0x5387, 0x7C,	// CMX control, CMX7
		0x5388, 0x6C,	// CMX control, CMX8
		0x5389, 0x10,	// CMX control, CMX9
		0x538A, 0x01,	// Cmxsign CMX9
		0x538B, 0x98,	// Cmxsign CMX1 to CMX8
		0x5300, 0x08,	// CIP threshold 1
		0x5301, 0x30,	// CIP threshold 2
		0x5302, 0x10,	// CIP offset 1
		0x5303, 0x00,	// CIP offset 2
		0x5304, 0x08,	// CIP DNS threshold 1
		0x5305, 0x30,	// CIP DNS threshold 2
		0x5306, 0x08,	// CIP DNS offset 1
		0x5307, 0x16,	// CIP DNS offset 2
		0x5309, 0x08,	// CIP Sharpen Threshold 1
		0x530A, 0x30,	// CIP Sharpen Threshold 2
		0x530B, 0x04,	// CIP Sharpen Offset 1
		0x530C, 0x06,	// CIP Sharpen Offset 2
		0x5480, 0x01,	// Gamma Control
		0x5481, 0x08,	// Gamma Y yst 00
		0x5482, 0x14,	// Gamma Y yst 01
		0x5483, 0x28,	// Gamma Y yst 02
		0x5484, 0x51,	// Gamma Y yst 03
		0x5485, 0x65,	// Gamma Y yst 04
		0x5486, 0x71,	// Gamma Y yst 05
		0x5487, 0x7D,	// Gamma Y yst 06
		0x5488, 0x87,	// Gamma Y yst 07
		0x5489, 0x91,	// Gamma Y yst 08
		0x548A, 0x9A,	// Gamma Y yst 09
		0x548B, 0xAA,	// Gamma Y yst 0A
		0x548C, 0xB8,	// Gamma Y yst 0B
		0x548D, 0xCD,	// Gamma Y yst 0C
		0x548E, 0xDD,	// Gamma Y yst 0D
		0x548F, 0xEA,	// Gamma Y yst 0E
		0x5490, 0x1D,	// Gamma Y yst 0F
		0x5580, 0x00,	// SDE control, contrast enable
		0x5583, 0x40,	// SDE control saturation U
		0x5584, 0x10,	// SDE control saturation V
		0x5589, 0x10,	// UV Adjust threshold 1
		0x558A, 0x00,	// UV Adjust threshold 2 D[8]
		0x558B, 0xF8,	// UV Adjust threshold 2 D[7:0]
		0x5800, 0x23,	// Lens correction Green matrix 00
		0x5801, 0x14,	// Lens correction Green matrix 01
		0x5802, 0x0F,	// Lens correction Green matrix 02
		0x5803, 0x0F,	// Lens correction Green matrix 03
		0x5804, 0x12,	// Lens correction Green matrix 04
		0x5805, 0x26,	// Lens correction Green matrix 05
		0x5806, 0x0C,	// Lens correction Green matrix 06
		0x5807, 0x08,	// Lens correction Green matrix 07
		0x5808, 0x05,	// Lens correction Green matrix 08
		0x5809, 0x05,	// Lens correction Green matrix 09
		0x580A, 0x08,	// Lens correction Green matrix 0A
		0x580B, 0x0D,	// Lens correction Green matrix 0B
		0x580C, 0x08,	// Lens correction Green matrix 0C
		0x580D, 0x03,	// Lens correction Green matrix 0D
		0x580E, 0x00,	// Lens correction Green matrix 0E
		0x580F, 0x00,	// Lens correction Green matrix 0F
		0x5810, 0x03,	// Lens correction Green matrix 10
		0x5811, 0x09,	// Lens correction Green matrix 11
		0x5812, 0x07,	// Lens correction Green matrix 12
		0x5813, 0x03,	// Lens correction Green matrix 13
		0x5814, 0x00,	// Lens correction Green matrix 14
		0x5815, 0x01,	// Lens correction Green matrix 15
		0x5816, 0x03,	// Lens correction Green matrix 16
		0x5817, 0x08,	// Lens correction Green matrix 17
		0x5818, 0x0D,	// Lens correction Green matrix 18
		0x5819, 0x08,	// Lens correction Green matrix 19
		0x581A, 0x05,	// Lens correction Green matrix 1A
		0x581B, 0x06,	// Lens correction Green matrix 1B
		0x581C, 0x08,	// Lens correction Green matrix 1C
		0x581D, 0x0E,	// Lens correction Green matrix 1D
		0x581E, 0x29,	// Lens correction Green matrix 1E
		0x581F, 0x17,	// Lens correction Green matrix 1F
		0x5820, 0x11,	// Lens correction Green matrix 20
		0x5821, 0x11,	// Lens correction Green matrix 21
		0x5822, 0x15,	// Lens correction Green matrix 22
		0x5823, 0x28,	// Lens correction Green matrix 23
		0x5824, 0x46,	// Lens correction Green matrix 00
		0x5825, 0x26,	// Lens correction Red/Blue matrix 01
		0x5826, 0x08,	// Lens correction Red/Blue matrix 02
		0x5827, 0x26,	// Lens correction Red/Blue matrix 03
		0x5828, 0x64,	// Lens correction Red/Blue matrix 04
		0x5829, 0x26,	// Lens correction Red/Blue matrix 05
		0x582A, 0x24,	// Lens correction Red/Blue matrix 06
		0x582B, 0x22,	// Lens correction Red/Blue matrix 07
		0x582C, 0x24,	// Lens correction Red/Blue matrix 08
		0x582D, 0x24,	// Lens correction Red/Blue matrix 09
		0x582E, 0x06,	// Lens correction Red/Blue matrix 20
		0x582F, 0x22,	// Lens correction Red/Blue matrix 21
		0x5830, 0x40,	// Lens correction Red/Blue matrix 22
		0x5831, 0x42,	// Lens correction Red/Blue matrix 23
		0x5832, 0x24,	// Lens correction Red/Blue matrix 24
		0x5833, 0x26,	// Lens correction Red/Blue matrix 30
		0x5834, 0x24,	// Lens correction Red/Blue matrix 31
		0x5835, 0x22,	// Lens correction Red/Blue matrix 32
		0x5836, 0x22,	// Lens correction Red/Blue matrix 33
		0x5837, 0x26,	// Lens correction Red/Blue matrix 34
		0x5838, 0x44,	// Lens correction Red/Blue matrix 40
		0x5839, 0x24,	// Lens correction Red/Blue matrix 41
		0x583A, 0x26,	// Lens correction Red/Blue matrix 42
		0x583B, 0x28,	// Lens correction Red/Blue matrix 43
		0x583C, 0x42,	// Lens correction Red/Blue matrix 44
		0x583D, 0xCE,	// Lens correction Red/Blue offset
		0x5025, 0x00,	// Undocumented
	#if 1	// STM Settings
		0x3A0F, 0x30, 0x3A10, 0x28, 0x3A11, 0x60,	// AEC Control
		0x3A1B, 0x30, 0x3A1E, 0x26, 0x3A1F, 0x14,	// AEC Control
	#endif
	#if 0	// OV doc Default Settings
		0x3A0F, 0x38, 0x3A10, 0x30, 0x3A11, 0x61, 	// AEC Control
		0x3A1B, 0x38, 0x3A1E, 0x30, 0x3A1F, 0x10,	// AEC Control
	#endif
	#if 0	// OV doc 0.3EV Settings
		0x3A0F, 0x40, 0x3A10, 0x38, 0x3A11, 0x71, 	// AEC Control
		0x3A1B, 0x40, 0x3A1E, 0x38, 0x3A1F, 0x10,	// AEC Control
	#endif
	#if 0	// OV doc 1.7EV Settings
		0x3A0F, 0x60, 0x3A10, 0x58, 0x3A11, 0xA0, 	// AEC Control
		0x3A1B, 0x60, 0x3A1E, 0x58, 0x3A1F, 0x20,	// AEC Control
	#endif
		0x3016, 0x02,	// Strobe output pad enable
		0x3B00, 0x03,	// Strobe off
		0x3B07, 0x82,	// FREX mode off
		0
	};
