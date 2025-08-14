/*
*	TFT_ILI9488P.cpp, Copyright Jonathan Mackey 2025
*	Class to control a 16 bit [Parallel TFT ILI9488 display controller.
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
#include "TFT_ILI9488P.h"
#include <DataStream.h>
#include "Arduino.h"

SRAM_HandleTypeDef hsram1;
static uint32_t FMC_Initialized = 0;
static uint32_t FMC_DeInitialized = 0;

/*
NOR / PSRAM timing:

From STM AN2784 doc, section 3.2 Timing computation.
Note that AN2784 was written for STM32F1x so it may not be applicable.
((ADDSET + 1) + (DATAST + 1)) × tHclk ≥ max (tWC)
DATAST × tHclk = tPWE1

HCLK = Internal AHB clock frequency	= 180 Mhz
tHclk = Internal AHB clock cycle = 1/HCLK = 5.56ns
tWC = Write cycle time = 40 ns in ILI9488 doc
tRC = Read cycle time = 180 ns in ILI9488 doc
tPWE1 = Write Enable low pulse width = twrl in ILI9488 doc = 15 ns
tAA = Address access time = 0 ns in ILI9488 doc

DATAST × tHclk = tPWE1
DATAST = tPWE1/tHclk = 2.69m rounded up = 3

twc from the ILI9488 doc = 40ns
((ADDSET + 1) + (DATAST + 1)) × tHclk ≥ max (tWC)
((0 + 1) + (3 + 1)) x 5.56 = 27.8 which doesn't satisfy the rule.
Adding 2 to ADDSET and 1 to DATAST satisfies the rule (assuming write only.)
((2 + 1) + (4 + 1)) x 5.56 = 44.4

The "Bus turn around time" setting, or BusTurnAroundDuration is only relevant 
for multiplexed NOR Flash memory, so this can be set to zero.

/******************************** TFT_ILI9488P ********************************/
TFT_ILI9488P::TFT_ILI9488P(
	pin_t		inResetPin, 
	pin_t		inBacklightPin,
	uint16_t	inHeight,
	uint16_t	inWidth,
	bool		inCentered,
	bool		inIsBGR)
	: DisplayController(inHeight, inWidth),
	  mResetPin(inResetPin),
	  mBacklightPin(inBacklightPin), mRowOffset(0), mColOffset(0),
	  mCentered(inCentered), mIsBGR(inIsBGR), mInvColAddrOrder(true)
{
}

/*********************************** begin ************************************/
void TFT_ILI9488P::begin(
	uint8_t	inRotation)
{
	if (mBacklightPin >= 0)
	{
		pinMode(mBacklightPin, OUTPUT);
		digitalWrite(mBacklightPin, LOW);	// Off
	}

	if (mResetPin >= 0)
	{
		pinMode(mResetPin, OUTPUT);
		digitalWrite(mResetPin, HIGH);
	}
	Init();
	SetRotation(inRotation);
}

