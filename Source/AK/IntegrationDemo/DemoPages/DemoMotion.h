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

// DemoMotion.h
/// \file
/// Contains the declaration for the DemoMotion class.

#pragma once

#include <AK/SoundEngine/Common/AkTypes.h>
#include "MultiplayerPage.h"
#include "IntegrationDemo.h"

/// Class representing the Motion Demo page. This page demonstrates how to set up force 
/// feedback events through the Wwise Sound Engine and Motion Engine.
class DemoMotion : public MultiplayerPage
{
public:

	/// DemoMotion class constructor
	DemoMotion( Menu& in_ParentMenu );

	/// Initializes the demo.
	/// \return True if successful and False otherwise.
	virtual bool Init();

	/// Releases resources used by the demo.
	virtual void Release();
	virtual void Draw();

private:

	/// Initializes the controls on the page.
	virtual void InitControls();

	/// Called when a player connects.
	/// Method declared Pure Virtual in MultiplayerPage.h
	virtual void OnPlayerConnect( int in_iPlayerIndex );

	/// Called when a player disconnects.
	/// Method declared Pure Virtual in MultiplayerPage.h
	virtual void OnPlayerDisconnect( int in_iPlayerIndex );

	/// Delegate function for the "Door Slide" buttons.
	void DoorSlide_Pressed( void* in_pSender, ControlEvent* in_pEvent );

	/// Delegate function for the "Shoot Gun" buttons.
	void GunShot_Pressed( void* in_pSender, ControlEvent* in_pEvent );	

#ifdef AK_NX
	void DualSingle_Pressed(void* in_pSender, ControlEvent* in_pEvent);
#endif

	/// ID for the 'door game object.
	static const AkGameObjectID GAME_OBJECT_DOOR = 100;

	/// IDs for the players as game objects.
	static const AkGameObjectID OBJ_FOR_PLAYER[MAX_PLAYERS];		
	AkOutputDeviceID m_PlayerAudioOutput[MAX_PLAYERS];
	AkOutputDeviceID m_PlayerMotionOutput[MAX_PLAYERS];
};
