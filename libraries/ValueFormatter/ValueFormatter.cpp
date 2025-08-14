/*
*	ValueFormatter.cpp, Copyright Jonathan Mackey 2023
*	Helper functions that format integer strings.
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
#include "ValueFormatter.h"
#include "BMP280Utils.h"
#include <string.h>

ValueFormatter::EPressureUnit ValueFormatter::sPressureUnit;
const char ValueFormatter::kHectopascalSuffixStr[] = " hPa";
const char ValueFormatter::kInchesOfWaterSuffixStr[] = "\"";

ValueFormatter::ETemperatureUnit ValueFormatter::sTemperatureUnit;
const char ValueFormatter::kCTempSuffixStr[] = "°C"; // Degree glyph, U00B0 UTF-8 C2 B0
const char ValueFormatter::kFTempSuffixStr[] = "°F";

/******************************** Int32ToString *******************************/
uint8_t ValueFormatter::Int32ToString(
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
	return(0);
}

/****************************** Decimal20ToString *****************************/
/*
*	inValue is of the form NNdd where dd is the decimal part of the value.
*	This formatter rounds up the decimal part leaving only the integer.
*	Ex: 3195 = 32, -3195 = -31, -3192 = -31
*/
uint8_t ValueFormatter::Decimal20ToString(
	int32_t	inValue,
	char*	outString)
{
	return(BMP280Utils::Int32ToIntStr(inValue, outString));
}

/****************************** Decimal21ToString *****************************/
/*
*	inValue is of the form NNdd where dd is the decimal part of the value.
*	This formatter rounds up the decimal part to only one place after the decimal.
*	The resulting string always has 1 place after the decimal point. 0 = 0.0
*/
uint8_t ValueFormatter::Decimal21ToString(
	int32_t	inValue,
	char*	outString)
{
	return(BMP280Utils::Int32ToDec21Str(inValue, outString));
}

/****************************** Decimal22ToString *****************************/
/*
*	inValue is of the form NNdd where dd is the decimal part of the value.
*	inValue is of the form -1234 = -12.34
*	The resulting string always has 2 places after the decimal point. 0 = 0.00
*/
uint8_t ValueFormatter::Decimal22ToString(
	int32_t	inValue,
	char*	outString)
{
	return(BMP280Utils::Int32ToDec22Str(inValue, outString));
}

/****************************** Decimal23ToString *****************************/
/*
*	inValue is of the form -1234 = -1.234
*	Returns the number of characters before the decimal point. -1.234 returns 2
*	The resulting string always has 2 places after the decimal point. 0 = 0.000
*/
uint8_t ValueFormatter::Decimal23ToString(
	int32_t	inValue,
	char*	outString)
{
	uint8_t	charsBeforeDec = 0;
	if (inValue < 0)
	{
		*(outString++) = '-';
		inValue = -inValue;
		charsBeforeDec++;
	}
	int32_t	num;
	int32_t	decNum = num = inValue/1000;
	for (; num/=10; outString++);
	char*	bufPtr = outString;
	do
	{
		*(bufPtr--) = (decNum % 10) + '0';
		decNum /= 10;
	} while (decNum);
	charsBeforeDec += (outString - bufPtr);
	outString[1] = '.';
	outString[5] = 0;
	bufPtr = &outString[4];
	decNum = inValue%1000;
	num = 3;
	do
	{
		*(bufPtr--) = (decNum % 10) + '0';
		decNum /= 10;
	} while (--num);
	return(charsBeforeDec);
}

/*************************** PressureToStringNoUnit ***************************/
/*
*	inValue is a hectopascal pressure value of the form NNdd where dd is the
*	decimal part of the value.
*	inValue is of the form -1234 = -12.34
*	The resulting string always has 2 places after the decimal point. 0 = 0.00
*	The current pressure unit suffix string is NOT appended.  The resulting
*	string will be something like: 12.34
*/
uint8_t ValueFormatter::PressureToStringNoUnit(
	int32_t	inValue,
	char*	outString)
{
	if (sPressureUnit == eInchesOfWater)
	{
		// Inches of water is a standard measurement unit for air
		// duct pressure in the USA. 1" of water = 248.84Pa or 2.4884hPa.
		inValue = (inValue * 10000)/24884;
		if (inValue < 0)
		{
			inValue = 0;
		}
	}
	return(BMP280Utils::Int32ToDec22Str(inValue, outString));
}

/****************************** PressureToString ******************************/
/*
*	inValue is a hectopascal pressure value of the form NNdd where dd is the
*	decimal part of the value.
*	inValue is of the form -1234 = -12.34
*	The resulting string always has 2 places after the decimal point. 0 = 0.00
*	The current pressure unit suffix string is appended.  The resulting string
*	will be something like: 12.34 hPa or 12.34"
*/
uint8_t ValueFormatter::PressureToString(
	int32_t	inValue,
	char*	outString)
{
	uint8_t	placesBefore = PressureToStringNoUnit(inValue, outString);
	strcpy(&outString[placesBefore+3], sPressureUnit == eInchesOfWater ?
			kInchesOfWaterSuffixStr : kHectopascalSuffixStr);
	return(placesBefore);
}

/**************************** TemperatureToString *****************************/
uint8_t ValueFormatter::TemperatureToString(
	int32_t	inValue,
	char*	outString)
{
	uint8_t placesBefore = BMP280Utils::Int32ToDec21Str(
					(sTemperatureUnit == eCelsius ? inValue : ((inValue * 9) / 5) + 3200),
						outString);
	strcpy(&outString[placesBefore+2], sTemperatureUnit == eCelsius ? 
			kCTempSuffixStr : kFTempSuffixStr);
	return(placesBefore);
}

