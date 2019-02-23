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
// AkDelayFX.cpp
//
// Sample delay FX implementation.
//
//////////////////////////////////////////////////////////////////////

#include "AkDelayFX.h"
#include <AK/Tools/Common/AkAssert.h>
#include <AK/AkWwiseSDKVersion.h>

/// Plugin mechanism. Instantiation method that must be registered to the plug-in manager.
AK::IAkPlugin* CreateAkDelayFX( AK::IAkPluginMemAlloc * in_pAllocator )
{
	return AK_PLUGIN_NEW( in_pAllocator, CAkDelayFX( ) );
}

/// Plugin mechanism. Instantiation method that must be registered to the plug-in manager.
AK::IAkPluginParam * CreateAkDelayFXParams(AK::IAkPluginMemAlloc * in_pAllocator)
{
	return AK_PLUGIN_NEW(in_pAllocator, CAkDelayFXParams());
}

AK_IMPLEMENT_PLUGIN_FACTORY(AkDelayFX, AkPluginTypeEffect, 0, 106)

/// Constructor.
CAkDelayFX::CAkDelayFX()
	: m_pParams( NULL )
	, m_pAllocator( NULL )
{
}

/// Destructor.
CAkDelayFX::~CAkDelayFX()
{
}

/// Initializes and allocate memory for the effect.
AKRESULT CAkDelayFX::Init(	AK::IAkPluginMemAlloc *			in_pAllocator,		/// Memory allocator interface.
							AK::IAkEffectPluginContext *	in_pFXCtx,			/// Sound engine plug-in execution context.
							AK::IAkPluginParam *			in_pParams,			/// Associated effect parameters node.
							AkAudioFormat &					in_rFormat			/// Input/output audio format.
							)
{
	m_pParams = (CAkDelayFXParams*)in_pParams;
	m_pAllocator = in_pAllocator;
 
	m_FXState.Setup( m_pParams, in_rFormat.uSampleRate );
	AKRESULT eResult = m_FXState.InitDelay( in_pAllocator, m_pParams, in_rFormat.channelConfig );
	m_FXState.ComputeTailLength( m_pParams->RTPC.bFeedbackEnabled, m_pParams->RTPC.fFeedback );
	m_pParams->NonRTPC.bHasChanged = false; 
	m_pParams->RTPC.bHasChanged = false;

	AK_PERF_RECORDING_RESET();

	return eResult;
}

/// Effect termination.
AKRESULT CAkDelayFX::Term( AK::IAkPluginMemAlloc * in_pAllocator )
{
	m_FXState.TermDelay( in_pAllocator );
	AK_PLUGIN_DELETE( in_pAllocator, this ); /// Effect must delete itself
	return AK_Success;
}

/// Actions to perform on FX reset (example on bypass)
AKRESULT CAkDelayFX::Reset( )
{
	m_FXState.ResetDelay();
	return AK_Success;
}

/// Effect info query.
AKRESULT CAkDelayFX::GetPluginInfo( AkPluginInfo & out_rPluginInfo )
{
	out_rPluginInfo.eType = AkPluginTypeEffect;
	out_rPluginInfo.bIsInPlace = true;
	out_rPluginInfo.uBuildVersion = AK_WWISESDK_VERSION_COMBINED;
	return AK_Success;
}

/// Effect plug-in DSP processing
void CAkDelayFX::Execute( AkAudioBuffer * io_pBuffer )
{
	if ( AK_EXPECT_FALSE( m_pParams->NonRTPC.bHasChanged ) ) 
	{
		AKRESULT eResult = m_FXState.InitDelay( m_pAllocator, m_pParams, io_pBuffer->GetChannelConfig() );
		if ( eResult != AK_Success )
			return; // passthrough
		m_FXState.ResetDelay();
		m_pParams->NonRTPC.bHasChanged = false; 
	}

	if ( AK_EXPECT_FALSE( m_pParams->RTPC.bHasChanged ) )
	{
		m_FXState.ComputeTailLength( m_pParams->RTPC.bFeedbackEnabled, m_pParams->RTPC.fFeedback );
		m_pParams->RTPC.bHasChanged = false;
	}
	
	AK_PERF_RECORDING_START( "Delay", 25, 30 );
	// Execute DSP processing synchronously here
	m_FXState.Process( io_pBuffer, m_pParams );
	AK_PERF_RECORDING_STOP( "Delay", 25, 30 );
}
