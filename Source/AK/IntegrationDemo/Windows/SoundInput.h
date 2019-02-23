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

#pragma once

#pragma warning( push )
#pragma warning( disable: 4201 )
#include <Mmsystem.h>
#pragma warning( pop )

#include "SoundInputBase.h"
#include <AK/Tools/Common/AkPlatformFuncs.h>
#include <AK/Tools/Common/AkLock.h>
#include <queue>

// The window size of the sample input

#ifdef AK_LOW_LATENCY
	#define AUDIO_INPUT_NUM_BUFFERS 8
	#define AUDIO_INPUT_BUFFER_SIZE 256
	#define NUM_TARGET_FREE_BUFFERS 5
#else
	#define AUDIO_INPUT_NUM_BUFFERS 8
	#define AUDIO_INPUT_BUFFER_SIZE 1024
	#define NUM_TARGET_FREE_BUFFERS (AUDIO_INPUT_NUM_BUFFERS/2)
#endif

class SoundInput : public SoundInputBase
{
public:
    SoundInput();
    virtual ~SoundInput();

	static SoundInput& Instance()
	{
		return ms_Instance;
	}

	virtual bool InputOn( unsigned int in_DevNumber = 0 );		// Start input recording
	virtual bool InputOff();

	virtual void Execute( AkAudioBuffer* io_pBufferOut );
	virtual void GetFormatCallback( AkAudioFormat&  io_AudioFormat );

private:

	BOOL SetInputDevice (unsigned int inDev);	// Mic. or Line

	void SendBufferRequest( DWORD in_bufferIndex );
	void ExecuteBufferRequest( DWORD in_bufferIndex );

	// Refil thread
	static AK_DECLARE_THREAD_ROUTINE( RefillThreadFunc );
	AKRESULT StartRefillThread();
	void StopRefillThread();

	static SoundInput ms_Instance;

	AkThread	m_RefillThread;
	AkEvent		m_eventRefill;
	bool		m_bStopRefillThread;
	CAkLock		m_refillLock;

	std::queue<DWORD> m_refillQueue;

	DWORD m_CurrentInputBufferIndex; // Current Transfer buffer index.
	AkUInt16 m_RemainingSize; // Offset where to read in the currently being read index buffer.
	void MoveCurrentIndex();
	AkUInt16 GetFullBufferSize();

	HWAVEIN			m_WaveInHandle;
	WAVEHDR			m_WaveHeader[ AUDIO_INPUT_NUM_BUFFERS ];    
	WAVEFORMATEX	m_WaveFormat;

	// WG-20421
	// SoundInput will sleep in destructor only when successfully initialized.
	// This should be fixed in a better way in future.
	bool			m_bSuccessfullyInitOnceAndMustSleepOnExit;

	AkUInt32		m_idDevice;
};
