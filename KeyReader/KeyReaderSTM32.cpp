/*
*	KeyReaderSTM32.cpp, Copyright Jonathan Mackey 2025
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
#include <Arduino.h>
#include "KeyReaderSTM32.h"
#include "SdFat.h"
#include <Wire.h>
#include "AT24CDataStream.h"
#include "SerialUtils.h"
#include "ValueReader.h"

/*
*	The pins numbers for the defualt Wire, SPI and Serial objects are defined
*	in build_opt.h.
*
*	Any additional instances need to be defined globally:
*/
TwoWire Wire2((uint32_t)Config::kSDA2Pin, (uint32_t)Config::kSCL2Pin);	// Used by the camera


#if 1
XFont	xFont;
// 8-bit fonts (antialiased)
#define UI20ptFont	MyriadPro_Regular_20::font
#include "MyriadPro-Regular_20.h"
//#define UI64ptFont	Avenir_64::font
//#include "Avenir_64.h"
#endif
#include "KRXViews.h"

static const char kKRSettingsPath[] = "KRSettings.txt";

SKeySpecU32	schlageKeySpec = {"Schlage", 5, 10, 9, 2000000, 150000, 2310000, 1562000};
SKeySpecU32	kwiksetKeySpec = {"Kwikset", 5, 7, 7, 1910000, 230000, 2470000, 1500000};

/***************************** KeyReaderSTM32 *****************************/
KeyReaderSTM32::KeyReaderSTM32(void)
  : mDisplay(Config::kDispResetPin, Config::kBacklightPin),
	mPreferences(Config::kAT24CDeviceAddr, Config::kAT24CDeviceCapacity),
    mTouchScreen(Config::kTouchCSPin, Config::kTouchIRQPin,
			Config::kDisplayHeight, Config::kDisplayWidth,
			0, 0, 0, 0, Config::kInvertTouchX, Config::kInvertTouchY),
	mButtonDebouncePeriod(DEBOUNCE_DELAY), mButtonPressed(false),
	mCamera(Wire2), mPreviewWasStoppedForSleep(false),mSendDebugStrings(false)
{
}

