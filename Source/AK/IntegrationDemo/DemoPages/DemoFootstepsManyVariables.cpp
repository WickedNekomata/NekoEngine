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

// DemoFootstepsManyVariables.cpp
/// \file
/// Defines all methods declared in DemoFootstepsManyVariables.h

#include "stdafx.h"

#include <math.h>
#include "Menu.h"
#include "MovableChip.h"
#include "Helpers.h"
#include "DemoFootstepsManyVariables.h"
#include "IntegrationDemo.h"
#include <AK/SoundEngine/Common/AkSoundEngine.h>    // Sound engine
#include <string>

//If you get a compiling error here, it means the file wasn't generated with the banks.  Did you generate the soundbanks before compiling?
#include "../WwiseProject/GeneratedSoundBanks/Wwise_IDs.h"		

//Our game object ID.  Completely arbitrary.
#define GAME_OBJECT_HUMAN 10

struct SurfaceInfo
{
	SurfaceInfo( const char* in_szName )
		: strBankFile( BuildBankFileName( in_szName ) )
		, idSwitch( AK::SoundEngine::GetIDFromString( in_szName ) )
	{
	}

	const std::string strBankFile;
	const AkUniqueID idSwitch;	

private:
	SurfaceInfo & operator=( const SurfaceInfo & in_other );

	static std::string BuildBankFileName( const char* in_szName )
	{
		std::string strBankFile( in_szName );
		strBankFile.append( ".bnk" );
		return strBankFile;
	}
};

static const SurfaceInfo k_surfaces[] =
{
	SurfaceInfo( "Dirt" ),
	SurfaceInfo( "Wood" ),
	SurfaceInfo( "Metal" ),
	SurfaceInfo( "Gravel" ),
};

#define DEMOFOOTSTEPS_SURFACE_COUNT	IntegrationDemoHelpers::AK_ARRAYSIZE( k_surfaces )

/////////////////////////////////////////////////////////////////////
// DemoFootstepsManyVariables Public Methods
/////////////////////////////////////////////////////////////////////

DemoFootstepsManyVariables::DemoFootstepsManyVariables( Menu& in_ParentMenu ): Page( in_ParentMenu, "Footsteps with multiple variables")
{
	m_weight = 25;
	m_LastX = 0;
	m_LastY = 0;
	m_maskCurrentBanks = 0;
	m_iSurface = -1;
	m_iLastFootstepTick = m_pParentMenu->GetTickCount();

	m_szHelp =  "This demo shows various ways to deal with footsteps in "
				"Wwise.  It also shows environmental effects usage.\n\n"
				"The screen is divided in 4 surfaces, which correspond "
				"to the Surface switch.  In the middle of the screen, "
				"where all 4 surfaces meet, there is a hangar.  When "
				"entering this zone, the Hangar environmental effect "
				"becomes active.\n\n"
				"To test the footsteps, move the 'o' around with "
				"the <<DIRECTIONAL_TYPE>> or with the right stick. "
				"The displacement of the stick drives the Footstep_Speed RTPC. "
				"You can change the weight with <<UG_BUTTON3>> and <<UG_BUTTON4>> "
				"which drives the Footstep_Weight RTPC."
#if defined( _DEMOFOOTSTEPS_DYNAMIC_BANK_LOADING )
				"\n\nIf you connect the Wwise Profiler, you will also see "
				"that banks are loaded dynamically in this demo."
#endif // defined( _DEMOFOOTSTEPS_DYNAMIC_BANK_LOADING )
				;
}

bool DemoFootstepsManyVariables::Init()
{
	// Register the "Human" game object
	AK::SoundEngine::RegisterGameObj( GAME_OBJECT_HUMAN, "Human" );

	// Load the sound bank
	ManageSurfaces(m_pParentMenu->GetWidth() / 2, m_pParentMenu->GetHeight() / 2, GAME_OBJECT_HUMAN);

	return Page::Init();
}

