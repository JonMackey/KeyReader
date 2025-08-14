/*
*	DCMI_OV5640.cpp, Copyright Jonathan Mackey 2025
*	Class to interface DCMI to the OV5640 camera.
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
#include "DCMI_OV5640.h"
#include <Wire.h>
#include "ValueReader.h"

DCMI_HandleTypeDef hdcmi;
DMA_HandleTypeDef hdma_dcmi;

uint16_t DCMI_OV5640::s2LineBuf[OV5640::kHRXOutputSize * 2];
uint16_t DCMI_OV5640::sKeyData[OV5640::kHRYOutputSize];
const uint32_t kHiResSampleFrameIndex = 4;
const uint32_t kHiResMaxRetries = 6;
uint32_t DCMI_OV5640::sFrameIndex;
uint32_t DCMI_OV5640::sHiResRetries;
uint32_t DCMI_OV5640::sError;
uint32_t DCMI_OV5640::sErrorCount;	// Number of consecutive errors
uint32_t DCMI_OV5640::kErrorCountThreshold = 15; // Cancel scan if < sErrorCount
uint32_t DCMI_OV5640::sErrorLine;
/*
*	The sBWThreshold is overridden by SetBWThreshold on startup.
*	sBWThreshold determins the amount of luminance that is considered white in
*	the HiResLineCompleteCallback.
*
*	sBWThreshold changed be set by saving the settings to SD, modify bwThreshold,
*	then load the settings from SD.  sBWThreshold can also be changed by the
*	command T.  Example "T 180".  Then press the Set button on the main window.
*	Note that in order for the Set button to be visible the "Show adjustment
*	controls" checkbox in the utilities dialog must be checked.
*/
uint16_t DCMI_OV5640::sBWThreshold = 150;
uint16_t DCMI_OV5640::sPreviewBWThreshold = 150;
bool	DCMI_OV5640::sPreviewIsRGB = false;
bool	DCMI_OV5640::sHiResFrameCaptured = false;

// Pixel clock can be no more than 54MHz as per STM32F429 doc.
//	const pin_t		kCameraXClkPin		= PA7;
//	const pin_t		kCameraPwrDnPin		= PB0;
//	const pin_t		kCameraResetPin		= PB1;

/******************************** DCMI_OV5640 *********************************/
DCMI_OV5640::DCMI_OV5640(
	TwoWire&	inWire)
	: mWire(inWire), mDCMIInitialized(false), mHiResInProgress(false),
	  mPreviewIsStreaming(false), mKeyDataIsValid(false)
{
}

/************************************ begin ***********************************/
void DCMI_OV5640::begin(
	PreviewModeCallback	inEnterPreviewModeCB,
	PreviewModeCallback	inExitPreviewModeCB,
	DataChangedCallback	inKeyDataChangedCB,
	ScanStatusCallback	inScanStatusCB)
{
	mKeyDataChangedCB = inKeyDataChangedCB;
	mExitPreviewModeCB = inExitPreviewModeCB;
	mEnterPreviewModeCB = inEnterPreviewModeCB;
	mScanStatusCB = inScanStatusCB;

	/*
	*	Setup a 24MHz XCLK pwm timer output on kCameraXClkPin PA7
	*	The XCLK pin defined in Config.h must support PWM or this will fail.
	*/
	{
		PinName pinName = digitalPinToPinName(Config::kCameraXClkPin);
		//PinName pinName = PA_7_ALT1;
		TIM_TypeDef*	timer = (TIM_TypeDef*)pinmap_peripheral(pinName, PinMap_TIM);
		uint32_t	timerParams = pinmap_find_function(pinName, PinMap_TIM);
		if (timer != nullptr &&
			timerParams)
		{
			// See PinMap_TIM in ~/Library/Arduino15/packages/STMicroelectronics/
			// hardware/stm32/2.5.0/variants/STM32F4xx/
			// F427V(G-I)T_F429V(E-G-I)T_F437V(G-I)T_F439V(G-I)T/PeripheralPins.c
			// Another option for PA7 PWM is PA_7_ALT2 on TIM8
			uint32_t channel = STM_PIN_CHANNEL(timerParams);
			mXCLKTimer.setup(timer);
			mXCLKTimer.setPWM(channel, pinName, 24000000, 50);
		#if 0
			Serial.printf("channel = %d, pinName = 0x%X\n", channel, pinName);
			Serial.printf("prescale = %d, overflow = %d\n",
				mXCLKTimer.getPrescaleFactor(), mXCLKTimer.getOverflow());
			if (timer == TIM1)Serial.printf("Timer is TIM1\n");
		#endif
		}
	}
	
	pinMode(Config::kKRBacklightPin, OUTPUT);
	digitalWrite(Config::kKRBacklightPin, LOW);

	DMA_Init();
	DCMI_Init();

	/*
	*	Configure the Power Down and Reset pins, then reset the camera.
	*	Don't communicate with the camera within begin().  The I2C will not be
	*	initialized until after begin() is called because FMC needs to be
	*	initialized after DCMI and before I2C otherwise I2C will hang.
	*/
	pinMode(Config::kCameraPwrDnPin, OUTPUT);
	pinMode(Config::kCameraResetPin, OUTPUT);
	digitalWrite(Config::kCameraResetPin, LOW);
	digitalWrite(Config::kCameraPwrDnPin, HIGH);
	delay(10);
	digitalWrite(Config::kCameraPwrDnPin, LOW);
	delay(10);	// Give time for the camera power to stablize
	digitalWrite(Config::kCameraResetPin, HIGH);
	delay(20);	
}

/******************************* SetBWThreshold *******************************/
void DCMI_OV5640::SetBWThreshold(
	uint16_t	inThreshold)
{
	if (inThreshold >= 100 &&
		inThreshold <= 200)
	{
		sBWThreshold = inThreshold;
	}
}

/**************************** SetBWThresholdFromStr ***************************/
void DCMI_OV5640::SetBWThresholdFromStr(
	const char*	inStr)
{
	uint32_t	newBWThreshold;
	ValueReader valueReader(inStr);
	valueReader.ReadUInt32Number(sBWThreshold, newBWThreshold);
	SetBWThreshold(newBWThreshold);
	Serial.printf(".BWThreshold = %hu\n", sBWThreshold);
}


