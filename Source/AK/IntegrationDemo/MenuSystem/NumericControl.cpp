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

// NumericControl.cpp
/// \file 
/// Defines the methods declared in NumericControl.h

#include "stdafx.h"
#include "NumericControl.h"
#include "Page.h"
#include "Menu.h"
using namespace std;


/////////////////////////////////////////////////////////////////////
// NumericControl Public Methods
/////////////////////////////////////////////////////////////////////

NumericControl::NumericControl( Page& in_pParentPage ):Control( in_pParentPage )
{
	m_szLabel = "[Numeric]";
	m_iPlayerInControl = -1;
	m_dblIncrement = 0;
	m_dblInitialIncrement = 0;
	m_dblMax = 0;
	m_dblMin = 0;
	m_dblValue = 0;
	m_iHTime = 500;
	m_iHTimeLeft = 0;
	m_bInHysteresis = false;
}

void NumericControl::Update( const UniversalGamepad& in_Gamepad )
{
	if ( m_iPlayerInControl != -1 && m_iPlayerInControl != in_Gamepad.GetPlayerIndex() )
	{
		// This is not the user who currently has control over this control.. don't update
		return;
	}

	if ( in_Gamepad.IsButtonDown( UG_DPAD_LEFT ) && m_dblValue > m_dblMin )
	{
		// Give this player control over this control
		m_iPlayerInControl = in_Gamepad.GetPlayerIndex();

		if ( m_iHTimeLeft )
		{
			--m_iHTimeLeft;
		}
		else
		{
			// "Left" button pressed, user wants to decrease the value
			
			if ( ! m_bInHysteresis )
			{
				// The user just triggered the slider, transition into hysteresis..
				CalcHysteresisTimeLeft();
				m_bInHysteresis = true;
				// Decrease the value by the regular decrement
				m_dblValue -= m_dblInitialIncrement;
			}
			else
			{
				// Decrease the value by the regular decrement
				m_dblValue -= m_dblIncrement;
			}

			// Make sure not to go below the lower bound.
			m_dblValue = max( m_dblValue, m_dblMin );

			// Set up the event and call the delegate
			ControlEvent* pEvent = new ControlEvent;
			pEvent->iPlayerIndex = in_Gamepad.GetPlayerIndex();
			CallDelegate( pEvent );
			delete pEvent;
		}
	}
	else if ( in_Gamepad.IsButtonDown( UG_DPAD_RIGHT ) && m_dblValue < m_dblMax )
	{
		// Give this player control over this control
		m_iPlayerInControl = in_Gamepad.GetPlayerIndex();

		if ( m_iHTimeLeft )
		{
			--m_iHTimeLeft;
		}
		else
		{
			// "Right" button pressed, user wants to increase the value
			
			if ( ! m_bInHysteresis )
			{
				// The user just triggered the slider, transition into hysteresis..
				CalcHysteresisTimeLeft();
				m_bInHysteresis = true;

				// Increase the value by the initial increment
				m_dblValue += m_dblInitialIncrement;
			}
			else
			{
				// Increase the value by the regular increment
				m_dblValue += m_dblIncrement;
			}
			
			// Make sure not to go above the top bound.
			m_dblValue = min( m_dblValue, m_dblMax );

			// Set up the event and call the delegate
			ControlEvent* pEvent = new ControlEvent;
			pEvent->iPlayerIndex = in_Gamepad.GetPlayerIndex();
			CallDelegate( pEvent );
			delete pEvent;
		}
	}
	else 
	{
		m_iPlayerInControl = -1;
		m_iHTimeLeft = 0;
		m_bInHysteresis = false;
	}
}

