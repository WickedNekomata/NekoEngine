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

#include "SoundInputBase.h"

#define MAX_NUM_SOUNDINPUT		4

#define INVALID_DEVICE_ID		0

typedef AkUInt32 SoundInputDevID;

// Forward definitions.
class SoundInput;
struct AkAudioFormat;
class AkAudioBuffer;


class SoundInputMgrBase
{
public:
	SoundInputMgrBase() {m_MicCount = 0;}

	struct DeviceToInputAssoc
	{
		SoundInputBase* pInput;
		SoundInputDevID idDevice;
	};

	virtual bool Initialize();
	virtual void Term();

	virtual AkUInt32 GetMicCount() { return m_MicCount; }
	virtual SoundInputDevID GetMicDeviceID(AkUInt32 in_index) { return m_DeviceInputAssoc[in_index].idDevice; }

	bool RegisterInput( SoundInputBase* in_pSoundInput, SoundInputDevID in_idDevice );
	void UnregisterInput( SoundInputBase* in_pSoundInput, SoundInputDevID in_idDevice );

protected:
	virtual ~SoundInputMgrBase() {}

	// Register detected devices (via platform API)
	void ClearDevices();
	bool RegisterDevice( SoundInputDevID in_idDevice );
	void UnregisterDevice( SoundInputDevID in_idDevice );

	// Callbacks that will be passed to Wwise.
	static void GetFormatCallback(
        AkPlayingID		/*in_playingID*/,   // Playing ID
        AkAudioFormat&  /*io_AudioFormat*/  // Already filled format, modify it if required.
	    );

	static void ExecuteCallback(
        AkPlayingID		in_playingID,  // Playing ID
        AkAudioBuffer*	io_pBufferOut  // Buffer to fill
	    );

	void Reset();
	SoundInputBase* GetInput( AkPlayingID in_playingID );

	DeviceToInputAssoc m_DeviceInputAssoc[ MAX_NUM_SOUNDINPUT ];

	AkUInt32 m_MicCount;
};
