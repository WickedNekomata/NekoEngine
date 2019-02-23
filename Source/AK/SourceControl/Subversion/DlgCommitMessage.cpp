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
////////////////////////////////////////////////////////////////////////
// DlgCommitMessage.cpp
//
// Implementation file for the CDlgCommitMessage dialog, used to let the user
// enter a Check In description
//
///////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include ".\DlgCommitMessage.h"
#include "resource.h"

#include <windowsx.h>

// Constructor
CDlgCommitMessage::CDlgCommitMessage( ISourceControlUtilities* in_pUtilities )
	: m_pUtilities( in_pUtilities )
	, m_bKeepLocks( false )
{
}

// Destructor
CDlgCommitMessage::~CDlgCommitMessage()
{
}

void CDlgCommitMessage::OnInitDialog( HWND in_hWnd )
{
	::SetFocus( ::GetDlgItem( in_hWnd, IDC_MESSAGE ) );
}

bool CDlgCommitMessage::OnBnClickedOk( HWND in_hWnd )
{
	bool bReturn = false;

	// Get the description
	CEdit* pControlMessage = (CEdit*)CWnd::FromHandle( ::GetDlgItem( in_hWnd, IDC_MESSAGE ) );

	pControlMessage->GetWindowText( m_csMessage );

	HWND hKeepLocks = ::GetDlgItem( in_hWnd, IDC_CHECK_KEEP_LOCKS );
	m_bKeepLocks = Button_GetCheck( hKeepLocks ) ? true : false;

	return bReturn;
}

const CString& CDlgCommitMessage::GetMessage() const
{
	return m_csMessage;
}

const bool CDlgCommitMessage::KeepLocks() const
{
	return m_bKeepLocks;
}

// ISourceControlDialogBase
HINSTANCE CDlgCommitMessage::GetResourceHandle() const
{    
	AFX_MANAGE_STATE( ::AfxGetStaticModuleState() );
	return ::AfxGetResourceHandle();
}

void CDlgCommitMessage::GetDialog( UINT & out_uiDialogID ) const
{
	out_uiDialogID = IDD_COMMIT_MESSAGE;
}

bool CDlgCommitMessage::HasHelp() const
{
	return false;
}

bool CDlgCommitMessage::Help( HWND in_hWnd ) const
{
	return false;
}

bool CDlgCommitMessage::WindowProc( HWND in_hWnd, UINT in_message, WPARAM in_wParam, LPARAM in_lParam, LRESULT & out_lResult )
{
	bool bReturn = false;

    if ( in_message == WM_INITDIALOG )
    {
		OnInitDialog( in_hWnd );
    }
    else if ( in_message == WM_COMMAND )
    {
        // Notification code
        switch ( HIWORD( in_wParam ) )
        {
			case BN_CLICKED:
				switch ( LOWORD( in_wParam ) )
				{
					case IDOK:
						bReturn = OnBnClickedOk( in_hWnd );
						break;
				}           
        }
    }

    return bReturn;
}