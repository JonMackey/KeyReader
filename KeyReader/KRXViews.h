/*
*	AAXViews.h, Copyright Jonathan Mackey 2025
*	UI and constant strings.
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
#ifndef AAXViews_h
#define AAXViews_h

#include "ST77XXToXPT2046Alignment.h"
#include "ValueFormatter.h"
#include "XAlert.h"
#include "XAnimatedFontIcon.h"
#include "XCheckboxButton.h"
#include "XColoredView.h"
#include "XDateValueField.h"
#include "XDialogBox.h"
#include "XMenuButton.h"
#include "XMenuItem.h"
#include "XNumberValueField.h"
#include "XPopUpButton.h"
#include "XPushButton.h"
#include "XRadioButton.h"
#include "XRootView.h"
#include "XStepper.h"
#include "XKeyView.h"

static const char kVerticalEllipsisStr[] = ".";
static const char kOKStr[] = "OK";
static const char kCancelStr[] = "Cancel";
static const char kCloseStr[] = "Close";

// Utilities dialog
static const char kUtilitiesStr[] = "Utilities";
static const char kSaveSettingsStr[] = "Settings to/from SD:";
static const char kSaveStr[] = "Save";
static const char kLoadStr[] = "Load";
static const char kSetStr[] = "Set";
static const char kSaveToSDFailedStr[] = "Save to SD failed.";
static const char kLoadFromSDFailedStr[] = "Load from SD failed.";
static const char kNoSDCardFoundStr[] = "No SD card found.";
static const char kSavedKRSettingsStr[] = "Saved to file KRSettings.txt";
static const char kLoadKRSettingsStr[] =	"Press OK to load file\n"
											"KRSettings.txt.  The board\n"
											"will restart if successful.";
static const char kAdjustmentsSavedStr[] = "Adjustments saved.";
static const char kTimeSetStr[] = "Time set.";
											
static const char kFailedToReadPrefsStr[] = "Failed to read preferences from EEPROM";
static const char kFailedToWritePrefsStr[] = "Failed to write preferences to EEPROM";
static const char kSaveLastScanToSDStr[] = "Save last scan to SD:";
static const char kNoScanDataAvailableStr[] = "No scan data available.";
static const char kDataFilePrefix[] = "const uint16_t kTestKey[] =\n{\n";
static const char kDataFileSuffix[] = "};\n";
static const char kSavedScanToSDStr[] = "Saved scan data to %s";
static const char kShowAdjustmentCtrlsStr[] = "Show adjustment controls";

static const char kUnableToReadPrefsStr[] = "Unable to read preferences";

// About Box
static const char kSoftwareNameStr[] = "Key Reader";
static const char kVersionStr[] = "STM32 version 1.0";
static const char kCopyrightStr[] = "Copyright Jonathan Mackey 2025";

// Info view
static const char kKeywayStr[] = "Keyway:";
static const char kPreviewFormatStr[] = "Preview:";
static const char kPinToleranceStr[] = "Pin Tolerance:";
static const char kPreviewStr[] = "Preview";
static const char kScanStr[] = "Scan";
static const char kCutStr[] = "Cut";
static const char kAdjPinCentersStr[] = "Pin Adjust  Centers:";
static const char kAdjPinDepthsStr[] = "Depths:";


// Touch screen alignment instructions.
static const char	kAlignmentInstMsg[] =
									"Precisely touch the tiny white dot\n"
									"within each circle. A magenta dot\n"
									"will reflect the alignment accuracy.\n"
									"The red circle at the top of the display\n"
									"will change to green when enough points\n"
									"have been entered to save. When satisfied\n"
									"with the alignment, press Enter to save.\n"
									"Press Cancel to quit alignment at any time.";


static const uint16_t	kMainViewTag = 300;
static const uint16_t	kInfoDateValueFieldTag = 301;
static const uint16_t	kFsValueFieldTag = 302;
static const uint16_t	kLeftIncrButtonTag = 303;
static const uint16_t	kKeyViewTag = 304;
static const uint16_t	kKeywayLabelTag = 305;
static const uint16_t	kKeywayPopUpTag = 306;
static const uint16_t	kKeywayMenuTag = 307;
static const uint16_t	kPinCountPopUpTag = 308;
static const uint16_t	kPinCountMenuTag = 309;
static const uint16_t	kPreviewFormatMenuTag = 310;
static const uint16_t	kPreviewFormatLabelTag = 311;
static const uint16_t	kPreviewFormatPopUpTag = 312;
static const uint16_t	kAdjustmentsViewTag = 313;
static const uint16_t	kPinToleranceLabelTag = 3130;
static const uint16_t	kPinToleranceValueFieldTag = 3131;
static const uint16_t	kPinToleranceStepperTag = 3132;
static const uint16_t	kPinCentersLabelTag = 3133;
static const uint16_t	kPinCentersValueFieldTag = 3134;
static const uint16_t	kPinCentersStepperTag = 3135;
static const uint16_t	kPinDepthsLabelTag = 3136;
static const uint16_t	kPinDepthsValueFieldTag = 3137;
static const uint16_t	kPinDepthsStepperTag = 3138;
static const uint16_t	kSaveAdjustmentsBtnTag = 3139;
static const uint16_t	kPreviewBtnTag = 314;
static const uint16_t	kScanBtnTag = 315;
static const uint16_t	kCutBtnTag = 316;
static const uint16_t	kCancelBtnTag = 317;

static const uint16_t	kUtilitiesDialogTag = 400;
static const uint16_t	kLoadSettingsBtnTag = 401;
static const uint16_t	kSaveSettingsLabelTag = 402;
static const uint16_t	kSaveSettingsBtnTag = 403;
static const uint16_t	kSaveLastScanBtnTag = 404;
static const uint16_t	kSaveLastScanLabelTag = 405;
static const uint16_t	kDateValueFieldTag = 406;
static const uint16_t	kDateValueStepperTag = 407;
static const uint16_t	kSetTimeBtnTag = 408;
static const uint16_t	kShowAdjustmentsCheckboxTag = 409;


static const uint16_t	kMainMenuBtnTag = 900;
static const uint16_t	kMainMenuTag = 901;

static const uint16_t	kAlertDialogTag = 600;
static const uint16_t	kWarningDialogTag = 700;
static const uint16_t	kTouchScreenAlignmentTag = 1000;

static const uint16_t	kAboutBoxTag = 1000;
static const uint16_t	kSoftwareNameLabelTag = 1001;
static const uint16_t	kVersionLabelTag = 1002;
static const uint16_t	kCopyrightLabelTag = 1003;

static const uint16_t	kSpace = 10;
static const uint16_t	kLabelYAdj = 5;
static const uint16_t	kRowHeight = 36;
static const uint16_t	kDialogBGColor = 0xF77D;

/*
*	The touch screen alignment view is displayed when the Enter button on the
*	board pressed.  This button is only attached/needed when alignment of the
*	touch screen is performed.  The result of the alignment is written to the
*	EEPROM.
*/
ST77XXToXPT2046Alignment touchScreenAlignment(kTouchScreenAlignmentTag,
				&UI20ptFont, nullptr,
				kAlignmentInstMsg);
				
