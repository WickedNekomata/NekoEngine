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

// Demo3dBus.cpp
/// \file
/// Defines all methods declared in Demo3dBus.h

#include "stdafx.h"

#include <math.h>
#include "Menu.h"
#include "MovableChip.h"
#include "Demo3dBus.h"
#include <AK/SoundEngine/Common/AkSoundEngine.h>    // Sound engine

//If you get a compiling error here, it means the file wasn't generated with the banks.  Did you generate the soundbanks before compiling?
#include "../WwiseProject/GeneratedSoundBanks/Wwise_IDs.h"		

//Our game object ID.  Completely arbitrary.
#define EMITTER_OBJ 100
#define ROOM_1_OBJ 101
#define ROOM_2_OBJ 102
#define LISTENER_OBJ 103
#define REPEAT_TIME 20

/////////////////////////////////////////////////////////////////////
// Demo3dBus Public Methods
/////////////////////////////////////////////////////////////////////

Demo3dBus::Demo3dBus(Menu& in_ParentMenu, Demo3dBus_Scenario in_scenario)
	: Page( in_ParentMenu, "3d Bus Demo")
	, m_pEmitterChip( NULL )
	, m_pListenerChip(NULL)
	, m_pRoom1Chip(NULL)
	, m_pRoom2Chip(NULL)
	, m_fGameObjectX( 0 )
	, m_fGameObjectZ( 0 )
	, m_fWidth( 0.0f )
	, m_fHeight( 0.0f )
	, m_bLooping(false)
	, m_uRepeat(0)
	, m_eScenario(in_scenario)

{
}

void Demo3dBus::Init_CoupledRoomsSimple()
{
	AK::SoundEngine::RegisterGameObj(EMITTER_OBJ, "Emitter");
	AK::SoundEngine::RegisterGameObj(ROOM_1_OBJ, "Portal");
	AK::SoundEngine::RegisterGameObj(LISTENER_OBJ, "Listener");

	// Set up direct path from EMITTER_OBJ -> LISTENER_OBJ
	static const int kNumLstnrsForEm = 1;
	static const AkGameObjectID aLstnrsForEmitter[kNumLstnrsForEm] = { LISTENER_OBJ };
	AK::SoundEngine::SetListeners(EMITTER_OBJ, aLstnrsForEmitter, kNumLstnrsForEm);

	// Set up direct path from ROOM_1_OBJ -> LISTENER_OBJ
	static const int kNumLstnrsForRm = 1;
	static const AkGameObjectID aLstnrsForRooms[kNumLstnrsForRm] = { LISTENER_OBJ };
	AK::SoundEngine::SetListeners(ROOM_1_OBJ, aLstnrsForRooms, kNumLstnrsForRm);

	AkAuxSendValue auxSends[2];

	// Indirect path: 
	//	ROOM_1_OBJ sends to <ListenerEnv,LISTENER_OBJ>
	auxSends[0].auxBusID = AK::SoundEngine::GetIDFromString("ListenerEnv");
	auxSends[0].fControlValue = 1.f;
	auxSends[0].listenerID = LISTENER_OBJ;
	AK::SoundEngine::SetGameObjectAuxSendValues(ROOM_1_OBJ, auxSends, 1);

	// Indirect path: 
	//	EMITTER_OBJ sends to <ListenerRoom,LISTENER_OBJ> 
	auxSends[0].auxBusID = AK::SoundEngine::GetIDFromString("ListenerEnv");
	auxSends[0].fControlValue = 1.f;
	auxSends[0].listenerID = LISTENER_OBJ;
	
	//	EMITTER_OBJ sends to <Room1,ROOM_1_OBJ> 
	auxSends[1].auxBusID = AK::SoundEngine::GetIDFromString("Room1");
	auxSends[1].fControlValue = 1.f;
	auxSends[1].listenerID = ROOM_1_OBJ;
	AK::SoundEngine::SetGameObjectAuxSendValues(EMITTER_OBJ, auxSends, 2);

	m_szHelp =
		"*** Please connect Wwise and open the Advanced Profiler to the Voice Graph.\n"
		"In this demo we have 3 game objects: \n"
		"* 'Emitter/[E]' - the position from which the sound originates.\n"
		"* 'Listener/[L]' - the end point, namely the main character or camera in a game. "
		"Has an aux bus instance named 'ListenerEnv', which simulates the room that the listener is currently inside. \n"
		"* 'Portal/[Room]' - the physical position of a window/door. "
		"Has a 3D-bus chain, 'Room1' -> 'Wet_Path_3D', simulating a room that the listener is not inside. The bus applies a "
		"reverb Effect, after which the output is positioned and spatialized, before being mixed at the"
		"Master Audio Bus. \n"
		"Controls: \n"
		"<<UG_RIGHT_STICK>> + <<DIRECTIONAL_TYPE>> - Move the emitter game object.\n"
		"<<UG_BUTTON5>> + <<UG_RIGHT_STICK>> + <<DIRECTIONAL_TYPE>> - Move the listener game object.\n"
		"<<UG_BUTTON3>> - Play/Stop looping sound.\n"
		"<<UG_BUTTON1>> - Play chirp sound.";
}

