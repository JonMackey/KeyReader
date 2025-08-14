/*
*	XKeyView.cpp, Copyright Jonathan Mackey 2025
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

#include "XRootView.h"
#include "XKeyView.h"
#include "XFont.h"
#include <stdio.h>

#ifdef __MACH__
namespace OV5640
{
	const uint32_t	kHRXOutputSize	= 1000;
	const uint32_t	kHRYOutputSize	= 1918;
	
};
#include "DisplayController.h"
#else
#include "OV5640.h"
#include "TFT_ILI9488P.h"
#include "SdFat.h"
#endif

static const char kFlatNotFoundStr[] = "Flat not found";

/*********************************** XKeyView ***********************************/
XKeyView::XKeyView(
	int16_t			inX,
	int16_t			inY,
	uint16_t		inWidth,
	uint16_t		inHeight,
	uint16_t		inTag,
	XView*			inNextView,
	XFont::Font*	inFont)
	: XView(inX, inY, inWidth, inHeight, inTag, inNextView, nullptr),
	  mPinCentersValid(false), mTolerance(5), mKeySpec(nullptr),
	  mFont(inFont), mCentersScale(6405), mDepthsScale(6633), mKeyData(nullptr),
	  mInPreviewMode(false), mStatusMessage(nullptr), mShowPinRootDelta(false)
{
	/*
	*	The mCentersScale is the vertical scale, and mDepthsScale is the
	*	horizontal relative to the scanned data.  The two scales turned out to
	*	not be the same at high resolution.
	*
	*	The mCentersScale is determined visually via the UI.  The mCentersScale
	*	was changed until the pin centers were visually drawn in the center of
	*	each cut.
	*
	*	The width of a Schlage key blank is between 0.3445 and 0.345 inches.
	*	Multiplied by 10M is 3445000 and 3450000.  The width of my sample
	*	Schlage key blank in pixels is 520.  This also had to be tweaked to
	*	improve the accuracy of the mDepthsScale.
	*
	*	I assume that whenever the camera or the camera focus is changed, these
	*	values may need to be adjusted.
	*/

}

/****************************** MakeFontCurrent *******************************/
XFont* XKeyView::MakeFontCurrent(void)
{
	XFont* xFont = nullptr;
	if (mFont)
	{
		xFont = mFont->MakeCurrent();
	}
	return(xFont);
}

const uint32_t	kDisplayScale = 3;
const uint32_t	kDisplayDataOffset = 375;
const int16_t	kInset = 2;
/********************************** DrawSelf **********************************/
void XKeyView::DrawSelf(void)
{
	DisplayController*	display = XRootView::GetInstance()->GetDisplay();
	if (display &&
		 mFont)
	{
		XFont*	xFont = MakeFontCurrent();
		display->FillRect(mX, mY, mWidth, mHeight, XFont::eWhite);
		xFont->SetTextColor(XFont::eBlack);
		xFont->SetBGTextColor(XFont::eWhite);
	#if 1
		if (mKeyData &&
			!mInPreviewMode)
		{
			uint32_t	k = kDisplayDataOffset;
			int16_t		x = mX+kInset;
			int16_t		y = mY+kInset;
			uint16_t	insetH = mHeight - (kInset*2);
			uint16_t	insetW = mWidth - (kInset*2);
			for (uint32_t i = 0; i < insetW; i++)
			{
				uint32_t	lineHeight = (mKeyData[k]/kDisplayScale) - 75;
				if (lineHeight < insetH)
				{
					display->FillRect(insetW-i+x, y-lineHeight+insetH, 1, lineHeight, XFont::eBlack);
				}
				k += (kDisplayScale);
			}
			if (mPinCentersValid)
			{
				char pinStr[12];
				for (uint32_t k = 0; k < mKeySpec->numPins; k++)
				{
					uint32_t	pinCenter = (mPinCenter[k]-kDisplayDataOffset)/kDisplayScale;
					display->FillRect(insetW-pinCenter+x, y+20, 1, insetH-20, XFont::eRed);
					display->MoveToRow(y+5);
					uint16_t	txLeft = insetW-pinCenter+x-25;
					pinStr[1] = 0;
					if (mPinRootIndex[k] < 10)
					{
						pinStr[0] = mPinRootIndex[k] + '0';
						/*
						*	For debugging...
						*	If showing the pin error deltas
						*/
						if (mShowPinRootDelta)
						{
							snprintf(pinStr + 1, 11, "  %d", mPinRootDelta[k]);
						}
					} else
					{
						pinStr[0] = 'C';
					}
					xFont->DrawCentered(pinStr, txLeft, txLeft+50);
				}
			} else if (mStatusMessage && mStatusMessage[0])
			{
				display->MoveTo(mY+kInset+5, mX+kInset);
				xFont->DrawStr(mStatusMessage);
			}
		#if 0
			{
				uint32_t	position = (1750-kDisplayDataOffset)/kDisplayScale;
				display->FillRect(insetW-position+x, y, 1, insetH, XFont::eGreen);
			}
		#endif
		/*
		*	If there is a status message AND
		*	the camera isn't streaming THEN
		*	draw the message.
		*/
		} else if (mStatusMessage && mStatusMessage[0] &&
			!mInPreviewMode)
		{
			display->MoveTo(mY+kInset+10, mX+kInset);
			xFont->DrawStr(mStatusMessage);
		}
	#endif
	}
}

