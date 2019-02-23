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

#include "stdafx.h"

#include "DlgConfiguration.h"
#include "Resource.h"

// To use Edit and ComboBox macros
#include <windowsx.h>

DlgConfiguration::DlgConfiguration( ISourceControlUtilities* in_pUtilities )
	: m_pSourceControlUtilities( in_pUtilities )
{
}

// ISourceControlDialogBase
HINSTANCE DlgConfiguration::GetResourceHandle() const
{
	AFX_MANAGE_STATE( ::AfxGetStaticModuleState() );
	return ::AfxGetResourceHandle();
}

void DlgConfiguration::GetDialog( UINT & out_uiDialogID ) const
{
	out_uiDialogID = IDD_CONFIGURATION;
}

bool DlgConfiguration::Help( HWND in_hWnd ) const
{
	return false;
}

bool DlgConfiguration::HasHelp() const
{
	return false;
}

bool DlgConfiguration::WindowProc( HWND in_hWnd, UINT in_message, WPARAM in_wParam, LPARAM in_lParam, LRESULT & out_lResult )
{
	bool bStopMsg = false;

	if ( in_message == WM_INITDIALOG )
	{
		bStopMsg = OnInitDialog( in_hWnd );
	}

	// E.g. Catch window command actions (only for the main dialog) to enable/disable controls
	else if ( in_message == WM_COMMAND )
	{
		// Notification code
		switch ( HIWORD( in_wParam ) )
		{
		case BN_CLICKED:
			// Check which button was clicked
			switch ( LOWORD( in_wParam ) )
			{
			case IDOK:
				// The user pressed the 'Ok' button
				bStopMsg = OnOK( in_hWnd );
				break;
			}           
		} // End switch hi word (notification code)

	} // End command window event

	// Return false to let the parent window deal with the message. Return true
	// for message you don't want the parent window to handle.

	return bStopMsg;
}

// Local functions
bool DlgConfiguration::OnInitDialog( HWND in_hWnd )
{
	AFX_MANAGE_STATE( ::AfxGetStaticModuleState() );

	CString csText;

	// Fill the combo box
	HWND hCombo = ::GetDlgItem( in_hWnd, IDC_COMBO );

	csText = _T("Combo item 1");
	ComboBox_InsertString( hCombo, 0, csText );
	
	csText = _T("Combo item 2");
	ComboBox_InsertString( hCombo, 1, csText );

	ComboBox_SetCurSel( hCombo, 1 );

	// Set the Caption text
	HWND hCaption = ::GetDlgItem( in_hWnd, IDC_CAPTION );

	csText = _T("Caption text");
	Edit_SetText( hCaption, csText ); 

	return false;
}

bool DlgConfiguration::OnOK( HWND in_hWnd )
{
	AFX_MANAGE_STATE( ::AfxGetStaticModuleState() );

	// Show a simple message box
	CString csMessage;
	CString csCaption;
	csMessage = _T("OK button pressed");
	csCaption = _T("Configuration");

	m_pSourceControlUtilities->MessageBox( in_hWnd, csMessage, csCaption, MB_OK );

	return false;
}