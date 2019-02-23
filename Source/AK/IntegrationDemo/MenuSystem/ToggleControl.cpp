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

// ToggleControl.cpp
/// \file 
/// Defines the methods declared in ToggleControl.h

#include "stdafx.h"

#include "ToggleControl.h"


ToggleControl::ToggleControl( Page& in_pParentPage ):Control( in_pParentPage )
{
	m_szLabel = "[Toggle]";
	m_iSelectedIndex = 0;
}

ToggleControl::~ToggleControl()
{
	m_Options.clear();
}

void ToggleControl::Update( const UniversalGamepad& in_Gamepad )
{
	// Check if there is more than one option available to the user
	if ( m_Options.size() > 1 )
	{
		if ( in_Gamepad.IsButtonTriggered( UG_DPAD_LEFT ) )
		{
			// User wants to go left
			--m_iSelectedIndex;
			if ( m_iSelectedIndex < 0 )
			{
				m_iSelectedIndex = (int)m_Options.size() - 1;
			}

			// Set up the event and call the delegate
			ControlEvent* pEvent = new ControlEvent;
			pEvent->iPlayerIndex = in_Gamepad.GetPlayerIndex();
			CallDelegate( pEvent );
			delete pEvent;
		}
		else if ( in_Gamepad.IsButtonTriggered( UG_DPAD_RIGHT ) )
		{
			// User wants to go right
			++m_iSelectedIndex;
			if ( m_iSelectedIndex >= (int)m_Options.size() )
			{
				m_iSelectedIndex = 0;
			}

			// Set up the event and call the delegate
			ControlEvent* pEvent = new ControlEvent;
			pEvent->iPlayerIndex = in_Gamepad.GetPlayerIndex();
			CallDelegate( pEvent );
			delete pEvent;
		}
	}
}

void ToggleControl::Draw( DrawStyle in_eDrawStyle )
{
	string szDisplayText = m_szLabel;

	if ( m_Options.size() )
	{
		if ( in_eDrawStyle == DrawStyle_Selected && m_Options.size() > 1 )
		{
			// Multiple options exist and the control is selected
			szDisplayText += " < " + m_Options[m_iSelectedIndex].text + " >";
		}
		else
		{
			// Either the control is not the selected control or only a single option exists
			szDisplayText += "  " + m_Options[m_iSelectedIndex].text;
		}
	}
	else
	{
		// No options exist to choose from
		szDisplayText += "  [No Options]";
	}

	DrawTextOnScreen( szDisplayText.c_str(), m_iXPos, m_iYPos, in_eDrawStyle );
}

void ToggleControl::OnFocus()
{
	// No implementation
}

void ToggleControl::OnLoseFocus()
{
	// No implementation
}

void ToggleControl::AddOption( const char* in_szText, void* in_pValue )
{
	OptionNode newNode;
	
	newNode.text = in_szText;
	newNode.value = in_pValue;
	m_Options.push_back( newNode );
}

int ToggleControl::SelectedIndex() const
{
	return m_iSelectedIndex;
}

const char* ToggleControl::SelectedText() const
{
	return ( m_Options.size() > 0 ) ? m_Options[m_iSelectedIndex].text.c_str() : "";
}

void* ToggleControl::SelectedValue() const
{
	return ( m_Options.size() > 0 ) ? m_Options[m_iSelectedIndex].value : NULL;
}