/********************************* SetKeyData *********************************/
void XKeyView::SetKeyData(
	const uint16_t*		inKeyData,
	bool				inUpdate)
{
	mKeyData = inKeyData;
	UpdatePinDepths();
	UpdatePinCenters();
	if (inUpdate)
	{
		DrawSelf();
/*
	DisplayController*	display = XRootView::GetInstance()->GetDisplay();
	if (display &&
		 mFont)
				display->MoveTo(y+10, insetW-pinCenter+x-5);
			xFont->SetTextColor(XFont::eBlack);
			xFont->SetBGTextColor(XFont::eWhite);
				xFont->DrawStr(pinStr);
*/
	}
}

/********************************* SetKeySpec *********************************/
void XKeyView::SetKeySpec(
	const SKeySpecU32*	inKeySpec,
	bool				inUpdate)
{
	if (mKeySpec != inKeySpec)
	{
		mKeySpec = inKeySpec;
		if (inUpdate && !mInPreviewMode)
		{
			UpdatePinDepths();
			UpdatePinCenters();
			DrawSelf();
		}
	}
}

/**************************** UpdateStatusMessage *****************************/
void XKeyView::UpdateStatusMessage(
	const char*	inStatusMessage)
{
	mStatusMessage = inStatusMessage;
	DrawSelf();
}

/****************************** EnterPreviewMode ******************************/
void XKeyView::EnterPreviewMode(
	bool	inEraseView)
{
	mStatusMessage = nullptr;
#ifndef __MACH__
	TFT_ILI9488P*	display = (TFT_ILI9488P*)XRootView::GetInstance()->GetDisplay();
	if (display)
	{
		if (inEraseView)
		{
			display->FillRect(mX, mY, mWidth, mHeight, XFont::eWhite);
		}
		display->TemporaryWindow(false, true, false,
									mX+100, mX+100+OV5640::kYOutputSize,
									mY+2, mY+2+OV5640::kXOutputSize);
		mInPreviewMode = true;
	}
#endif
}

/****************************** ExitPreviewMode *******************************/
void XKeyView::ExitPreviewMode(
	bool	inEraseView)
{
	mStatusMessage = nullptr;
#ifndef __MACH__
	TFT_ILI9488P*	display = (TFT_ILI9488P*)XRootView::GetInstance()->GetDisplay();
	if (display)
	{
		display->RestoreRotation();
		mInPreviewMode = false;
		if (inEraseView)
		{
			display->FillRect(mX, mY, mWidth, mHeight, XFont::eWhite);
		}
	}
#endif
}

