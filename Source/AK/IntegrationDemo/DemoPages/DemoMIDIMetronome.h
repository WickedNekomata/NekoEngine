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

// DemoMIDIMetronome.h
/// \file
/// Contains the declaration for the DemoMIDIMetronome class.

#pragma once

#include <AK/SoundEngine/Common/AkTypes.h>
#include <AK/Tools/Common/AkLock.h>
#include "Page.h"


/// Class representing the RTPC Demo page. This page demonstrates how to use real-time parameter controls
/// to modify the audio being played. In this example, an "RPM" RTPC is set up to modify the sound of an
/// engine,
class DemoMIDIMetronome : public Page
{
public:

	/// DemoMIDIMetronome class constructor
	DemoMIDIMetronome( Menu& in_ParentMenu );

	/// Initializes the demo.
	/// \return True if successful and False otherwise.
	virtual bool Init();

	/// Releases resources used by the demo.
	virtual void Release();

	/// Registered global callback function.
	static void StaticCallback(AK::IAkGlobalPluginContext * in_pContext, AkGlobalCallbackLocation in_eLocation, void * in_pCookie);

private:

	/// Object callback function; MUST only be called by static callback function.
	void ObjectCallback();

private:

	/// Initializes the controls on the page.
	virtual void InitControls();

	/// Delegate function for the "Start/Stop Metronome" button.
	void PlayMetronomeButton_Pressed( void* in_pSender, ControlEvent* in_pEvent );

	/// Delegate function for the "BPM" numeric control.
	void BPMNumeric_ValueChanged( void* in_pSender, ControlEvent* in_pEvent );

	// Callback function management
	bool PrepareCallback();
	void ReleaseCallback();

	// Post MIDI events
	void PostMIDIEvents( AkUInt32 in_uSampleOffset );

	// Stop posted MIDI events
	void StopMIDIPosts();

	/// Whether or not the metronome is currently playing.
	bool m_bPlayingMetronome;

	/// Game Object ID for the "Metronome".
	static const AkGameObjectID GAME_OBJECT_METRONOME = 100;

	static DemoMIDIMetronome* m_pCallbackObj;
	static CAkLock m_lockCallback;

	AkUniqueID m_eventID;

	AkUInt32 m_uPostCntr;
	AkUInt32 m_uPostLenSamples;
	AkUInt32 m_uSamplesPerCallback;
	AkUInt32 m_uCallbackCntr;
	AkReal64 m_dblMsPerCallback;
	AkReal64 m_dblInterPostTimeMs;
	AkReal64 m_dblNextPostTimeMs;
};