/********************************** InitFMC ***********************************/
bool TFT_ILI9488P::InitFMC(void)
{
	FMC_NORSRAM_TimingTypeDef Timing = {0};

	hsram1.Instance = FMC_NORSRAM_DEVICE;
	hsram1.Extended = FMC_NORSRAM_EXTENDED_DEVICE;
	/* hsram1.Init */
	hsram1.Init.NSBank = FMC_NORSRAM_BANK1;
	hsram1.Init.DataAddressMux = FMC_DATA_ADDRESS_MUX_DISABLE;
	hsram1.Init.MemoryType = FMC_MEMORY_TYPE_SRAM;
	hsram1.Init.MemoryDataWidth = FMC_NORSRAM_MEM_BUS_WIDTH_16;
	hsram1.Init.BurstAccessMode = FMC_BURST_ACCESS_MODE_DISABLE;
	hsram1.Init.WaitSignalPolarity = FMC_WAIT_SIGNAL_POLARITY_LOW;
	hsram1.Init.WrapMode = FMC_WRAP_MODE_DISABLE;
	hsram1.Init.WaitSignalActive = FMC_WAIT_TIMING_BEFORE_WS;
	hsram1.Init.WriteOperation = FMC_WRITE_OPERATION_ENABLE;
	hsram1.Init.WaitSignal = FMC_WAIT_SIGNAL_DISABLE;
	hsram1.Init.ExtendedMode = FMC_EXTENDED_MODE_DISABLE;
	hsram1.Init.AsynchronousWait = FMC_ASYNCHRONOUS_WAIT_DISABLE;
	hsram1.Init.WriteBurst = FMC_WRITE_BURST_DISABLE;
	hsram1.Init.ContinuousClock = FMC_CONTINUOUS_CLOCK_SYNC_ONLY;
	hsram1.Init.PageSize = FMC_PAGE_SIZE_NONE;
	/*
	*	Timing
	*
	*	See the AC characteristics for DBI Type B in the ILI9488 documentation.
	*/
	Timing.AddressSetupTime = 0;
	Timing.AddressHoldTime = 15;
	/*
	*	The data setup time: 
	*
	*	If reading frame memory DataSetupTime = 58
	*		((Read access time FM)/(1/HCLK) = 340ns/(1/168MHz) = 57.12)
	*
	*	If reading register values DataSetupTime = 7
	*		((Read access time)/(1/HCLK) = 40ns/(1/168MHz) = 6.72)
	*
	*	If write only DataSetupTime = 2
	*		((Write data setup time)/(1/HCLK) = 10ns/(1/168MHz) = 1.68)
	*/
	Timing.DataSetupTime = 2;
	Timing.BusTurnAroundDuration = 0;	// n/a for this type of memory
	Timing.CLKDivision = 16;
	Timing.DataLatency = 17;
	Timing.AccessMode = FMC_ACCESS_MODE_A;
	/* ExtTiming */
	
	return(HAL_SRAM_Init(&hsram1, &Timing, NULL) == HAL_OK);
}