#ifdef __MACH__
/************************************ Dump ************************************/
void XKeyView::Dump(void)
{
	int32_t		cutIndex = mKeySpec->deepestCutIndex;
	int32_t		cutIndexInc = mKeySpec->CutIndexInc();
	if (mKeyData &&
		mKeySpec)
	{
		fprintf(stderr, "%s Pin Depths:\n", mKeySpec->name);
		for (uint32_t i = 0; i < mKeySpec->numPinDepths; i++)
		{
			fprintf(stderr, "\t[%u] %u\n", cutIndex, mRootDepth[i]);
			cutIndex += cutIndexInc;
		}
	
		fprintf(stderr, "Centers Scale = %u, Depths Scale = %u, Tolerance = %u\n",
								mCentersScale, mDepthsScale, mTolerance);
		for (uint32_t i = 0; i < mKeySpec->numPins; i++)
		{
			fprintf(stderr, "[%u] % 4d\t%u\t%hhu %d\n", i, mPinCenter[i],  mPinDepth[i], mPinRootIndex[i], mPinRootDelta[i]);
		}
	}
}
#else
/************************************ Dump ************************************/
void XKeyView::Dump(
	SdFile*	inFile)
{
	int32_t		cutIndex = mKeySpec->deepestCutIndex;
	int32_t		cutIndexInc = mKeySpec->CutIndexInc();
	if (mKeyData &&
		mKeySpec)
	{
		char	buff[1000];
		int buffIdx = snprintf(buff, 1000, "/*\n*\t%s Pin Depths:\n", mKeySpec->name);
		if (inFile)
		{
			inFile->write(buff , buffIdx);
		} else
		{
			Serial.printf(buff);
		}
		buffIdx = 0;
		for (uint32_t i = 0; i < mKeySpec->numPinDepths; i++)
		{
			buffIdx += snprintf(buff + buffIdx, 1000 - buffIdx, "*\t\t\t\t[%u] %u\n", cutIndex, mRootDepth[i]);
			cutIndex += cutIndexInc;
		}
		if (inFile)
		{
			inFile->write(buff , buffIdx);
		} else
		{
			Serial.printf(buff);
		}
		buffIdx = snprintf(buff, 1000, "*\n*\tCenters Scale = %u, Depths Scale = %u, Tolerance = %u\n",
								mCentersScale, mDepthsScale, mTolerance);
		for (uint32_t i = 0; i < mKeySpec->numPins; i++)
		{
			buffIdx += snprintf(buff + buffIdx, 1000 - buffIdx, "*\t[%u] % 4d\t%u\t%u %d\n",
								i, mPinCenter[i], mPinDepth[i],
								(uint32_t)mPinRootIndex[i], mPinRootDelta[i]);
		}
		buffIdx += snprintf(buff + buffIdx, 1000 - buffIdx, "*/\n\n\n");
		if (inFile)
		{
			inFile->write(buff , buffIdx);
		} else
		{
			Serial.printf(buff);
		}
	} else if (!inFile)
	{
		Serial.printf(".No data available.\n");
	}
}
#endif

/****************************** GetCutKeyCmdStr *******************************/
/*
*	If there is valid key data, outCutKeyCmdStr will contain the following keys:
*		name = Name of the key spec
*		pins = Number of pins
*		code = 4, 5, or 6 digit uint32 representing the key depths, bow to tip.
*		custom = Appearing only when the scanned key does not conform to
*				 standard pin depths.  Zero placeholders will be inserted for
*				 any pins that do match a pin depth.
*	Custom example: the 2nd and 4th pins are custom, all others match a key spec
*		pin depth:
			C {name=kwikset, pins=5, code=40103, custom={0, 550, 0, 627}}
			
*		The non-zero numbers are in decimal 22 format, where the last two
*		digits are 100th of a mm.  e.g. 550 is 5.50mm
*
*	The size of outCutKeyCmdStr must be at least 100 bytes.
*/
bool XKeyView::GetCutKeyCmdStr(
	char*	outCutKeyCmdStr)
{
	int32_t		cutIndex = mKeySpec->deepestCutIndex;
	int32_t		cutIndexInc = mKeySpec->CutIndexInc();
	bool	success = mKeyData != nullptr &&
						mKeySpec != nullptr &&
							outCutKeyCmdStr != nullptr;
	if (success)
	{
		uint32_t	highestCustomIndex = 0;
		uint32_t	keyCode = 0;
		for (uint32_t i = 0; i < mKeySpec->numPins;)
		{
			uint32_t	thisRootIndex = mPinRootIndex[i++];
			if (thisRootIndex == 99)
			{
				thisRootIndex = 0;
				highestCustomIndex = i;
			}
			keyCode = (keyCode * 10) + thisRootIndex;
		}
		int buffIdx = snprintf(outCutKeyCmdStr, 100, "C {name=%s, pins=%u, code=%05u",
							mKeySpec->name, mKeySpec->numPins, keyCode);
		if (highestCustomIndex)
		{
			buffIdx += snprintf(outCutKeyCmdStr + buffIdx, 100 - buffIdx, ", custom={");
			for (uint32_t i = 0; i < highestCustomIndex; i++)
			{
				uint32_t	customValue;
				if (mPinRootIndex[i] == 99)
				{
					customValue = (mCustomPin[i] * 254) / 1000000;
				} else
				{
					customValue = 0;
				}
				buffIdx += snprintf(outCutKeyCmdStr + buffIdx, 100 - buffIdx, i ? ", %u" : "%u", customValue);
			}
			buffIdx += snprintf(outCutKeyCmdStr + buffIdx, 100 - buffIdx, "}");
		}
		snprintf(outCutKeyCmdStr + buffIdx, 100 - buffIdx, "}");
	} else
	{
		outCutKeyCmdStr[0] = 0;
	}
	return(success);
}