// The alertDialog is displayed within the filter settings dialog so it has
// the same FG and BG color.
XAlert	alertDialog(kAlertDialogTag, &touchScreenAlignment,
				kOKStr, kCancelStr, nullptr,
				&UI20ptFont,
				nullptr, kDialogBGColor, kDialogBGColor);

// The warning dialog is displayed on a black background
XAlert	warningDialog(kWarningDialogTag, &alertDialog,
				kOKStr, nullptr, nullptr,
				&UI20ptFont,
				nullptr, kDialogBGColor, kDialogBGColor);


// Utilities dialog
XLabel		saveLastScanLabel(0, kLabelYAdj, 172, 26,
				kSaveLastScanLabelTag, nullptr, kSaveLastScanToSDStr,
				&UI20ptFont, nullptr,
				XFont::eBlack, kDialogBGColor, XFont::eAlignRight);
XPushButton saveLastScanBtn(264+kSpace, 0, 80, 0,
				kSaveLastScanBtnTag, &saveLastScanLabel, kSaveStr,
				&UI20ptFont,
				XFont::eWhite, kDialogBGColor);
XLabel		saveSettingsLabel(0, kLabelYAdj + kRowHeight, 172, 26,
				kSaveSettingsLabelTag, &saveLastScanBtn, kSaveSettingsStr,
				&UI20ptFont, nullptr,
				XFont::eBlack, kDialogBGColor, XFont::eAlignRight);
