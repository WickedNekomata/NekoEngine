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
// DlgPerforceDescription.cpp
//
// Implementation file for the CDlgPerforceDescription dialog, used to let the user
// enter a Check In description
//
///////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include ".\DlgPerforceDescription.h"
#include "resource.h"

#include <windowsx.h>

// Constructor
CDlgPerforceDescription::CDlgPerforceDescription( ISourceControlUtilities* in_pUtilities )
	: m_pUtilities( in_pUtilities )
	, m_bCheckOutAfterSubmit( false )
{
}

// Destructor
CDlgPerforceDescription::~CDlgPerforceDescription()
{
}

void CDlgPerforceDescription::OnInitDialog( HWND in_hWnd )
{
	::SetFocus( ::GetDlgItem( in_hWnd, IDC_DESCRIPTION ) );
}

bool CDlgPerforceDescription::OnBnClickedOk( HWND in_hWnd )
{
	bool bReturn = false;

	// Get the description
	CEdit* pControlDescription = (CEdit*)CWnd::FromHandle( ::GetDlgItem( in_hWnd, IDC_DESCRIPTION ) );

	pControlDescription->GetWindowText( m_csDescription );

	if ( m_csDescription.IsEmpty() )
	{
		AFX_MANAGE_STATE( AfxGetStaticModuleState() );

		CString csMessage;
		CString csCaption;

		csMessage= _T("The description is invalid");
		csCaption= _T("Perforce plug-in");

		m_pUtilities->MessageBox( in_hWnd, csMessage, csCaption, MB_OK );

		bReturn = true;
	}

	HWND hCheckOut = ::GetDlgItem( in_hWnd, IDC_CHECK_OUT_AFTER_SUBMIT );
	m_bCheckOutAfterSubmit = Button_GetCheck( hCheckOut ) ? true : false;

	return bReturn;
}

const CString& CDlgPerforceDescription::GetDescription() const
{
	return m_csDescription;
}

bool CDlgPerforceDescription::CheckOutAfterSubmit() const
{
	return m_bCheckOutAfterSubmit;
}

// ISourceControlDialogBase
HINSTANCE CDlgPerforceDescription::GetResourceHandle() const
{    
	AFX_MANAGE_STATE( ::AfxGetStaticModuleState() );
	return ::AfxGetResourceHandle();
}

void CDlgPerforceDescription::GetDialog( UINT & out_uiDialogID ) const
{
	out_uiDialogID = IDD_PERFORCE_DESCRIPTION;
}

bool CDlgPerforceDescription::HasHelp() const
{
	return false;
}

bool CDlgPerforceDescription::Help( HWND in_hWnd ) const
{
	return false;
}

bool CDlgPerforceDescription::WindowProc( HWND in_hWnd, UINT in_message, WPARAM in_wParam, LPARAM in_lParam, LRESULT & out_lResult )
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