void DemoFootstepsManyVariables::Release()
{
	AK::SoundEngine::UnregisterGameObj( GAME_OBJECT_HUMAN );
	for ( size_t i = 0; i < DEMOFOOTSTEPS_SURFACE_COUNT; ++i )
	{
		int iBit = 1 << i;
		if ( m_maskCurrentBanks & iBit )
		{
			AK::SoundEngine::UnloadBank( k_surfaces[i].strBankFile.c_str(), NULL );
		}
	}

	Page::Release();
}

#define HANGAR_TRANSITION_ZONE 25.f
#define HANGAR_SIZE 70
#define BUFFER_ZONE 20
#define RUN_SPEED 5.0f					
#define DIST_TO_SPEED (10/RUN_SPEED)	//The RTPC for speed is between 0 and 10.
#define WALK_PERIOD 30

void DemoFootstepsManyVariables::UpdateGameObjPos()
{
	float x, y;
	m_pChip->GetPos(x, y);
		
	//Check on which surface we are.  In this demo, the screen is divided in 4 surfaces.
	ManageSurfaces((int)x, (int)y, GAME_OBJECT_HUMAN);

	//Set the environment ratios for this game object.
	ManageEnvironement((int)x, (int)y, GAME_OBJECT_HUMAN);

	//Compute the speed RTPC
	float dx = x - m_LastX;
	float dy = y - m_LastY;
	float dist = sqrt((float)dx*dx + dy*dy);

	float speed = dist * DIST_TO_SPEED;
	AK::SoundEngine::SetRTPCValue(AK::GAME_PARAMETERS::FOOTSTEP_SPEED, speed, GAME_OBJECT_HUMAN);

	float period = WALK_PERIOD - speed;	//Just to simulate that when running, the steps are faster.  No funky maths here, just a fudge factor.

	//Post the Footstep event if appropriate (if we are moving!)
	if (dist < 0.1f && m_iLastFootstepTick != -1)
	{
		//It stopped.  Play one last footstep.  Make it lighter (half) as if the other foot just came to rest.
		AK::SoundEngine::SetRTPCValue(AK::GAME_PARAMETERS::FOOTSTEP_WEIGHT, m_weight / 2.0f, GAME_OBJECT_HUMAN);
		AK::SoundEngine::PostEvent(AK::EVENTS::PLAY_FOOTSTEPS, GAME_OBJECT_HUMAN);			
		m_iLastFootstepTick = -1;
	}
	else if (dist > 0.1f && m_pParentMenu->GetTickCount() - m_iLastFootstepTick > period)
	{
		//Reset the RTPC to its original value so it has the proper value when starting again.			
		AK::SoundEngine::SetRTPCValue(AK::GAME_PARAMETERS::FOOTSTEP_WEIGHT, m_weight, GAME_OBJECT_HUMAN);
		AK::SoundEngine::PostEvent(AK::EVENTS::PLAY_FOOTSTEPS, GAME_OBJECT_HUMAN);
		m_iLastFootstepTick = m_pParentMenu->GetTickCount();
	}

	m_LastX = x;
	m_LastY = y;
}

bool DemoFootstepsManyVariables::Update()
{
	bool bRedraw = false;

	UniversalInput::Iterator it;
	for ( it = m_pParentMenu->Input()->Begin(); it != m_pParentMenu->Input()->End(); it++ )
	{
		// Skip this input device if it's not connected
		if ( ! it->IsConnected() )
			continue;

		m_pChip->Update(*it);
		
		if( it->IsButtonDown( UG_BUTTON3 ) )
		{
			m_weight += 1.0f;
			if (m_weight > 100.f) 
			{
				m_weight = 100;
			}
		}
		
		if( it->IsButtonDown( UG_BUTTON4 ) )
		{
			m_weight -= 1.0f;
			if (m_weight < 0.f) 
			{
				m_weight = 0;
			}
		}

		bRedraw = true;
	}

	if (bRedraw)
	{
		UpdateGameObjPos();
	}

	return Page::Update();
}