XPushButton saveSettingsBtn(184, kRowHeight, 80, 0,
				kSaveSettingsBtnTag, &saveSettingsLabel, kSaveStr,
				&UI20ptFont,
				XFont::eWhite, kDialogBGColor);
XPushButton loadSettingsBtn(264+kSpace, kRowHeight, 80, 0,
				kLoadSettingsBtnTag, &saveSettingsBtn, kLoadStr,
				&UI20ptFont,
				XFont::eWhite, kDialogBGColor);
XDateValueField	dateValueField(0, kRowHeight*2, 0,
				kDateValueFieldTag, &loadSettingsBtn,
				&UI20ptFont,
				XFont::eBlack, kDialogBGColor);
XStepper	dateValueStepper(247, kRowHeight*2, 0, 0,
				kDateValueStepperTag, &dateValueField);
XPushButton setTimeBtn(264+kSpace, kRowHeight*2, 80, 0,
				kSetTimeBtnTag, &dateValueStepper, kSetStr,
				&UI20ptFont,
				XFont::eWhite, kDialogBGColor);
XCheckboxButton showAdjustmentsCheckbox(0, kRowHeight*3, 221+36, 0,
				kShowAdjustmentsCheckboxTag, &setTimeBtn, kShowAdjustmentCtrlsStr,
				&UI20ptFont,
				XCheckboxButton::eLargeCheckSize,
				XFont::eBlack, kDialogBGColor);

XDialogBox	utilitiesDialog(&showAdjustmentsCheckbox,
				kUtilitiesDialogTag, &warningDialog,
				kCloseStr, nullptr, kUtilitiesStr,
				&UI20ptFont,
				nullptr, kDialogBGColor);

// About box
XLabel		softwareNameLabel(0, 0, 280, 26,
				kSoftwareNameLabelTag, nullptr, kSoftwareNameStr,
				&UI20ptFont, nullptr,
				XFont::eBlack, kDialogBGColor, XFont::eAlignCenter);
XLabel		versionLabel(0, kRowHeight, 280, 26,
				kVersionLabelTag, &softwareNameLabel, kVersionStr,
				&UI20ptFont, nullptr,
				XFont::eBlack, kDialogBGColor, XFont::eAlignCenter);
XLabel		copyrightLabel(0, kRowHeight*2, 280, 26,
				kCopyrightLabelTag, &versionLabel, kCopyrightStr,
				&UI20ptFont, nullptr,
				XFont::eBlack, kDialogBGColor, XFont::eAlignCenter);
XDialogBox	aboutBox(&copyrightLabel,
				kAboutBoxTag, &utilitiesDialog,
				kCloseStr, nullptr, nullptr,
				&UI20ptFont,
				nullptr, kDialogBGColor);
				
// Info view
//	Pin count menu
static const char k6PinStr[] = "6 Pin";
static const char k5PinStr[] = "5 Pin";
static const char k4PinStr[] = "4 Pin";

static const uint16_t	k6PinMenuItem = 6;
static const uint16_t	k5PinMenuItem = 5;
static const uint16_t	k4PinMenuItem = 4;
XMenuItem	fourPinMenuItem(k4PinMenuItem, k4PinStr);
XMenuItem	fivePinMenuItem(k5PinMenuItem, k5PinStr, &fourPinMenuItem);
XMenuItem	sixPinMenuItem(k6PinMenuItem, k6PinStr, &fivePinMenuItem);
XMenu		pinCountMenu(kPinCountMenuTag,
				&UI20ptFont, &sixPinMenuItem, nullptr, kDialogBGColor);

