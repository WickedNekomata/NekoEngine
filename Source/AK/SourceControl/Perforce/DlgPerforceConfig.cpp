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
// DlgPerforceConfig.cpp
//
// Implementation file for the CDlgPerforceConfig dialog, used to let the user
// enter a Check In description
//
///////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include ".\DlgPerforceConfig.h"
#include "Perforce.h"
#include "resource.h"

#include <atlbase.h>
#include <windowsx.h>

// Constructor
CDlgPerforceConfig::CDlgPerforceConfig( ISourceControlUtilities* in_pUtilities )
	: m_pUtilities( in_pUtilities )
{
}

// Destructor
CDlgPerforceConfig::~CDlgPerforceConfig()
{
}

void CDlgPerforceConfig::OnInitDialog( HWND in_hWnd )
{
	::SetFocus( ::GetDlgItem( in_hWnd, IDC_EDIT_CLIENT ) );

	ClientApi client;
	PerforceSourceControl::InitClientFromRegistry( client, m_pUtilities );

	SetDlgItemText( in_hWnd, IDC_EDIT_CLIENT, CString( client.GetClient().Text() ) );

	CString csServer, csPort;
	if ( PerforceSourceControl::ExtractServerAndPort( CString( client.GetPort().Text() ), csServer, csPort ) )
	{
		SetDlgItemText( in_hWnd, IDC_EDIT_SERVER, csServer );
		SetDlgItemText( in_hWnd, IDC_EDIT_PORT, csPort );
	}

	SetDlgItemText( in_hWnd, IDC_EDIT_HOST, CString( client.GetHost().Text() ) );
	SetDlgItemText( in_hWnd, IDC_EDIT_USER, CString( client.GetUser().Text() ) );

	WCHAR szDiffCommand[MAX_PATH] = {0};
	ULONG uSize = ARRAYSIZE( szDiffCommand );
	m_pUtilities->GetUserPreferenceString( k_csRegFolder + k_csRegKeyDiffCommand, szDiffCommand, uSize );

	SetDlgItemText( in_hWnd, IDC_DIFF_TOOL, szDiffCommand );

	DWORD dwUseAKWaveViewerForDiff = 1;
	m_pUtilities->GetUserPreferenceDword( k_csRegFolder + k_csUseAKWaveViewerForDiff, dwUseAKWaveViewerForDiff );

	CheckDlgButton( in_hWnd, IDC_WAVEVIEWER_DIFF_CHECK, dwUseAKWaveViewerForDiff ? BST_CHECKED : BST_UNCHECKED );
}

bool CDlgPerforceConfig::OnBnClickedOk( HWND in_hWnd )
{
	bool bReturn = false;

	CWnd* pEditClient = CWnd::FromHandle( ::GetDlgItem( in_hWnd, IDC_EDIT_CLIENT ) );
	CWnd* pEditServer = CWnd::FromHandle( ::GetDlgItem( in_hWnd, IDC_EDIT_SERVER ) );
	CWnd* pEditPort = CWnd::FromHandle( ::GetDlgItem( in_hWnd, IDC_EDIT_PORT ) );
	CWnd* pEditHost = CWnd::FromHandle( ::GetDlgItem( in_hWnd, IDC_EDIT_HOST ) );
	CWnd* pEditUser = CWnd::FromHandle( ::GetDlgItem( in_hWnd, IDC_EDIT_USER ) );
	CWnd* pEditDiff = CWnd::FromHandle( ::GetDlgItem( in_hWnd, IDC_DIFF_TOOL ) );

	CString csClient, csServer, csPort, csHost, csUser, csDiffCommand;

	pEditClient->GetWindowText( csClient );
	pEditServer->GetWindowText( csServer );
	pEditPort->GetWindowText( csPort );
	pEditHost->GetWindowText( csHost );
	pEditUser->GetWindowText( csUser );
	pEditDiff->GetWindowText( csDiffCommand );

	if ( csClient.IsEmpty() || csPort.IsEmpty() || csUser.IsEmpty() )
	{
		AFX_MANAGE_STATE( AfxGetStaticModuleState() );

		CString csMessage;
		CString csCaption;

		csMessage = _T("One or more required fields are empty");
		csCaption = _T("Perforce plug-in");

		m_pUtilities->MessageBox( in_hWnd, csMessage, csCaption, MB_OK );

		bReturn = true;
	}
	else
	{
		CW2A strClient( csClient );
		CW2A strServerAndPort( PerforceSourceControl::ServerAndPort( csServer, csPort ) );
		CW2A strHost( csHost );
		CW2A strUser( csUser );

		m_pUtilities->SetUserPreferenceString( k_csRegFolder + k_csRegKeyClient, csClient );
		m_pUtilities->SetUserPreferenceString( k_csRegFolder + k_csRegKeyServer, csServer );		
		m_pUtilities->SetUserPreferenceString( k_csRegFolder + k_csRegKeyPort, csPort );
		m_pUtilities->SetUserPreferenceString( k_csRegFolder + k_csRegKeyHost, csHost );
		m_pUtilities->SetUserPreferenceString( k_csRegFolder + k_csRegKeyUser, csUser );
		m_pUtilities->SetUserPreferenceString( k_csRegFolder + k_csRegKeyDiffCommand, csDiffCommand );

		m_pUtilities->SetUserPreferenceDword( k_csRegFolder + k_csUseAKWaveViewerForDiff, IsDlgButtonChecked( in_hWnd, IDC_WAVEVIEWER_DIFF_CHECK ) == BST_CHECKED );
	}

	return bReturn;
}

// ISourceControlDialogBase
HINSTANCE CDlgPerforceConfig::GetResourceHandle() const
{    
	AFX_MANAGE_STATE( ::AfxGetStaticModuleState() );
	return ::AfxGetResourceHandle();
}

void CDlgPerforceConfig::GetDialog( UINT & out_uiDialogID ) const
{
	out_uiDialogID = IDD_PERFORCE_CONFIG;
}

bool CDlgPerforceConfig::HasHelp() const
{
	return false;
}

bool CDlgPerforceConfig::Help( HWND in_hWnd ) const
{
	return false;
}

bool CDlgPerforceConfig::OnBnClickedDiffBrowse( HWND in_hWnd )
{
	bool bReturn = false;

	CFileDialog fileOpen( TRUE, L"exe", NULL, OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST, L"Executable Files (*.exe)|*.exe|All Files (*.*)|*.*||" );

	if ( fileOpen.DoModal() == IDOK )
	{
		CString csCommand = L"\"" + fileOpen.GetPathName() + L"\" %1 %2";
		SetDlgItemText( in_hWnd, IDC_DIFF_TOOL, csCommand );
	}

	return bReturn;
}

bool CDlgPerforceConfig::WindowProc( HWND in_hWnd, UINT in_message, WPARAM in_wParam, LPARAM in_lParam, LRESULT & out_lResult )
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
					case IDC_DIFF_BROWSE:
						bReturn = OnBnClickedDiffBrowse( in_hWnd );
						break;
				}           
        }
    }

    return bReturn;
}