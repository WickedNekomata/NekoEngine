/*******************************************************************************
The content of this file includes portions of the AUDIOKINETIC Wwise Technology
released in source code form as part of the SDK installer package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use this file in accordance with the end user license agreement provided 
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

  Version: v2017.2.6  Build: 6636
  Copyright (c) 2006-2018 Audiokinetic Inc.
*******************************************************************************/

#ifndef _AK_DELAYFXDSP_H_
#define _AK_DELAYFXDSP_H_

#include <AK/SoundEngine/Common/AkTypes.h>
#include <AK/Plugin/PluginServices/AkFXTailHandler.h>
#include <AK/SoundEngine/Common/IAkPluginMemAlloc.h>
#include <AK/SoundEngine/Common/AkCommonDefs.h>
#include "AkDelayFXParams.h"
#include <AK/DSP/AkDelayLineMemoryStream.h>

/// Delay FX DSP Processing class
class CAkDelayFXDSP
{
public:

	CAkDelayFXDSP();
	~CAkDelayFXDSP();
	void Setup( AkDelayFXParams * pInitialParams, AkUInt32 in_uSampleRate  );
	AKRESULT InitDelay( AK::IAkPluginMemAlloc *	in_pAllocator, AkDelayFXParams * pParams, AkChannelConfig in_channelConfig );
	void ResetDelay( );
	void TermDelay( AK::IAkPluginMemAlloc * in_pAllocator );
	void ComputeTailLength( bool in_bFeedbackEnabled, AkReal32 in_fFeedbackValue );

	void Process( 
		AkAudioBuffer * io_pBuffer, 
		AkDelayFXParams * pCurrentParams
		);

private:

#ifdef AK_VOICE_MAX_NUM_CHANNELS
	AK::DSP::CAkDelayLineMemoryStream<AkReal32,AK_VOICE_MAX_NUM_CHANNELS> m_DelayMem;		/// Delay information
#else
	AK::DSP::CAkDelayLineMemoryStream<AkReal32> m_DelayMem;		/// Delay information
#endif

	AkFXTailHandler				m_FXTailHandler;			/// FX tail handling services
	AkDelayFXParams				m_PreviousParams;			/// Cache previous parameters for parameter ramps
	AkUInt32					m_uNumProcessedChannels;	/// Number of channels delayed
	AkUInt32					m_uSampleRate;				/// Sample rate
	AkUInt32					m_uTailLength;				/// Cached tail length based on feedback and delay lengths parameters
	bool						m_bProcessLFE;				/// Delay or not the LFE channel

} AK_ALIGN_DMA;

#endif // _AK_DELAYFXDSP_H_