//	Keyway menu
static const char kSchlageSC1Str[] = "Schlage SC1";
static const char kKwiksetKW1Str[] = "Kwikset KW1";

static const uint16_t	kSchlageSC1MenuItem = 2;
static const uint16_t	kKwiksetKW1MenuItem = 1;

XMenuItem	kwikseteMenuItem(kKwiksetKW1MenuItem, kKwiksetKW1Str);
XMenuItem	schlageMenuItem(kSchlageSC1MenuItem, kSchlageSC1Str, &kwikseteMenuItem);
XMenu		keywayMenu(kKeywayMenuTag,
				&UI20ptFont, &schlageMenuItem);

//	Preview Format menu
static const char kBlackAndWhiteStr[] = "B&W";
static const char kColorStr[] = "Color";

static const uint16_t	kBlackAndWhiteItem = 2;
static const uint16_t	kColorMenuItem = 1;

XMenuItem	blackAndWhiteItem(kBlackAndWhiteItem, kBlackAndWhiteStr);
XMenuItem	colorMenuItem(kColorMenuItem, kColorStr, &blackAndWhiteItem);
XMenu		previewFormatMenu(kPreviewFormatMenuTag,
				&UI20ptFont, &colorMenuItem);

//	Main menu
static const char kAboutStr[] = "About";

static const uint16_t	kAboutMenuItem = 1;
static const uint16_t	kUtilitiesMenuItem = 2;

XMenuItem	utilitiesMenuItem(kUtilitiesMenuItem, kUtilitiesStr);
XMenuItem	aboutMenuItem(kAboutMenuItem, kAboutStr, &utilitiesMenuItem);
XMenu		mainMenu(kMainMenuTag, &UI20ptFont, &aboutMenuItem);
XMenuButton mainMenuBtn(480-32, 2, 27, 0,
				kMainMenuBtnTag, &mainMenu, nullptr,
				kVerticalEllipsisStr, &UI20ptFont);

XDateValueField	infoDateValueField(136,3,0,
				kInfoDateValueFieldTag, &mainMenuBtn,
				&UI20ptFont);
XKeyView		keyView(20, 40, 440, 128, kKeyViewTag, &infoDateValueField, &UI20ptFont);
XLabel		keywayLabel(20, 182+kLabelYAdj, 75, 26,
				kKeywayLabelTag, &keyView, kKeywayStr,
				&UI20ptFont, nullptr,
				XFont::eWhite, XFont::eBlack, XFont::eAlignRight);
XPopUpButton keywayPopUp(20+75+kSpace, 182, 145, 0,
				kKeywayPopUpTag, &keywayMenu, &keywayLabel,
				&UI20ptFont,
				XPopUpButton::eLargePopUpSize,
				XFont::eWhite, XFont::eBlack);
