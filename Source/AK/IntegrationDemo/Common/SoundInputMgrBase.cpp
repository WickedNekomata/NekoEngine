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

#include "stdafx.h"
#include "SoundInputMgrBase.h"
#include "SoundInputMgr.h"
#include <AK/Plugin/AkAudioInputPlugin.h>

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// SoundInputMgrBase implementation
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

bool SoundInputMgrBase::Initialize()
{
	// Reset members.
	Reset();
	// Set Wwise required Callbacks
	SetAudioInputCallbacks( ExecuteCallback, GetFormatCallback );

	// Cannot fail for now, return true.
    return true;
}

void SoundInputMgrBase::Term()
{
}

void SoundInputMgrBase::GetFormatCallback(
        AkPlayingID		in_playingID,   // Playing ID
        AkAudioFormat&  io_AudioFormat  // Already filled format, modify it if required.
	    )
{
	SoundInputBase* pInput = SoundInputMgr::Instance().GetInput( in_playingID ) ;
	if( pInput )
	{
		pInput->GetFormatCallback( io_AudioFormat );
	}
}

void SoundInputMgrBase::ExecuteCallback(
        AkPlayingID		in_playingID,  // Playing ID
        AkAudioBuffer*	io_pBufferOut  // Buffer to fill
	    )
{
    io_pBufferOut->uValidFrames = 0;

    SoundInputBase* pInput = SoundInputMgr::Instance().GetInput( in_playingID ) ;
    if( pInput == NULL )
    {
        // No Input... fake starvation, we are waiting the user to connect in the microphone.
		io_pBufferOut->eState = AK_NoDataReady;
        io_pBufferOut->uValidFrames = 0;
		return;
    }

    // Execute will fill io_pBufferOut->uValidFrames with the correct value.
    pInput->Execute( io_pBufferOut );

    if( io_pBufferOut->uValidFrames > 0 )
        io_pBufferOut->eState = AK_DataReady;
    else
        io_pBufferOut->eState = AK_NoDataReady;
}

SoundInputBase* SoundInputMgrBase::GetInput( AkPlayingID in_playingID )
{
	for ( int i = 0; i < MAX_NUM_SOUNDINPUT; ++i )
	{
		if ( m_DeviceInputAssoc[i].pInput && m_DeviceInputAssoc[i].pInput->GetPlayingID() == in_playingID )
		{
			return m_DeviceInputAssoc[i].pInput;
		}
	}
#ifdef AK_WIN
	// Default to find one, any...
	// A real game should not adopt this behavior, this should only be done for playback in Wwise.
	for ( int i = 0; i < MAX_NUM_SOUNDINPUT; ++i )
	{
		if ( m_DeviceInputAssoc[i].pInput )
		{
			return m_DeviceInputAssoc[i].pInput;
		}
	}
#endif //WIN32
	return NULL;
}

void SoundInputMgrBase::Reset()
{
	for ( int i = 0; i < MAX_NUM_SOUNDINPUT; ++i )
	{
		m_DeviceInputAssoc[i].idDevice = INVALID_DEVICE_ID;
		m_DeviceInputAssoc[i].pInput = NULL;
	}

	m_MicCount = 0;
}

void SoundInputMgrBase::ClearDevices()
{
	Reset();
}

bool SoundInputMgrBase::RegisterDevice( SoundInputDevID in_idDevice )
{
	int idxFree = ~0;

	for ( int i = 0; i < MAX_NUM_SOUNDINPUT; ++i )
	{
		if ( m_DeviceInputAssoc[i].idDevice == INVALID_DEVICE_ID && idxFree == ~0 )
			idxFree = i;
		else if ( m_DeviceInputAssoc[i].idDevice == in_idDevice )
			return false;
	}

	if ( idxFree != ~0 )
	{
		AKASSERT( m_DeviceInputAssoc[ idxFree ].pInput == NULL );
		m_DeviceInputAssoc[ idxFree ].idDevice = in_idDevice;
		m_DeviceInputAssoc[ idxFree ].pInput = NULL;
		m_MicCount++;
		return true;
	}

	//printf("Cannot register more than MAX_NUM_SOUNDINPUT microphone devices simultaneously\n");
	return false;
}

void SoundInputMgrBase::UnregisterDevice( SoundInputDevID in_idDevice )
{
	for( int i = 0; i < MAX_NUM_SOUNDINPUT; ++i )
	{
		if( m_DeviceInputAssoc[i].idDevice == in_idDevice )
		{
			//AKASSERT( m_DeviceInputAssoc[i].pInput == NULL );
			m_DeviceInputAssoc[i].idDevice = INVALID_DEVICE_ID;
			m_DeviceInputAssoc[i].pInput = NULL;
			m_MicCount = ( m_MicCount > 0 ) ? m_MicCount - 1 : 0;
			break;
		}
	}
}

bool SoundInputMgrBase::RegisterInput( SoundInputBase* in_pSoundInput, SoundInputDevID in_idDevice )
{
	for ( int i = 0; i < MAX_NUM_SOUNDINPUT; ++i )
	{
		if ( m_DeviceInputAssoc[i].idDevice == in_idDevice )
		{
			if ( m_DeviceInputAssoc[i].pInput )
				return false;

			m_DeviceInputAssoc[i].pInput = in_pSoundInput;
			return true;
		}
	}

	//printf("Cannot register more than MAX_NUM_SOUNDINPUT microphone devices simultaneously\n");
	return false;
}

void SoundInputMgrBase::UnregisterInput( SoundInputBase* in_pSoundInput, SoundInputDevID in_idDevice )
{
	(void) in_idDevice;

	for( int i = 0; i < MAX_NUM_SOUNDINPUT; ++i )
	{
		if( m_DeviceInputAssoc[i].pInput == in_pSoundInput )
		{
			AKASSERT( m_DeviceInputAssoc[i].idDevice == in_idDevice );
			m_DeviceInputAssoc[i].pInput = NULL;
			break;
		}
	}
}