void Demo3dBus::Init_CoupledRoomsWithFeedback()
{
	AK::SoundEngine::RegisterGameObj(EMITTER_OBJ, "Emitter");
	AK::SoundEngine::RegisterGameObj(ROOM_1_OBJ, "Room_1");
	AK::SoundEngine::RegisterGameObj(ROOM_2_OBJ, "Room_2");
	AK::SoundEngine::RegisterGameObj(LISTENER_OBJ, "Listener");

	static const int kNumLstnrsForEm = 1;
	static const AkGameObjectID aLstnrsForEmitter[kNumLstnrsForEm] = { LISTENER_OBJ };
	AK::SoundEngine::SetListeners(EMITTER_OBJ, aLstnrsForEmitter, kNumLstnrsForEm);

	static const int kNumAuxEm = 2;
	AkAuxSendValue auxSend[kNumAuxEm];
	auxSend[0].auxBusID = AK::SoundEngine::GetIDFromString("Room1");
	auxSend[0].fControlValue = 1.f;
	auxSend[0].listenerID = ROOM_1_OBJ;
	auxSend[1].auxBusID = AK::SoundEngine::GetIDFromString("Room2");
	auxSend[1].fControlValue = 1.f;
	auxSend[1].listenerID = ROOM_2_OBJ;
	AK::SoundEngine::SetGameObjectAuxSendValues(EMITTER_OBJ, auxSend, kNumAuxEm);

	static const int kNumLstnrsForRm = 1;
	static const AkGameObjectID aLstnrsForRooms[kNumLstnrsForRm] = { LISTENER_OBJ };
	AK::SoundEngine::SetListeners(ROOM_1_OBJ, aLstnrsForRooms, kNumLstnrsForRm);
	AK::SoundEngine::SetListeners(ROOM_2_OBJ, aLstnrsForRooms, kNumLstnrsForRm);

	{
		AkAuxSendValue _auxSend;

		//Room1 sends to Room2
		_auxSend.auxBusID = AK::SoundEngine::GetIDFromString("Room2");
		_auxSend.fControlValue = 1.f;
		_auxSend.listenerID = ROOM_2_OBJ;
		AK::SoundEngine::SetGameObjectAuxSendValues(ROOM_1_OBJ, &_auxSend, 1);

		//Room2 sends to Room1
		_auxSend.auxBusID = AK::SoundEngine::GetIDFromString("Room1");
		_auxSend.fControlValue = 1.f;
		_auxSend.listenerID = ROOM_1_OBJ;
		AK::SoundEngine::SetGameObjectAuxSendValues(ROOM_2_OBJ, &_auxSend, 1);
	}

	m_szHelp = 
		"*** Please connect Wwise and open the Advanced Profiler to the Voice Graph.\n"
		"In this demo we have 4 game objects: \n"
		"* 'Emitter/[E]' - the position from which the sound originates.\n"
		"* 'Listener/[L]' - the end point, ie. the main character or camera in a game.\n"

		"* 'Room1/[R1]' and 'Room2/[R2]' - 2 different rooms that the listener is not inside. "
		"Depending on the positions of the game objects, the rooms may be close enough to be excited by the output from [E]. "
		"The physical positions of [R1]/[R2] each simulate the position of a window/door, and each game object has "
		"an instance of a 3D-bus chain - 'RoomX' -> 'Wet_Path_3D'. Each bus applies a "
		"reverb Effect, after which the output is positioned and spatialized before being mixed at the"
		"Master Audio Bus. \n"

		"Controls: \n"
		"<<UG_RIGHT_STICK>> + <<DIRECTIONAL_TYPE>> - Move the emitter game object.\n"
		"<<UG_BUTTON5>> + <<UG_RIGHT_STICK>> + <<DIRECTIONAL_TYPE>> - Move the listener game object.\n"
		"<<UG_BUTTON3>> - Play/Stop looping sound.\n"
		"<<UG_BUTTON1>> - Play chirp sound.";


}