XPopUpButton pinCountPopUp(20+75+kSpace+145+kSpace, 182, 75, 0,
				kPinCountPopUpTag, &pinCountMenu, &keywayPopUp,
				&UI20ptFont,
				XPopUpButton::eLargePopUpSize,
				XFont::eWhite, XFont::eBlack);
	
	// Subviews of adjustmentsView
	XLabel		pinToleranceLabel(75+kSpace+145+kSpace,
								kLabelYAdj, 119, 26,
					kPinToleranceLabelTag, nullptr, kPinToleranceStr,
					&UI20ptFont, nullptr,
					XFont::eWhite, XFont::eBlack, XFont::eAlignRight);
	XNumberValueField pinToleranceValueField(75+kSpace+145+kSpace+119,
								 0, 30,
					kPinToleranceValueFieldTag, &pinToleranceLabel,
					&UI20ptFont, 5, 15, 1, 1, false, false,
					nullptr, XFont::eWhite, XFont::eBlack, XFont::eAlignLeft);
	XStepper	pinToleranceStepper(75+kSpace+145+kSpace+119+30+kSpace,
								 0, 0, 0,
					kPinToleranceStepperTag, &pinToleranceValueField);				
	XLabel		pinCentersLabel(0,
								kLabelYAdj+kRowHeight, 162, 26,
					kPinCentersLabelTag, &pinToleranceStepper, kAdjPinCentersStr,
					&UI20ptFont, nullptr,
					XFont::eWhite, XFont::eBlack, XFont::eAlignRight);
	XNumberValueField pinCentersValueField(162,
								 kRowHeight, 40,
					kPinCentersValueFieldTag, &pinCentersLabel,
					&UI20ptFont, 6405, 7000, 6000, 5, false, false,
					nullptr, XFont::eWhite, XFont::eBlack, XFont::eAlignLeft);
	XStepper	pinCentersStepper(162+40+kSpace,
								 kRowHeight, 0, 0,
					kPinCentersStepperTag, &pinCentersValueField);				
	XLabel		pinDepthsLabel(75+kSpace+145+kSpace,
								kLabelYAdj+kRowHeight, 64, 26,
					kPinDepthsLabelTag, &pinCentersStepper, kAdjPinDepthsStr,
					&UI20ptFont, nullptr,
					XFont::eWhite, XFont::eBlack, XFont::eAlignRight);
	XNumberValueField pinDepthsValueField(75+kSpace+145+kSpace+64,
								 kRowHeight, 40,
					kPinDepthsValueFieldTag, &pinDepthsLabel,
					&UI20ptFont, 6633, 7000, 6000, 5, false, false,
					nullptr, XFont::eWhite, XFont::eBlack, XFont::eAlignLeft);
	XStepper	pinDepthsStepper(75+kSpace+145+kSpace+64+40+kSpace,
								 kRowHeight, 0, 0,
					kPinDepthsStepperTag, &pinDepthsValueField);				
	XPushButton saveAdjustmentsBtn(75+kSpace+145+kSpace+64+40+kSpace+25, kRowHeight, 60, 0,
					kSaveAdjustmentsBtnTag, &pinDepthsStepper, kSaveStr,
					&UI20ptFont);


XView		adjustmentsView(20, 182+kRowHeight, 440, kRowHeight*2,
				kAdjustmentsViewTag, &pinCountPopUp, &saveAdjustmentsBtn,
				nullptr, false);
					
XLabel		previewFormatLabel(20, 182+kLabelYAdj+kRowHeight, 75, 26,
				kPreviewFormatLabelTag, &adjustmentsView, kPreviewFormatStr,
				&UI20ptFont, nullptr,
				XFont::eWhite, XFont::eBlack, XFont::eAlignRight);
XPopUpButton previewFormatPopUp(20+75+kSpace, 182+kRowHeight, 145, 0,
				kPreviewFormatPopUpTag, &previewFormatMenu, &previewFormatLabel,
				&UI20ptFont,
				XPopUpButton::eLargePopUpSize,
				XFont::eWhite, XFont::eBlack);
XPushButton previewBtn(20, 294, 100, 0,
				kPreviewBtnTag, &previewFormatPopUp, kPreviewStr,
				&UI20ptFont);
XPushButton scanBtn(133, 294, 100, 0,
				kScanBtnTag, &previewBtn, kScanStr,
				&UI20ptFont);
XPushButton cutBtn(247, 294, 100, 0,
				kCutBtnTag, &scanBtn, kCutStr,
				&UI20ptFont);
XPushButton cancelBtn(480-20-100, 294, 100, 0,
				kCancelBtnTag, &cutBtn, kCancelStr,
				&UI20ptFont);

XColoredView mainView(0, 0, 480, 320,
				kMainViewTag, &aboutBox, &cancelBtn,
				nullptr, false);

XRootView	rootView(&mainView);

#endif // AAXViews_h
