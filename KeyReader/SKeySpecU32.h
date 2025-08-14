/*
*	SKeySpecU32.h, Copyright Jonathan Mackey 2025
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
#ifndef SKeySpecU32_h
#define SKeySpecU32_h

#include <inttypes.h>

struct SKeySpecU32
{
	char		name[20];
	uint32_t	numPins;
	uint32_t	numPinDepths;
	uint32_t	deepestCutIndex;
	// The next 4 values are in inches * 10,000,000
	uint32_t	deepestCut;
	uint32_t	pinDepthInc;
	uint32_t	firstPinCenter;
	uint32_t	pinSpacing;
	
	int32_t					CutIndexInc(void) const
								{return(deepestCutIndex > 1 ? -1 : 1);}
};
#endif // SKeySpecU32_h