/******************************** ResetCamera ********************************/
void DCMI_OV5640::ResetCamera(void)
{
	WriteReg(0x3103, 0x11);	// Select system input clock from pad clock (D1=0)
	WriteReg(0x3008, 0x82);	// Software reset
	delay(5);
}

/*
*	When the output from the camera is YUV422, the selected format is YUYV.  In
*	the YUV422 format the U (Cb) and V (Cr) are shared between two pixels.  So
*	you need to process pixels in pairs.
*	Because the 16-bit data is little endian, the byte order for YUYV is UYVY.
*
*	For preview to the display the output is RGB565.
*/
/*
R = Y + (351*(Cr – 128)) >> 8
G = Y – (179*(Cr – 128) + 86*(Cb – 128))>>8
B = Y + (443*(Cb – 128)) >> 8
*/
/****************************** StartHiResStream ******************************/
void DCMI_OV5640::StartHiResStream(
	bool inResetRetries)
{
	if (!mHiResInProgress)
	{
		StopPreviewStream();
		/*
		*	At this point the camera has been powered up and a hardware reset has
		*	occured.
		*/
		mHiResInProgress = true;
		mKeyDataIsValid = false;
		sHiResFrameCaptured = false;
		sFrameIndex = 0;
		sError = 0;
		sErrorLine = 0;
		sErrorCount = 0;
		if (inResetRetries)
		{
			// Number of times to reset the camera if the data contains garbage.
			sHiResRetries = kHiResMaxRetries;
		}
		
		digitalWrite(Config::kKRBacklightPin, HIGH);
		ResetCamera();
		delay(50);
		WriteRegArray(OV5640::kCommonInit);
		/*
		*	Manual exposure and gain.
		*/
		{
			WriteReg(0x3503, 0x07);	// Turn off AGC & AEC (i.e. go to manual mode)
			//WriteReg(0x3500, 0x00);	// Exposure [19:16]
			WriteReg(0x3501, 0x14);	// Exposure [15:8]	0x12	0x0E
			WriteReg(0x3502, 0x00);	// Exposure [7:0]	 0x70
			//WriteReg(0x350A, 0x00);	// Gain [9:8]
			WriteReg(0x350B, 0x0A);	// Gain [7:0]	0x10
		}
		//WriteRegArray(OV5640::k1080P_Init);
		WriteRegArray(OV5640::kHiResInit);
		/*
		*	The DMA data size is Word.  In terms of STM32 DMA, that's 32 bits,
		*	so inLineBufLen is half the number of pixels in inLineBuf.
		*/
		StartDMA(&hdcmi, DCMI_MODE_CONTINUOUS, (uint32_t)s2LineBuf,
						OV5640::kHRXOutputSize, OV5640::kHRYOutputSize, false);
	}
}

/****************************** StopHiResStream *******************************/
void DCMI_OV5640::StopHiResStream(
	bool	inCancel)
{
	if (mHiResInProgress)
	{
		mHiResInProgress = false;
		
		HAL_DCMI_Stop(&hdcmi);
	#if 0
		{
			WriteReg(0x3503, 0x07);	// Turn off AGC & AEC (i.e. go to manual mode)
			WriteReg(0x3F00, 0x03);	// Capture
			uint32_t	exp19_16 = ReadReg(0x3500);
			uint32_t	exp16_8 = ReadReg(0x3501);
			uint32_t	exp7_0 = ReadReg(0x3502);
			uint32_t	gain16_8 = ReadReg(0x350A);
			uint32_t	gain7_0 = ReadReg(0x350B);
			Serial.printf(".%u 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X\n", sError, exp19_16, exp16_8, exp7_0, gain16_8, gain7_0);
		}
	#endif
		WriteReg(0x3008, 0x42);	// Software power down
	
	
		if (inCancel)
		{
			digitalWrite(Config::kKRBacklightPin, LOW);
			strcpy(mStatusMessage, "Scan Cancelled");
			mScanStatusCB(mStatusMessage);
		} else if (sError == 0)
		{
			digitalWrite(Config::kKRBacklightPin, LOW);
			mKeyDataIsValid = true;
			mKeyDataChangedCB(sKeyData);
		} else if (sHiResRetries)
		{
			sHiResRetries--;
			snprintf(mStatusMessage, 128, "Retry %u, err %u at line %u",
						kHiResMaxRetries - sHiResRetries, sError, sErrorLine);
			mScanStatusCB(mStatusMessage);
			StartHiResStream(false);
		} else
		{
			digitalWrite(Config::kKRBacklightPin, LOW);
			mKeyDataChangedCB(nullptr);
			strcpy(mStatusMessage, "Scan Failed");
			mScanStatusCB(mStatusMessage);
		}
	}
}

/********************************* GetKeyData *********************************/
const uint16_t* DCMI_OV5640::GetKeyData(void)
{
	return(mKeyDataIsValid ? sKeyData : nullptr);
}

/****************************** InitPreviewStream *****************************/
/*
*	This is code factored out of StartPreviewStream so that it can be shared
*	with both StartPreviewStream and ChangePreviewFormat.
*/
void DCMI_OV5640::InitPreviewStream(
	bool	inPreviewIsRGB)
{
	ResetCamera();
	WriteRegArray(OV5640::kCommonInit);
	sPreviewIsRGB = inPreviewIsRGB;
	if (inPreviewIsRGB)
	{
		digitalWrite(Config::kKRBacklightPin, LOW); // Key backlight OFF
		WriteReg(0x4300, 0x6F); // Set RGB565 Format
		WriteReg(0x501F, 0x01); // Format 1 = RGB
		WriteReg(0x3B00, 0x83); // Strobe/LED ON
		
		WriteReg(0x3503, 0x07);	// Turn off AGC & AEC (i.e. go to manual mode)
		//WriteReg(0x3500, 0x00);	// Exposure [19:16]
		WriteReg(0x3501, 0x60);	// Exposure [15:8]
		WriteReg(0x3502, 0x50);	// Exposure [7:0]
		//WriteReg(0x350A, 0x00);	// Gain [9:8]
		WriteReg(0x350B, 0x8B);	// Gain [7:0]	0xFB
	} else
	{
		digitalWrite(Config::kKRBacklightPin, HIGH); // Key backlight ON
		WriteReg(0x4300, 0x30); // Set YUV422 Format
		WriteReg(0x501F, 0x00); // Format 0=YUV
		//WriteReg(0x3B00, 0x03); // Strobe/LED is OFF by default

		WriteReg(0x3503, 0x07);	// Turn off AGC & AEC (i.e. go to manual mode)
		//WriteReg(0x3500, 0x00);	// Exposure [19:16]
		WriteReg(0x3501, 0x20);	// Exposure [15:8]	0x12
		WriteReg(0x3502, 0x00);	// Exposure [7:0]
		//WriteReg(0x350A, 0x00);	// Gain [9:8]
		WriteReg(0x350B, 0x10);	// Gain [7:0]
	}
	WriteRegArray(OV5640::kPreviewInit);
}