/************************************ begin ***********************************/
void KeyReaderSTM32::begin(void)
{
	pinMode(Config::kCancelBtnPin, INPUT_PULLUP);
	pinMode(Config::kEnterBtnPin, INPUT_PULLUP);

	// Define button pin interrupt routine
	// The buttons are used to setup the touchscreen.
	//
	// Note that the doc for attachInterrupt recommends passing the pin through
	// digitalPinToInterrupt(), but all digitalPinToInterrupt does is validate
	// the passed pin using digitalPinToPinName(), and if it's not NC it
	// returns the same pin value passed.  attachInterrupt passes the pin to
	// digitalPinToPinName in the first line, so passing the pin through to 
	// digitalPinToInterrupt is pointless.
	attachInterrupt(Config::kCancelBtnPin,
		std::bind(&KeyReaderSTM32::ButtonPressedISR, this), FALLING);
	attachInterrupt(Config::kEnterBtnPin,
		std::bind(&KeyReaderSTM32::ButtonPressedISR, this), FALLING);

	/*
	*	The following pins are not used and are pulled up to prevent them from
	*	floating.
	*/
	pinMode(Config::kPA5Pin, INPUT_PULLUP);
	pinMode(Config::kPB12Pin, INPUT_PULLUP);
	pinMode(Config::kPB13Pin, INPUT_PULLUP);
	pinMode(Config::kPB15Pin, INPUT_PULLUP);
	pinMode(Config::kPC0Pin, INPUT_PULLUP);
	pinMode(Config::kPC1Pin, INPUT_PULLUP);
	pinMode(Config::kPC2Pin, INPUT_PULLUP);
	pinMode(Config::kPC3Pin, INPUT_PULLUP);
	pinMode(Config::kPC13Pin, INPUT_PULLUP);
	pinMode(Config::kPD6Pin, INPUT_PULLUP);
	pinMode(Config::kPD12Pin, INPUT_PULLUP);
	pinMode(Config::kPD13Pin, INPUT_PULLUP);

	
	/*
	*	The status LEDs
	*/
	pinMode(Config::kYellowLEDPin, OUTPUT);
	pinMode(Config::kGreenLEDPin, OUTPUT);
	pinMode(Config::kRedLEDPin, OUTPUT);
	pinMode(Config::kBlueLEDPin, OUTPUT);
	digitalWrite(Config::kYellowLEDPin, LOW);
	digitalWrite(Config::kGreenLEDPin, LOW);
	digitalWrite(Config::kRedLEDPin, LOW);
	digitalWrite(Config::kBlueLEDPin, LOW);
		
	STM32UnixRTC::RTCInit();
	{
		RCC_OscInitTypeDef RCC_OscInitStruct = {0};
		RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
		uint32_t	pFLatency;
		HAL_RCC_GetOscConfig(&RCC_OscInitStruct);
		HAL_RCC_GetClockConfig(&RCC_ClkInitStruct, &pFLatency);
		
		Serial.printf(".OscillatorType = ");
		if (RCC_OscInitStruct.OscillatorType & RCC_OSCILLATORTYPE_HSE)
		{
			Serial.printf("RCC_OSCILLATORTYPE_HSE");
		}
		if (RCC_OscInitStruct.OscillatorType & RCC_OSCILLATORTYPE_LSE)
		{
			Serial.printf("|RCC_OSCILLATORTYPE_LSE");
		}
		Serial.printf("\n");
		Serial.printf(".RCC_OscInitStruct.LSEState = %s\n", RCC_OscInitStruct.LSEState == RCC_LSE_ON ? "RCC_LSE_ON":"RCC_LSE_OFF");
	}
	pinMode(Config::kSDDetectPin, INPUT_PULLUP);
	pinMode(Config::kSDSelectPin, OUTPUT);
	digitalWrite(Config::kSDSelectPin, HIGH);	// Deselect the SD card.
	
	/*
	*	Initialization must take place in the following order:
	*	(Specific order copied from STM32CubeIDE auto generated code)
	*	DCMI, FMC, I2C, SPI
	*
	*	Initially I had I2C initializing before FMC.  When attempting to use I2C
	*	after initializing the FMC, the board rebooted.  I couldn't locate the
	*	errata sheet regarding this issue but there was chatter online.  The
	*	issue went away once I changed the init order.
	*/
	{
		using namespace std::placeholders;

		mCamera.begin(std::bind(&XKeyView::EnterPreviewMode, keyView, _1),
						std::bind(&XKeyView::ExitPreviewMode, keyView, _1),
						std::bind(&KeyReaderSTM32::KeyDataChanged, this, _1),
						std::bind(&XKeyView::UpdateStatusMessage, keyView, _1));

		mDisplay.begin(Config::kDisplayRotation);	// Init TFT
		Wire.begin();
		Wire2.begin();
		SPI.begin();
		delay(20);	// As per doc, delay 20ms after kCameraResetPin goes high
		if (mCamera.ChipIDIs5640())
		{
			Serial.printf(".Camera is OV5640\n");
		} else
		{
			Serial.printf(".Camera is not responding\n");
		}
	}
	mTouchScreen.begin(Config::kDisplayRotation);
#if 0
	/*
	*	I2C address scanning
	*/
	{
		uint8_t error, address;
		uint32_t nDevices;
		bool	errorRecorded = false;
		
		Serial.printf("Scanning Wire2...\n");
		
		nDevices = 0;
		for (address = 2; address < 127; address++)
		{
			// The i2c_scanner uses the return value of
			// the Write.endTransmisstion to see if
			// a device did acknowledge to the address.
			
			Wire2.beginTransmission(address);
			error = Wire2.endTransmission();
			
			if (error == 0)
			{
				Serial.printf("I2C device found at address 0x%02X\n", (int)address);
				
				nDevices++;
			}
			else if (!errorRecorded &&
				error == 4)
			{
				errorRecorded = true;
				Serial.printf("Unknown error at address 0x%02X\n", (int)address);
			}
		}
		if (nDevices == 0)
			Serial.printf("No I2C devices found on Wire2\n");
		else
			Serial.printf("done\n");
	}
#endif
	
#if 0
	{
		Wire.beginTransmission(Config::kAT24CDeviceAddr);
		Serial.printf(".AT24C is%s responding.\n", Wire.endTransmission() ? " not":"");
	}
#endif
	{
		Wire.beginTransmission(Config::kAT24CDeviceAddr);
		bool	at24C64IsResp = Wire.endTransmission(true) == 0;
	
		/*
		*	Load the preferences...
		*	If the EEPROM is responding...
		*/
		if (at24C64IsResp)
		{
			Config::SKRSettings	prefs;

			if (KeyReaderSTM32::ReadAllPrefs(prefs))
			{
				Serial.printf(".Preferences read\n");
				UnixTime::SetFormat24Hour(prefs.krPrefs.clockFormat == 0);
		
				/*
				*	Initialize the touchscreen alignment values if previously saved
				*/
				if (prefs.krPrefs.tsMinMax[0] != 0xFFFF)
				{
					mTouchScreen.SetMinMax(prefs.krPrefs.tsMinMax);
					Serial.printf(".prefs.tsMinMax: %hd, %hd, %hd, %hd\n",
						prefs.krPrefs.tsMinMax[0], prefs.krPrefs.tsMinMax[1],
						prefs.krPrefs.tsMinMax[2], prefs.krPrefs.tsMinMax[3]);
				}
				/*
				*	Initialize if showAdjustments is valid
				*/
				if (prefs.utilsPrefs.showAdjustments <= 1)
				{
					bool	showAdjustments = prefs.utilsPrefs.showAdjustments != 0;
					showAdjustmentsCheckbox.SetState(showAdjustments ? XControl::eOn : XControl::eOff, false);
					adjustmentsView.SetVisible(showAdjustments);
					keyView.ShowPinRootDelta(showAdjustments);
				}
				/*
				*	Initialize if centersScale is valid
				*/
				{
					if (prefs.keyViewPrefs.centersScale < 7000)
					{
						keyView.Setup(prefs.keyViewPrefs.centersScale,
										prefs.keyViewPrefs.depthsScale,
										prefs.keyViewPrefs.tolerance);
						DCMI_OV5640::SetBWThreshold(prefs.keyViewPrefs.bwThreshold);
					} else
					{
						prefs.keyViewPrefs.centersScale = keyView.GetCentersScale();
						prefs.keyViewPrefs.depthsScale = keyView.GetDepthsScale();
						prefs.keyViewPrefs.tolerance = keyView.GetTolerance();
						prefs.keyViewPrefs.bwThreshold = mCamera.GetBWThreshold();
					}
					pinCentersValueField.SetValue(prefs.keyViewPrefs.centersScale, false);
					pinDepthsValueField.SetValue(prefs.keyViewPrefs.depthsScale, false);
					pinToleranceValueField.SetValue(prefs.keyViewPrefs.tolerance, false);
					Serial.printf(".c %u, d %u, %t %u, T %hu\n",
							keyView.GetCentersScale(),
							keyView.GetDepthsScale(),
							keyView.GetTolerance(),
							mCamera.GetBWThreshold());
				}
				{
					/*
					*	Initialize if keywayMenuItemTag is valid
					*/
					if (prefs.mainViewPrefs.keywayMenuItemTag != 0xFFFF)
					{
						keywayPopUp.SelectMenuItem(prefs.mainViewPrefs.keywayMenuItemTag);
						pinCountPopUp.SelectMenuItem(prefs.mainViewPrefs.pinCountMenuItemTag);
						previewFormatPopUp.SelectMenuItem(prefs.mainViewPrefs.previewFormatMenuItemTag);
					}
					keyView.SetKeySpec(prefs.mainViewPrefs.keywayMenuItemTag == kSchlageSC1MenuItem ? &schlageKeySpec : &kwiksetKeySpec, false);
				}
			}
		} else
		{
			Serial.printf(".AT24C64 is not responding.\n");
		}
	}
	Serial.printf(".MCU Frequency = %d\n", F_CPU);
		
	/*
	*	Setup the display...
	*/
	cutBtn.Enable(false, false);	// Nothing to cut
	cancelBtn.Enable(false, false);	// Nothing to cancel

	rootView.SetSize(Config::kDisplayHeight, Config::kDisplayWidth);
	rootView.SetDisplay(&mDisplay);
	//rootView.SetModalView(&mainMenuBtn);
	rootView.SetViewChangedDelegate(this);
	warningDialog.SetViewChangedDelegate(this);
	warningDialog.SetMinDialogSize();
	xFont.SetDisplay(&mDisplay, &UI20ptFont);	// To initialize mDisplay of xFont
	
	ShowMainView();
}

