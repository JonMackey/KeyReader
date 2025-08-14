/*
*	KRSettings.cpp, Copyright Jonathan Mackey 2025
*
*	This is a minimal parser for key=value files of the same form as
*	boards.txt and platform.txt accept this class interprets the values as
*	either strings or numbers with support for notted values and hex values.
*	Only the select set of keys defined below are read.  Any other keys are
*	ignored.
*
*	To keep the code size small, very little error checking performed.
*	It's assumed that the config file was created by KRSettings::WriteFile
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
#include "KRSettings.h"
#ifndef __MACH__
#include <Arduino.h>
#include "SdFat.h"
#include "sdios.h"
#else
#include <string>
#define _BV(bit) (1 << (bit))
#endif
#include "UnixTime.h"

/*
*	List of keys
*/
const char kBWThresholdStr[]		= "bwThreshold";
const char kCentersScaleStr[]		= "centersScale";
const char kDepthsScaleStr[]		= "depthsScale";
const char kHourFormat24Str[]		= "hourFormat";
const char kKeywayStr[]				= "keyway";
const char kPinCountStr[]			= "pinCount";
const char kPinToleranceStr[]		= "pinTolerance";
const char kPreviewFormatStr[]		= "previewFormat";
const char kShowAdjustmentsStr[]	= "showAdjustments";
const char kTsXMaxStr[]				= "tsXMax";
const char kTsXMinStr[]				= "tsXMin";
const char kTsYMaxStr[]				= "tsYMax";
const char kTsYMinStr[]				= "tsYMin";

const char* const kSettingsKeys[] =
{	// Sorted alphabetically
	kBWThresholdStr,
	kCentersScaleStr,
	kDepthsScaleStr,
	kHourFormat24Str,
	kKeywayStr,
	kPinCountStr,
	kPinToleranceStr,
	kPreviewFormatStr,
	kShowAdjustmentsStr,
	kTsXMaxStr,
	kTsXMinStr,
	kTsYMaxStr,
	kTsYMinStr
};

enum EKeyIndexes
{
	eInvalidKeyIndex,
	// Must align with kSettingsKeys
	eBWThreshold,
	eCentersScale,
	eDepthsScale,
	eHourFormat24,
	eKeyway,
	ePinCount,
	ePinTolerance,
	ePreviewFormat,
	eShowAdjustments,
	eTsXMax,
	eTsXMin,
	eTsYMax,
	eTsYMin,
	eKeyCount
};

static const uint32_t kNumericKeysMask =
						_BV(eBWThreshold) |
						_BV(eCentersScale) |
						_BV(eDepthsScale) |
						_BV(eHourFormat24) |
						_BV(eKeyway) |
						_BV(ePinCount) |
						_BV(ePinTolerance) |
						_BV(ePreviewFormat) |
						_BV(eShowAdjustments) |
						_BV(eTsXMax) |
						_BV(eTsXMin) |
						_BV(eTsYMax) |
						_BV(eTsYMin);

/********************************* KRSettings **********************************/
KRSettings::KRSettings(void)
: mFile(nullptr)
{
}

/********************************** ReadFile **********************************/
/*
*	It's assumed SdFat.begin was successfully called prior to calling this
*	routine.
*
*	Pass inSettings to optionally initalize mSettings so that only the keys
*	in the file are updated/changed.
*
*	This routine returns true if the file was opened.
*/
bool KRSettings::ReadFile(
	const char*					inPath,
	const Config::SKRSettings*	inSettings)
{
	mSettings = {0};
#ifndef __MACH__
	SdFile file;
	bool	fileOpened = file.open(inPath, O_RDONLY);
	mFile = &file;
#else
	mFile = fopen(inPath, "r+");
	bool	fileOpened = mFile != nullptr;
#endif
	if (inSettings)
	{
		memcpy(&mSettings, inSettings, sizeof(mSettings));
	}
	if (fileOpened)
	{
		char	thisChar = NextChar();
		while (thisChar)
		{
			thisChar = SkipWhitespaceAndHashComments(thisChar);
			if (thisChar)
			{
				char keyStr[32];
				keyStr[0] = thisChar;
				thisChar = ReadStr('=', 31, &keyStr[1]);
				while (thisChar)
				{
					uint8_t keyIndex = FindKeyIndex(keyStr);
					if (keyIndex)
					{
						if (kNumericKeysMask & (_BV(keyIndex)))
						{
							uint32_t	value;
							thisChar = ReadUInt32Number(value);
							switch (keyIndex)
							{
								case eBWThreshold:
									mSettings.keyViewPrefs.bwThreshold = value;
									break;
								case eCentersScale:
									mSettings.keyViewPrefs.centersScale = value;
									break;
								case eDepthsScale:
									mSettings.keyViewPrefs.depthsScale = value;
									break;
								case eHourFormat24:
									mSettings.krPrefs.clockFormat = value;
									break;
								case eKeyway:
									mSettings.mainViewPrefs.keywayMenuItemTag = value;
									break;
								case ePinCount:
									mSettings.mainViewPrefs.pinCountMenuItemTag = value;
									break;
								case ePinTolerance:
									mSettings.keyViewPrefs.tolerance = value;
									break;
								case ePreviewFormat:
									mSettings.mainViewPrefs.previewFormatMenuItemTag = value;
									break;
								case eShowAdjustments:
									mSettings.utilsPrefs.showAdjustments = value;
									break;
								case eTsXMax:
									mSettings.krPrefs.tsMinMax[1] = value;
									break;
								case eTsXMin:
									mSettings.krPrefs.tsMinMax[0] = value;
									break;
								case eTsYMax:
									mSettings.krPrefs.tsMinMax[3] = value;
									break;
								case eTsYMin:
									mSettings.krPrefs.tsMinMax[2] = value;
									break;
							}
						}/* else
						{
							char valueStr[10];
							thisChar = ReadStr('\n', sizeof(valueStr), valueStr);
							bool	isTrue = valueStr[0] == 't';
							switch (keyIndex)
							{
								case eShowAdjustments:
									mSettings.showAdjustments = isTrue;
									break;
							}
						}*/
					} else
					{
						thisChar = SkipToNextLine();
					}
					break;
				}
			}
		}
	#ifndef __MACH__
		mFile->close();
	#else
		fclose(mFile);
	#endif
		mFile = nullptr;
	}
	return(fileOpened);
}

