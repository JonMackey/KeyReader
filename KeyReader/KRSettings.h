/*
*	KRSettings.h, Copyright Jonathan Mackey 2025
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


#ifndef KRSettings_h
#define KRSettings_h

#include <inttypes.h>

#ifdef __MACH__
#include <stdio.h>
#define SdFile	FILE
#else
class SdFile;
#endif

namespace Config
{
	struct SKRreferences
	{
		uint8_t		unused[2];
		uint8_t		clockFormat;	// The time format (0=24, 1=12)
		uint8_t		unused2;
		uint16_t	tsMinMax[4];	// Touch screen alignment values
	}; // __attribute__ ((packed)); (was getting compiler warnings)
	struct SUtilitiesDialogPrefs
	{
		// Utilities dialog preferences
		uint16_t	showAdjustments;	// Show adjustment controls in mainView.
	};
	struct SKeyViewPrefs
	{
		// XKeyView class preferences
		uint32_t	centersScale;	// Scale lengthwise along key
		uint32_t	depthsScale;	// Scale for cross section line widths
		uint32_t	tolerance;		// ± Tolerance for pixel to pin lookup
				// Actually a DCMI_OV5640 pref, bwThreshold determines whether
		uint16_t	bwThreshold; 	// a pixel's luminance is black or white.
	};
	struct SMainViewPrefs
	{
		uint16_t	keywayMenuItemTag;
		uint16_t	pinCountMenuItemTag;
		uint16_t	previewFormatMenuItemTag;
	};
	
	struct SKRSettings
	{
		SKRreferences			krPrefs;
		SUtilitiesDialogPrefs	utilsPrefs;
		SKeyViewPrefs			keyViewPrefs;
		SMainViewPrefs			mainViewPrefs;
	};
}

class KRSettings
{
public:
							KRSettings(void);
	bool					ReadFile(
								const char*				inPath,
								const Config::SKRSettings* inSettings = nullptr);
	bool					WriteFile(
								const char*				inPath,
								const Config::SKRSettings& inSettings);
	const Config::SKRSettings& Settings(void) const
								{return(mSettings);}
protected:
	SdFile*		mFile;
	Config::SKRSettings	mSettings;
	
	char					NextChar(void);
	uint8_t					FindKeyIndex(
								const char*				inKey);
	char					SkipWhitespace(
								char					inCurrChar);
	char					SkipWhitespaceAndHashComments(
								char					inCurrChar);
	char					SkipToNextLine(void);
	void					Int32ToString(
								int32_t					inValue,
								char*					outString);
	char					ReadUInt32Number(
								uint32_t&				outValue);
	char					ReadStr(
								char					inDelimiter,
								uint8_t					inMaxStrLen,
								char*					outStr);
	void					WriteStr(
								const char*				inStr);
	void					WriteChar(
								char					inChar);
};

#endif /* KRSettings_h */