/************************************ Init ************************************/
void TFT_ILI9488P::Init(void)
{
	InitFMC();	// STM32F429 specific initialization code.
	
	// These settings were copied (mostly) from Adafruit_ILI9488.cpp
	static const uint8_t initCmds[] PROGMEM
	{
		eGMCTRP1Cmd, 15,	// 0xE0 Positive Gamma Correction
		0x00, 0x03, 0x09,
		0x08, 0x16, 0x0A,
		0x3F, 0x78, 0x4C,
		0x09, 0x0A, 0x08,
		0x16, 0x1A, 0x0F,
		eGMCTRN1Cmd, 15,	// 0xE1 Negative Gamma Correction
		0x00, 0x16, 0x19,
		0x03, 0x0F, 0x05,
		0x32, 0x45, 0x46,
		0x04, 0x0E, 0x0D,
		0x35, 0x37, 0x0F,
		ePWCTR1Cmd, 2,		// 0xC0 Power Control 1
		0x17, 0x15,
		ePWCTR2Cmd, 1,		// 0xC1 Power Control 2
		0x41,
		eVMCTR1Cmd, 3,		// 0xC5 VCM control
		0x00, 0x12, 0x80,
		//eINTMDCTRLCmd, 1,		// 0xB0 Interface Mode Control
		//0x00,				// SDO/MISO in use (00=default)
		//eINTMDCTRLCmd, 1,		// 0xB0 Interface Mode Control
		//0x80,				// SDO/MISO don't use
		eCOLMODCmd, 1,		// 0x3A Interface Pixel Format
		0x55,				// 565 16 bit
		eFRMCTR1Cmd, 1,		// 0xB1 Frame Rate Control
		0xA0,				// 60Hz
		eINVCTRCmd, 1,		// 0xB4 Display Inversion Control
		0x02,				// 2-Dot
		
		//eDISSET5Cmd, 3,		// 0xB6 Display Function Control
		//0x02, 0x02, 0x3B,	//	3B = 480 lines (default)
		//0x08, 0x82, 0x27, //	27 = 320 lines
		eSETIMGFUNCCmd, 1,	// 0xE9 Set Image Function
		0x00,				// Disable 24 bit data
		//eADJCTR3Cmd, 4,		// 0xF7 Adjust Control 3
		//0xA9, 0x51, 0x2C, 0x82, // loose (default)
		//0xA9, 0x51, 0x2C, 0x02, // stream
		eDISPONCmd, 0x80,	// 0x29 Display on
		0					// End of list
	};
	delay(150);

	// Init does a hardware or software reset depending on whether the reset
	// pin is defined followed by a Wake command.
	if (mResetPin >= 0)
	{
		delay(1);
		digitalWrite(mResetPin, LOW);
		delay(1);
		digitalWrite(mResetPin, HIGH);
	} else
	{
		WriteCmd(eSWRESETCmd);
	}
	// Per docs: After reset, delay 150ms before sending the next command.
	// (The controller IC is in the process of writing the defaults.)
	delay(150);
	WriteWakeUpCmds();	// By default the controller is asleep after reset.

#if 1
	WriteCmds(initCmds);
#else
	/*
	*	Note: In order to use code that reads from the display, the DataSetupTime
	*	in InitFMC() must be increased.  See notes in InitFMC().
	*
	*	The expected values for the ILI9488 are:
	*	ID1 = 0x54, ID2 = 0x80, ID3 = 0x66
	*	colmodcmdB = 0x6, colmodcmdA = 0x5	(tests to see if the value is being set)
	*	manufacturerID = 0x54	should be the same as ID1
	*	versionID = 0x80		should be the same as ID2
	*	driverID = 0x66			should be the same as ID3
	*
	*	If the values are wrong, try playing with DataSetupTime in InitFMC()
	*/
	uint32_t	idInfoID1 = ReadCmdReg(0x4);
	uint32_t	idInfoID2 = ReadData();
	uint32_t	idInfoID3 = ReadData();
	uint32_t	colmodcmdB = ReadCmdReg(0xC);
	WriteCmds(initCmds);
	uint32_t	colmodcmdA = ReadCmdReg(0xC);
	uint32_t	manufacturerID = ReadCmdReg(0xDA);
	uint32_t	versionID = ReadCmdReg(0xDB);
	uint32_t	driverID = ReadCmdReg(0xDC);

	Serial.printf("ID1 = 0x%X, ID2 = 0x%X, ID3 = 0x%X\n", idInfoID1, idInfoID2, idInfoID3);
	Serial.printf("colmodcmdB = 0x%X, colmodcmdA = 0x%X\n", colmodcmdB, colmodcmdA);
	Serial.printf("manufacturerID = 0x%X\n", manufacturerID);
	Serial.printf("versionID = 0x%X\n", versionID);
	Serial.printf("driverID = 0x%X\n", driverID);
	uint32_t	dispStatP2 = ReadCmdReg(0x9);
	uint32_t	dispStatP3 = ReadData();
	uint32_t	dispStatP4 = ReadData();
	uint32_t	dispStatP5 = ReadData();
	Serial.printf("Disp status:\n\tP2 = 0x%X\n\tP3 = 0x%X\n\tP4 = 0x%X\n\tP5 = 0x%X\n", dispStatP2, dispStatP3, dispStatP4, dispStatP5);
/*
Disp status, expect to see:
	P2 = 0x80	0b10000000	Booster ON
	P3 = 0x53	0b01010011	Display Normal Mode ON, Sleep OUT Mode, 16-bit/pixel
	P4 = 0x4	0b00000100	Display is ON
	P5 = 0x0	0b00000000
*/
#endif
}

/********************************* ReadCmdReg *********************************/
uint8_t TFT_ILI9488P::ReadCmdReg(
	uint8_t inCmd) const
{
	WriteCmd(inCmd);
	ReadData();	// Skip dummy data
	return(ReadData());
}

/********************************** WriteCmd **********************************/
void TFT_ILI9488P::WriteCmd(
	uint8_t			inCmd,
	const uint8_t*	inCmdData,
	uint8_t			inCmdDataLen,
	bool			inPGM) const
{
	WriteCmd(inCmd);
	WriteData(inCmdData, inCmdDataLen, inPGM);
}

/********************************* WriteCmds **********************************/
/*
*	A null terminated array of commands of the format:
*	{Cmd, cmdDataLen, [cmdData]}, ... {0}
*	inCmds is an address in PROGMEM
*/
void TFT_ILI9488P::WriteCmds(
	const uint8_t*	inCmds) const
{
	uint8_t	cmd, cmdDataLen;
	while (true)
	{
		cmd = pgm_read_byte(inCmds++);
		if (cmd)
		{
			WriteCmd(cmd);
			cmdDataLen = pgm_read_byte(inCmds++);
			bool delayNext = (cmdDataLen & 0x80) != 0;
			cmdDataLen &= 0x7F;
			if (cmdDataLen)
			{
				do 
				{
					*FMC_DataAddr = pgm_read_byte(inCmds++);
				} while (--cmdDataLen);
			}
			if (delayNext)
			{
 				delay(150);
			}
			continue;
		}
		break;
	}
}

