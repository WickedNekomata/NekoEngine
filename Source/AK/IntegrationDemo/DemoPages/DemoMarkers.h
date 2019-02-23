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

// DemoMarkers.h
/// \file
/// Contains the declaration for the DemoMarkers class.

#pragma once

#include <AK/SoundEngine/Common/AkTypes.h>
#include <AK/SoundEngine/Common/AkCallback.h>
#include "Page.h"
#include <string>

/// Class representing the Markers Demo page. This page demonstrates how to use the markers
/// in a wav file to synchronize events in the game with the audio being played.
class DemoMarkers : public Page
{
public:

	/// DemoMarkers class constructor
	DemoMarkers( Menu& in_ParentMenu );

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

	/// Delegate function for the "Play Markers" button.
	void PlayMarkersButton_Pressed( void* in_pSender, ControlEvent* in_pEvent );

	/// Callback method for the events raised by Wwise while playing the markers demo audio.
	static void MarkersCallback( 
		AkCallbackType in_eType,			///< - The type of the callback
		AkCallbackInfo* in_pCallbackInfo	///< - Structure containing info about the callback
		);

	/// Tracks the current line during audio playback to sync audio with captions.
	unsigned int m_unCurrentLine;

	/// Whether the audio is currently playing or not.
	bool m_bPlayingMarkers;

	/// Holds the playing ID of the launched PLAY_MARKERS event.
	AkPlayingID m_iPlayingID;

	/// The number of lines of text to display in the captions.
	static const int NUM_LINES = 7;

	/// The caption text, one extra for a blank entry.
	static const char* SUBTITLES[NUM_LINES + 1];

	/// The caption text, wrapped to platform-specific max width,
	/// with one extra for a blank entry.
	std::string m_subtitles[NUM_LINES + 1];

	/// Game Object ID for the "Narrator".
	static const AkGameObjectID GAME_OBJECT_NARRATOR = 100;
};