void Demo3dBus::Init_3dSubmix()
{
	AK::SoundEngine::RegisterGameObj(EMITTER_OBJ, "Emitter");
	AK::SoundEngine::RegisterGameObj(LISTENER_OBJ, "Listener");

	static const int kNumLstnrsForEm = 1;
	static const AkGameObjectID aLstnrsForEmitter[kNumLstnrsForEm] = { LISTENER_OBJ };
	AK::SoundEngine::SetListeners(EMITTER_OBJ, aLstnrsForEmitter, kNumLstnrsForEm);

	m_szHelp =
		"*** Please connect Wwise and open the Advanced Profiler to the Voice Graph. \n"
		"You will notice that there are 3 sounds playing, however the sounds have "
		"'Enable Positioning' un-checked in the actor-mixer-hierarchy.  "
		"In this demo, positioning is only applied in the bus hierarchy.  The bus named '3D-Submix_Bus' has "
		"'Enable Positioning' checked and an attenuation assigned.   The sound engine applies spatialization "
		"only after the 3 sounds are mixed together. \n"
		"Controls: \n"
		"<<UG_RIGHT_STICK>> + <<DIRECTIONAL_TYPE>> - Move the emitter game object.\n"
		"<<UG_BUTTON5>> + <<UG_RIGHT_STICK>> + <<DIRECTIONAL_TYPE>> - Move the listener game object.";
}

bool Demo3dBus::Init()
{
	switch (m_eScenario)
	{
	case Scenario_CoupledRoomsSimple:
		Init_CoupledRoomsSimple();
		break;
	case Scenario_CoupledRoomsWithFeedback:
		Init_CoupledRoomsWithFeedback();
		break;
	case Scenario_3dSubmix:
		Init_3dSubmix();
		break;
	}

	// Load the sound bank
	AkBankID bankID; // Not used
	if ( AK::SoundEngine::LoadBank( "Bus3d_Demo.bnk", AK_DEFAULT_POOL_ID, bankID ) != AK_Success )
	{
		SetLoadFileErrorMessage( "Bus3d_Demo.bnk" );
		return false;
	}

	return Page::Init();
}

void Demo3dBus::Release()
{
	AK::SoundEngine::StopAll();
	AK::SoundEngine::UnregisterGameObj(EMITTER_OBJ);
	AK::SoundEngine::UnregisterGameObj(ROOM_1_OBJ);
	AK::SoundEngine::UnregisterGameObj(ROOM_2_OBJ);
	AK::SoundEngine::UnregisterGameObj(LISTENER_OBJ);
	AK::SoundEngine::UnloadBank( "Bus3d_Demo.bnk", NULL );

	Page::Release();
}

#define HELICOPTER_CLONE_X_OFFSET (50)
#define POSITION_RANGE (200.0f)

float Demo3dBus::PixelsToAKPos_X(float in_X)
{
	return ((in_X / m_fWidth) - 0.5f) * POSITION_RANGE;
}

float Demo3dBus::PixelsToAKPos_Y(float in_y)
{
	return -((in_y / m_fHeight) - 0.5f) * POSITION_RANGE;
}

void Demo3dBus::UpdateGameObjPos(MovableChip* in_pChip, AkGameObjectID in_GameObjectId)
{
	if (in_pChip)
	{
		float x, y;
		in_pChip->GetPos(x, y);

		// Converting X-Y UI into X-Z world plan.
		AkVector position;
		m_fGameObjectX = position.X = PixelsToAKPos_X(x);
		position.Y = 0;
		m_fGameObjectZ = position.Z = PixelsToAKPos_Y(y);
		AkVector orientationFront;
		orientationFront.Z = 1;
		orientationFront.Y = orientationFront.X = 0;
		AkVector orientationTop;
		orientationTop.X = orientationTop.Z = 0;
		orientationTop.Y = 1;

		AkSoundPosition soundPos;
		soundPos.Set(position, orientationFront, orientationTop);
		AK::SoundEngine::SetPosition(in_GameObjectId, soundPos);
	}
	
}

