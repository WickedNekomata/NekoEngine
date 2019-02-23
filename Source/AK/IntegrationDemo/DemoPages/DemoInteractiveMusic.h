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
/// Contains the declaration for the DemoInteractiveMusic class.

#pragma once

#include <AK/SoundEngine/Common/AkTypes.h>
#include "Page.h"

//If you get a compiling error here, it means the file wasn't generated with the banks.  Did you generate the soundbanks before compiling?
#include "../WwiseProject/GeneratedSoundBanks/Wwise_IDs.h"		

#define POST_EVENT_DECLARE(x) void PostEvent_## x ( void*, ControlEvent* )\
{\
	AK::SoundEngine::PostEvent( AK::EVENTS::x, GAME_OBJECT_MUSIC );\
}\

#define POST_EVENT(x) PostEvent_## x 

/// Class representing the Music Callbacks Demo page. This page demonstrates how to use the music
/// callabacks in the interactive music system.
class DemoInteractiveMusic : public Page
{
public:
	/// DemoInteractiveMusic class constructor
	DemoInteractiveMusic( Menu& in_ParentMenu );

	/// Initializes the demo.
	/// \return True if successful and False otherwise.
	virtual bool Init();

	/// Releases resources used by the demo.
	virtual void Release();

	virtual void Draw();

private:
	/// Initializes the controls on the page.
	virtual void InitControls();

	// Declaring delagate functions that simply call AK::SoundEngine::PostEvent on GAME_OBJECT_MUSIC
	POST_EVENT_DECLARE( IM_EXPLORE )
	POST_EVENT_DECLARE( IM_COMMUNICATION_BEGIN )
	POST_EVENT_DECLARE( IM_THEYAREHOSTILE )
	POST_EVENT_DECLARE( IM_1_ONE_ENEMY_WANTS_TO_FIGHT )
	POST_EVENT_DECLARE( IM_2_TWO_ENEMIES_WANT_TO_FIGHT )
	POST_EVENT_DECLARE( IM_3_SURRONDED_BY_ENEMIES )
	POST_EVENT_DECLARE( IM_4_DEATH_IS_COMING )
	POST_EVENT_DECLARE( IM_WINTHEFIGHT )
	POST_EVENT_DECLARE( IM_GAMEOVER )

	/// Game Object ID for the Music.
	static const AkGameObjectID GAME_OBJECT_MUSIC = 100;

	/// Holds the playing ID of the launched PLAY_MARKERS event.
	AkPlayingID m_iPlayingID;
};