/********************************** WriteFile *********************************/
bool KRSettings::WriteFile(
	const char*					inPath,
	const Config::SKRSettings&	inSettings)
{
#ifndef __MACH__
	SdFile file;
	SdFile::dateTimeCallback(UnixTime::SDFatDateTimeCB);
	bool	fileOpened = file.open(inPath, O_WRONLY | O_TRUNC | O_CREAT);
	mFile = &file;
#else
	mFile = fopen(inPath, "w");
	bool	fileOpened = mFile != nullptr;
#endif
	if (fileOpened)
	{

		for (uint8_t keyIndex = 1; keyIndex < eKeyCount; keyIndex++)
		{

			WriteStr(kSettingsKeys[keyIndex-1]);
			WriteChar('=');
			if (kNumericKeysMask & (_BV(keyIndex)))
			{
				int32_t	value = 0;
				switch (keyIndex)
				{
					case eBWThreshold:
						value = inSettings.keyViewPrefs.bwThreshold;
						break;
					case eCentersScale:
						value = inSettings.keyViewPrefs.centersScale;
						break;
					case eDepthsScale:
						value = inSettings.keyViewPrefs.depthsScale;
						break;
					case eHourFormat24:
						value = inSettings.krPrefs.clockFormat;
						break;
					case eKeyway:
						value = inSettings.mainViewPrefs.keywayMenuItemTag;
						break;
					case ePinCount:
						value = inSettings.mainViewPrefs.pinCountMenuItemTag;
						break;
					case ePinTolerance:
						value = inSettings.keyViewPrefs.tolerance;
						break;
					case ePreviewFormat:
						value = inSettings.mainViewPrefs.previewFormatMenuItemTag;
						break;
					case eShowAdjustments:
						value = mSettings.utilsPrefs.showAdjustments;
						break;
					case eTsXMax:
						value = inSettings.krPrefs.tsMinMax[1];
						break;
					case eTsXMin:
						value = inSettings.krPrefs.tsMinMax[0];
						break;
					case eTsYMax:
						value = inSettings.krPrefs.tsMinMax[3];
						break;
					case eTsYMin:
						value = inSettings.krPrefs.tsMinMax[2];
						break;
				}
				char valueStr[15];
				Int32ToString(value, valueStr);
				WriteStr(valueStr);
			}/* else
			{
				bool	isTrue = false;;
				switch (keyIndex)
				{
					case eShowAdjustments:
						isTrue = mSettings.showAdjustments;
						break;
				}
				WriteStr(isTrue ? "true":"false");
			}*/
			WriteChar('\n');
		}
	#ifndef __MACH__
		mFile->close();
	#else
		fclose(mFile);
	#endif
	}
	return(fileOpened);
}

/********************************** NextChar **********************************/
char KRSettings::NextChar(void)
{
	char	thisChar;
#ifdef __MACH__
	thisChar = getc(mFile);
	if (thisChar == -1)
	{
		thisChar = 0;
	}
#else
	if (mFile->read(&thisChar,1) != 1)
	{
		thisChar = 0;
	}
#endif
	return(thisChar);
}

/******************************** FindKeyIndex ********************************/
/*
*	Returns the index of inKey within the array kSettingsKeys + 1.
*	Returns 0 if inKey is not found.
*/
uint8_t KRSettings::FindKeyIndex(
	const char*	inKey)
{
	uint8_t leftIndex = 0;
	uint8_t rightIndex = (sizeof(kSettingsKeys)/sizeof(char*)) -1;
	while (leftIndex <= rightIndex)
	{
		uint8_t current = (leftIndex + rightIndex) / 2;
		const char* currentPtr = kSettingsKeys[current];
		int cmpResult = strcmp(inKey, currentPtr);
		if (cmpResult == 0)
		{
			return(current+1);	// Add 1, 0 is reserved for "not found"
		} else if (cmpResult <= 0)
		{
			rightIndex = current - 1;
		} else
		{
			leftIndex = current + 1;
		}
	}
	return(0);
}