bool Demo3dBus::Update()
{
	//Always update the MovableChip

	bool bMoved = false;
	UniversalInput::Iterator it;
	for ( it = m_pParentMenu->Input()->Begin(); it != m_pParentMenu->Input()->End(); it++ )
	{
		// Skip this input device if it's not connected
		if ( ! it->IsConnected() )
			continue;

		if ((*it).IsButtonDown(UG_BUTTON4))
		{
			if (m_pRoom1Chip)
				m_pRoom1Chip->Update(*it);
		}
		else if ((*it).IsButtonDown(UG_BUTTON6))
		{
			if (m_pRoom2Chip)
				m_pRoom2Chip->Update(*it);
		}
		else if ((*it).IsButtonDown(UG_BUTTON5))
		{
			m_pListenerChip->Update(*it);
		}
		else
		{
			m_pEmitterChip->Update(*it);
		}

		bMoved = true;

		if (m_uRepeat == 0)
		{
			if ((*it).IsButtonDown(UG_BUTTON1))
			{
				AK::SoundEngine::PostEvent("Play_Chirp", EMITTER_OBJ);
				m_uRepeat = REPEAT_TIME;
			}

			if ((*it).IsButtonDown(UG_BUTTON3))
			{
				if (!m_bLooping)
				{
					if (m_eScenario == Scenario_3dSubmix)
						AK::SoundEngine::PostEvent("Play_Cluster", EMITTER_OBJ);
					else
						AK::SoundEngine::PostEvent("Play_3dBus_Demo", EMITTER_OBJ);
				}
				else
				{
					if (m_eScenario == Scenario_3dSubmix)
						AK::SoundEngine::PostEvent("Stop_Cluster", EMITTER_OBJ);
					else
						AK::SoundEngine::PostEvent("Stop_3dBus_Demo", EMITTER_OBJ);
				}

				m_bLooping = !m_bLooping;
				m_uRepeat = REPEAT_TIME;
			}
		}
		else
		{
			m_uRepeat--;
		}
	}

	if ( bMoved )
	{
		UpdateGameObjPos(m_pEmitterChip, EMITTER_OBJ);
		UpdateGameObjPos(m_pListenerChip, LISTENER_OBJ);
		UpdateGameObjPos(m_pRoom1Chip, ROOM_1_OBJ);
		UpdateGameObjPos(m_pRoom2Chip, ROOM_2_OBJ);
	}

	return Page::Update();
}

void Demo3dBus::Draw()
{
	Page::Draw();

	if (m_pEmitterChip)
		m_pEmitterChip->Draw();
	if (m_pListenerChip)
		m_pListenerChip->Draw();
	if (m_pRoom1Chip)
		m_pRoom1Chip->Draw();
	if (m_pRoom2Chip)
		m_pRoom2Chip->Draw();

	char strBuf[50];
	snprintf( strBuf, 50, "X: %.2f\nZ: %.2f", m_fGameObjectX, m_fGameObjectZ );

	static int s_nOffset = 2 * GetLineHeight( DrawStyle_Text );

	DrawTextOnScreen( strBuf, 5, m_pParentMenu->GetHeight() - s_nOffset, DrawStyle_Text );

	// Display instructions at the bottom of the page
	int iInstructionsY = m_pParentMenu->GetHeight() - 3 * GetLineHeight( DrawStyle_Text );
	DrawTextOnScreen( "(Press <<UG_BUTTON2>> To Go Back...)", m_pParentMenu->GetWidth() / 4, iInstructionsY, DrawStyle_Text );
}

bool Demo3dBus::OnPointerEvent( PointerEventType in_eType, int in_x, int in_y )
{
	if ( in_eType == PointerEventType_Moved )
	{
		bool bMoveListener = false;
		for (UniversalInput::Iterator it = m_pParentMenu->Input()->Begin(); it != m_pParentMenu->Input()->End(); it++)
		{
			if ((*it).IsButtonDown(UG_BUTTON5))
			{
				bMoveListener = true;
				break;
			}
		}

		if (bMoveListener)
			m_pListenerChip->SetPos((float)in_x, (float)in_y);
		else
			m_pEmitterChip->SetPos((float)in_x, (float)in_y);

		UpdateGameObjPos(m_pEmitterChip, EMITTER_OBJ);
		UpdateGameObjPos(m_pListenerChip, LISTENER_OBJ);
	}

	return Page::OnPointerEvent( in_eType, in_x, in_y );
}

