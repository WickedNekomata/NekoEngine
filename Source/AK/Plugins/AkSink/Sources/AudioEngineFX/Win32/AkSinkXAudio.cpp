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
// AkSinkXAudio.cpp
//
// Platform dependent part
//
//////////////////////////////////////////////////////////////////////

#include "AkSinkXAudio.h"
#include "SinkPluginParams.h"
#include <AK/SoundEngine/Common/AkSimd.h>
#include <AK/AkWwiseSDKVersion.h>

AK::IAkPlugin* CreateSink( AK::IAkPluginMemAlloc * in_pAllocator )
{
	AKASSERT( in_pAllocator != NULL );
	return AK_PLUGIN_NEW( in_pAllocator, AkSinkXAudio() );
}

AK::IAkPluginParam* CreateSinkParams(AK::IAkPluginMemAlloc * in_pAllocator)
{
	AKASSERT(in_pAllocator != NULL);
	return AK_PLUGIN_NEW( in_pAllocator, SinkPluginParams() );
}

AK::PluginRegistration AkSinkRegistration(AkPluginTypeSink, AKCOMPANYID_AUDIOKINETIC, 152, CreateSink, CreateSinkParams);

#if defined AK_CPU_X86 || defined AK_CPU_X86_64
static const AKSIMD_V4F32 vkMin = { (AkReal32) INT_MIN, (AkReal32) INT_MIN, (AkReal32) INT_MIN, (AkReal32) INT_MIN };
static const AKSIMD_V4F32 vkMax = { (AkReal32) INT_MAX, (AkReal32) INT_MAX, (AkReal32) INT_MAX, (AkReal32) INT_MAX };
#endif

#if defined AK_CPU_ARM_NEON
static const DirectX::XMVECTORF32 vkMin = { (AkReal32) INT_MIN, (AkReal32) INT_MIN, (AkReal32) INT_MIN, (AkReal32) INT_MIN };
static const DirectX::XMVECTORF32 vkMax = { (AkReal32) INT_MAX, (AkReal32) INT_MAX, (AkReal32) INT_MAX, (AkReal32) INT_MAX };
#endif

AkSinkXAudio::AkSinkXAudio()
	: m_pSinkPluginContext( NULL )
	, m_pParams( NULL )
	, m_pXAudio2( NULL )
	, m_pMasteringVoice( NULL )
	, m_pSourceVoice( NULL )
	, m_pvAudioBuffer( NULL )
	, m_uWriteBufferIndex( 0 )
	, m_uReadBufferIndex( 0 )
	, m_uNbBuffersRB( 0 )
	, m_usBlockAlign( 0 )
	, m_bStarved( false )
	, m_bCriticalError( false )
	, m_bDataReady( false )
{
	for ( AkUInt32 i = 0; i < NUM_VOICE_BUFFERS; ++i )
		m_ppvRingBuffer[i] = NULL;
}

AkSinkXAudio::~AkSinkXAudio()
{
}

