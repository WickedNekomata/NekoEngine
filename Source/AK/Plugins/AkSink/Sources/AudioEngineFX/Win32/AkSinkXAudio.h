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
// AkSink.h
//
// Platform dependent part
//
//////////////////////////////////////////////////////////////////////
#pragma once

#include <AK/SoundEngine/Common/AkSoundEngine.h>
#include <AK/SoundEngine/Common/IAkPlugin.h>
#include <AK/Tools/Common/AkLock.h>
#include "xaudio2.h"

#define NUM_VOICE_BUFFERS	(3)

class SinkPluginParams;

class AkSinkXAudio
	: public AK::IAkSinkPlugin
	, public IXAudio2EngineCallback
	, public IXAudio2VoiceCallback
{
public:
	AkSinkXAudio();
	~AkSinkXAudio();

	//
	// AK::IAkSinkPlugin implementation.
	//
	virtual AKRESULT Init( 
		AK::IAkPluginMemAlloc *	in_pAllocator,				// Interface to memory allocator to be used by the effect.
		AK::IAkSinkPluginContext *	in_pSinkPluginContext,	// Interface to sink plug-in's context.
		AK::IAkPluginParam *		in_pParams,				// Interface to plug-in parameters.
		AkAudioFormat &			in_rFormat					// Audio data format of the input signal. 
		);
	virtual AKRESULT Term( 
		AK::IAkPluginMemAlloc * in_pAllocator 	// UNUSED interface to memory allocator to be used by the plug-in
		);
	virtual AKRESULT Reset();
	virtual AKRESULT GetPluginInfo( 
		AkPluginInfo & out_rPluginInfo	// Reference to the plug-in information structure to be retrieved
		);
	virtual void Consume(
		AkAudioBuffer *			in_pInputBuffer,		///< Input audio buffer data structure. Plugins should avoid processing data in-place.
		AkRamp					in_gain					///< Volume gain to apply to this input (prev corresponds to the beginning, next corresponds to the end of the buffer).
		);
	virtual void OnFrameEnd();
	virtual bool IsStarved();
	virtual void ResetStarved();
	virtual AKRESULT IsDataNeeded( AkUInt32 & out_uBuffersNeeded );
	
	//
	//	IXAudio2EngineCallback interface implementation
	//

	// Called by XAudio2 just before an audio processing pass begins.
    STDMETHOD_(void, OnProcessingPassStart) (THIS);

    // Called just after an audio processing pass ends.
    STDMETHOD_(void, OnProcessingPassEnd) (THIS);

    // Called in the event of a critical system error which requires XAudio2
    // to be closed down and restarted.  The error code is given in Error.
    STDMETHOD_(void, OnCriticalError) (THIS_ HRESULT Error);

	//=================================================
	//	IXAudio2VoiceCallback interface implementation
	//=================================================
	
	// Called just before this voice's processing pass begins.
    STDMETHOD_(void, OnVoiceProcessingPassStart) (THIS_ UINT32 BytesRequired);

    // Called just after this voice's processing pass ends.
    STDMETHOD_(void, OnVoiceProcessingPassEnd) (THIS);

    // Called when this voice has just finished playing a buffer stream
    // (as marked with the XAUDIO2_END_OF_STREAM flag on the last buffer).
    STDMETHOD_(void, OnStreamEnd) (THIS);

    // Called when this voice is about to start processing a new buffer.
    STDMETHOD_(void, OnBufferStart) (THIS_ void* pBufferContext);

    // Called when this voice has just finished processing a buffer.
    // The buffer can now be reused or destroyed.
    STDMETHOD_(void, OnBufferEnd) (THIS_ void* pBufferContext);

    // Called when this voice has just reached the end position of a loop.
    STDMETHOD_(void, OnLoopEnd) (THIS_ void* pBufferContext);

    // Called in the event of a critical error during voice processing,
    // such as a failing XAPO or an error from the hardware XMA decoder.
    // The voice may have to be destroyed and re-created to recover from
    // the error.  The callback arguments report which buffer was being
    // processed when the error occurred, and its HRESULT code.
    STDMETHOD_(void, OnVoiceError) (THIS_ void* pBufferContext, HRESULT Error);


protected:
	void SubmitPacketRB();

	AK::IAkSinkPluginContext *	m_pSinkPluginContext;
	SinkPluginParams *		m_pParams;
	
	AkChannelConfig			m_speakerConfig;

	// XAudio source voice
	IXAudio2*				m_pXAudio2;
	IXAudio2MasteringVoice* m_pMasteringVoice;
	IXAudio2SourceVoice*	m_pSourceVoice;

	// Ring buffer
	void *					m_pvAudioBuffer;
	AkUInt16				m_uWriteBufferIndex;	// Ring buffer write index: this member belongs to the EventMgr thread
	AkUInt16				m_uReadBufferIndex;		// Ring buffer read index: this member belong to the XAudio2 callback thread
	void *					m_ppvRingBuffer[NUM_VOICE_BUFFERS];	// Ring buffer
	LONG volatile			m_uNbBuffersRB;			// Number of full buffers in ring buffer.
	AkUInt16				m_usBlockAlign;	

	// status
	bool					m_bStarved;
	bool					m_bCriticalError;

	bool					m_bDataReady;
};