/*************************** NoModalDialogDisplayed ***************************/
bool KeyReaderSTM32::NoModalDialogDisplayed(void) const
{
	return(rootView.ModalView() == nullptr);
}

/*********************************** Update ***********************************/
/*
*	Called from loop()
*/
bool KeyReaderSTM32::Update(void)
{
	if (mTouchScreen.PenStateChanged())
	{
		if (mTouchScreen.PenIsDown())
		{
			if (!mDisplaySleeping)
			{
				UnixTime::ResetSleepTime();
				uint16_t	z;
				mTouchScreen.Read(mX, mY, z);
				mHitView = rootView.HitTest(mX, mY);
				if (mHitView)
				{
					mCamera.SuspendPreview();
					mHitView->MouseDown(mX,mY);
				}
			} else
			{
				WakeUp();
			}
		} else	// Else, pen is up
		{
			if (mHitView)
			{
				mHitView->MouseUp(mX, mY);
				mHitView = nullptr;
				mCamera.ResumePreview();
			}
		}
	}
	CheckButtons();	// Buttons are used to setup the touchscreen.
	mCamera.Update();

	/*
	*	If the display isn't sleeping...
	*/
	if (!mDisplaySleeping)
	{
		bool	noModalDialogDisplayed = NoModalDialogDisplayed();
		/*
		*	If there are no modal dialogs visible THEN
		*	update the info view.
		*/
		if (noModalDialogDisplayed)
		{
			if (!mainMenu.IsVisible())
			{
				UpdateMainView();
			}
		} else if (UnixTime::TimeChanged())
		{
			UnixTime::ResetTimeChanged();
			if (utilitiesDialog.IsVisible())
			{
				dateValueField.SetValue(dateValueField.Value() +1, !warningDialog.IsVisible());
			}
		}
		
		if (UnixTime::TimeToSleep())
		{
			if (noModalDialogDisplayed)
			{
				GoToSleep();
			} else
			{
				UnixTime::ResetSleepTime();
			}
		}
	}

#if 1
	/*
	*	Serial commands
	*/
	if (Serial.available())
	{
		switch (Serial.read())
		{
			case '>':
				Serial.printf(".>\n");
				// Set the time.  A hexadecimal ASCII UNIX time follows
						// Use >65920071 for all-fields-change test (15s delay)
				UnixTime::SetUnixTimeFromSerial();
				STM32UnixRTC::SyncRTCToTime();
				break;
			case 'A':	// Start touchscreen alignment
				Serial.flush();
				ShowTouchscreenAlignment();
				break;
			case 'C':	// Cancel touchscreen alignment
				Serial.flush();
				CancelTouchscreenAlignment();
				break;
			case 'S':
				Serial.flush();
				SaveTouchscreenAlignment();
				break;
			case 'D':
			{
				/*
				*	Toggles sending debug strings (by default OFF)
				*/
				Serial.flush();
				mSendDebugStrings = !mSendDebugStrings;
				Serial.printf("Debug Strings %s\n", mSendDebugStrings ? "ON":"OFF");
				break;
			}
			case 'T':
			{
				char line[255];
				SerialUtils::LoadLine(254, line);
				DCMI_OV5640::SetBWThresholdFromStr(line);
				break;
			}
			case 'c':
			{
				bool showAdjustments = showAdjustmentsCheckbox.GetState() == XControl::eOn;
				char line[255];
				SerialUtils::LoadLine(254, line);
				uint32_t	centersScale = keyView.GetCentersScale();
				ValueReader valueReader(line);
				valueReader.ReadUInt32Number(centersScale, centersScale);
				keyView.SetCentersScale(centersScale, true);
				pinCentersValueField.SetValue(centersScale, showAdjustments);
				break;
			}
			case 'd':
			{
				bool showAdjustments = showAdjustmentsCheckbox.GetState() == XControl::eOn;
				char line[255];
				SerialUtils::LoadLine(254, line);
				uint32_t	depthsScale = keyView.GetDepthsScale();
				ValueReader valueReader(line);
				valueReader.ReadUInt32Number(depthsScale, depthsScale);
				keyView.SetDepthsScale(depthsScale, true);
				pinDepthsValueField.SetValue(depthsScale, showAdjustments);
				break;
			}
			case 't':
			{
				bool showAdjustments = showAdjustmentsCheckbox.GetState() == XControl::eOn;
				char line[255];
				SerialUtils::LoadLine(254, line);
				uint32_t	tolerance = keyView.GetTolerance();
				ValueReader valueReader(line);
				valueReader.ReadUInt32Number(tolerance, tolerance);
				keyView.SetTolerance(tolerance, true);
				pinToleranceValueField.SetValue(tolerance, showAdjustments);
				break;
			}
			case 's':
				Serial.flush();
				SaveKeyViewAdjustments();
				break;
			case 'i':
				Serial.flush();
				keyView.Dump(nullptr);
				break;
			case 'w':
			{
				/*
				*	Load the image window registers from command line.
				*
				*	See DCMI_OV5640::ConfWinFromStr()
				*/
				char line[255];
				SerialUtils::LoadLine(254, line);
				mCamera.ConfWinFromStr(line);
				break;
			}
			case 'h':
			{
				char line[255];
				SerialUtils::LoadLine(254, line);
				mCamera.SetHueFromStr(line);
				break;
			}
			default:
				Serial.flush();
				break;
		}
	}
#endif	

	return(false);
}