/***************************** StartPreviewStream *****************************/
void DCMI_OV5640::StartPreviewStream(
	bool	inPreviewIsRGB)
{
	/*
	*	At this point the camera has been powered up and a hardware reset has
	*	occured.
	*/
	if (!mPreviewIsStreaming)
	{
		StopHiResStream(true);
		digitalWrite(Config::kGreenLEDPin, HIGH);
		mPreviewIsStreaming = true;
		mPreviewSuspended = 0;
		sFrameIndex = 0;
		InitPreviewStream(inPreviewIsRGB);
		// Setup XKeyView for preview
		mEnterPreviewModeCB(true);
		/*
		*	s2LineBuf is the length of two lines, uint16_t for each pixel.
		*
		*	The DMA data size is Word.  In terms of STM32 DMA, a Word is 32
		*	bits, so inLineBufLen sent to StartDMA is kXOutputSize, half the
		*	number of pixels in s2LineBuf.
		*	When streaming, DMA is writing to one half of s2LineBuf while the
		*	other half is being copied to the display.
		*/
		StartDMA(&hdcmi, DCMI_MODE_CONTINUOUS, (uint32_t)s2LineBuf, OV5640::kXOutputSize, OV5640::kYOutputSize, true);
	}
}

/****************************** StopPreviewStream *****************************/
bool DCMI_OV5640::StopPreviewStream(void)
{
	bool	previewStopped = mPreviewIsStreaming;
	if (mPreviewIsStreaming)
	{
		digitalWrite(Config::kGreenLEDPin, LOW);
		mPreviewIsStreaming = false;
		mPreviewSuspended = 0;
		HAL_DCMI_Stop(&hdcmi);
		if (sPreviewIsRGB)
		{
			WriteReg(0x3B00, 0x03); // Strobe/LED OFF
		} else
		{
			digitalWrite(Config::kKRBacklightPin, LOW); // Key backlight OFF
		}
	#if 0
		{
			WriteReg(0x3503, 0x07);	// Turn off AGC & AEC (i.e. go to manual mode)
			WriteReg(0x3F00, 0x03);	// Capture
			uint32_t	exp19_16 = ReadReg(0x3500);
			uint32_t	exp16_8 = ReadReg(0x3501);
			uint32_t	exp7_0 = ReadReg(0x3502);
			uint32_t	gain16_8 = ReadReg(0x350A);
			uint32_t	gain7_0 = ReadReg(0x350B);
			Serial.printf(".0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X\n", exp19_16, exp16_8, exp7_0, gain16_8, gain7_0);
		}
	#endif
		WriteReg(0x3008, 0x42);	// Software power down
		mExitPreviewModeCB(true);
		//Serial.printf(".sFrameIndex = %u\n", sFrameIndex);
	}
	return(previewStopped);
}

/******************************* SuspendPreview *******************************/
/*
*	SuspendPreview stops preview from drawing to the display so that other
*	drawing can take place.
*	SuspendPreview and ResumePreview can be nested.
*	SuspendPreview only suspends the DCMI.  The camera does not stop streaming.
*/
void DCMI_OV5640::SuspendPreview(void)
{
	if (mPreviewIsStreaming)
	{
		if (mPreviewSuspended == 0)
		{
			digitalWrite(Config::kGreenLEDPin, LOW);
			HAL_DCMI_Suspend(&hdcmi);
			mExitPreviewModeCB(false);
		}
		mPreviewSuspended++;
	}
}

/******************************* ChangePreviewFormat *******************************/
/*
*	ChangePreviewFormat changes the preview format while streaming.
*	SuspendPreview must have been called before calling this routine.
*/
void DCMI_OV5640::ChangePreviewFormat(
	bool	inPreviewIsRGB)
{
	if (mPreviewIsStreaming)
	{
		if (sPreviewIsRGB)
		{
			WriteReg(0x3B00, 0x03); // Strobe/LED OFF
		} else
		{
			digitalWrite(Config::kKRBacklightPin, LOW); // Key backlight OFF
		}
		WriteReg(0x3008, 0x42);	// Software power down
		InitPreviewStream(inPreviewIsRGB);
	}
}

/******************************** ResumePreview *******************************/
/*
*	ResumePreview, when resuming, will start at the beginning of next frame.
*/
void DCMI_OV5640::ResumePreview(void)
{
	if (mPreviewIsStreaming &&
		mPreviewSuspended > 0)
	{
		mPreviewSuspended--;
		if (mPreviewSuspended == 0)
		{
			digitalWrite(Config::kGreenLEDPin, HIGH);
			mEnterPreviewModeCB(false);
			HAL_DCMI_Resume(&hdcmi);
		}
	}
}