AKRESULT AkSinkXAudio::Init( 
	AK::IAkPluginMemAlloc *	in_pAllocator,				// Interface to memory allocator to be used by the effect.
	AK::IAkSinkPluginContext *	in_pSinkPluginContext,	// Interface to sink plug-in's context.
	AK::IAkPluginParam *		in_pParams,				// Interface to plug-in parameters.
	AkAudioFormat &			io_rFormat					// Audio data format of the input signal. 
	)
{
	m_pParams = (SinkPluginParams*)in_pParams;
	m_pSinkPluginContext = in_pSinkPluginContext;

	// Create XAudio2 engine with default options
	UINT32 Flags = AK_XAUDIO2_FLAGS;
	HRESULT hr;

	hr = XAudio2Create( &m_pXAudio2, Flags, XAUDIO2_DEFAULT_PROCESSOR );
	if(  hr != S_OK )
	{
		// Failure here usually means that XAudio2 is not installed on the system.
		return AK_NotCompatible;
	}
	AKASSERT( m_pXAudio2 );

	// Create a 6-channel 48 kHz mastering voice feeding the default device
	UINT32 uDesiredNumChannels = XAUDIO2_DEFAULT_CHANNELS;
	if( !SUCCEEDED( m_pXAudio2->CreateMasteringVoice( &m_pMasteringVoice, uDesiredNumChannels, io_rFormat.uSampleRate ) ) )
		return AK_Fail;
	
	m_pXAudio2->RegisterForCallbacks( this );

	m_speakerConfig = io_rFormat.channelConfig;

	// Get current channel mask from xaudio2 device
	if( !io_rFormat.channelConfig.IsValid() )
	{
#if defined(AK_USE_UWP_API)
		AkChannelMask uChannelMask;
		if ( !SUCCEEDED( m_pMasteringVoice->GetChannelMask( &uChannelMask ) ) )
		{
			AKASSERT( !"Unable to get mastering voice channel mask" );
			return AK_Fail;
		}
		
		m_speakerConfig.SetStandard( uChannelMask );

#else
		XAUDIO2_DEVICE_DETAILS deviceDetails;
		if( !SUCCEEDED( m_pXAudio2->GetDeviceDetails( 0, &deviceDetails ) ) )
		{
			AKASSERT(!"Unable to query XAudio2 mastering voice");
			return AK_Fail;
		}
		
		m_speakerConfig.SetStandardOrAnonymous( deviceDetails.OutputFormat.Format.nChannels, deviceDetails.OutputFormat.dwChannelMask );
#endif	
	}
	
	m_usBlockAlign = (AkUInt16) ( m_speakerConfig.uNumChannels * sizeof(AkReal32) );

	AkUInt32 uBufferBytes = in_pSinkPluginContext->GlobalContext()->GetMaxBufferLength() * m_usBlockAlign;

	// Allocate ring buffer 
	m_pvAudioBuffer = AK_PLUGIN_ALLOC( in_pAllocator, NUM_VOICE_BUFFERS * uBufferBytes );
	if ( m_pvAudioBuffer == NULL )
		return AK_Fail;

	::ZeroMemory( m_pvAudioBuffer, NUM_VOICE_BUFFERS * uBufferBytes );

	// Initialize ring buffer ptrs
	for ( AkUInt32 i = 0; i < NUM_VOICE_BUFFERS; ++i )
		m_ppvRingBuffer[i] = (AkUInt8*)m_pvAudioBuffer + i*uBufferBytes;

	// Create the source voice
	WAVEFORMATEXTENSIBLE formatX;
	formatX.dwChannelMask = io_rFormat.channelConfig.uChannelMask; 
	formatX.SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
	formatX.Samples.wValidBitsPerSample = 32;
	formatX.Format.nChannels = (WORD)m_speakerConfig.uNumChannels;
	formatX.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
	formatX.Format.nSamplesPerSec = io_rFormat.uSampleRate;
	formatX.Format.nBlockAlign = formatX.Format.nChannels * sizeof( AkReal32 );
	formatX.Format.nAvgBytesPerSec = formatX.Format.nSamplesPerSec * formatX.Format.nBlockAlign;
	formatX.Format.wBitsPerSample = sizeof( AkReal32 ) * 8;
	formatX.Format.cbSize = sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX);

	if( FAILED( m_pXAudio2->CreateSourceVoice( &m_pSourceVoice, (WAVEFORMATEX*)&(formatX), 0, 1.0, this ) ) )
		return AK_Fail;

	m_pSourceVoice->SetVolume(1.0f);

	io_rFormat.channelConfig = m_speakerConfig;
	/// TEST: Force another configuration for the master bus.
	///io_rFormat.channelConfig.SetStandard( AK_SPEAKER_SETUP_AURO_13POINT1_751 );

	return AK_Success;
}