/*********************************** WakeUp ***********************************/
/*
*	Wakup the display from sleep and/or keep it awake if not sleeping.
*/
void KeyReaderSTM32::WakeUp(void)
{
	if (mDisplaySleeping)
	{
		mDisplaySleeping = false;
		mDisplay.WakeUp();
		rootView.Draw(0, 0, 999, 999);
		if (mPreviewWasStoppedForSleep)
		{
			mPreviewWasStoppedForSleep = false;
			mCamera.StartPreviewStream(previewFormatMenu.GetSelectedItem()->Tag() == kColorMenuItem);
		}
	}
	UnixTime::ResetSleepTime();
}

/********************************* GoToSleep **********************************/
/*
*	Puts the display to sleep.
*/
void KeyReaderSTM32::GoToSleep(void)
{
	if (!mDisplaySleeping)
	{
		mPreviewWasStoppedForSleep = mCamera.StopPreviewStream();
		mDisplay.Fill();
		mDisplay.Sleep();
		mDisplaySleeping = true;
	}
}

/******************************** CheckButtons ********************************/
/*
*	CheckButtons is used to setup the display orientation and the touch screen
*	alignment values. 
*/
void KeyReaderSTM32::CheckButtons(void)
{
	if (mButtonPressed)
	{
		uint32_t	btnPinsState = (~GPIOE->IDR) & Config::kPINBtnMask;
		if (btnPinsState)
		{
			/*
			*	If the state hasn't changed since last check
			*/
			if (mButtonPinState == btnPinsState)
			{
				/*
				*	Wakeup the display when any key is pressed.
				*/
				WakeUp();
				/*
				*	If a debounce period has passed
				*/
				if (mButtonDebouncePeriod.Passed())
				{
					UnixTime::ResetSleepTime();
					mButtonPressed = false;
					mButtonPinState = 0xFF;
					switch (btnPinsState)
					{
						case Config::kCancelBtnMask:	// Cancel button pressed
							CancelTouchscreenAlignment();
							break;
						case Config::kEnterBtnMask:	// Enter button pressed
							/*
							*	If the touchscreen alignment is visible THEN
							*	save the alignment (if valid/OK)
							*	Else, show the touchscreen alignment.
							*/
							if (touchScreenAlignment.IsVisible())
							{
								SaveTouchscreenAlignment();
							} else
							{
								ShowTouchscreenAlignment();
							}
							break;
						default:
							mButtonDebouncePeriod.Start();
							break;
					}
				}
			} else
			{
				mButtonPinState = btnPinsState;
				mButtonDebouncePeriod.Start();
			}
		}
	}

}