/******************************** ChipIDIs5640 ********************************/
bool DCMI_OV5640::ChipIDIs5640(void) const
{
#if 0
	mWire.beginTransmission(Config::kOV5640DeviceAddr);
	Serial.printf(".Camera is%s responding.\n", mWire.endTransmission() ? " not":"");

	uint8_t	idH = ReadReg(eChipIDHReg);
	uint8_t idL = ReadReg(eChipIDLReg);
	Serial.printf(".IDH = 0x%X, IDL = 0x%X\n", (uint32_t)idH, (uint32_t)idL);
	uint8_t	idd = ReadReg(0x3100);
	Serial.printf(".idd = 0x%X\n", (uint32_t)idd);
/*	Serial.printf("0x3630 = 0x%X\n", (uint32_t)ReadReg(0x3630));
	Serial.printf("0x3631 = 0x%X\n", (uint32_t)ReadReg(0x3631));
	Serial.printf("0x3632 = 0x%X\n", (uint32_t)ReadReg(0x3632));
	Serial.printf("0x3633 = 0x%X\n", (uint32_t)ReadReg(0x3633));
	Serial.printf("0x3621 = 0x%X\n", (uint32_t)ReadReg(0x3621));
	Serial.printf("0x3704 = 0x%X\n", (uint32_t)ReadReg(0x3704));
	Serial.printf("0x3703 = 0x%X\n", (uint32_t)ReadReg(0x3703));
*/
	return(idH == eChipIDHValue &&
			idL == eChipIDLValue);
#else
	return(ReadReg(eChipIDHReg) == eChipIDHValue &&
			ReadReg(eChipIDLReg) == eChipIDLValue);
#endif
}

/******************************* SetPixelFormat *******************************/
void DCMI_OV5640::SetPixelFormat(
	OV5640::EPixelFormat	inPixelFormat)
{
	WriteReg(0x4300, OV5640::kPixelFormat[inPixelFormat]);
	WriteReg(0x501F, OV5640::kPixelFormat[inPixelFormat+1]);
}

/******************************** DumpWinConf *********************************/
/*
*	Dumps the image window registers.
*	The camera must be streaming when this routine is called.
*/
void DCMI_OV5640::DumpWinConf(void) const
{
	if (mPreviewIsStreaming ||
		mHiResInProgress)
	{
		uint32_t	xStart = (ReadReg(0x3800) << 8) + ReadReg(0x3801);
		uint32_t	yStart = (ReadReg(0x3802) << 8) + ReadReg(0x3803);
		uint32_t	xEnd = (ReadReg(0x3804) << 8) + ReadReg(0x3805);
		uint32_t	yEnd = (ReadReg(0x3806) << 8) + ReadReg(0x3807);
		uint32_t	oWidth = (ReadReg(0x3808) << 8) + ReadReg(0x3809);
		uint32_t	oHeight = (ReadReg(0x380A) << 8) + ReadReg(0x380B);	
		uint32_t	tWidth = (ReadReg(0x380C) << 8) + ReadReg(0x380D);
		uint32_t	tHeight = (ReadReg(0x380E) << 8) + ReadReg(0x380F);	
		uint32_t	xInset = (ReadReg(0x3810) << 8) + ReadReg(0x3811);
		uint32_t	yInset = (ReadReg(0x3812) << 8) + ReadReg(0x3813);
		Serial.printf("w S(%u,%u),E(%u,%u),O(%u,%u),T(%u,%u),I(%u,%u)\n",
			xStart, yStart, xEnd, yEnd, oWidth, oHeight, tWidth, tHeight, xInset, yInset);
	} else
	{
		Serial.printf("DumpWinConf only valid when the camera is active.\n");
	}
}

/******************************* ConfWinFromStr *******************************/
/*
*	Load the image window registers from serial command line.
*	See KeyReaderSTM32::Update(void).
*	The camera must be streaming when this routine is called.
*
*			S(x,y),E(x,y),O(x,y),T(x,y),I(x,y)
*
*	Where	S = xStart, yStart	[0x3800, 0x3801],[0x3802, 0x3803]
*			E = xEnd, yEnd		[0x3804, 0x3805],[0x3806, 0x3807]
*			O = oWidth, oHeight	[0x3808, 0x3809],[0x380A, 0x380B]
*			T = tWidth, tHeight	[0x380C, 0x380D],[0x380E, 0x380F]
*			I = xInset, yInset	[0x3810, 0x3811],[0x3812, 0x3813]
*
*	Example: "w S(0,3),E(2623,1947),O(320,240),T(1600,1000),I(16,6)"
*	All values are optional: "w T(2844,),O(320,240)"
*	Hexadecimal values are allowed when prefixed with "0x".
*/
void DCMI_OV5640::ConfWinFromStr(
	const char*	inStr)
{
	if (mPreviewIsStreaming)
	{
		ValueReader valueReader(inStr);
		uint32_t	xValue, yValue;
		uint16_t	regAddr;
		char a;
		
		HAL_DCMI_Stop(&hdcmi);
		WriteReg(0x3008, 0x42);	// Software power down
		/*
		*	When a value is omitted, i.e. S(,3), the omitted value is set to
		*	9999 as a flag so that no value is set.  This is handy if you only
		*	want to set one axis.
		*/
		while ((a = valueReader.ReadXYValue(9999, xValue, yValue)) != 0)
		{
			switch(a)
			{
				case 'S':
					regAddr = 0x3800;
					break;
				case 'E':
					regAddr = 0x3804;
					break;
				case 'O':
					regAddr = 0x3808;
					break;
				case 'T':
					regAddr = 0x380C;
					break;
				case 'I':
					regAddr = 0x3810;
					break;
				default:
					continue;
			}
					
			if (xValue <= 2844)
			{
				WriteReg(regAddr++, xValue>>8);
				WriteReg(regAddr++, xValue);
			} else
			{
				regAddr+=2;
			}
			if (yValue <= 1968)
			{
				WriteReg(regAddr++, yValue>>8);
				WriteReg(regAddr, yValue);
			}
		}

		mEnterPreviewModeCB(false);
		WriteReg(0x3008, 0x02);	// Software resume
		StartDMA(&hdcmi, DCMI_MODE_CONTINUOUS, (uint32_t)s2LineBuf, OV5640::kXOutputSize, OV5640::kYOutputSize, mPreviewIsStreaming);
	} else
	{
		Serial.printf("ConfWinFromStr only valid when streaming\n");
	}
}

/******************************** SetHueFromStr *******************************/
void DCMI_OV5640::SetHueFromStr(
	const char*	inStr)
{
	int32_t	hue;
	ValueReader valueReader(inStr);
	valueReader.ReadInt32Number(999, hue);
	if (hue >= -6 && hue <= 5)
	{
		if (mPreviewIsStreaming)
		{
			HAL_DCMI_Stop(&hdcmi);
			WriteReg(0x3008, 0x42);	// Software power down
			SetHue(hue);
			mEnterPreviewModeCB(false);
			WriteReg(0x3008, 0x02);	// Software resume
			StartDMA(&hdcmi, DCMI_MODE_CONTINUOUS, (uint32_t)s2LineBuf, OV5640::kXOutputSize, OV5640::kYOutputSize, mPreviewIsStreaming);
			
		} else
		{
			Serial.printf("SetHueFromStr only valid when streaming\n");
		}
	}
}