#if defined( _DEMOFOOTSTEPS_DYNAMIC_BANK_LOADING )
int DemoFootstepsManyVariables::ComputeUsedBankMask(int x, int y)
{
	/*
	The screen is divided in 4 sections for the surfaces.
	There is a buffer section between each where the banks of the adjacent sections are loaded
	so the footsteps aren't delayed by bank loading.  So there is a total 9 possible areas to manage.
		LD	   RD
	Dirt | D+W | Wood
	----------------- TD
	D+M  | All | W+G
	----------------- BD
	Metal| M+G | Gravel

	LD = Left Division
	RD = Right Division
	TD = Top Division
	BD = Bottom Division
	*/

	int iHalfWidth = m_pParentMenu->GetWidth() / 2;
	int iHalfHeight = m_pParentMenu->GetHeight() / 2;
	int iBufferZone = (int)BUFFER_ZONE * 2;

	int bLeftDiv = x > iHalfWidth - iBufferZone;
	int bRightDiv = x < iHalfWidth + iBufferZone;
	int bTopDiv = y > iHalfHeight - iBufferZone;
	int bBottomDiv = y < iHalfHeight + iBufferZone;

	int maskBanks = ((bRightDiv & bBottomDiv) << 0)	|	//Is the Dirt bank needed
					((bLeftDiv & bBottomDiv) << 1) |	//Is the Wood bank needed
					((bRightDiv & bTopDiv) << 2) |		//Is the Metal bank needed
					((bLeftDiv & bTopDiv) << 3);		//Is the Gravel bank needed

	return maskBanks;
}
#endif // defined( _DEMOFOOTSTEPS_DYNAMIC_BANK_LOADING )

void DemoFootstepsManyVariables::ManageSurfaces(int x, int y, int in_GameObject)
{
#if defined( _DEMOFOOTSTEPS_DYNAMIC_BANK_LOADING )

	int maskBanks = ComputeUsedBankMask(x, y);

	for(size_t i = 0; i < DEMOFOOTSTEPS_SURFACE_COUNT; i++)
	{
		AkBankID bankID; // Not used
		int iBit = 1 << i;
		if ((maskBanks & iBit) && !(m_maskCurrentBanks & iBit))
		{
			//Load banks asynchronously to avoid blocking the game thread.
			if (AK::SoundEngine::LoadBank(k_surfaces[i].strBankFile.c_str(), NULL, NULL, AK_INVALID_POOL_ID, bankID) != AK_Success)
				maskBanks &= ~iBit;	//This bank could not be loaded.
		}

		//Unload banks asynchronously to avoid blocking the game thread.
		if (!(maskBanks & iBit) && (m_maskCurrentBanks & iBit))
		{
			if (AK::SoundEngine::UnloadBank(k_surfaces[i].strBankFile.c_str(), NULL, NULL, NULL ) != AK_Success)
				maskBanks |= iBit;	//This bank is still loaded
		}
	}

	//Remember which banks we loaded.
	m_maskCurrentBanks = maskBanks;

#else  // defined( _DEMOFOOTSTEPS_DYNAMIC_BANK_LOADING )

	if ( m_maskCurrentBanks == 0 )
	{
		for(int i = 0; i < DEMOFOOTSTEPS_SURFACE_COUNT; i++)
		{
			// Load banks synchronously to make sure they're all available right from the start
			AkBankID bankID; // Not used
			if ( AK::SoundEngine::LoadBank( k_surfaces[i].strBankFile.c_str(), AK_INVALID_POOL_ID, bankID ) == AK_Success )
				m_maskCurrentBanks |= 1 << i; // Remember which banks we loaded.
		}
	}

#endif // defined( _DEMOFOOTSTEPS_DYNAMIC_BANK_LOADING )

	//Find which surface we are actually walking on.
	int iHalfWidth = m_pParentMenu->GetWidth() / 2;
	int iHalfHeight = m_pParentMenu->GetHeight() / 2;
	int indexSurface = (x > iHalfWidth) | ((y > iHalfHeight) << 1);
	if (indexSurface != m_iSurface)
	{
		AK::SoundEngine::SetSwitch(AK::SWITCHES::SURFACE::GROUP, k_surfaces[indexSurface].idSwitch, in_GameObject);
		m_iSurface = indexSurface;
	}
}