AKRESULT AkSinkXAudio::Term( AK::IAkPluginMemAlloc * in_pAllocator )
{
	if( m_pSourceVoice )//must check since Stop may be called on Term after the Init Failed
	{
		m_pSourceVoice->DestroyVoice(); // call this before clearing buffer to avoid race with voice callback
		m_pSourceVoice = NULL;
	}

	if( m_pMasteringVoice )
	{
		m_pMasteringVoice->DestroyVoice();
		m_pMasteringVoice = NULL;
	}

	if( m_pXAudio2 )
	{
		m_pXAudio2->UnregisterForCallbacks( this );
		m_pXAudio2->Release();
		m_pXAudio2 = NULL;
	}

	// Clear ring buffer
	m_uWriteBufferIndex = 0;
	m_uReadBufferIndex = 0;
	m_uNbBuffersRB = 0;
	for ( AkUInt32 i = 0; i < NUM_VOICE_BUFFERS; ++i )
		m_ppvRingBuffer[i] = NULL;

	if ( m_pvAudioBuffer )
	{
		AK_PLUGIN_FREE( in_pAllocator, m_pvAudioBuffer );
		m_pvAudioBuffer = NULL;
	}

	AK_PLUGIN_DELETE( in_pAllocator, this );
	return AK_Success;
}

AKRESULT AkSinkXAudio::Reset()
{
	HRESULT hr = m_pSourceVoice->Start(0);
	return SUCCEEDED( hr ) ? AK_Success : AK_Fail;
}

bool AkSinkXAudio::IsStarved()
{
	return m_bStarved;
}

void AkSinkXAudio::ResetStarved()
{
	m_bStarved = false;
}

AKRESULT AkSinkXAudio::GetPluginInfo( 
	AkPluginInfo & out_rPluginInfo	///< Reference to the plug-in information structure to be retrieved
	)
{
	out_rPluginInfo.eType			= AkPluginTypeSink;
	out_rPluginInfo.bIsInPlace		= false;
	out_rPluginInfo.uBuildVersion = AK_WWISESDK_VERSION_COMBINED;
	return AK_Success;
}