/************************** ShowTouchscreenAlignment **************************/
void KeyReaderSTM32::ShowTouchscreenAlignment(void)
{
	if (NoModalDialogDisplayed())
	{
		mainView.SetVisible(false);
		touchScreenAlignment.Start(&mTouchScreen);
	}
}

/************************** SaveTouchscreenAlignment **************************/
void KeyReaderSTM32::SaveTouchscreenAlignment(void)
{
	if (touchScreenAlignment.IsVisible() &&
		touchScreenAlignment.OKToSave())
	{
		Serial.printf(".TS alignment saved\n");
		// Stop alignment and save
		mainView.SetVisible(true);
		touchScreenAlignment.Stop(false);
		Config::SKRreferences	prefs;
		prefs.clockFormat = !UnixTime::Format24Hour();
		mTouchScreen.GetMinMax(prefs.tsMinMax);
		mPreferences.Write(Config::kKRPreferencesAddr,
						sizeof(Config::SKRreferences),
						(const uint8_t*)&prefs);
	}
}

/************************* CancelTouchscreenAlignment *************************/
void KeyReaderSTM32::CancelTouchscreenAlignment(void)
{
	if (touchScreenAlignment.IsVisible())
	{
		// Stop and cancel/quit (restore old alignment)
		Serial.printf(".TS alignment cancelled\n");
		mainView.SetVisible(true);
		touchScreenAlignment.Stop(true);
	}
}

/******************************* UpdateMainView *******************************/
/*
*	Called from Update() when no modal dialogs are displayed.
*/
void KeyReaderSTM32::UpdateMainView(void)
{
	if (mainView.IsVisible() &&
		!mCamera.HiResInProgress())
	{
		if (UnixTime::TimeChanged())
		{
			mCamera.SuspendPreview();
			UnixTime::ResetTimeChanged();
			infoDateValueField.SetValue(UnixTime::Time());
			mCamera.ResumePreview();
		}
	}
}

/****************************** ButtonPressedISR ******************************/
/*
*	Called via an interrupt.  The button press is handled in CheckButtons()
*	after debouncing.
*/
void KeyReaderSTM32::ButtonPressedISR(void)
{
	mButtonPressed = true;
}
	
/******************************* ValuesAreValid *******************************/
/*
*	ValuesAreValid is a member of the XValidatorDelegate mixin class.
*	It's called when a dialog's OK button is pressed.
*	True is returned if the values are valid and the dialog can be dismissed,
*	otherwise the dialog either just stays visible (no action) or the dialog
*	displays an error explaining why the dialog can't be dismissed.
*/
bool KeyReaderSTM32::ValuesAreValid(
	XDialogBox*	inDialog)
{
	bool	valuesAreValid = false;
	if (inDialog)
	{
	}
	return(valuesAreValid);
}