/************************************ Setup ***********************************/
void XKeyView::Setup(
	uint32_t	inCentersScale,
	uint32_t	inDepthsScale,
	uint32_t	inTolerance)
{
	mCentersScale = inCentersScale;
	mDepthsScale = inDepthsScale;
	mTolerance = inTolerance;
}

/******************************** SetTolerance ********************************/
void XKeyView::SetTolerance(
	uint32_t	inTolerance,
	bool	inUpdate)
{
	mTolerance = inTolerance;
	UpdatePinRootIndexes();
	if (inUpdate)
	{
		DrawSelf();
	}
}

/****************************** SetCentersScale *******************************/
void XKeyView::SetCentersScale(
	uint32_t	inScale,
	bool	inUpdate)
{
	mCentersScale = inScale;
	UpdatePinCenters();
	if (inUpdate)
	{
		DrawSelf();
	}
}

/******************************* SetDepthsScale *******************************/
void XKeyView::SetDepthsScale(
	uint32_t	inScale,
	bool	inUpdate)
{
	mDepthsScale = inScale;
	UpdatePinDepths();
	UpdatePinRootIndexes();
	if (inUpdate)
	{
		DrawSelf();
	}
}

/****************************** UpdatePinDepths *******************************/
void XKeyView::UpdatePinDepths(void)
{		
	uint32_t	scaledDepth = mKeySpec->deepestCut;
	uint32_t	scaledPinDepthInc = mKeySpec->pinDepthInc;
	for (uint32_t i = 0; i < mKeySpec->numPinDepths; i++)
	{
		mRootDepth[i] = scaledDepth/mDepthsScale;
		scaledDepth += scaledPinDepthInc;
	}
}

/****************************** UpdatePinCenters ******************************/
void XKeyView::UpdatePinCenters(void)
{
	uint32_t	firstPinCenter = 0;
	mPinCentersValid = false;
	/*
	*	The picture taken is a high resolution backlit key but only transition
	*	data is recorded.
	*
	*	Each entry of mKeyData is the width of the key edge as recorded from each
	*	data line sent from the camera.
	*
	*	The camera data is recorded with the key pointing up, so the tip of the
	*	key is at a lower index within the data than the bow.
	*
	*	There are 1918 entries/lines of data, but the useful data is in the
	*	index range 500:1750, and within that range only the pin center indexes
	*	are used.  A ring buffer is used to average the delta between each data
	*	entry pair to determine the slope.
	*
	*	The routine NextDepth is used to skip any data errors, data that is out
	*	of bounds.
	*
	*	A key flat is the area that the tip of a pin rests on when the key is
	*	fully inserted in the lock.  The first task is to locate the first flat,
	*	the flat closest to the bow.  Once located, the center of this flat is
	*	the first pin center.  The remaining centers are calculated offsets from
	*	the first center.  The offset is scaled from the key specification units
	*	to pixels.
	*
	*	The pixel depths at the pin centers are converted to key indexes OR, if
	*	the key is custom/outside of tolerance, a value in mm.
	*/
	if (mKeyData)
	{
		const uint32_t	kScanStart = 1700;
		const uint32_t	kScanEnd = 1300;	// Index to stop at if a flat isn't located.
		
		uint32_t	sloping = 0;
		uint32_t	flatStart = 0;
		uint32_t	flatEnd = 0;
		uint32_t	j = kScanStart;
		int32_t		depth = NextDepth(j, kScanEnd, j);
		int32_t		nextDepth = NextDepth(j, kScanEnd, j);
		int32_t		delta = 0;
		int32_t		deltaAcc = 0;
		
		const uint32_t	kRingBufferSize = 10;
		int32_t			deltaRingBuffer[kRingBufferSize];
		memset(deltaRingBuffer, 0, sizeof(deltaRingBuffer));
		uint32_t		deltaRingIndex = 0;

		bool		isInSlope = false;
		for (; j > kScanEnd; nextDepth = NextDepth(j, kScanEnd, j))
		{
			delta = depth - nextDepth;
			depth = nextDepth;
			deltaAcc -= deltaRingBuffer[deltaRingIndex];
			deltaRingBuffer[deltaRingIndex++] = delta;
			deltaAcc += delta;
			if (deltaRingIndex == kRingBufferSize)
			{
				deltaRingIndex = 0;
			}
			
			if (deltaAcc/5)
			{
				if (sloping < 6)
				{
					sloping++;
				} else
				{
					isInSlope = true;
					if (flatStart)
					{
						flatEnd = (j);
						if ((flatEnd - flatStart) < 20)
						{
							flatStart = 0;
							flatEnd = 0;
						} else
						{
							break;
						}
					}
				}
			} else if (sloping)
			{
				sloping--;
				if (isInSlope && sloping == 0)
				{
					if (flatStart == 0)
					{
						flatStart = j;
					}
				}
			}
		}
		mPinCentersValid = (flatStart != 0 && flatEnd != 0);
		if (mPinCentersValid)
		{
			// The +10 below accounts for the delay introduced by the ring buffer.
			firstPinCenter = flatStart - ((flatStart - flatEnd)/2) + 15;
			//fprintf(stderr, "Flat %u:%u %u\n", flatStart, flatEnd, firstPinCenter);
			/*
			*	Create a pin centers array scaled to pixels.
			*/
			{
				uint32_t	scaledCenterInc = mKeySpec->pinSpacing/mCentersScale;
				uint32_t	pixelPinCenter = firstPinCenter;
				for (uint32_t i = 0; i < mKeySpec->numPins; i++)
				{
					mPinCenter[i] = pixelPinCenter;
					pixelPinCenter -= scaledCenterInc;
				}
			}
		} else
		{
			// >>>>>>>  Flat Not Found <<<<<<<
			UpdateStatusMessage(kFlatNotFoundStr);
		}
	}
	UpdatePinRootIndexes();
}

