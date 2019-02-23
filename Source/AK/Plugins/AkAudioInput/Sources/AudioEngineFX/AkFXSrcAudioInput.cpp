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
// AkFXSrcAudioInput.cpp
//
// Sample capture of audio input to be used as a source.
//
//////////////////////////////////////////////////////////////////////

#include "AkFXSrcAudioInput.h"
#include <AK/Tools/Common/AkAssert.h>
#include <math.h>
#include <AK/AkWwiseSDKVersion.h>

// Holds audio input information
struct AudioInputStream
{
	AkReal32 *		m_pData;
	AkUInt32		m_uDataSize;
	AkUInt32		m_uSampleRate;
	AudioInputStream() : m_pData(NULL), m_uDataSize(0), m_uSampleRate(0) {}
};

AkAudioInputPluginExecuteCallbackFunc CAkFXSrcAudioInput::m_pfnExecCallback = NULL;
AkAudioInputPluginGetFormatCallbackFunc CAkFXSrcAudioInput::m_pfnGetFormatCallback = NULL;
AkAudioInputPluginGetGainCallbackFunc CAkFXSrcAudioInput::m_pfnGetGainCallback = NULL;

// Useful definitions
static const AkReal32 RAMPMAXTIME	= 0.1f;		// 100 ms ramps, worst-case


// Plugin mechanism. FX create function and register its address to the FX manager.
AK::IAkPlugin* CreateAudioInputSource( AK::IAkPluginMemAlloc * in_pAllocator )
{
	return AK_PLUGIN_NEW( in_pAllocator, CAkFXSrcAudioInput() );
}

// Plugin mechanism. Parameter node create function to be registered to the FX manager.
AK::IAkPluginParam * CreateAudioInputSourceParams(AK::IAkPluginMemAlloc * in_pAllocator)
{
	return AK_PLUGIN_NEW(in_pAllocator, CAkFxSrcAudioInputParams());
}

AK::PluginRegistration AkAudioInputSourceRegistration(AkPluginTypeSource, AKCOMPANYID_AUDIOKINETIC, 200, CreateAudioInputSource, CreateAudioInputSourceParams);

// Constructor.
CAkFXSrcAudioInput::CAkFXSrcAudioInput()
: m_pSharedParams( NULL )
, m_pSourceFXContext( NULL )
{
}

// Destructor.
CAkFXSrcAudioInput::~CAkFXSrcAudioInput()
{

}

// Initialization.
AKRESULT CAkFXSrcAudioInput::Init(	AK::IAkPluginMemAlloc *			in_pAllocator,    		// Memory allocator interface.
									AK::IAkSourcePluginContext *	in_pSourceFXContext,	// Source FX context
									AK::IAkPluginParam *			in_pParams,				// Effect parameters.
									AkAudioFormat &					io_rFormat				// Output audio format.
                           )
{
	// Keep source FX context (looping info etc.)
	m_pSourceFXContext = in_pSourceFXContext;

	// Set parameters access
    m_pSharedParams = reinterpret_cast<CAkFxSrcAudioInputParams*>(in_pParams);

    if( m_pfnGetFormatCallback )
    {
        m_pfnGetFormatCallback( in_pSourceFXContext->GetVoiceInfo()->GetPlayingID(), io_rFormat );
    }
    m_Format = io_rFormat; // keeping track of the audio format for future use on Execute().

	// Gain ramp initialization
	AkReal32 fGainIncrement = 1.f/(RAMPMAXTIME*io_rFormat.uSampleRate);

	m_GainRamp.RampSetup( fGainIncrement, GetGain() );

    return AK_Success;
}

// Term: The effect must destroy itself herein
AKRESULT CAkFXSrcAudioInput::Term( AK::IAkPluginMemAlloc * in_pAllocator )
{
	AK_PLUGIN_DELETE( in_pAllocator, this );
    return AK_Success;
}


// Reset. Reinitialize processing state.
AKRESULT CAkFXSrcAudioInput::Reset( )
{
	return AK_Success;
}

// Effect info query.
AKRESULT CAkFXSrcAudioInput::GetPluginInfo( AkPluginInfo & out_rPluginInfo )
{
    out_rPluginInfo.eType = AkPluginTypeSource;
	out_rPluginInfo.bIsInPlace = true;
	out_rPluginInfo.uBuildVersion = AK_WWISESDK_VERSION_COMBINED;
    return AK_Success;
}

AkReal32 CAkFXSrcAudioInput::GetGain()
{
	AkReal32 gain = 1.0;
	if( m_pSharedParams )
		gain = m_pSharedParams->GetGain();

    if( m_pfnGetGainCallback )
        gain *= m_pfnGetGainCallback( m_pSourceFXContext->GetVoiceInfo()->GetPlayingID() );

    return gain;
}