void Demo3dBus::InitControls_CoupledRoomsSimple()
{
	float fMargine = (float)m_pEmitterChip->GetRightBottomMargin();

	float x = (m_pParentMenu->GetWidth() - fMargine) * 1.f / 2.0f;
	float y = (m_pParentMenu->GetHeight() - fMargine) * 1.f / 4.0f;

	m_pRoom1Chip = new MovableChip(*this);
	m_pRoom1Chip->SetLabel("[Room]");
	m_pRoom1Chip->UseJoystick(UG_STICKRIGHT);
	m_pRoom1Chip->SetPos(x-30, y);
	m_pRoom1Chip->Update(*(m_pParentMenu->Input()->Begin()));
	UpdateGameObjPos(m_pRoom1Chip, ROOM_1_OBJ);
}

void Demo3dBus::InitControls_CoupledRoomsWithFeedback()
{
	float fMargine = (float)m_pEmitterChip->GetRightBottomMargin();
	{
		float x = (m_pParentMenu->GetWidth() - fMargine) * 1.f / 4.0f;
		float y = (m_pParentMenu->GetHeight() - fMargine) * 1.f / 4.0f;

		m_pRoom1Chip = new MovableChip(*this);
		m_pRoom1Chip->SetLabel("[R1]");
		m_pRoom1Chip->UseJoystick(UG_STICKRIGHT);
		m_pRoom1Chip->SetPos(x, y);
		m_pRoom1Chip->Update(*(m_pParentMenu->Input()->Begin()));
		UpdateGameObjPos(m_pRoom1Chip, ROOM_1_OBJ);
	}
	{
		float x = (m_pParentMenu->GetWidth() - fMargine) * 3.f / 4.0f;
		float y = (m_pParentMenu->GetHeight() - fMargine) * 3.f / 4.0f;

		m_pRoom2Chip = new MovableChip(*this);
		m_pRoom2Chip->SetLabel("[R2]");
		m_pRoom2Chip->UseJoystick(UG_STICKRIGHT);
		m_pRoom2Chip->SetPos(x, y);
		m_pRoom2Chip->Update(*(m_pParentMenu->Input()->Begin()));
		UpdateGameObjPos(m_pRoom2Chip, ROOM_2_OBJ);
	}
}

void Demo3dBus::InitControls()
{
	float fMargine = (float)m_pEmitterChip->GetRightBottomMargin();
	float x = (m_pParentMenu->GetWidth() - fMargine) * 1.f / 2.0f;
	float y = (m_pParentMenu->GetHeight() - fMargine) * 1.f / 2.0f;

	m_pEmitterChip = new MovableChip(*this);
	m_pEmitterChip->SetLabel( "<E>" );
	m_pEmitterChip->UseJoystick(UG_STICKRIGHT);
	m_pEmitterChip->SetPos(x - 30, y);
	m_pEmitterChip->SetNonLinear();

	m_pListenerChip = new MovableChip(*this);
	m_pListenerChip->SetLabel("<L>");
	m_pListenerChip->UseJoystick(UG_STICKRIGHT);
	m_pListenerChip->SetNonLinear();
	m_pListenerChip->SetPos(x + 30, y);
	m_pListenerChip->Update(*(m_pParentMenu->Input()->Begin()));

	m_fWidth = (float)m_pParentMenu->GetWidth() - fMargine;
	m_fHeight = (float)m_pParentMenu->GetHeight() - fMargine;
	
	switch (m_eScenario)
	{
	case Scenario_CoupledRoomsSimple:
		InitControls_CoupledRoomsSimple();
		AK::SoundEngine::PostEvent("Play_3dBus_Demo", EMITTER_OBJ);
		m_bLooping = true;
		break;
	case Scenario_CoupledRoomsWithFeedback:
		InitControls_CoupledRoomsWithFeedback();
		AK::SoundEngine::PostEvent("Play_3dBus_Demo", EMITTER_OBJ);
		m_bLooping = true;
		break;
	case Scenario_3dSubmix:
		AK::SoundEngine::PostEvent("Play_Cluster", EMITTER_OBJ);
		m_bLooping = true;
		break;
	}

}