/******************************* SkipWhitespace *******************************/
char KRSettings::SkipWhitespace(
	char	inCurrChar)
{
	char	thisChar = inCurrChar;
	for (; thisChar != 0; thisChar = NextChar())
	{
		if (isspace(thisChar))
		{
			continue;
		}
		break;
	}
	return(thisChar);
}

/*********************** SkipWhitespaceAndHashComments ************************/
char KRSettings::SkipWhitespaceAndHashComments(
	char	inCurrChar)
{
	char	thisChar = inCurrChar;
	while (thisChar)
	{
		if (isspace(thisChar))
		{
			thisChar = NextChar();
			continue;
		} else if (thisChar == '#')
		{
			thisChar = SkipToNextLine();
			continue;
		}
		break;
	}
	return(thisChar);
}

/******************************* SkipToNextLine *******************************/
/*
*	Returns the character following the newline (if any).
*	Does not support Windows CRLF line endings.
*/
char KRSettings::SkipToNextLine(void)
{
	char thisChar = NextChar();
	for (; thisChar; thisChar = NextChar())
	{
		if (thisChar != '\n')
		{
			continue;
		}
		thisChar = NextChar();
		break;
	}
	return(thisChar);
}

/******************************** Int32ToString *******************************/
void KRSettings::Int32ToString(
	int32_t	inValue,
	char*	outString)
{
	if (inValue < 0)
	{
		*(outString++) = '-';
		inValue = -inValue;
	}
	for (int32_t num = inValue; num/=10; outString++);
	outString[1] = 0;
	do
	{
		*(outString--) = (inValue % 10) + '0';
		inValue /= 10;
	} while (inValue);
}

/****************************** ReadUInt32Number ******************************/
char KRSettings::ReadUInt32Number(
	uint32_t&	outValue)
{
	bool		bitwiseNot = false;
	bool 		isHex = false;
	uint32_t	value = 0;
	char		thisChar = SkipWhitespaceAndHashComments(NextChar());
	if (thisChar)
	{
		bitwiseNot = thisChar == '~';
		/*
		*	If notted THEN
		*	get the next char after the not.
		*/
		if (bitwiseNot)
		{
			thisChar = SkipWhitespaceAndHashComments(NextChar());
		}
		if (thisChar == '0')
		{
			thisChar = NextChar();	// Get the character following the leading zero.
			isHex = thisChar == 'x';
			/*
			*	If this is a hex prefix THEN
			*	get the character following it.
			*/
			if (isHex)
			{
				thisChar = NextChar();
			}
		}
		// Now just consume characters till a non valid numeric char is hit.
		if (isHex)
		{
			for (; isxdigit(thisChar); thisChar = NextChar())
			{
				thisChar -= '0';
				if (thisChar > 9)
				{
					thisChar -= 7;
					if (thisChar > 15)
					{
						thisChar -= 32;
					}
				}
				value = (value << 4) + thisChar;
			}
		} else
		{
			for (; isdigit(thisChar); thisChar = NextChar())
			{
				value = (value * 10) + (thisChar - '0');
			}
		}
		thisChar = SkipWhitespaceAndHashComments(thisChar);
	}
	if (bitwiseNot)
	{
		value = ~value;
	}
	outValue = value;
	return(thisChar);
}

/********************************** ReadStr ***********************************/
/*
*	Reads the string till the delimiter is hit.
*	Once the string is full (inMaxStrLen), characters are ignored till the
*	delimiter is hit.
*	Hash (#) comments are not allowed because the strings aren't quoted.
*/
char KRSettings::ReadStr(
	char	inDelimiter,
	uint8_t	inMaxStrLen,
	char*	outStr)
{
	char		thisChar;
	char*		endOfStrPtr = &outStr[inMaxStrLen -1];

	while((thisChar = NextChar()) != 0)
	{
		if (thisChar != inDelimiter)
		{
			/*
			*	If outStr isn't full THEN
			*	append thisChar to it.
			*/
			if (outStr < endOfStrPtr)
			{
				*(outStr++) = thisChar;
			} // else discard the character, the outStr is full
			continue;
		}
		break;
	}
	*outStr = 0;	// Terminate the string
	return(thisChar);
}

/********************************** WriteStr **********************************/
void KRSettings::WriteStr(
	const char*	inStr)
{
#ifdef __MACH__
	fwrite(inStr, strlen(inStr), 1, mFile);
#else
	mFile->write(inStr ,strlen(inStr));
#endif
}

/********************************** WriteChar *********************************/
void KRSettings::WriteChar(
	char	inChar)
{
#ifdef __MACH__
	putc(inChar, mFile);
#else
	mFile->write(&inChar ,1);
#endif
}

