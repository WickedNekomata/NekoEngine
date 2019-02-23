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

#include <AK/SoundEngine/Common/AkTypes.h>
#include <AK/SoundEngine/Common/IAkPlugin.h>


// Forward definition
struct AkAudioFormat;
class AkAudioBuffer;

class SoundInputBase
{
public:
	
	virtual bool InputOn( unsigned int in_DevNumber = 0 )  = 0;		// Start input recording
	virtual bool InputOff() = 0;									// Stop input recording

	// For callback purpose.
	virtual void Execute( AkAudioBuffer* io_pBufferOut ) = 0;
	virtual void GetFormatCallback( AkAudioFormat& io_AudioFormat ) = 0;

	void SetPlayingID( AkPlayingID in_playingID );
	AkPlayingID GetPlayingID();

	SoundInputBase();
    virtual ~SoundInputBase();

private:
	AkPlayingID m_playingID;
};