/****************************** HandleViewChange ******************************/
void KeyReaderSTM32::HandleViewChange(
	XView*		inView,
	uint16_t	inAction)
{
	if (inView)
	{
	#if 0
		if (mSendDebugStrings)
		{
			Serial.printf(".Change: Tag = %hd, Action = %hd\n", inView->Tag(), inAction);
		}
	#endif
		switch (inView->Tag())
		{
			case kMainMenuTag:
				switch (inAction)
				{
					case kAboutMenuItem:
						mCamera.SuspendPreview();
						aboutBox.Show();
						break;
					case kUtilitiesMenuItem:
						mCamera.SuspendPreview();
						dateValueField.SetValue(UnixTime::Time(), false);
						utilitiesDialog.Show();
						break;
				}
				break;
			case kUtilitiesDialogTag+XDialogBox::eOKTagOffset:
			{
				/*
				*	If the user just released the Close button THEN
				*	save any change to the showAdjustmentsCheckbox state.
				*
				*	When inAction is XControl::eOff, the dialog has just been
				*	hidden.
				*/ 
				if (inAction == XControl::eOn)
				{
					SaveUtilitiesDialogChanges();
					mCamera.ResumePreview();
				}
				break;
			}
			case kAboutBoxTag+XDialogBox::eOKTagOffset:
			{
				if (inAction == XControl::eOn)
				{
					mCamera.ResumePreview();
				}
				break;
			}
			case kSaveLastScanBtnTag:
			{
				if (inAction == XControl::eOff)
				{
					SaveScanDataToSD();
				}
				break;
			}
			case kKeywayPopUpTag:
			case kPreviewFormatPopUpTag:
			case kPinCountPopUpTag:
				/*
				*	If the menu just closed THEN
				*	save the changes to the 3 popup menus,
				*	then resume the preview stream.
				*/
				if (inAction  == XControl::eOff)
				{
					SaveMainViewChanges();
					mCamera.ResumePreview();
				// Else suspend preview while the menu is open.
				} else
				{
					mCamera.SuspendPreview();
				}
				break;
			case kPreviewBtnTag:
			{
				if (inAction == XControl::eOff &&
					!mCamera.PreviewIsStreaming())
				{
					cancelBtn.Enable();
					cutBtn.Enable(false, true);		// Nothing to cut
					mCamera.StopHiResStream(true);	// If running
					mCamera.StartPreviewStream(previewFormatMenu.GetSelectedItem()->Tag() == kColorMenuItem);
				}
				break;
			}
			case kScanBtnTag:
			{
				if (inAction == XControl::eOff &&
					!mCamera.HiResInProgress())
				{
					cancelBtn.Enable();
					cutBtn.Enable(false, true);		// Nothing to cut
					mCamera.StopPreviewStream();	// If streaming
					mCamera.StartHiResStream(true);
				}
				break;
			}
			case kCutBtnTag:
			{
				if (inAction == XControl::eOff)
				{
					char cutKeyCmdStr[100];
					/*
					*	If valid scan data is available THEN
					*	sent a cut command to the key code cutter.
					*/
					if (keyView.GetCutKeyCmdStr(cutKeyCmdStr))
					{
						Serial.printf("%s\n", cutKeyCmdStr);
					}
				}
				break;
			}
			case kCancelBtnTag:
			{
				if (inAction == XControl::eOff)
				{
					cancelBtn.Enable(false, true);
					mCamera.StopPreviewStream();
					mCamera.StopHiResStream(true);
				}
				break;
			}
			case kSaveAdjustmentsBtnTag:
			{
				if (inAction == XControl::eOff)
				{
					SaveKeyViewAdjustments();
				}
				break;
			}
			case kLoadSettingsBtnTag:
			{
				if (inAction == XControl::eOff)
				{
					LoadKRSettingsFromSD();
				}
				break;
			}
			case kSaveSettingsBtnTag:
			{
				if (inAction == XControl::eOff)
				{
					SaveKRSettingsToSD();
				}
				break;
			}
			case kSetTimeBtnTag:
				if (inAction == 1)
				{
					UnixTime::SetTime(dateValueField.Value());
					STM32UnixRTC::SyncRTCToTime();
					warningDialog.DoMessage(kTimeSetStr);
				}
				break;
			case kPinCentersStepperTag:
				if (inAction == 0)
				{
					keyView.SetCentersScale(pinCentersValueField.Value(), true);
					keyView.Dump(nullptr);
				}
				break;
			case kPinDepthsStepperTag:
				if (inAction == 0)
				{
					keyView.SetDepthsScale(pinDepthsValueField.Value(), true);
					keyView.Dump(nullptr);
				}
				break;
			case kPinToleranceStepperTag:
				if (inAction == 0)
				{
					keyView.SetTolerance(pinToleranceValueField.Value(), true);
					keyView.Dump(nullptr);
				}
				break;
		}
	}
}

/******************************** ShowMainView ********************************/
void KeyReaderSTM32::ShowMainView(void)
{
	if (!mainView.IsVisible())
	{
		mainView.SetVisible(true);
		rootView.Draw(0, 0, 480, 320);
	}
}