/*********************************** SetHue ***********************************/
/*
*	inHueIndex should be a value from -6 to 5.
*/
void DCMI_OV5640::SetHue(
	int32_t inHueIndex)
{
	const uint8_t kHueCtrl1[] =
					{
						0x80, 0x6F, 0x40, 0x00, 0x40, 0x6F,
						0x80, 0x6F, 0x40, 0x00, 0x40, 0x6F
					};
	const uint8_t kHueCtrl2[] =
					{
						0x00, 0x40, 0x6F, 0x80, 0x6F, 0x40,
						0x00, 0x40, 0x6F, 0x80, 0x6F, 0x40
					};
	const uint8_t kHueCtrl8[] =
					{
						0x32, 0x32, 0x32, 0x02, 0x02, 0x02,
						0x01, 0x01, 0x01, 0x31, 0x31, 0x31
					};
	//WriteReg(0x5001, 0xFF);
	WriteReg(0x5580, 0x01);	// Enable hue
	WriteReg(0x5581, kHueCtrl1[inHueIndex + 6]);
	WriteReg(0x5582, kHueCtrl2[inHueIndex + 6]);
	WriteReg(0x5588, kHueCtrl8[inHueIndex + 6]);
}

/******************************** ShowTestPattern ********************************/
void DCMI_OV5640::ShowTestPattern(
	OV5640::ETestPattern	inTestPattern)
{
	WriteReg(0x5584, OV5640::kTestPatternCmd[inTestPattern]);
	WriteReg(0x503D, OV5640::kTestPatternCmd[inTestPattern+1]);
}

/********************************** StrobeOn **********************************/
/*
*	This simplistic version of the strobe implementation simply treats the
*	strobe like a flashlight, either it's on or off.  The camera has more
*	detailed modes of strobe operation that are not used here.
*/
void DCMI_OV5640::StrobeOn(void)
{
	WriteReg(0x3B00, 0x83);	// Strobe on
}

/********************************** StrobeOff *********************************/
/*
*	Note that the strobe is automaticaly turned off when the camera software is
*	shutdown.  This routine is only needed when you want to turn off the LEDs
*	while the camera is streaming.
*/
void DCMI_OV5640::StrobeOff(void)
{
	WriteReg(0x3B00, 0x03);	// Strobe off
}

/*********************************** Update ***********************************/
void DCMI_OV5640::Update(void)
{
	if (mHiResInProgress &&
		sHiResFrameCaptured)
	{
		StopHiResStream();
	}
}

/********************************** ReadReg ***********************************/
uint8_t DCMI_OV5640::ReadReg(
	uint16_t	inAddress) const
{
	mWire.beginTransmission(Config::kOV5640DeviceAddr);
	mWire.write(inAddress >> 8);
	mWire.write(inAddress);
	mWire.endTransmission(true);
	if (mWire.requestFrom(Config::kOV5640DeviceAddr, (uint8_t)1) == 1)
	{
		return((uint8_t)mWire.read());
	}
	return(0);
}

/********************************** WriteReg **********************************/
void DCMI_OV5640::WriteReg(
	uint16_t	inAddress,
	uint8_t		inValue) const
{
	mWire.beginTransmission(Config::kOV5640DeviceAddr);
	mWire.write(inAddress >> 8);
	mWire.write(inAddress);
	mWire.write(inValue);
	mWire.endTransmission(true);
}

/******************************** WriteRegArray *******************************/
void DCMI_OV5640::WriteRegArray(
	const uint16_t*	inRegArray) const
{
	/*
	*	Note that even though the I2C supports writing an array of bytes, the
	*	camera interface requires a begin/end transaction for each register
	*	access.
	*/
	for (uint16_t regAddr = *(inRegArray++); regAddr; regAddr = *(inRegArray++))
	{
		WriteReg(regAddr, *(inRegArray++));
	}
}

/********************************** DCMI_Init *********************************/
/*
*	Code copied from STM32CubeIDE generated code, originally MX_DCMI_Init
*	Calls HAL_DCMI_Init which also initializes DMA.
*/
void DCMI_OV5640::DCMI_Init(void)
{

	/* USER CODE BEGIN DCMI_Init 0 */

	/* USER CODE END DCMI_Init 0 */

	/* USER CODE BEGIN DCMI_Init 1 */

	/* USER CODE END DCMI_Init 1 */
	hdcmi.Instance = DCMI;
	hdcmi.Init.SynchroMode = DCMI_SYNCHRO_HARDWARE;
	hdcmi.Init.PCKPolarity = DCMI_PCKPOLARITY_RISING;	// Must match camera setting 0x4740
	hdcmi.Init.VSPolarity = DCMI_VSPOLARITY_HIGH;	// Must match camera setting 0x4740
	hdcmi.Init.HSPolarity = DCMI_HSPOLARITY_HIGH;	// Must match camera setting 0x4740
	hdcmi.Init.CaptureRate = DCMI_CR_ALL_FRAME;
	hdcmi.Init.ExtendedDataMode = DCMI_EXTEND_DATA_8B;
	hdcmi.Init.JPEGMode = DCMI_JPEG_DISABLE;
	mDCMIInitialized = HAL_DCMI_Init(&hdcmi) == HAL_OK;
	/* USER CODE BEGIN DCMI_Init 2 */

	/* USER CODE END DCMI_Init 2 */
}

/********************************** DMA_Init **********************************/
/*
*	Code copied from STM32CubeIDE generated code, originally MX_DMA_Init.
*	Note that HAL_DCMI_Init called from DCMI_Init also initializes part of the
*	DMA.
*/
void DCMI_OV5640::DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);

}
#if 0
/******************************** DCMI_Deinit *********************************/
void DCMI_OV5640::DCMI_Deinit(void)
{
	if (mDCMIInitialized)
	{
		mDCMIInitialized = false;
		HAL_DCMI_DeInit(&hdcmi);
	}
}

