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

// DemoPositioning.cpp
/// \file
/// Defines all methods declared in DemoPositioning.h

#include "stdafx.h"

#include <math.h>
#include "Menu.h"
#include "MovableChip.h"
#include "DemoPositioning.h"
#include <AK/SoundEngine/Common/AkSoundEngine.h>    // Sound engine

//If you get a compiling error here, it means the file wasn't generated with the banks.  Did you generate the soundbanks before compiling?
#include "../WwiseProject/GeneratedSoundBanks/Wwise_IDs.h"		

//Our game object ID.  Completely arbitrary.
#define GAME_OBJECT_POSTEST 100

/////////////////////////////////////////////////////////////////////
// DemoPositioning Public Methods
/////////////////////////////////////////////////////////////////////

DemoPositioning::DemoPositioning(Menu& in_ParentMenu, bool in_bIsMultiPosition)
	: Page( in_ParentMenu, "Positioning Demo")
	, m_pChip( NULL )
	, m_pCloneChip( NULL )
	, m_fGameObjectX( 0 )
	, m_fGameObjectZ( 0 )
	, m_fWidth( 0.0f )
	, m_fHeight( 0.0f )
	, m_bMultiposition(in_bIsMultiPosition)

{
	m_szHelp =  "This demo shows how to do 3D positioning in "
				"Wwise. A helicopter sound starts playing as soon "
				"as you enter the page. Move the 'o' around "
                "in X and Z, i.e., the plane of the screen, using the "
				"following keys: <<UG_RIGHT_STICK>> <<DIRECTIONAL_TYPE>>,"
				"and hear the sound move along with it. "
				"Coordinates are displayed at the bottom-left of the screen.";
}

bool DemoPositioning::Init()
{
	// Register the "Human" game object
	AK::SoundEngine::RegisterGameObj( GAME_OBJECT_POSTEST, "Helicopter" );

	// Load the sound bank
	AkBankID bankID; // Not used
	if ( AK::SoundEngine::LoadBank( "Positioning_Demo.bnk", AK_DEFAULT_POOL_ID, bankID ) != AK_Success )
	{
		SetLoadFileErrorMessage( "Positioning_Demo.bnk" );
		return false;
	}

	AK::SoundEngine::PostEvent( "Play_Positioning_Demo", GAME_OBJECT_POSTEST );

	return Page::Init();
}

void DemoPositioning::Release()
{
	AK::SoundEngine::StopAll();
	AK::SoundEngine::UnregisterGameObj( GAME_OBJECT_POSTEST );
	AK::SoundEngine::UnloadBank( "Positioning_Demo.bnk", NULL );

	Page::Release();
}

#define HELICOPTER_CLONE_X_OFFSET (50)
#define POSITION_RANGE (200.0f)

float DemoPositioning::PixelsToAKPos_X(float in_X)
{
	return ((in_X / m_fWidth) - 0.5f) * POSITION_RANGE;
}

float DemoPositioning::PixelsToAKPos_Y(float in_y)
{
	return -((in_y / m_fHeight) - 0.5f) * POSITION_RANGE;
}

void DemoPositioning::UpdateGameObjPos()
{
	float x, y;
	m_pChip->GetPos(x, y);
	
	// Set 3D position
	if (m_bMultiposition)
	{
		// Two helicopters in one sound.
		const int numMultiPositions = 2;

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

		AkSoundPosition soundPos[numMultiPositions];
		AkSoundPosition& firstPos = soundPos[0];
		firstPos.Set(position, orientationFront, orientationTop);

		// Make the second position of the second Helicopter to always be HELICOPTER_CLONE_X_OFFSET Distance units offset from main Helicopter.
		position = firstPos.Position();
		position.X += HELICOPTER_CLONE_X_OFFSET;
		AkSoundPosition& secondPos = soundPos[1];
		secondPos.Set(position, orientationFront, orientationTop);

		// Update the Second Chip position on screen.
		m_pCloneChip->SetPosition(int(x + (HELICOPTER_CLONE_X_OFFSET*m_fWidth / POSITION_RANGE)), (int)y);

		AK::SoundEngine::SetMultiplePositions(GAME_OBJECT_POSTEST, soundPos, numMultiPositions);
	}
	else
	{
		// Single Helicopter.

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
		AK::SoundEngine::SetPosition(GAME_OBJECT_POSTEST, soundPos);
	}
}

bool DemoPositioning::Update()
{
	//Always update the MovableChip

	bool bMoved = false;
	UniversalInput::Iterator it;
	for ( it = m_pParentMenu->Input()->Begin(); it != m_pParentMenu->Input()->End(); it++ )
	{
		// Skip this input device if it's not connected
		if ( ! it->IsConnected() )
			continue;

		m_pChip->Update(*it);

		bMoved = true;
	}

	if ( bMoved )
	{
		UpdateGameObjPos();
	}

	return Page::Update();
}

void DemoPositioning::Draw()
{
	Page::Draw();

	m_pChip->Draw();
	if (m_bMultiposition)
	{
		m_pCloneChip->Draw();
	}

	char strBuf[50];
	snprintf( strBuf, 50, "X: %.2f\nZ: %.2f", m_fGameObjectX, m_fGameObjectZ );

	static int s_nOffset = 2 * GetLineHeight( DrawStyle_Text );

	DrawTextOnScreen( strBuf, 5, m_pParentMenu->GetHeight() - s_nOffset, DrawStyle_Text );

	// Display instructions at the bottom of the page
	int iInstructionsY = m_pParentMenu->GetHeight() - 3 * GetLineHeight( DrawStyle_Text );
	DrawTextOnScreen( "(Press <<UG_BUTTON2>> To Go Back...)", m_pParentMenu->GetWidth() / 4, iInstructionsY, DrawStyle_Text );
}

bool DemoPositioning::OnPointerEvent( PointerEventType in_eType, int in_x, int in_y )
{
	if ( in_eType == PointerEventType_Moved )
	{
		m_pChip->SetPos( (float) in_x, (float) in_y );
		UpdateGameObjPos();
	}

	return Page::OnPointerEvent( in_eType, in_x, in_y );
}

void DemoPositioning::InitControls()
{
	m_pChip = new MovableChip(*this);
	m_pChip->SetLabel( "o" );
	m_pChip->UseJoystick(UG_STICKRIGHT);
	m_pChip->SetNonLinear();

	if (m_bMultiposition)
	{
		m_pChip->SetLabel("0");
		m_pCloneChip = new MovableChip(*this);
		m_pCloneChip->SetLabel("1");
	}

	m_fWidth = (float)m_pParentMenu->GetWidth() - (float)m_pChip->GetRightBottomMargin();
	m_fHeight = (float)m_pParentMenu->GetHeight() - (float)m_pChip->GetRightBottomMargin();
}

