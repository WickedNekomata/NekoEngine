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
// DlgConfiguration.cpp
//
// Implementation file for the CDlgConfiguration dialog, used to let the user
// enter a Check In description
//
///////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include ".\DlgConfiguration.h"
#include "resource.h"

#include <windowsx.h>

// Constructor
CDlgConfiguration::CDlgConfiguration( ISourceControlUtilities* in_pUtilities, const CString& in_csDiffToolPath, bool in_bUseAKWavViewer )
	: m_pUtilities( in_pUtilities )
	, m_csDiffToolPath( in_csDiffToolPath )
	, m_bUseAKWavViewer( in_bUseAKWavViewer )
{
}

// Destructor
CDlgConfiguration::~CDlgConfiguration()
{
}

void CDlgConfiguration::OnInitDialog( HWND in_hWnd )
{
	CEdit* pControlDiffTool = (CEdit*)CWnd::FromHandle( ::GetDlgItem( in_hWnd, IDC_DIFF_TOOL ) );

	pControlDiffTool->SetWindowText( m_csDiffToolPath );

	CheckDlgButton( in_hWnd, IDC_WAVEVIEWER_DIFF_CHECK, m_bUseAKWavViewer ? BST_CHECKED : BST_UNCHECKED );
}

bool CDlgConfiguration::OnBnClickedOk( HWND in_hWnd )
{
	bool bReturn = false;

	// Get the diff tool path
	CEdit* pControlDiffTool = (CEdit*)CWnd::FromHandle( ::GetDlgItem( in_hWnd, IDC_DIFF_TOOL ) );

	pControlDiffTool->GetWindowText( m_csDiffToolPath );

	m_bUseAKWavViewer = IsDlgButtonChecked( in_hWnd, IDC_WAVEVIEWER_DIFF_CHECK ) == BST_CHECKED;

	return bReturn;
}

bool CDlgConfiguration::OnBnClickedDiffBrowse( HWND in_hWnd )
{
	bool bReturn = false;

	CFileDialog fileOpen( TRUE, L"exe", NULL, OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST, L"Executable Files (*.exe)|*.exe|All Files (*.*)|*.*||" );

	if ( fileOpen.DoModal() == IDOK )
	{
		SetDlgItemText( in_hWnd, IDC_DIFF_TOOL, fileOpen.GetPathName() );
	}

	return bReturn;
}

const CString& CDlgConfiguration::GetDiffToolPath() const
{
	return m_csDiffToolPath;
}

// ISourceControlDialogBase
HINSTANCE CDlgConfiguration::GetResourceHandle() const
{    
	AFX_MANAGE_STATE( ::AfxGetStaticModuleState() );
	return ::AfxGetResourceHandle();
}

void CDlgConfiguration::GetDialog( UINT & out_uiDialogID ) const
{
	out_uiDialogID = IDD_CONFIGURATION;
}

bool CDlgConfiguration::HasHelp() const
{
	return false;
}

bool CDlgConfiguration::Help( HWND in_hWnd ) const
{
	return false;
}

bool CDlgConfiguration::WindowProc( HWND in_hWnd, UINT in_message, WPARAM in_wParam, LPARAM in_lParam, LRESULT & out_lResult )
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

bool CDlgConfiguration::GetUseAKWavViewer() const
{
	return m_bUseAKWavViewer;
}