/********************************* DMA_Deinit *********************************/
/*
*	Note that HAL_DCMI_DeInit called from DCMI_Deinit also deinitializes part of
*	the DMA.
*/
void DCMI_OV5640::DMA_Deinit(void)
{
  /* DMA interrupt deinit */
  /* DMA2_Stream1_IRQn interrupt configuration */
  HAL_NVIC_DisableIRQ(DMA2_Stream1_IRQn);

  /* DMA controller clock disable */
  __HAL_RCC_DMA2_CLK_DISABLE();
}
#endif

/********************************** StartDMA **********************************/
/*
*	Starts the DCMI DMA request and enables DCMI capture.
*
*	The inLineBuf is used as a buffer to store two complete lines. The DMA
*	controller is setup as a circular buffer, with interrupts at the halfway
*	mark and when full.  When the interrupt occurs, half of the buffer is
*	processed while the other half is being filled by the DMA controller.
*
*	Note that this is not be standard DCMI behavior.  The standard behavior is
*	to have a frame buffer the size of the image being recorded.  This is a
*	modified version of HAL_DCMI_Start_DMA found in stm32f4xx_hal_dcmi.c
*/
HAL_StatusTypeDef DCMI_OV5640::StartDMA(
	DCMI_HandleTypeDef*	inHDCMI,
	uint32_t			inDCMI_Mode,
	uint32_t			inLineBuf,
	uint32_t			inLineBufLen,
	uint32_t			inNumLines,
	bool				inIsPreview)
{
	/* Check function parameters */
	assert_param(IS_DCMI_CAPTURE_MODE(inDCMI_Mode));

	/* Lock access to inHDCMI */
	/*
	*	Warning: Note that the __HAL_LOCK macro contains a return statement that
	*	is ececuted if the handle is already locked.  If you don't know this it
	*	would appear that StartDMA always returns HAL_OK.
	*/
	__HAL_LOCK(inHDCMI);

	/* Lock the DCMI peripheral state */
	inHDCMI->State = HAL_DCMI_STATE_BUSY;

	/* Enable DCMI by setting DCMIEN bit */
	__HAL_DCMI_ENABLE(inHDCMI);

	/* Configure the DCMI Mode */
	inHDCMI->Instance->CR &= ~(DCMI_CR_CM);
	inHDCMI->Instance->CR |= inDCMI_Mode;

	/* Set the DMA memory0 conversion complete callback */
	{
		DMA_HandleTypeDef*	dmaHndl = inHDCMI->DMA_Handle;
		if (inIsPreview)
		{
			dmaHndl->XferCpltCallback =
				dmaHndl->XferHalfCpltCallback = PreviewLineCompleteCallback;
		} else
		{
			dmaHndl->XferCpltCallback =
				dmaHndl->XferHalfCpltCallback = HiResLineCompleteCallback;
		}
	}
	/* Set the DMA error callback */
	inHDCMI->DMA_Handle->XferErrorCallback = DMAErrorCallback;

	/* Set the dma abort callback */
	inHDCMI->DMA_Handle->XferAbortCallback = nullptr;

	/* Reset transfer counters value */
	inHDCMI->XferCount = 0;
	inHDCMI->XferTransferNumber = inNumLines;

	/* Enable the DMA Stream */
	// The 2nd Param is the source address
	HAL_DMA_Start_IT(inHDCMI->DMA_Handle, (uint32_t)&inHDCMI->Instance->DR, (uint32_t)inLineBuf, inLineBufLen);

	/* Enable Capture */
	inHDCMI->Instance->CR |= DCMI_CR_CAPTURE;

	/* Release Lock */
	__HAL_UNLOCK(inHDCMI);

	/* Return function status */
	return HAL_OK;
}

/************************ PreviewLineCompleteCallback *************************/
/*
*	Gets called by the DMA controller when either half of the line buffer is
*	full.  This callback copies either the RGB565 data to the display or the
*	Y of the YUV422 data interpreted as either 100% black or 100% white.
*/
void DCMI_OV5640::PreviewLineCompleteCallback(
	DMA_HandleTypeDef*	inHDMA)
{
	DCMI_HandleTypeDef* hdcmi = ( DCMI_HandleTypeDef* )((DMA_HandleTypeDef* )inHDMA)->Parent;
	
	uint16_t*	lineBufferPtr = s2LineBuf;

	/*
	*	When odd, copy from the 2nd half of the line buffer
	*/
	if (hdcmi->XferCount & 1)
	{
		lineBufferPtr += OV5640::kXOutputSize;
	}
	if (sPreviewIsRGB)
	{
		for (uint32_t i = 0; i < OV5640::kXOutputSize; i++)
		{
			*FMC_DataAddr = lineBufferPtr[i];
		}
	/*
	*	Else B&W preview
	*/
	} else
	{
		for (uint32_t i = 0; i < OV5640::kXOutputSize; i++)
		{
			*FMC_DataAddr = (lineBufferPtr[i] & 0xFF) < sPreviewBWThreshold ? 0:0xFFFF;
		}
	}
	hdcmi->XferCount++;
	/* Check if the frame is transferred */
	if (hdcmi->XferCount == hdcmi->XferTransferNumber)
	{
		sFrameIndex++;
		/* Enable the Frame interrupt */
		__HAL_DCMI_ENABLE_IT(hdcmi, DCMI_IT_FRAME);

		/* When snapshot mode, set dcmi state to ready */
		if ((hdcmi->Instance->CR & DCMI_CR_CM) == DCMI_MODE_SNAPSHOT)
		{
			hdcmi->State = HAL_DCMI_STATE_READY;
		}
		hdcmi->XferCount = 0;
	}
}

