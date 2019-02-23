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
//////////////////////////////////////////////////////////////////////
//
// AkFXSrcSine.cpp
//
// Sample sine tone source. Implements a very basic oscillator.
//
//////////////////////////////////////////////////////////////////////

#include "AkFXSrcSine.h"
#include <AK/Tools/Common/AkAssert.h>
#include <AK/AkWwiseSDKVersion.h>

/// Plugin mechanism. Instanciation method that must be registered to the plug-in manager.
AK::IAkPlugin* CreateSineSource( AK::IAkPluginMemAlloc * in_pAllocator )
{
	return AK_PLUGIN_NEW( in_pAllocator, CAkFXSrcSine() );
}

// Plugin mechanism. Parameter node create function to be registered to the FX manager.
AK::IAkPluginParam * CreateSineSourceParams(AK::IAkPluginMemAlloc * in_pAllocator)
{
	return AK_PLUGIN_NEW(in_pAllocator, CAkFxSrcSineParams());
}

#include <AK/SoundEngine/Common/IAkPlugin.h>
AK_IMPLEMENT_PLUGIN_FACTORY(SineSource, AkPluginTypeSource, 0, 100)

/// Constructor.
CAkFXSrcSine::CAkFXSrcSine() : m_pParams(NULL)
{
}

/// Destructor.
CAkFXSrcSine::~CAkFXSrcSine()
{

}

/// Initializes the effect.
AKRESULT CAkFXSrcSine::Init(	
							AK::IAkPluginMemAlloc *			/*in_pAllocator*/, 	/// Memory allocator interface.
							AK::IAkSourcePluginContext *	in_pSourceFXContext,/// Sound engine plug-in execution context.
							AK::IAkPluginParam *			in_pParams,			/// Associated effect parameters node.
							AkAudioFormat &					io_rFormat			/// Output audio format.
                           )
{
    m_pParams = (CAkFxSrcSineParams*)in_pParams;

	// Notify pipeline of chosen output format change.
	// You must also change other format field when changing number of output channel or sample type.
	io_rFormat.channelConfig.SetStandard( m_pParams->uChannelMask );
	if ( !io_rFormat.IsChannelConfigSupported() )
	{
		// LFE channel is not supported on the some platforms. 
		// Output just mono channel in these case.
		io_rFormat.channelConfig.SetStandard( AK_SPEAKER_SETUP_MONO );
	}
	
	m_SineState.m_fPreviousGain = m_pParams->fGain;	// Gain ramp initialization

	// Setup helper to handle looping and possibly changing duration of synthesis
	m_SineState.DurationHandler.Setup( m_pParams->fDuration, in_pSourceFXContext->GetNumLoops(), io_rFormat.uSampleRate ); 
	m_SineState.Oscillator.Setup( m_pParams->fFrequency, 0.f, io_rFormat.uSampleRate ); // Prepare sine oscillator DSP

	m_pParams->m_ParamChangeHandler.ResetAllParamChanges(); // All parameter changes have been applied
	AK_PERF_RECORDING_RESET();

    return AK_Success;
}

/// The effect must destroy itself herein.
AKRESULT CAkFXSrcSine::Term( AK::IAkPluginMemAlloc * in_pAllocator )
{
	AK_PLUGIN_DELETE( in_pAllocator, this );
    return AK_Success;
}


/// Reinitialize processing state.
AKRESULT CAkFXSrcSine::Reset( )
{
	m_SineState.DurationHandler.Reset();
    return AK_Success;
}

/// Effect info query.
AKRESULT CAkFXSrcSine::GetPluginInfo( AkPluginInfo & out_rPluginInfo )
{
    out_rPluginInfo.eType = AkPluginTypeSource;
	out_rPluginInfo.bIsInPlace = true;
	out_rPluginInfo.uBuildVersion = AK_WWISESDK_VERSION_COMBINED;
    return AK_Success;
}

/// Skip processing of some frames when voice is virtual from elapsed time.
AKRESULT CAkFXSrcSine::TimeSkip( AkUInt32 &io_uFrames )
{
	// Continue keeping track of update of duration parameter
	if ( m_pParams->m_ParamChangeHandler.HasChanged( AK_SINE_FXPARAM_DURATION_ID ) )
		m_SineState.DurationHandler.SetDuration( m_pParams->fDuration );
	m_pParams->m_ParamChangeHandler.ResetParamChange( AK_SINE_FXPARAM_DURATION_ID );
	m_SineState.m_fPreviousGain = m_pParams->fGain;

	AkUInt16 uValidFrames = (AkUInt16)io_uFrames;
	AKRESULT eResult = m_SineState.DurationHandler.ProduceBuffer( (AkUInt16)io_uFrames, uValidFrames );
	io_uFrames = uValidFrames;
	return eResult;
}

//Effect processing.
void CAkFXSrcSine::Execute(	AkAudioBuffer *							io_pBufferOut		// Output buffer interface.
							)
{
	if ( m_pParams->m_ParamChangeHandler.HasChanged( AK_SINE_FXPARAM_FREQ_ID ) )
		m_SineState.Oscillator.SetFrequency( m_pParams->fFrequency );
	if ( m_pParams->m_ParamChangeHandler.HasChanged( AK_SINE_FXPARAM_DURATION_ID ) )
		m_SineState.DurationHandler.SetDuration( m_pParams->fDuration );
	m_pParams->m_ParamChangeHandler.ResetAllParamChanges();

	m_SineState.DurationHandler.ProduceBuffer( io_pBufferOut ); 
	AKASSERT( io_pBufferOut->uValidFrames % 4 == 0 ); // For SIMD processing
	// uValidFrames now indicates how many frames to produce in this execution
	const AkUInt32 uProcessFrames = io_pBufferOut->uValidFrames;

	if ( uProcessFrames )
	{
		AK_PERF_RECORDING_START( "Sine", 25, 30 );

		AkSampleType * pfBufOut = io_pBufferOut->GetChannel(0);
		m_SineState.Oscillator.Process( pfBufOut, uProcessFrames, m_SineState.m_fPreviousGain, m_pParams->fGain );
		m_SineState.m_fPreviousGain = m_pParams->fGain;

		AK_PERF_RECORDING_STOP( "Sine", 25, 30 );
	}
}

// Get the duration of the source in mSec.
AkReal32 CAkFXSrcSine::GetDuration() const
{
	return m_SineState.DurationHandler.GetDuration() * 1000.f;
}

// Stop playback after the current loop iteration
AKRESULT CAkFXSrcSine::StopLooping()
{
	m_SineState.DurationHandler.SetLooping( 1 ); 
	return AK_Success;
}
