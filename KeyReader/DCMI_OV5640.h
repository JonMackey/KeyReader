/*
*	DCMI_OV5640.h, Copyright Jonathan Mackey 2025
*	Class to interface DCMI to the OV5640 camera.
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
#ifndef DCMI_OV5640_h
#define DCMI_OV5640_h


#include "Config.h"
#include "HardwareTimer.h"
#include "OV5640.h"

class TwoWire;
typedef std::function<void(const uint16_t*)> DataChangedCallback;
typedef std::function<void(bool)> PreviewModeCallback;
typedef std::function<void(const char*)> ScanStatusCallback;

class DCMI_OV5640
{
public:
							DCMI_OV5640(
								TwoWire&				inWire);
	void					begin(
								PreviewModeCallback		inEnterPreviewModeCB,
								PreviewModeCallback		inExitPreviewModeCB,
								DataChangedCallback		inKeyDataChangedCB,
								ScanStatusCallback		inScanStatusCB);
	static void				SetBWThreshold(
								uint16_t				inThreshold);
	static uint16_t			GetBWThreshold(void)
								{return(sBWThreshold);}
	static void				SetBWThresholdFromStr(
								const char*				inStr);
	bool					ChipIDIs5640(void) const;
	void					StartHiResStream(
								bool					inResetRetries = true);
	void					StopHiResStream(
								bool					inCancel = false);
	void					StartPreviewStream(
								bool					inPreviewIsRGB = true);
	void					InitPreviewStream(
								bool					inPreviewIsRGB);
	bool					StopPreviewStream(void);
	void					ChangePreviewFormat(
								bool					inPreviewIsRGB);
	void					SuspendPreview(void);
	void					ResumePreview(void);
	bool					PreviewIsStreaming(void) const
								{return(mPreviewIsStreaming);}
	void					SetPixelFormat(
								OV5640::EPixelFormat	inPixelFormat);
	void					ShowTestPattern(
								OV5640::ETestPattern	inTestPattern);
	void					StrobeOn(void);
	void					StrobeOff(void);
	void					DumpWinConf(void) const;
	void					ConfWinFromStr(
								const char*				inStr);
	void					SetHueFromStr(
								const char*				inStr);
	void					SetHue(
								int32_t					inHueIndex);
	void					Update(void);
	bool					KeyDataIsValid(void) const
								{return(mKeyDataIsValid);}
	const uint16_t*			GetKeyData(void);
	bool					HiResInProgress(void) const
								{return(mHiResInProgress);}
protected:
	HardwareTimer	mXCLKTimer;
	TwoWire&		mWire;
	uint32_t		mPreviewSuspended;
	bool			mDCMIInitialized;
	bool			mPreviewIsStreaming;
	bool			mHiResInProgress;
	bool			mKeyDataIsValid;
	DataChangedCallback mKeyDataChangedCB;
	PreviewModeCallback	mExitPreviewModeCB;
	PreviewModeCallback	mEnterPreviewModeCB;
	ScanStatusCallback	mScanStatusCB;
	char			mStatusMessage[128];
	static DCMI_HandleTypeDef sHDCMI;
	static uint16_t	s2LineBuf[];
	static uint16_t	sKeyData[];
	static uint32_t	sLineLength;
	static uint32_t	sLineCount;
	static uint32_t	sFrameIndex;
	static uint32_t	sSampleFrameIndex;
	static uint32_t	sHiResRetries;
	static uint32_t	sErrorCount;
	static uint32_t	kErrorCountThreshold;
	static uint32_t	sError;
	static uint32_t	sErrorLine;
	static uint16_t	sBWThreshold;
	static uint16_t	sPreviewBWThreshold;
	static bool		sPreviewIsRGB;
	static bool		sHiResFrameCaptured;
	enum
	{
		eChipIDHReg		= 0x300A,
		eChipIDHValue	= 0x56,
		eChipIDLReg		= 0x300B,
		eChipIDLValue	= 0x40,
	};
	void					DMA_Init(void);
	void					DCMI_Init(void);
	void					ResetCamera(void);
	uint8_t					ReadReg(
								uint16_t				inAddress) const;
	void					WriteReg(
								uint16_t				inAddress,
								uint8_t					inValue) const;
	void					WriteRegArray(
								const uint16_t*			inRegArray) const;
	static HAL_StatusTypeDef StartDMA(
								DCMI_HandleTypeDef*		inHDCMI,
								uint32_t				inDCMI_Mode,
								uint32_t				inLineBuf,
								uint32_t				inLineBufLen,
								uint32_t				inNumLines,
								bool					inIsPreview);
	static void				PreviewLineCompleteCallback(
								DMA_HandleTypeDef*		inHDMA);
	static void				HiResLineCompleteCallback(
								DMA_HandleTypeDef*		inHDMA);
public:
	static void				DMAErrorCallback(
								DMA_HandleTypeDef*		inHDMA);
};

/*
*	The interrupt handlers must be linked as C functions in order to override
*	the weak default handlers defined in startup_stm32f429xx.s
*/
extern "C"
{
	void DMA2_Stream1_IRQHandler(void);
	void DCMI_IRQHandler(void);
}

#endif // DCMI_OV5640_h