/************************** HiResLineCompleteCallback *************************/
/*
*	Gets called by the DMA controller when either half of the line buffer is
*	full.  This hi-res version  of the line complete callback scans the line
*	recording only the first transition from white to black and the following
*	transition from black to white.  The line data is YUV422, and only the
*	luminance, or Y value of YUV is used.
*/
void DCMI_OV5640::HiResLineCompleteCallback(
	DMA_HandleTypeDef*	inHDMA)
{
	DCMI_HandleTypeDef* hdcmi = ( DCMI_HandleTypeDef* )((DMA_HandleTypeDef* )inHDMA)->Parent;
	/*
	*	Not taking the first frame allows the camera time to stabilze.
	*
	*	If this is the sample frame THEN
	*	do the sample.
	*/
	if (sFrameIndex == kHiResSampleFrameIndex)
	{
		uint16_t*	lineBufferPtr = s2LineBuf;
		/*
		*	When odd, copy from the 2nd half of the line buffer
		*/
		if (hdcmi->XferCount & 1)
		{
			lineBufferPtr += OV5640::kHRXOutputSize;
		}
		/*
		*	Scan the YUV422 YUYV line, only looking at the luminance Y value to
		*	determine if the pixel is black or white.  The delta of the two B&W
		*	transitions are stored in sKeyData
		*/
		{
			uint16_t	left = 1;
			uint16_t	right = 2;
			bool isBlack, isWhite;
			for (uint16_t i = 0; i < OV5640::kHRXOutputSize; i++)
			{
				isBlack = (lineBufferPtr[i] & 0xFF) < sBWThreshold;
				if (isBlack)continue;
				for (i+=10; i < OV5640::kHRXOutputSize; i++)
				{
					isWhite = (lineBufferPtr[i] & 0xFF) >= sBWThreshold;
					if (isWhite)continue;
					left = i;
					for (i+=10; i < OV5640::kHRXOutputSize; i++)
					{
						isBlack = (lineBufferPtr[i] & 0xFF) < sBWThreshold;
						if (isBlack)continue;
						right = i;
						break;
					}
					break;
				}
				break;
			}
			sKeyData[hdcmi->XferCount] = right > left ? (right-left) : 2;
			
			/*
			*
			*	There's a known band of white before the key starts.
			*	If the lines within the middle of this band are not white THEN
			*	skip this frame and try the next frame (within reason)
			*
			*	If the image isn't stable (has noise, garbage in data), then
			*	give up and reset the camera.
			*/
		#if 1
			if (hdcmi->XferCount >= 450 &&
				hdcmi->XferCount <= 1700 &&
				right == 2)
			{
				sErrorCount++;
				sError = 2;
				sErrorLine = hdcmi->XferCount;
			} else
			{
				sErrorCount = 0;
			}
		#endif
		}
	}
	hdcmi->XferCount++;
	/* Check if the frame is transferred */
	if (hdcmi->XferCount == hdcmi->XferTransferNumber ||
		sErrorCount > kErrorCountThreshold)
	{
		if (sFrameIndex == kHiResSampleFrameIndex)
		{
			sHiResFrameCaptured = true;
		}
		
		sFrameIndex++;
		/* Enable the Frame interrupt */
		__HAL_DCMI_ENABLE_IT(hdcmi, DCMI_IT_FRAME);

		/* When snapshot mode, set dcmi state to ready */
		if ((hdcmi->Instance->CR & DCMI_CR_CM) == DCMI_MODE_SNAPSHOT)
		{
			hdcmi->State = HAL_DCMI_STATE_READY;
		}
		hdcmi->XferCount = 0;
	}
}

/****************************** DMAErrorCallback ******************************/
/**
  * @brief  DMA error callback
  * @param  inHDMA pointer to a DMA_HandleTypeDef structure that contains
  *                the configuration information for the specified DMA module.
  * @retval None
  */
void DCMI_OV5640::DMAErrorCallback(
	DMA_HandleTypeDef*	inHDMA)
{
	DCMI_HandleTypeDef* hdcmi = ( DCMI_HandleTypeDef* )((DMA_HandleTypeDef* )inHDMA)->Parent;

	/*
	*	If the error isn't a FIFO error THEN
	*	Ignore the error.
	*/
	if (hdcmi->DMA_Handle->ErrorCode != HAL_DMA_ERROR_FE)
	{
	/* Initialize the DCMI state*/
		hdcmi->State = HAL_DCMI_STATE_READY;
	}

	/* DCMI error Callback */
	//HAL_DCMI_ErrorCallback(hdcmi);

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
*	needed for this project.  Only the DCMI and TIM functions were copied from
*	the original stm32f4xx_hal_msp.c because the original STM32CubeIDE generated
*	file contains initialization routines for SPI, I2C, sysclock, RTC, etc.,
*	that are handled by Arduino core code.
*/
void HAL_DCMI_MspInit(DCMI_HandleTypeDef* hdcmi)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(hdcmi->Instance==DCMI)
  {
    /* USER CODE BEGIN DCMI_MspInit 0 */

    /* USER CODE END DCMI_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_DCMI_CLK_ENABLE();

    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**DCMI GPIO Configuration
    PE4     ------> DCMI_D4
    PE5     ------> DCMI_D6
    PE6     ------> DCMI_D7
    PA4     ------> DCMI_HSYNC
    PA6     ------> DCMI_PIXCLK
    PC6     ------> DCMI_D0
    PC7     ------> DCMI_D1
    PC8     ------> DCMI_D2
    PC9     ------> DCMI_D3
    PD3     ------> DCMI_D5
    PB7     ------> DCMI_VSYNC
    */
    GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;	// GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF13_DCMI;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;	// GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF13_DCMI;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;	// GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF13_DCMI;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;	// GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF13_DCMI;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;	// GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF13_DCMI;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

   /* DCMI DMA Init */
    /* DCMI Init */
    hdma_dcmi.Instance = DMA2_Stream1;
    hdma_dcmi.Init.Channel = DMA_CHANNEL_1;
    hdma_dcmi.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_dcmi.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_dcmi.Init.MemInc = DMA_MINC_ENABLE;
    hdma_dcmi.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma_dcmi.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    hdma_dcmi.Init.Mode = DMA_CIRCULAR;
    hdma_dcmi.Init.Priority = DMA_PRIORITY_HIGH;
    hdma_dcmi.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_dcmi) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(hdcmi,DMA_Handle,hdma_dcmi);

    /* DCMI interrupt Init */
    HAL_NVIC_SetPriority(DCMI_IRQn, 15, 0);
    HAL_NVIC_EnableIRQ(DCMI_IRQn);
    /* USER CODE BEGIN DCMI_MspInit 1 */

    /* USER CODE END DCMI_MspInit 1 */

  }

}