void AkSinkXAudio::Consume(
	AkAudioBuffer *			in_pInputBuffer,		///< Input audio buffer data structure. Plugins should avoid processing data in-place.
	AkRamp					in_gain					///< Volume gain to apply to this input (prev corresponds to the beginning, next corresponds to the end of the buffer).
	)
{
	if ( in_pInputBuffer->uValidFrames > 0 )
	{
		if (m_pParams)
		{
			// Simply demonstrating a usage for the sample plug-in property value.
			// This additional gain is not accurate but is there to add au auditive cue so we know this parameter gets updated live.
			in_gain.fPrev *= m_pParams->GetCurrentParams().fDirectGain;
			in_gain.fNext *= m_pParams->GetCurrentParams().fDirectGain;
		}
		// Do final pass on data (interleave, format conversion, and the like) before presenting to sink.
		
		AkUInt32 uNumFrames = in_pInputBuffer->uValidFrames;
		AKASSERT( uNumFrames != 0 );
		AkReal32 * pBufferOut = (AkReal32 *)m_ppvRingBuffer[m_uWriteBufferIndex];
		
		// Apply volumes and interleave
		AkReal32 fGainStart = in_gain.fPrev;
		AkReal32 fDelta = ( in_gain.fNext - in_gain.fPrev ) / (AkReal32)uNumFrames;
		
		/// Test: Normally, in_pInputBuffer would have the same channel config as XAudio2. But we cheated in Init(),
		/// so we need to handle this here by dropping channels that we do not want.

		AkChannelMask uChanMaskIn = in_pInputBuffer->GetChannelConfig().uChannelMask;

		AkUInt32 uStrideOut = m_speakerConfig.uNumChannels;

		// for all channels
		AkUInt32 uChanIn = 0;
		AkUInt32 uOutChanOffset = 0;
		AkChannelMask uChannelBit = 1;		
		while ( uChannelBit <= m_speakerConfig.uChannelMask ) 
		{
			bool bExistsInInput = ( ( uChannelBit & in_pInputBuffer->GetChannelConfig().uChannelMask ) != 0 );

			// if channel exists in output buffer.
			if ( uChannelBit & m_speakerConfig.uChannelMask )
			{
				AkReal32 fGain = fGainStart;
				AkReal32 * pOut = pBufferOut + uOutChanOffset;

				// if channel exists in input buffer
				if ( bExistsInInput )
				{
					// Copy, apply gain and interleave.
					AkReal32 * pIn = in_pInputBuffer->GetChannel( AkAudioBuffer::StandardToPipelineIndex( in_pInputBuffer->GetChannelConfig(), uChanIn ) );
					AkReal32 * pEnd = pIn + uNumFrames;
					while ( pIn < pEnd )
					{
						*pOut = fGain * *pIn++;

						pOut += uStrideOut;
						fGain += fDelta;			
					}
				}
				else
				{
					// Interleave silence
					AkReal32 * pEnd = pOut + uStrideOut*uNumFrames;
					while ( pOut < pEnd )
					{
						*pOut = 0;
						pOut += uStrideOut;
					}
				}
				++uOutChanOffset;
			}

			// increment index if channel exists in input buffer
			if ( bExistsInInput )
				++uChanIn;

			uChannelBit <<= 1;
		}

		/** LX TODO 
		CAkMixer m_Mixer;
		m_Mixer.Init( in_pInputBuffer->MaxFrames() );
		m_Mixer.MixAndInterleave( in_pInputBuffer, &rBuffer, in_gain );
		**/

		/**
		// Clip samples to [-1.0, 1.0]
		AKSIMD_V4F32 * AK_RESTRICT pvFrom = (AKSIMD_V4F32 *) pbSourceData;
		AKSIMD_V4F32 * AK_RESTRICT pvEnd = (AKSIMD_V4F32 *) ( (AkReal32 *) pbSourceData + lNumSamples );
		AKSIMD_V4F32 * AK_RESTRICT pvTo = (AKSIMD_V4F32 *) pvBuffer;

		for ( ; pvFrom < pvEnd; pvFrom++, pvTo++ )
			*pvTo = AKSIMD_MAX_V4F32( vkMin, AKSIMD_MIN_V4F32( vkMax, *pvFrom ) );
			**/

		++m_uWriteBufferIndex;
		if ( m_uWriteBufferIndex == NUM_VOICE_BUFFERS )
			m_uWriteBufferIndex = 0;

		LONG lNbBuffers = ::InterlockedIncrement( &m_uNbBuffersRB );
		AKASSERT( lNbBuffers <= NUM_VOICE_BUFFERS );

		m_bDataReady = true;
	}
}

void AkSinkXAudio::OnFrameEnd()
{
	if ( !m_bDataReady )
	{
		// Frame has occurred with ConsumeBuffer() being called with data. Pass silence.
		void* pvBuffer = m_ppvRingBuffer[m_uWriteBufferIndex];
		AkUInt32 uNumBytes = m_pSinkPluginContext->GlobalContext()->GetMaxBufferLength() * m_speakerConfig.uNumChannels * sizeof(AkReal32);
		memset( pvBuffer, 0, uNumBytes );

		++m_uWriteBufferIndex;
		if ( m_uWriteBufferIndex == NUM_VOICE_BUFFERS )
			m_uWriteBufferIndex = 0;

		LONG lNbBuffers = ::InterlockedIncrement( &m_uNbBuffersRB );
		AKASSERT( lNbBuffers <= NUM_VOICE_BUFFERS );
	}
	
	// Prepare for next frame.
	m_bDataReady = false;
}