void NumericControl::Draw( DrawStyle in_eDrawStyle )
{
	char szConvBuffer[50];			// Temporary buffer to convert numeric values
	string szDisplayText = m_szLabel;	// String to build the text displayed on screen
	
	// Check for decimal precision
	if ( m_dblValue != (int)m_dblValue )
	{
		// Show to 2 decimal places
		snprintf( szConvBuffer, 50, "%.2f", m_dblValue );
	}
	else
	{
		snprintf( szConvBuffer, 50, "%d", (int)m_dblValue );
	}

	if ( in_eDrawStyle == DrawStyle_Selected )
	{
		if ( m_dblMin == m_dblMax )
		{
			// The control is set with a range of 1 value, add no arrows
			szDisplayText += "  ";
			szDisplayText += szConvBuffer;
		}
		else if ( m_dblValue <= m_dblMin )
		{
			// The control's value is at the minimum, only add right arrow
			szDisplayText += "   ";
			szDisplayText += szConvBuffer;
			szDisplayText += " >";
		}
		else if ( m_dblValue >= m_dblMax )
		{
			// The control's value is at the maximum, only add left arrow
			szDisplayText += " < ";
			szDisplayText += szConvBuffer;
		}
		else
		{
			// The control's value is somewhere in the middle, add both arrows
			szDisplayText += " < ";
			szDisplayText += szConvBuffer;
			szDisplayText += " >";
		}
	}
	else
	{
		szDisplayText += " ";
		szDisplayText += szConvBuffer;
	}
	
	DrawTextOnScreen( szDisplayText.c_str(), m_iXPos, m_iYPos, in_eDrawStyle );
}

bool NumericControl::OnPointerEvent( PointerEventType in_eType, int in_x, int )
{
	static int x = -1;

	if ( in_eType == PointerEventType_Pressed )
	{
		x = in_x;
	} 
	else if ( in_eType == PointerEventType_Moved )
	{
		if ( x != -1 )
		{
			int xdelta = in_x - x;
			SetValue( m_dblValue + m_dblIncrement * xdelta );
			x = in_x;

			// Set up the event and call the delegate
			ControlEvent* pEvent = new ControlEvent;
			pEvent->iPlayerIndex = 0;
			CallDelegate( pEvent );
			delete pEvent;
		}
	} 
	else if ( in_eType == PointerEventType_Released )
	{
		x = -1;
	}

	return true;
}

void NumericControl::OnFocus()
{
	// No implementation
}

void NumericControl::OnLoseFocus()
{
	m_iPlayerInControl = -1;
	m_iHTimeLeft = 0;
	m_bInHysteresis = false;
}

AkReal64 NumericControl::GetMaxValue() const
{
	return m_dblMax;
}

AkReal64 NumericControl::GetMinValue() const
{
	return m_dblMin;
}

AkReal64 NumericControl::GetIncrement() const
{
	return m_dblIncrement;
}

AkReal64 NumericControl::GetValue() const
{
	return m_dblValue;
}

void NumericControl::SetMaxValue( AkReal64 in_dblMax )
{
	m_dblMax = in_dblMax;

	// If the control's value is out of the new range, adjust it.
	m_dblValue = min( m_dblValue, m_dblMax );
}

void NumericControl::SetMinValue( AkReal64 in_dblMin )
{
	m_dblMin = in_dblMin;

	// If the control's value is out of the new range, adjust it.
	m_dblValue = max( m_dblValue, m_dblMin );
}

void NumericControl::SetInitialIncrement( AkReal64 in_dblInitialIncrement )
{
	m_dblInitialIncrement = in_dblInitialIncrement;
}

void NumericControl::SetIncrement( AkReal64 in_dblIncrement )
{
	if ( m_dblIncrement == m_dblInitialIncrement )
	{
		m_dblInitialIncrement = in_dblIncrement;
	}
	m_dblIncrement = in_dblIncrement;
}

void NumericControl::SetValue( AkReal64 in_dblVal )
{
	m_dblValue = in_dblVal;

	// If the value set is out of the [min,max] range, adjust it.
	m_dblValue = min( m_dblValue, m_dblMax );
	m_dblValue = max( m_dblValue, m_dblMin );
}

void NumericControl::SetHysteresisTime( AkInt32 in_iHTime )
{
	m_iHTime = in_iHTime;
}


/////////////////////////////////////////////////////////////////////
// NumericControl Private Methods
/////////////////////////////////////////////////////////////////////

void NumericControl::CalcHysteresisTimeLeft()
{
	m_iHTimeLeft = (AkInt32)((float)m_iHTime / 1000 * m_pParentPage->ParentMenu()->GetFrameRate());
}
