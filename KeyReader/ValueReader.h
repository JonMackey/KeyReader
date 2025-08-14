/*
*	ValueReader.h, Copyright Jonathan Mackey 2025
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


#ifndef ValueReader_h
#define ValueReader_h

#include <inttypes.h>

class ValueReader
{
public:
							ValueReader(
								const char*				inString);
	char					ReadXYValue(
								uint32_t				inNulValue,
								uint32_t&				outXValue,
								uint32_t&				outYValue);
	char					ReadUInt32Number(
								uint32_t				inNulValue,
								uint32_t&				outValue);
	char					ReadInt32Number(
								int32_t					inNulValue,
								int32_t&				outValue);
	bool					ReadToken(
								char					inDelimiter,
								uint32_t				inMaxTokenLen,
								char*					outToken);
	static uint32_t			FindKeyIndex(
								const char*				inKey,
								const char* const		inKeys[],
								uint32_t				inNumKeys);
	char					NextChar(void);
	char					CurrChar(void);
	char					SkipWhitespace(
								char					inCurrChar);
protected:
	const char*	mString;
};

#endif /* ValueReader_h */
