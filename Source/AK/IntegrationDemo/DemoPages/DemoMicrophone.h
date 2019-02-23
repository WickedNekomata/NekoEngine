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

// DemoMicrophone.h
/// \file
/// Contains the declaration for the DemoMicrophone class.

#pragma once

#include <AK/SoundEngine/Common/AkTypes.h>
#include <AK/SoundEngine/Common/AkCallback.h>
#include "Page.h"
#include "SoundInput.h"


/// Class representing the Markers Demo page. This page demonstrates how to use the markers
/// in a wav file to synchronize events in the game with the audio being played.
class DemoMicrophone : public Page
{
public:

	/// DemoMicrophone class constructor
	DemoMicrophone( Menu& in_ParentMenu );

	virtual ~DemoMicrophone();

	/// Initializes the demo.
	/// \return True if successful and False otherwise.
	virtual bool Init();

	/// Releases resources used by the demo.
	virtual void Release();

	/// Override of the Page::Draw() method.
	virtual void Draw();

private:

	/// Initializes the controls on the page.
	virtual void InitControls();
	void PlayMicroButton_Pressed( void* in_pSender, ControlEvent* in_pEvent );
	void DelayButton_Pressed( void* in_pSender, ControlEvent* in_pEvent );
	void StartRecording();
	void StopRecording();

	SoundInput* m_pInput;
	bool m_bPlaying;
	bool m_bDelayed;
};