template <class T>
void ProcessGainInt(AkUInt32 in_uNumFrames, AkUInt32 in_uNumChannels, AkAudioBuffer * io_pBufferOut, AK::CAkValueRamp& in_rRamp)
{
    T * AK_RESTRICT pBufOut = (T*)(io_pBufferOut->GetChannel( 0 ));
    for( AkUInt32 i = 0; i < in_uNumFrames; ++i )
    {
		// Tick gain interpolation ramp
		AkReal32 fCurGain = in_rRamp.Tick();
		for( AkUInt32 j = 0; j < in_uNumChannels; ++j )
		{
			*pBufOut = (T)(*pBufOut * fCurGain);
			pBufOut++;
		}
    }
}

template <class T>
void ProcessGain(AkUInt32 in_uNumFrames, AkUInt32 in_uNumChannels, AkAudioBuffer * io_pBufferOut, AK::CAkValueRamp& in_rRamp)
{
	// Each channel will have to process the ramp individually
	// Since channel are not interleaved, backup the Ramp and restore it at every iteration.
	AK::CAkValueRamp LocalBackup = in_rRamp;
    for ( AkUInt32 iChannel = 0; iChannel < in_uNumChannels; ++iChannel )
    {
		if( iChannel != 0 )
			in_rRamp = LocalBackup;//Restore the ramp for the new channel.

	    T * AK_RESTRICT pBufOut = (T*)(io_pBufferOut->GetChannel( iChannel ));

	    for( AkUInt32 i = 0; i < in_uNumFrames; ++i )
	    {
		    // Tick gain interpolation ramp
		    AkReal32 fCurGain = in_rRamp.Tick();
		    *pBufOut = (T)(*pBufOut * fCurGain);
            pBufOut++;
	    }
    }
}

//Effect processing.
void CAkFXSrcAudioInput::Execute( AkAudioBuffer *	io_pBufferOut )
{
    if( !m_pfnExecCallback )
    {
	    io_pBufferOut->eState = AK_Fail;
	    io_pBufferOut->uValidFrames = 0;
        return;
    }

    m_GainRamp.SetTarget( GetGain() );

    m_pfnExecCallback( m_pSourceFXContext->GetVoiceInfo()->GetPlayingID(), io_pBufferOut);

    // Change target gain if necessary (RTPC value)

    AkUInt32 uNumFrames = io_pBufferOut->uValidFrames;
    AkUInt32 uNumChannels = io_pBufferOut->NumChannels();

    if( m_Format.GetTypeID() == AK_FLOAT )
    {
        ProcessGain<AkReal32>(uNumFrames, uNumChannels,io_pBufferOut, m_GainRamp);
    }
    else
    {
        AKASSERT( m_Format.GetTypeID() == AK_INT );
        switch( m_Format.GetBlockAlign() / m_Format.GetNumChannels() )
        {
        case 1:
            ProcessGainInt<AkInt8>(uNumFrames, uNumChannels, io_pBufferOut, m_GainRamp);
            break;
        case 2:
            ProcessGainInt<AkInt16>(uNumFrames, uNumChannels, io_pBufferOut, m_GainRamp);
            break;
        case 4:
            ProcessGainInt<AkInt32>(uNumFrames, uNumChannels, io_pBufferOut, m_GainRamp);
            break;
        default:
			AKASSERT( !"Unsupported format, no gain applied in AudioInput" );
            break;
        }
    }
}

AKRESULT CAkFXSrcAudioInput::StopLooping()
{
	// Since this plug-in is infinite playing, a stoplooping should result in stoping the sound.
	// returning AK_Fail will do it.
	return AK_Fail;
}

void CAkFXSrcAudioInput::SetAudioInputCallbacks(
                    AkAudioInputPluginExecuteCallbackFunc in_pfnExecCallback, 
                    AkAudioInputPluginGetFormatCallbackFunc in_pfnGetFormatCallback,
                    AkAudioInputPluginGetGainCallbackFunc in_pfnGetGainCallback
                    )
 {
     if( in_pfnExecCallback && in_pfnGetFormatCallback )
     {
         m_pfnExecCallback =        in_pfnExecCallback;
         m_pfnGetFormatCallback =   in_pfnGetFormatCallback;
         m_pfnGetGainCallback =     in_pfnGetGainCallback;
     }
 }

void SetAudioInputCallbacks(
                AkAudioInputPluginExecuteCallbackFunc in_pfnExecCallback, 
                AkAudioInputPluginGetFormatCallbackFunc in_pfnGetFormatCallback /*= NULL*/, // Optional
                AkAudioInputPluginGetGainCallbackFunc in_pfnGetGainCallback /*= NULL*/      // Optional
                )
{
    CAkFXSrcAudioInput::SetAudioInputCallbacks( in_pfnExecCallback, in_pfnGetFormatCallback, in_pfnGetGainCallback );
}