/********************************* WriteData **********************************/
void TFT_ILI9488P::WriteData(
	const uint8_t*	inData,
	uint16_t		inDataLen,
	bool			inPGM) const
{
	if (inDataLen)
	{
		if (inPGM)
		{
			do 
			{
				*FMC_DataAddr = pgm_read_byte(inData++);
			} while (--inDataLen);
		} else
		{
			do
			{
				*FMC_DataAddr = *(inData++);
			} while (--inDataLen);
		}
	}
}

/******************************** WriteData16 *********************************/
void TFT_ILI9488P::WriteData16(
	const uint16_t*	inData,
	uint16_t		inDataLen) const
{
	for (uint32_t i = 0; i < inDataLen; i++)
	{
		*FMC_DataAddr = inData[i];
	}
}

/******************************** WriteDataHL16 *******************************/
/*
*	Breaks inData into inDataH inDataL.  Used by commands
*/
void TFT_ILI9488P::WriteDataHL16(
	uint16_t	inData) const
{
	*FMC_DataAddr = inData >> 8;
	*FMC_DataAddr = inData;
}

/******************************** SetRotation *********************************/
void TFT_ILI9488P::SetRotation(
	uint8_t	inRotation)
{
/*
	MADCTL for ST77XX.  For the ILI9340 display I have MX is inverted.
		MY	MX	MV	
[0]	0	0	0	0	
[1]	90	0	1	1	
[2]	180	1	1	0	
[3]	270	1	0	1	

	Bit mask
MY  = 0x80 Row Address Order
MX  = 0x40 Column Address Order
MV  = 0x20 Row / Column Exchange
ML  = 0x10 Vertical Refresh Order (when set it's bottom to top)
RGB = 0x08 RGB-BGR Order (when set it's BGR)
MH  = 0x04 Horizontal Refresh Order (when set it's right to left)

For both the 89 and 35R the RGB order is BGR.

*/
	inRotation &= 3;
	/*
	*	It looks like there's a manufacturing mistake in the 160x80 displays
	*	that I have (maybe all?) where RGB is BGR based on the controller
	*	RGB/BGR order bit is the opposite of what you expect.  So either there's
	*	a bug in the ST7735S controller or the display manufacturer screwed up.
	*
	*	This is the reason for the mIsBGR flag.  
	*
	*	The other thing about the 160x80 display is that rather than pick 0,0
	*	to be the origin, they centered it in ST7735S's 162x132 memory window,
	*	so all xy values need to be offset regardless of rotation.  For the
	*	240x240 display they did not center it, so you only need to offset when
	*	rotating by 180 and 270.
	*
	*	This is the reason for the mCentered flag.
	*/
	//											0    90    180   270
	static const uint8_t cmdData[] PROGMEM = {0x08, 0x68, 0xC8, 0xA8};
	uint8_t	madctlParam = pgm_read_byte(&cmdData[inRotation]);
	if (mIsBGR)
	{
		madctlParam &= ~8;
	}
	if (mInvColAddrOrder)
	{
		// Added for TFT_ILI9341.  The ILI9340 board I have needs this inverted.
		// I'll assume all of the boards using this driver family need this.
		madctlParam ^= 0x40;
	}

	mMADCTL = madctlParam;
	WriteCmd(eMADCTLCmd);
	*FMC_DataAddr = madctlParam;

	{
		uint16_t	vDelta = VerticalRes() - mRows;
		uint16_t	hDelta = HorizontalRes() - mColumns;
		if (mCentered)
		{
			mRowOffset = vDelta /= 2;
			mColOffset = hDelta /= 2;
		} else
		{
			if (inRotation & 2)
			{
				mRowOffset = vDelta;
				mColOffset = hDelta;
			} else
			{
				mRowOffset = 0;
				mColOffset = 0;
			}
		}
	}
	if (inRotation & 1)
	{
		uint16_t	temp = mRows;
		mRows = mColumns;
		mColumns = temp;
		temp = mRowOffset;
		mRowOffset = mColOffset;
		mColOffset = temp;
	}
}