/************************* SaveUtilitiesDialogChanges *************************/
void KeyReaderSTM32::SaveUtilitiesDialogChanges(void)
{
	Config::SUtilitiesDialogPrefs	prefs;
	bool	prefsRead = mPreferences.Read(Config::kUtilitiesDialogPrefsAddr,
							sizeof(Config::SUtilitiesDialogPrefs), (uint8_t*)&prefs);
	if (prefsRead)
	{
		prefs.showAdjustments = showAdjustmentsCheckbox.GetState();
		mPreferences.Write(Config::kUtilitiesDialogPrefsAddr, sizeof(Config::SUtilitiesDialogPrefs), (uint8_t*)&prefs);
		if (prefs.showAdjustments != 0)
		{
			adjustmentsView.SetVisible(true);
		} else
		{
			adjustmentsView.Hide();
		}
		keyView.ShowPinRootDelta(prefs.showAdjustments != 0);
	}
}

/**************************** SaveMainViewChanges *****************************/
void KeyReaderSTM32::SaveMainViewChanges(void)
{
	Config::SMainViewPrefs	prefs;
	bool	prefsRead = mPreferences.Read(Config::kMainViewPrefsAddr,
							sizeof(Config::SMainViewPrefs), (uint8_t*)&prefs);
	if (prefsRead)
	{
		bool	previewFormatChanged = prefs.previewFormatMenuItemTag != previewFormatMenu.GetSelectedItem()->Tag();
		prefs.keywayMenuItemTag = keywayMenu.GetSelectedItem()->Tag();
		prefs.pinCountMenuItemTag = pinCountMenu.GetSelectedItem()->Tag();
		prefs.previewFormatMenuItemTag = previewFormatMenu.GetSelectedItem()->Tag();
		mPreferences.Write(Config::kMainViewPrefsAddr, sizeof(Config::SMainViewPrefs), (uint8_t*)&prefs);
		
		keyView.SetKeySpec(prefs.keywayMenuItemTag == kSchlageSC1MenuItem ? &schlageKeySpec : &kwiksetKeySpec, true);

		if (previewFormatChanged)
		{
			mCamera.ChangePreviewFormat(previewFormatMenu.GetSelectedItem()->Tag() == kColorMenuItem);
		}
	}
}

/*************************** SaveKeyViewAdjustments ***************************/
void KeyReaderSTM32::SaveKeyViewAdjustments(void)
{
	Config::SKeyViewPrefs	prefs;
	bool	prefsRead = mPreferences.Read(Config::kSKeyViewPrefsAddr,
							sizeof(Config::SKeyViewPrefs), (uint8_t*)&prefs);
	if (prefsRead)
	{
		prefs.centersScale = pinCentersValueField.Value();
		prefs.depthsScale = pinDepthsValueField.Value();
		prefs.tolerance = pinToleranceValueField.Value();
		prefs.bwThreshold = mCamera.GetBWThreshold();
		mPreferences.Write(Config::kSKeyViewPrefsAddr, sizeof(Config::SKeyViewPrefs), (uint8_t*)&prefs);
		warningDialog.DoMessage(kAdjustmentsSavedStr);
	}
}

/******************************* KeyDataChanged *******************************/
/*
*	This is called by DCMI_OV5640 as the registered callback for the
*	KeyDataChangedCB called from StopHiResStream().  See DCMI_OV5640::begin().
*/
void KeyReaderSTM32::KeyDataChanged(
	const uint16_t*	inKeyData)
{
	// When inKeyData is a nullptr, the camera failed to take a hi res image
	cancelBtn.Enable(false, true);
	cutBtn.Enable(inKeyData!=nullptr, true);
	keyView.SetKeyData(inKeyData, true);
	if (mSendDebugStrings)keyView.Dump(nullptr);
}

/****************************** SaveScanDataToSD ******************************/
/*
*	Saves the last scan data to SD as a header file named for the current unix
*	time as an 8 byte hex string.
*	Example: 5D960E10.h -> 03-OCT-2019 at 3:04:48 PM
*/
void KeyReaderSTM32::SaveScanDataToSD(void)
{
	if (digitalRead(Config::kSDDetectPin) == LOW)
	{
		static char	saveSuccessMsg[50];
		const uint16_t*	keyData = mCamera.GetKeyData();
		if (keyData)
		{
			SdFat sd;
			bool	success = sd.begin(Config::kSDSelectPin, SD_SCK_MHZ(4));
			if (success)
			{
				SdFile file;
				char filename[15];
				char	line[100];
				snprintf(filename, 15, "%08X.h", UnixTime::Time());
				snprintf(saveSuccessMsg, 50, kSavedScanToSDStr, filename);
				SdFile::dateTimeCallback(UnixTime::SDFatDateTimeCB);
				bool	fileOpened = file.open(filename, O_WRONLY | O_TRUNC | O_CREAT);
				if (fileOpened)
				{
					keyView.Dump(&file);
					int lineIdx = 1;
					line[0] = '\t';
					file.write(kDataFilePrefix , sizeof(kDataFilePrefix)-1);
					for (uint32_t i = 0; i < OV5640::kHRYOutputSize; )
					{
						lineIdx += snprintf(line + lineIdx, 100 - lineIdx, "% 3u, ", keyData[i]);
						i++;
						if ((i & 0xF) != 0)
						{
							continue;
						}
						line[lineIdx-1] = '\n';
						file.write(line , lineIdx);
						lineIdx = 1;
					}
					if (lineIdx > 2)
					{
						lineIdx--;
						line[lineIdx-1] = '\n';
						file.write(line , lineIdx);
					}
					file.write(kDataFileSuffix , sizeof(kDataFileSuffix)-1);
					file.close();
				}
			} else
			{
				sd.initErrorHalt();
			}
			warningDialog.DoMessage(success ? saveSuccessMsg : kSaveToSDFailedStr);
		} else
		{
			warningDialog.DoMessage(kNoScanDataAvailableStr);
			
		}
	} else
	{
		warningDialog.DoMessage(kNoSDCardFoundStr);
	}
}

