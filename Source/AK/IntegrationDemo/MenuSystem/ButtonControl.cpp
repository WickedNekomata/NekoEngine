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

// ButtonControl.cpp
/// \file 
/// Defines the methods declared in ButtonControl.h

#include "stdafx.h"

#include "ButtonControl.h"


/////////////////////////////////////////////////////////////////////
// DemoMotion Public Methods
/////////////////////////////////////////////////////////////////////

ButtonControl::ButtonControl( Page& in_pParentPage ):Control( in_pParentPage )
{
	m_szLabel = "[Button]";
}

void ButtonControl::Update( const UniversalGamepad& in_Gamepad )
{
	if ( in_Gamepad.IsButtonTriggered( UG_BUTTON1 ) )
	{
		// Set up the event and call the delegate
		ControlEvent* pEvent = new ControlEvent;
		pEvent->iPlayerIndex = in_Gamepad.GetPlayerIndex();
		CallDelegate( pEvent );
		delete pEvent;
	}
}

void ButtonControl::Draw( DrawStyle in_eDrawStyle )
{
	DrawTextOnScreen( m_szLabel.c_str(), m_iXPos, m_iYPos, in_eDrawStyle );
}

void ButtonControl::OnFocus()
{
	// No implementation
}

void ButtonControl::OnLoseFocus()
{
	// No implementation
}

bool ButtonControl::OnPointerEvent( PointerEventType in_eType, int, int )
{
	if ( in_eType == PointerEventType_Pressed )
	{
		// Set up the event and call the delegate
		ControlEvent* pEvent = new ControlEvent;
		pEvent->iPlayerIndex = 0;
		CallDelegate( pEvent );
		delete pEvent;
	}

	return true;
}