/****************************** TemporaryWindow *******************************/
/*
*	Sets up a temporary window with its own rotation settings.  It should only
*	be used by code that writes directly to the FMC data address, such as a
*	camera.  When finished with the window call RestoreRotation() otherwise all
*	standard display controller calls will not draw as expected.
*/
void TFT_ILI9488P::TemporaryWindow(
	bool		inFlipRows,
	bool		inFlipColumns,
	bool		inSwapRowColumns,
	uint16_t	inStartRow,
	uint16_t	inEndRow,
	uint16_t	inStartColumn,
	uint16_t	inEndColumn)
{
	uint8_t	madctlParam = mMADCTL & 0x1F;
	if (inFlipRows)
	{
		madctlParam |= 0x80;
	}
	if (inFlipColumns)
	{
		madctlParam |= 0x40;
	}
	if (inSwapRowColumns)
	{
		madctlParam |= 0x20;
	}
	WriteCmd(eMADCTLCmd);
	*FMC_DataAddr = madctlParam;
	WriteCmd(eCASETCmd);
	WriteDataHL16(inStartColumn);
	WriteDataHL16(inEndColumn-1);
	WriteCmd(eRASETCmd);
	WriteDataHL16(inStartRow);
	WriteDataHL16(inEndRow-1);
	WriteCmd(eRAMWRCmd); // Resets controller memory ptr to inStartColumn and
						 // the start of current the row frame 
}

/****************************** RestoreRotation *******************************/
void TFT_ILI9488P::RestoreRotation(void)
{
	WriteCmd(eMADCTLCmd);
	*FMC_DataAddr = mMADCTL;
}

/*********************************** Sleep ************************************/
void TFT_ILI9488P::Sleep(void)
{
	WriteSleepCmds();
}

/******************************* WriteSleepCmds *******************************/
void TFT_ILI9488P::WriteSleepCmds(void)
{
	WriteCmd(eSLPINCmd);
	// Per docs: When going to Sleep, delay 120ms before sending the next command.
	// delay(120);  << This assumes there will be no commands after calling
	// this routine.
	
	if (mBacklightPin >= 0)
	{
		digitalWrite(mBacklightPin, LOW);	// Off
	}
}

/*********************************** WakeUp ***********************************/
void TFT_ILI9488P::WakeUp(void)
{
	WriteWakeUpCmds();
}

/****************************** WriteWakeUpCmds *******************************/
void TFT_ILI9488P::WriteWakeUpCmds(void)
{
	WriteCmd(eSLPOUTCmd);
	// Per docs: When waking from Sleep, delay 150ms before sending the next command.
	// Some of the displays are as low as 5ms.
	delay(150);
	if (mBacklightPin >= 0)
	{
		digitalWrite(mBacklightPin, HIGH);	// On
	}

}

/********************************* FillPixels *********************************/
void TFT_ILI9488P::FillPixels(
	uint32_t	inPixelsToFill,
	uint16_t	inFillColor)
{
	for (; inPixelsToFill; inPixelsToFill--)
	{
		*FMC_DataAddr = inFillColor;
	}
}

/*********************************** MoveTo ***********************************/
// No bounds checking.  Blind move.
void TFT_ILI9488P::MoveTo(
	uint16_t	inRow,
	uint16_t	inColumn)
{
	MoveToRow(inRow);
	mColumn = inColumn;
}

/********************************* MoveToRow **********************************/
// No bounds checking.  Blind move.
void TFT_ILI9488P::MoveToRow(
	uint16_t inRow)
{
	WriteCmd(eRASETCmd);
	WriteDataHL16(inRow + mRowOffset);
	WriteDataHL16(mRows + mRowOffset -1);
	mRow = inRow;
}

/******************************** MoveToColumn ********************************/
// No bounds checking.  Blind move.
// Doesn't actually make any changes to the controller.  Used by  other
// functions to set the start column relative to mColumn.
// (e.g. the relative version of SetColumnRange as defined in DisplayController)
void TFT_ILI9488P::MoveToColumn(
	uint16_t inColumn)
{
	mColumn = inColumn;
}

/******************************* SetColumnRange *******************************/
void TFT_ILI9488P::SetColumnRange(
	uint16_t	inStartColumn,
	uint16_t	inEndColumn)
{
	WriteCmd(eCASETCmd);
	WriteDataHL16(inStartColumn + mColOffset);
	WriteDataHL16(inEndColumn + mColOffset);
	WriteCmd(eRAMWRCmd); // Resets controller memory ptr to inStartColumn and
						 // the start of current the row frame 
}

