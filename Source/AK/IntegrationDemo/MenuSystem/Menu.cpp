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

// Menu.cpp
/// \file 
/// Defines the methods declared in Menu.h

#include "stdafx.h"

#include "Menu.h"
#include "MessagePage.h"



/////////////////////////////////////////////////////////////////////
// Menu Public Methods
/////////////////////////////////////////////////////////////////////

Menu::Menu()
{
	m_iTickCount = 0;
	m_pInput = NULL;
	m_bQuitApp = false;
}

Menu::~Menu()
{
	Release();
}

void Menu::Init( UniversalInput* in_pInput, int in_iWidth, int in_iHeight, int in_iFrameRate, Page& in_BasePage )
{
	m_pInput = in_pInput;
	m_iMenuWidth = in_iWidth;
	m_iMenuHeight = in_iHeight;
	m_iFrameRate = in_iFrameRate;
	StackPage( &in_BasePage );
}

void Menu::Release()
{
	while ( ! m_PageStack.empty() )
	{
		PopPageStack();
	}
}

void Menu::Back()
{
	if ( m_PageStack.empty() )
		return;

	// The Page wants to close.. remove it from the stack.
	Page* p = m_PageStack.top();
	m_PageStack.pop();

	// Check if the Page closed due to an error.
	if ( p->ErrorOccured() )
	{
		MessagePage* pErrorPage = new MessagePage( *this );
		pErrorPage->SetTitle( "An Error Occurred" );
		pErrorPage->SetMessage( p->RetrieveErrorMsg(), DrawStyle_Error );
		StackPage( pErrorPage );
	}

	p->Release();
}

bool Menu::Update()
{
	++m_iTickCount;

	if ( m_PageStack.size() > 0 )
	{
		// Call the Page's update..
		if ( ! m_PageStack.top()->Update() )
		{
			Back();
		}
	}
	else
	{
		// Nothing left on the stack, quit the app
		QuitApplication();
	}

	return !m_bQuitApp;
}

void Menu::OnPointerEvent( PointerEventType in_eType, int in_x, int in_y )
{
	if ( m_PageStack.size() > 0 )
	{
		bool bStay = m_PageStack.top()->OnPointerEvent( in_eType, in_x, in_y );
		if ( !bStay )
		{
			Back();
		}
	}
		
}

void Menu::Draw()
{
	BeginDrawing();
	if ( m_PageStack.size() > 0 )
	{
		m_PageStack.top()->Draw();
	}
	DoneDrawing();
}

void Menu::QuitApplication()
{
	m_bQuitApp = true;
}

UniversalInput* Menu::Input() const
{
	return m_pInput;
}

int Menu::GetFrameRate() const
{
	return m_iFrameRate;
}

int Menu::GetHeight() const
{
	return m_iMenuHeight;
}

int Menu::GetTickCount() const
{
	return m_iTickCount;
}

int Menu::GetWidth() const
{
	return m_iMenuWidth;
}

void Menu::StackPage( Page* in_pPage )
{
	// Initialize the page.
	if ( in_pPage->Init() )
	{
		// Initialization succeeded, stack the page.
		m_PageStack.push( in_pPage );
	}
	else
	{
		// Error occured during initialization, show an error page instead.
		MessagePage* pErrorPage = new MessagePage( *this );
		pErrorPage->SetTitle( "An Error Occurred" );
		pErrorPage->SetMessage( in_pPage->RetrieveErrorMsg(), DrawStyle_Error );
		StackPage( pErrorPage );
		
		// Release the page that had the error
		in_pPage->Release();
	}
}

void Menu::PopPageStack()
{
	Page* pg = m_PageStack.top();
	pg->Release();
	m_PageStack.pop();
}