/****************************** DCMI_IRQHandler *******************************/
void DCMI_IRQHandler(void)
{
#if 0
	HAL_DCMI_IRQHandler(&hdcmi);
#else
	uint32_t isr_value = READ_REG(hdcmi.Instance->MISR);

	/* Synchronization error interrupt management *******************************/
	if ((isr_value & DCMI_FLAG_ERRRI) == DCMI_FLAG_ERRRI)
	{
		digitalWrite(Config::kRedLEDPin, HIGH);
		/* Clear the Synchronization error flag */
		__HAL_DCMI_CLEAR_FLAG((&hdcmi), DCMI_FLAG_ERRRI);

		/* Update error code */
		hdcmi.ErrorCode |= HAL_DCMI_ERROR_SYNC;

		/* Change DCMI state */
		hdcmi.State = HAL_DCMI_STATE_ERROR;

		/* Set the synchronization error callback */
		hdcmi.DMA_Handle->XferAbortCallback = DCMI_OV5640::DMAErrorCallback;

		/* Abort the DMA Transfer */
		HAL_DMA_Abort_IT(hdcmi.DMA_Handle);
	}
	/* Overflow interrupt management ********************************************/
	if ((isr_value & DCMI_FLAG_OVRRI) == DCMI_FLAG_OVRRI)
	{
		digitalWrite(Config::kRedLEDPin, HIGH);
		/* Clear the Overflow flag */
		__HAL_DCMI_CLEAR_FLAG((&hdcmi), DCMI_FLAG_OVRRI);

		/* Update error code */
		hdcmi.ErrorCode |= HAL_DCMI_ERROR_OVR;

		/* Change DCMI state */
		hdcmi.State = HAL_DCMI_STATE_ERROR;

		/* Set the overflow callback */
		hdcmi.DMA_Handle->XferAbortCallback = DCMI_OV5640::DMAErrorCallback;

		/* Abort the DMA Transfer */
		HAL_DMA_Abort_IT(hdcmi.DMA_Handle);
	}
	/* Line Interrupt management ************************************************/
	if ((isr_value & DCMI_FLAG_LINERI) == DCMI_FLAG_LINERI)
	{
		//digitalWrite(Config::kBlueLEDPin, HIGH);
		/* Clear the Line interrupt flag */
		__HAL_DCMI_CLEAR_FLAG((&hdcmi), DCMI_FLAG_LINERI);

		HAL_DCMI_LineEventCallback((&hdcmi));
	}
	/* VSYNC interrupt management ***********************************************/
	if ((isr_value & DCMI_FLAG_VSYNCRI) == DCMI_FLAG_VSYNCRI)
	{
		/* Clear the VSYNC flag */
		__HAL_DCMI_CLEAR_FLAG((&hdcmi), DCMI_FLAG_VSYNCRI);

		HAL_DCMI_VsyncEventCallback((&hdcmi));
	}
	/* FRAME interrupt management ***********************************************/
	if ((isr_value & DCMI_FLAG_FRAMERI) == DCMI_FLAG_FRAMERI)
	{
		/* When snapshot mode, disable Vsync, Error and Overrun interrupts */
		if ((hdcmi.Instance->CR & DCMI_CR_CM) == DCMI_MODE_SNAPSHOT)
		{
			/* Disable the Line, Vsync, Error and Overrun interrupts */
			__HAL_DCMI_DISABLE_IT((&hdcmi), DCMI_IT_LINE | DCMI_IT_VSYNC | DCMI_IT_ERR | DCMI_IT_OVR);
		}

		/* Disable the Frame interrupt */
		__HAL_DCMI_DISABLE_IT((&hdcmi), DCMI_IT_FRAME);

		HAL_DCMI_FrameEventCallback((&hdcmi));
	}
#endif
}

/************************** DMA2_Stream1_IRQHandler ***************************/
void DMA2_Stream1_IRQHandler(void)
{
	HAL_DMA_IRQHandler(&hdma_dcmi);
}

/**
  * @brief DCMI MSP De-Initialization
  * This function freeze the hardware resources used in this example
  * @param hdcmi: DCMI handle pointer
  * @retval None
  */
void HAL_DCMI_MspDeInit(DCMI_HandleTypeDef* hdcmi)
{
  if(hdcmi->Instance==DCMI)
  {
    /* USER CODE BEGIN DCMI_MspDeInit 0 */

    /* USER CODE END DCMI_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_DCMI_CLK_DISABLE();

    /**DCMI GPIO Configuration
    PE4     ------> DCMI_D4
    PE5     ------> DCMI_D6
    PE6     ------> DCMI_D7
    PA4     ------> DCMI_HSYNC
    PA6     ------> DCMI_PIXCLK
    PC6     ------> DCMI_D0
    PC7     ------> DCMI_D1
    PC8     ------> DCMI_D2
    PC9     ------> DCMI_D3
    PD3     ------> DCMI_D5
    PB7     ------> DCMI_VSYNC
    */
    HAL_GPIO_DeInit(GPIOE, GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6);

    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_4|GPIO_PIN_6);

    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9);

    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_3);

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_7);

    /* DCMI DMA DeInit */
    HAL_DMA_DeInit(hdcmi->DMA_Handle);

    /* DCMI interrupt DeInit */
    HAL_NVIC_DisableIRQ(DCMI_IRQn);
    /* USER CODE BEGIN DCMI_MspDeInit 1 */

    /* USER CODE END DCMI_MspDeInit 1 */
  }

}

#if 0
    HAL_DCMI_FrameEventCallback;
    HAL_DCMI_VsyncEventCallback;
    HAL_DCMI_LineEventCallback;
    HAL_DCMI_ErrorCallback;

void HAL_DCMI_VsyncEventCallback(DCMI_HandleTypeDef *hdcmi)
{
	/* Prevent unused argument(s) compilation warning */
	UNUSED(hdcmi);
}
#endif
void HAL_DCMI_FrameEventCallback(DCMI_HandleTypeDef *hdcmi)
{
	/* Prevent unused argument(s) compilation warning */
	UNUSED(hdcmi);
}
void HAL_DCMI_ErrorCallback(DCMI_HandleTypeDef *hdcmi)
{
	/* Prevent unused argument(s) compilation warning */
	UNUSED(hdcmi);
}