/******************************* SetRowRange *******************************/
void TFT_ILI9488P::SetRowRange(
	uint16_t	inStartRow,
	uint16_t	inEndRow)
{
	WriteCmd(eRASETCmd);
	WriteDataHL16(inStartRow + mRowOffset);
	WriteDataHL16(inEndRow + mRowOffset);
	// Does not send a start RAM write command.
	// SetRowRange should be called before SetColumnRange.
}

/******************************** StreamCopy **********************************/
void TFT_ILI9488P::StreamCopy(
	DataStream* inDataStream,	// A 16 bit data stream
	uint16_t	inPixelsToCopy)
{
	uint16_t	buffer[96];	// WritePixelData's buffer holds 96 pixels.
	while (inPixelsToCopy)
	{
		uint16_t pixelsToWrite = inPixelsToCopy > 96 ? 96 : inPixelsToCopy;
		inPixelsToCopy -= pixelsToWrite;
		inDataStream->Read(pixelsToWrite, buffer);
		WritePixelData(buffer, pixelsToWrite);
	}
}

/******************************** CopyPixels **********************************/
void TFT_ILI9488P::CopyPixels(
	const void*		inPixels,
	uint16_t		inPixelsToCopy)
{
	WritePixelData((const uint16_t*)inPixels, inPixelsToCopy);
}

/******************************* WritePixelData *******************************/
/*
*	inPixelData is an address in SRAM that points to RGB565 16 bit values.
*	The RGB565 values are converted to RGB666 values.
*/
void TFT_ILI9488P::WritePixelData(
	const uint16_t* inPixelData,
	uint16_t		inDataLen) const
{
	for (uint16_t i = 0; i < inDataLen; i++)
	{
		*FMC_DataAddr = inPixelData[i];
	}
}

/***************************** CopyTintedPattern ******************************/
/*
*	Added as an optimization for drawing anti-aliased lines.  The pattern is
*	copied inReps times in either the horizontal or vertical direction.
*
*	The foreground and background colors need to be set using SetFGColor and
*	SetBGColor prior to calling this routine.
*
*	inTintPattern is an array of tint values (0 to 255.) The tints are converted
*	to colors using the foreground and background colors. When inReverseOrder is
*	true, the tint conversion to color values starts at offset inPatternLen-1
*	and ends at offset 0.
*/
void TFT_ILI9488P::CopyTintedPattern(
	uint16_t		inX,
	uint16_t		inY,
	const uint8_t*	inTintPattern,
	uint16_t		inPatternLen,
	uint16_t		inReps,
	bool			inVertical,
	bool			inReverseOrder)
{	
	uint16_t	colorPattern[inPatternLen];
	uint8_t		thisTint;
	uint8_t		lastTint;
	uint16_t	color = 0;
	if (inReverseOrder)
	{
		const uint8_t*	patternPtr = &inTintPattern[inPatternLen-1];
		lastTint = *patternPtr + 1;
		for (uint16_t i = 0; i < inPatternLen; i++)
		{
			thisTint = *(patternPtr--);
			if (lastTint != thisTint)
			{
				lastTint = thisTint;
				color = Calc565Color(mFGColor, mBGColor, thisTint);
			}
			colorPattern[i] = color;
		}
	} else
	{
		lastTint = inTintPattern[0] + 1;
		for (uint16_t i = 0; i < inPatternLen; i++)
		{
			thisTint = inTintPattern[i];
			if (lastTint != thisTint)
			{
				lastTint = thisTint;
				color = Calc565Color(mFGColor, mBGColor, thisTint);
			}
			colorPattern[i] = color;
		}
	}
	uint16_t	relativeWidth = inVertical ? 1 : inPatternLen;
	for (uint16_t i = inReps; i; i--)
	{
		MoveTo(inY, inX);
		DisplayController::SetColumnRange(relativeWidth);
		if (inVertical)
		{
			inX++;
		} else
		{
			inY++;
		}
		CopyPixels(colorPattern, inPatternLen);
	}
}

