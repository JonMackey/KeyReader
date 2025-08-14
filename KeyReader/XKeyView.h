/*
*	XKeyView.h, Copyright Jonathan Mackey 2025
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
#ifndef XKeyView_h
#define XKeyView_h

#include "XView.h"
#include "XFont.h"
#include "SKeySpecU32.h"
#ifndef __MACH__
class SdFile;
#endif
class XKeyView : public XView
{
public:
							XKeyView(
								int16_t					inX,
								int16_t					inY,
								uint16_t				inWidth,
								uint16_t				inHeight,
								uint16_t				inTag,
								XView*					inNextView = nullptr,
								XFont::Font*			inFont = nullptr);
	virtual void			DrawSelf(void);
	void					Setup(
								uint32_t				inCentersScale,
								uint32_t				inDepthsScale,
								uint32_t				inTolerance);								
	void					SetCentersScale(
								uint32_t				inScale,
								bool					inUpdate);
	uint32_t				GetCentersScale(void) const
								{return(mCentersScale);}
	void					SetDepthsScale(
								uint32_t				inScale,
								bool					inUpdate);
	uint32_t				GetDepthsScale(void) const
								{return(mDepthsScale);}
	void					SetTolerance(
								uint32_t				inTolerance,
								bool					inUpdate);
	uint32_t				GetTolerance(void) const
								{return(mTolerance);}
	void					ShowPinRootDelta(
								bool					inShowPinRootDelta)
								{mShowPinRootDelta = inShowPinRootDelta;}
#ifdef __MACH__
	void					Dump(void);
#else
	void					Dump(
								SdFile*					inFile);
#endif
	bool					GetCutKeyCmdStr(
								char*					outCutKeyCmdStr);
	void					SetKeySpec(
								const SKeySpecU32*		inKeySpec,
								bool					inUpdate);
	void					SetKeyData(
								const uint16_t*			inKeyData,
								bool					inUpdate = false);
	void					UpdateStatusMessage(
								const char*				inStatusMessage);
	void					EnterPreviewMode(
								bool					inEraseView = false);
	void					ExitPreviewMode(
								bool					inEraseView = false);
	bool					InPreviewMode(void) const
								{return(mInPreviewMode);}
	bool					DataIsValid(void) const
								{return(mPinCentersValid);}
protected:
	XFont::Font*		mFont;
	const char*			mStatusMessage;
	const SKeySpecU32*	mKeySpec;
	const uint16_t*		mKeyData;
	uint32_t			mCentersScale;
	uint32_t			mDepthsScale;
	uint32_t			mPinCenter[6];
	uint32_t			mRootDepth[10];
	uint8_t				mPinRootIndex[6];
	int32_t				mPinRootDelta[6];
	uint32_t			mPinDepth[6];
	uint32_t			mCustomPin[6];
	uint32_t			mTolerance;
	bool				mPinCentersValid;
	bool				mInPreviewMode;
	bool				mShowPinRootDelta;

	XFont*					MakeFontCurrent(void);
	void					UpdatePinDepths(void);
	void					UpdatePinCenters(void);
	void					UpdatePinRootIndexes(void);
	uint32_t				NextDepth(
								uint32_t				inIndex,
								uint32_t				inMinIndex,
								uint32_t&				outNextIndex);
};
#endif // XKeyView_h