void DemoFootstepsManyVariables::ManageEnvironement(int x, int y, int )
{
	AkAuxSendValue aHangarEnv;
	aHangarEnv.auxBusID = AK::SoundEngine::GetIDFromString( "Hangar_Env" );
	aHangarEnv.fControlValue = 0.f;

	//There is a hangar in the middle of the screen with a transition zone around it where 
	//the walker is still outside but starts to hear the effects of the hangar.
	int iHalfWidth = m_pParentMenu->GetWidth() / 2;
	int iHalfHeight = m_pParentMenu->GetHeight() / 2;
	int iDiffX = abs(x - iHalfWidth);
	int iDiffY = abs(y - iHalfHeight);

	//Ramp the environment value in the transition zone.  If the object is outside this zone,
	//the value will be capped anyway.  The result of this ramp is <0 when outside
	//the hangar and >1 when totally inside.
	float fPercentOutsideX = AkMax((iDiffX - HANGAR_SIZE)/HANGAR_TRANSITION_ZONE, 0.0f);
	float fPercentOutsideY = AkMax((iDiffY - HANGAR_SIZE)/HANGAR_TRANSITION_ZONE, 0.0f);

	aHangarEnv.fControlValue = 1.0f - AkMax(fPercentOutsideX, fPercentOutsideY);
	aHangarEnv.fControlValue = AkMax(0.0f, aHangarEnv.fControlValue);
	aHangarEnv.listenerID = LISTENER_ID;

	AK::SoundEngine::SetGameObjectOutputBusVolume(GAME_OBJECT_HUMAN, LISTENER_ID, 1.0f - aHangarEnv.fControlValue / 2.0f);
	AK::SoundEngine::SetGameObjectAuxSendValues( GAME_OBJECT_HUMAN, &aHangarEnv, 1 );
}

void DemoFootstepsManyVariables::Weight_ValueChanged( void* in_pSender, ControlEvent* )
{
	NumericControl* sender = (NumericControl*)in_pSender;
	AK::SoundEngine::SetRTPCValue(AK::GAME_PARAMETERS::FOOTSTEP_WEIGHT, (AkRtpcValue)sender->GetValue(), GAME_OBJECT_HUMAN);	
}

void DemoFootstepsManyVariables::Draw()
{
	//Identify the 4 zones
	const int iTextWidth = 40;	//Approx.
	int iTextHeight = GetLineHeight(DrawStyle_Control);
	int iHalfWidth = m_pParentMenu->GetWidth() / 2;
	int iHalfHeight = m_pParentMenu->GetHeight() / 2;
	DrawTextOnScreen("Dirt", iHalfWidth - BUFFER_ZONE - iTextWidth, iHalfHeight - BUFFER_ZONE - iTextHeight, DrawStyle_Control);
	DrawTextOnScreen("Metal", iHalfWidth - BUFFER_ZONE - iTextWidth, iHalfHeight + BUFFER_ZONE, DrawStyle_Control);
	DrawTextOnScreen("Wood", iHalfWidth + BUFFER_ZONE, iHalfHeight - BUFFER_ZONE - iTextHeight, DrawStyle_Control);
	DrawTextOnScreen("Gravel", iHalfWidth + BUFFER_ZONE, iHalfHeight + BUFFER_ZONE, DrawStyle_Control);
	
	
	char strBuf[50];
	int iPosX = m_pParentMenu->GetWidth() / 10;
	int iPosY = m_pParentMenu->GetHeight() / 5;
	
	snprintf( strBuf, 50, "Weight is: %.2f", m_weight );
	
	// Draw the play position and subtitles
	DrawTextOnScreen( strBuf, iPosX, iPosY, DrawStyle_Text );

	m_pChip->Draw();

	Page::Draw();
}

bool DemoFootstepsManyVariables::OnPointerEvent( PointerEventType in_eType, int in_x, int in_y )
{
	if ( in_eType == PointerEventType_Moved )
	{
		m_pChip->SetPos( (float) in_x, (float) in_y );
		UpdateGameObjPos();
	}

	return Page::OnPointerEvent( in_eType, in_x, in_y );
}

void DemoFootstepsManyVariables::InitControls()
{
	m_pChip = new MovableChip(*this);
	m_pChip->SetLabel( "o" );
	m_pChip->UseJoystick(UG_STICKRIGHT);
	m_pChip->SetMaxSpeed(RUN_SPEED);
	m_pChip->SetNonLinear();
}
