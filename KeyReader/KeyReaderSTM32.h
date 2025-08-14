/*
*	KeyReaderSTM32.h, Copyright Jonathan Mackey 2025
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
#ifndef KeyReaderSTM32_h
#define KeyReaderSTM32_h

#include "Config.h"
#include "KRSettings.h"
#include "AT24C.h"
#include "DataStream.h"
#include "TFT_ILI9488P.h"
#include "XPT2046.h"
#include "DCMI_OV5640.h"
#include "XDialogBox.h"
#include "MSPeriod.h"
#include "STM32UnixRTC.h"

class TwoWire;

class KeyReaderSTM32 : public XViewChangedDelegate,
								public XValidatorDelegate
{
public:
							KeyReaderSTM32(void);
		
	virtual void			begin(void);
	virtual bool			Update(void);

	virtual void			HandleViewChange(
								XView*					inView,
								uint16_t				inAction);
	virtual bool			ValuesAreValid(
								XDialogBox*				inDialog);
protected:
	XView*			mHitView;
	TFT_ILI9488P	mDisplay;
	XPT2046			mTouchScreen;
	DCMI_OV5640		mCamera;
	AT24C			mPreferences;
	bool			mDisplaySleeping;
	bool			mButtonPressed;
	bool			mPreviewWasStoppedForSleep;
	MSPeriod		mButtonDebouncePeriod;
	bool			mSendDebugStrings;
	uint32_t		mButtonPinState;
	uint16_t		mX, mY;
	
protected:
	bool					NoModalDialogDisplayed(void) const;
	void					ShowMainView(void);
	void					UpdateMainView(void);
	void					WakeUp(void);
	void					GoToSleep(void);
	void					ButtonPressedISR(void);
	void					CheckButtons(void);
	void					ShowTouchscreenAlignment(void);
	void					SaveTouchscreenAlignment(void);
	void					CancelTouchscreenAlignment(void);
	void					SaveUtilitiesDialogChanges(void);
	void					SaveMainViewChanges(void);
	void					SaveKeyViewAdjustments(void);
	void					KeyDataChanged(
								const uint16_t*			inKeyData);
	void					SaveScanDataToSD(void);
	bool					ReadAllPrefs(
								Config::SKRSettings&	outSettings);
	bool					WriteAllPrefs(
								const Config::SKRSettings&	inSettings);
	void					SaveKRSettingsToSD(void);
	void					LoadKRSettingsFromSD(void);
};

#endif // KeyReaderSTM32_h