/******************************** ReadAllPrefs ********************************/
bool KeyReaderSTM32::ReadAllPrefs(
	Config::SKRSettings&	outSettings)
{
	bool	prefsRead = mPreferences.Read(Config::kKRPreferencesAddr,
									sizeof(Config::SKRreferences),
										(uint8_t*)&outSettings.krPrefs) != 0 &&
						mPreferences.Read(Config::kUtilitiesDialogPrefsAddr,
									sizeof(Config::SUtilitiesDialogPrefs),
										(uint8_t*)&outSettings.utilsPrefs) != 0 &&
						mPreferences.Read(Config::kSKeyViewPrefsAddr,
									sizeof(Config::SKeyViewPrefs),
										(uint8_t*)&outSettings.keyViewPrefs) != 0 &&
						mPreferences.Read(Config::kMainViewPrefsAddr,
									sizeof(Config::SMainViewPrefs),
										(uint8_t*)&outSettings.mainViewPrefs) != 0;
	return(prefsRead);
}

/******************************** WriteAllPrefs *******************************/
bool KeyReaderSTM32::WriteAllPrefs(
	const Config::SKRSettings&	inSettings)
{
	bool	prefsWritten = mPreferences.Write(Config::kKRPreferencesAddr,
									sizeof(Config::SKRreferences),
										(const uint8_t*)&inSettings.krPrefs) != 0 &&
						mPreferences.Write(Config::kUtilitiesDialogPrefsAddr,
									sizeof(Config::SUtilitiesDialogPrefs),
										(const uint8_t*)&inSettings.utilsPrefs) != 0 &&
						mPreferences.Write(Config::kSKeyViewPrefsAddr,
									sizeof(Config::SKeyViewPrefs),
										(const uint8_t*)&inSettings.keyViewPrefs) != 0 &&
						mPreferences.Write(Config::kMainViewPrefsAddr,
									sizeof(Config::SMainViewPrefs),
										(const uint8_t*)&inSettings.mainViewPrefs) != 0;
	return(prefsWritten);
}

/***************************** SaveKRSettingsToSD *****************************/
void KeyReaderSTM32::SaveKRSettingsToSD(void)
{
	if (digitalRead(Config::kSDDetectPin) == LOW)
	{
		KRSettings	kmSettings;
		Config::SKRSettings	settings;
		if (ReadAllPrefs(settings))
		{
			SdFat sd;
			bool	success = sd.begin(Config::kSDSelectPin, SD_SCK_MHZ(4));
			if (success)
			{
				success = kmSettings.WriteFile(kKRSettingsPath, settings);
			} else
			{
				sd.initErrorHalt();
			}
			warningDialog.DoMessage(success ? kSavedKRSettingsStr : kSaveToSDFailedStr);
		} else
		{
			warningDialog.DoMessage(kFailedToReadPrefsStr);
		}
	} else
	{
		warningDialog.DoMessage(kNoSDCardFoundStr);
	}
}

/**************************** LoadKRSettingsFromSD ****************************/
void KeyReaderSTM32::LoadKRSettingsFromSD(void)
{
	if (digitalRead(Config::kSDDetectPin) == LOW)
	{
		SdFat sd;
		KRSettings	kmSettings;
		bool	success = sd.begin(Config::kSDSelectPin, SD_SCK_MHZ(4));
		if (success)
		{
			Config::SKRSettings	currentSettings;
			ReadAllPrefs(currentSettings);
			success = kmSettings.ReadFile(kKRSettingsPath, &currentSettings);
		} else
		{
			sd.initErrorHalt();
		}
		if (success)
		{
			if (WriteAllPrefs(kmSettings.Settings()))
			{
				/*
				*	Restart the board.
				*/
				HAL_NVIC_SystemReset();
			} else
			{
				warningDialog.DoMessage(kFailedToWritePrefsStr);
			}
		} else
		{
			warningDialog.DoMessage(kLoadFromSDFailedStr);
		}
	} else
	{
		warningDialog.DoMessage(kNoSDCardFoundStr);
	}
}