/******************************** UpdatePinRootIndexes *********************************/
void XKeyView::UpdatePinRootIndexes(void)
{
	if (mPinCentersValid &&
		mKeyData)
	{		
		uint32_t	endIndex, j, thisPinDepth;
		int32_t		cutIndexInc = mKeySpec->CutIndexInc();
		for (uint32_t k = 0; k < mKeySpec->numPins; k++)
		{
			/*
			*	The mRootDepth array goes from the deepest cut to the
			*	shallowest. The mRootDepth values are in pixels.
			*	The routine NextDepth() is used to skip over any errors in the
			*	data. The data index range allows for three attempts before an
			*	error occurs.
			*/ 
			j = mPinCenter[k];
			endIndex = j - 2;
			thisPinDepth = NextDepth(j, endIndex, j);
			mPinDepth[k] = thisPinDepth;	// Saved for debugging
			for (uint32_t i = 0; i < mKeySpec->numPinDepths; i++)
			{
				uint32_t	pinDepth = mRootDepth[i];
				if (thisPinDepth > (pinDepth + mTolerance))
				{
					continue;
				} else if (thisPinDepth < (pinDepth - mTolerance))
				{
					mCustomPin[k] = thisPinDepth*mDepthsScale;
					mPinRootIndex[k] = 99;
					break;
				}
				mPinRootIndex[k] = mKeySpec->deepestCutIndex + (cutIndexInc * i);
				mPinRootDelta[k] = (int32_t)pinDepth - thisPinDepth;
				break;
			}
		}
	} else
	{
		for (uint32_t k = 0; k < mKeySpec->numPins; k++)
		{
			mPinRootIndex[k] = 99;
		}
	}
}

/********************************** NextDepth *********************************/
/*
*	Returns the next depth within a range, skipping any invalid depths in the
*	data.
*/
uint32_t XKeyView::NextDepth(
	uint32_t	inIndex,
	uint32_t	inMinIndex,
	uint32_t&	outNextIndex)
{
	uint32_t	depth;
	while (inIndex > inMinIndex)
	{
		depth = mKeyData[inIndex];
		inIndex--;
		if (depth > 100 &&
			depth < 600)
		{
			outNextIndex = inIndex;
			return(depth);
		}
	}
	outNextIndex = inMinIndex;
	return(0);
}
