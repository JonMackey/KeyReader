/*
*	ValueReader.cpp, Copyright Jonathan Mackey 2025
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
#include "ValueReader.h"
#include <string>
#include <string.h>

/********************************* ValueReader **********************************/
ValueReader::ValueReader(
	const char*	inString)
	: mString(inString)
{
}

/********************************** ReadValue *********************************/
/*
*	Format A(X,Y), where:
*		A is a single alpha character, A-Z, a-z
*		'(' and ')' are xy value delimiters
*		',' a comma delimits X and Y
*		X and Y are unsigned integers, either hex or decimal
*		X and Y can be left blank in which case inNulValue is returned.
*
*	Returns A, the alpha character or 0 if no single alpha character is found.
*
*	outXValue contains X OR inNulValue if no value for X was entered before the
*	comma delimiter.
*
*	outYValue contains Y OR inNulValue if no value for Y was entered before the
*	right parenthesis delimiter.
*/
char ValueReader::ReadXYValue(
	uint32_t	inNulValue,
	uint32_t&	outXValue,
	uint32_t&	outYValue)
{
	char	alphaChar = 0;
	char	thisChar = NextChar();
	if (thisChar)
	{
		thisChar = SkipWhitespace(thisChar);
		if (isalpha(thisChar))
		{
			alphaChar = thisChar;
			thisChar = NextChar();
			thisChar = SkipWhitespace(thisChar);
			if (thisChar == '(')
			{
				thisChar = ReadUInt32Number(inNulValue, outXValue);
				if (thisChar == ',')
				{
					thisChar = ReadUInt32Number(inNulValue, outYValue);
					if (thisChar == ')')
					{
						const char*	mark = mString;
						// Consume the next comma, if any
						thisChar = SkipWhitespace(NextChar());
						if (thisChar != ',')
						{
							mString = mark;
						}
					} else
					{
						alphaChar = 0;
					}
				}			
			}
		}
	}
	return(alphaChar);
}

/********************************** NextChar **********************************/
char ValueReader::NextChar(void)
{
	char	thisChar = *mString;

	if (thisChar)
	{
		mString++;
	}
	return(thisChar);
}

/********************************** CurrChar **********************************/
char ValueReader::CurrChar(void)
{
	return(*mString);
}

/******************************* SkipWhitespace *******************************/
char ValueReader::SkipWhitespace(
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

/****************************** ReadUInt32Number ******************************/
/*
*	Returns the first non-whitespace character following the number.
*	If no digits were found, inNulValue is returned in outValue.
*/
char ValueReader::ReadUInt32Number(
	uint32_t	inNulValue,
	uint32_t&	outValue)
{
	bool 		isHex = false;
	uint32_t	value = 0;
	bool		digitsConsumed = false;
	char		thisChar = SkipWhitespace(NextChar());
	if (thisChar)
	{
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
			} else
			{
				digitsConsumed = true;
			}
		}
		// Now just consume characters till a non valid numeric char is hit.
		if (isHex)
		{
			for (; isxdigit(thisChar); thisChar = NextChar())
			{
				digitsConsumed = true;
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
				digitsConsumed = true;
				value = (value * 10) + (thisChar - '0');
			}
		}
		thisChar = SkipWhitespace(thisChar);
	}

	outValue = digitsConsumed > 0 ? value : inNulValue;
	return(thisChar);
}

/****************************** ReadInt32Number *******************************/
/*
*	Returns the first non-whitespace character following the number.
*	If no digits were found, inNulValue is returned in outValue.
*/
char ValueReader::ReadInt32Number(
	int32_t		inNulValue,
	int32_t&	outValue)
{
	int32_t	value = 0;
	bool	isNeg = false;
	bool	digitsConsumed = false;
	char	thisChar = SkipWhitespace(NextChar());
	if (thisChar)
	{
		if (thisChar == '-')
		{
			isNeg = true;
			thisChar = NextChar();
		}
		for (; isdigit(thisChar); thisChar = NextChar())
		{
			digitsConsumed = true;
			value = (value * 10) + (thisChar - '0');
		}
		if (isNeg)
		{
			value = -value;
		}
	}
	thisChar = SkipWhitespace(thisChar);

	outValue = digitsConsumed > 0 ? value : inNulValue;
	return(thisChar);
}

/********************************** ReadToken *********************************/
/*
*	Tokens must begin with a letter.  Tokens can contain letters and digits. 
*	Leading whitespace is ignored.	The delimiter must follow the end of the
*	token.  Whitespace between the token and the delimiter is allowed.
*	The token will be stored in outToken.
*	Returns true if a valid token was found. The string pointer mString points
*	to the character after the delimiter.
*/
bool ValueReader::ReadToken(
	char		inDelimiter,
	uint32_t	inMaxTokenLen,
	char*		outToken)
{
	bool	success = false;
	const char*	savedMString = mString;	// In case no token is found or error
	char	thisChar = SkipWhitespace(NextChar());
	if (isalpha(thisChar) &&
		inMaxTokenLen > 1)
	{
		const char*	tokenStartPtr = mString -1;
		for (; thisChar != 0; thisChar = NextChar())
		{
			if (isalnum(thisChar))
			{
				continue;
			}
			{
				uint32_t	strLen = (uint32_t)(mString - tokenStartPtr) -1;
				thisChar = SkipWhitespace(thisChar);
				success = thisChar == inDelimiter &&
							strLen < inMaxTokenLen;
				if (success)
				{
					memcpy(outToken, tokenStartPtr, strLen);
					outToken[strLen] = 0;
				}
			}
			break;
		}
	}
	if (!success)
	{
		mString = savedMString;
	}
	return(success);
}

/******************************** FindKeyIndex ********************************/
/*
*	Returns the index of inKey within the array inKeys + 1.
*	Returns 0 if inKey is not found.
*/
uint32_t ValueReader::FindKeyIndex(
	const char*			inKey,
	const char* const	inKeys[],
	uint32_t			inNumKeys)
{
	uint32_t leftIndex = 0;
	uint32_t rightIndex = inNumKeys -1;
	while (leftIndex <= rightIndex)
	{
		uint32_t current = (leftIndex + rightIndex) / 2;
		const char* currentPtr = inKeys[current];
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