/**
  ******************************************************************************
  * @file         Code copied from stm32f4xx_hal_msp.c
  * @brief        This file provides code for the MSP Initialization
  *               and de-Initialization codes.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
  
/*
*	The code below was generated using STM32CubeIDE based on the configuration
*	needed for this project.  Only the FMC and SRAM functions were copied from
*	the original stm32f4xx_hal_msp.c because the original STM32CubeIDE generated
*	file contains initialization routines for SPI, I2C, sysclock, RTC, etc.,
*	that are handled by Arduino core code.
*/

static void HAL_FMC_MspInit(void){
  /* USER CODE BEGIN FMC_MspInit 0 */

  /* USER CODE END FMC_MspInit 0 */
  GPIO_InitTypeDef GPIO_InitStruct ={0};
  if (FMC_Initialized) {
    return;
  }
  FMC_Initialized = 1;

  /* Peripheral clock enable */
  __HAL_RCC_FMC_CLK_ENABLE();

  /** FMC GPIO Configuration
  PE7   ------> FMC_D4
  PE8   ------> FMC_D5
  PE9   ------> FMC_D6
  PE10   ------> FMC_D7
  PE11   ------> FMC_D8
  PE12   ------> FMC_D9
  PE13   ------> FMC_D10
  PE14   ------> FMC_D11
  PE15   ------> FMC_D12
  PD8   ------> FMC_D13
  PD9   ------> FMC_D14
  PD10   ------> FMC_D15
  PD11   ------> FMC_A16
  PD14   ------> FMC_D0
  PD15   ------> FMC_D1
  PD0   ------> FMC_D2
  PD1   ------> FMC_D3
  PD4   ------> FMC_NOE
  PD5   ------> FMC_NWE
  PD7   ------> FMC_NE1
  */
  GPIO_InitStruct.Pin = GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10
                          |GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14
                          |GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11
                          |GPIO_PIN_14|GPIO_PIN_15|GPIO_PIN_0|GPIO_PIN_1
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* USER CODE BEGIN FMC_MspInit 1 */

  /* USER CODE END FMC_MspInit 1 */
}

void HAL_SRAM_MspInit(SRAM_HandleTypeDef* hsram){
  /* USER CODE BEGIN SRAM_MspInit 0 */

  /* USER CODE END SRAM_MspInit 0 */
  HAL_FMC_MspInit();
  /* USER CODE BEGIN SRAM_MspInit 1 */

  /* USER CODE END SRAM_MspInit 1 */
}

static void HAL_FMC_MspDeInit(void){
  /* USER CODE BEGIN FMC_MspDeInit 0 */

  /* USER CODE END FMC_MspDeInit 0 */
  if (FMC_DeInitialized) {
    return;
  }
  FMC_DeInitialized = 1;
  /* Peripheral clock enable */
  __HAL_RCC_FMC_CLK_DISABLE();

  /** FMC GPIO Configuration
  PE7   ------> FMC_D4
  PE8   ------> FMC_D5
  PE9   ------> FMC_D6
  PE10   ------> FMC_D7
  PE11   ------> FMC_D8
  PE12   ------> FMC_D9
  PE13   ------> FMC_D10
  PE14   ------> FMC_D11
  PE15   ------> FMC_D12
  PD8   ------> FMC_D13
  PD9   ------> FMC_D14
  PD10   ------> FMC_D15
  PD11   ------> FMC_A16
  PD14   ------> FMC_D0
  PD15   ------> FMC_D1
  PD0   ------> FMC_D2
  PD1   ------> FMC_D3
  PD4   ------> FMC_NOE
  PD5   ------> FMC_NWE
  PD7   ------> FMC_NE1
  */
  HAL_GPIO_DeInit(GPIOE, GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10
                          |GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14
                          |GPIO_PIN_15);

  HAL_GPIO_DeInit(GPIOD, GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11
                          |GPIO_PIN_14|GPIO_PIN_15|GPIO_PIN_0|GPIO_PIN_1
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_7);

  /* USER CODE BEGIN FMC_MspDeInit 1 */

  /* USER CODE END FMC_MspDeInit 1 */
}

void HAL_SRAM_MspDeInit(SRAM_HandleTypeDef* hsram){
  /* USER CODE BEGIN SRAM_MspDeInit 0 */

  /* USER CODE END SRAM_MspDeInit 0 */
  HAL_FMC_MspDeInit();
  /* USER CODE BEGIN SRAM_MspDeInit 1 */

  /* USER CODE END SRAM_MspDeInit 1 */
}


