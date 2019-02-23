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

// Control.cpp
/// \file 
/// Defines the methods declared in Control.h


#include "stdafx.h"

#include "Control.h"


/////////////////////////////////////////////////////////////////////
// Control Public Methods
/////////////////////////////////////////////////////////////////////

Control::Control( Page& in_pParentPage )
{
	m_pParentPage = &in_pParentPage;
	m_szLabel = "";
	m_pDelegateFunc = NULL;	
	m_iXPos = 0;
	m_iYPos = 0;
}

void Control::SetDelegate( PageMFP in_pDelegateFunction )
{
	m_pDelegateFunc = in_pDelegateFunction;
}

void Control::CallDelegate( ControlEvent* in_pEvent ) const
{
	if ( m_pParentPage && m_pDelegateFunc )
	{
		(m_pParentPage->*m_pDelegateFunc)( (void*)this, in_pEvent );
	}
}

void Control::SetLabel( const char* in_szLabel )
{
	m_szLabel = in_szLabel;
}

void Control::SetPosition( int in_iXPos, int in_iYPos )
{
	m_iXPos = in_iXPos;
	m_iYPos = in_iYPos;
}