void AkSinkXAudio::SubmitPacketRB( )
{
	// Build and submit the packet
	XAUDIO2_BUFFER XAudio2Buffer = { 0 };
	XAudio2Buffer.AudioBytes = m_pSinkPluginContext->GlobalContext()->GetMaxBufferLength() * m_usBlockAlign;
	XAudio2Buffer.pAudioData = (BYTE*)( m_ppvRingBuffer[m_uReadBufferIndex] );
	++m_uReadBufferIndex;

	// Reset the index when required.
	if ( m_uReadBufferIndex == NUM_VOICE_BUFFERS )
		m_uReadBufferIndex = 0;

	HRESULT hResult = m_pSourceVoice->SubmitSourceBuffer(&XAudio2Buffer);
	AKASSERT( SUCCEEDED(hResult) );

	// Do not release the buffer in ring buffer here, as SubmitSourceBuffer holds the pointer
	// until OnBufferEnd.
}

AKRESULT AkSinkXAudio::IsDataNeeded( AkUInt32 & out_uBuffersNeeded )
{
	if (m_uNbBuffersRB < NUM_VOICE_BUFFERS)
		out_uBuffersNeeded = NUM_VOICE_BUFFERS - m_uNbBuffersRB;
	else
		out_uBuffersNeeded = 0;

	return m_bCriticalError ? AK_Fail : AK_Success;
}

//=================================================
//	IXAudio2EngineCallback interface implementation
//=================================================	 

void AkSinkXAudio::OnProcessingPassStart()
{
}

void AkSinkXAudio::OnProcessingPassEnd()
{
}

void AkSinkXAudio::OnCriticalError( HRESULT Error )
{
	m_bCriticalError = true;
	m_pSinkPluginContext->SignalAudioThread();
}

//=================================================
//	IXAudio2VoiceCallback interface implementation
//=================================================

// Called just before this voice's processing pass begins.
void AkSinkXAudio::OnVoiceProcessingPassStart( UINT32 BytesRequired )
{
	XAUDIO2_VOICE_STATE X2VoiceState;
	m_pSourceVoice->GetState( &X2VoiceState );

	if ( X2VoiceState.BuffersQueued < NUM_VOICE_BUFFERS )
	{
		// set only, don't reset; do not report initial starvation (right after voice start)
		if ( !X2VoiceState.BuffersQueued && X2VoiceState.SamplesPlayed )
			m_bStarved = true;

		// Submit all buffers that have not been submitted yet.
		while ( m_uNbBuffersRB - X2VoiceState.BuffersQueued )
		{
			SubmitPacketRB();
			X2VoiceState.BuffersQueued++;
		}

		if ( X2VoiceState.BuffersQueued < NUM_VOICE_BUFFERS )
			m_pSinkPluginContext->SignalAudioThread(); // Voice still not 'full': trigger buffer generation now.
	}
}

// Called just after this voice's processing pass ends.
void AkSinkXAudio::OnVoiceProcessingPassEnd()
{
	//Not used
}

// Called when this voice has just finished playing a buffer stream
// (as marked with the XAUDIO2_END_OF_STREAM flag on the last buffer).
void AkSinkXAudio::OnStreamEnd()
{
	//Not used
}

// Called when this voice is about to start processing a new buffer.
void AkSinkXAudio::OnBufferStart( void* pBufferContext)
{
	//Not used
}

// Called when this voice has just finished processing a buffer.
// The buffer can now be reused or destroyed.
void AkSinkXAudio::OnBufferEnd( void* pBufferContext)
{
	LONG lNbBuffers = ::InterlockedDecrement( &m_uNbBuffersRB );
	AKASSERT( lNbBuffers >= 0 );

	m_pSinkPluginContext->SignalAudioThread();
}

// Called when this voice has just reached the end position of a loop.
void AkSinkXAudio::OnLoopEnd( void* pBufferContext)
{
	//Not used
}

// Called in the event of a critical error during voice processing,
// such as a failing XAPO or an error from the hardware XMA decoder.
// The voice may have to be destroyed and re-created to recover fromF
// the error.  The callback arguments report which buffer was being
// processed when the error occurred, and its HRESULT code.
void AkSinkXAudio::OnVoiceError( void* pBufferContext, HRESULT Error )
{
	//Not used
}
