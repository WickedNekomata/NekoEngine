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
// DlgPerforceOnNewFiles.cpp
//
// Implementation file for the CDlgPerforceOnNewFiles dialog, used to let the user
// choose a new name for a file.
//
///////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include ".\DlgPerforceOnNewFiles.h"
#include "resource.h"
#include <AK/Wwise/SourceControl/ISourceControl.h>

#include <windowsx.h>

// Constructor
CDlgPerforceOnNewFiles::CDlgPerforceOnNewFiles( 
	bool in_bEnableAddFiles, 
	bool in_bEnableCheckOutFiles, 
	bool in_bAddFiles, 
	bool in_bCheckOutFiles,
	AK::Wwise::ISourceControlUtilities* in_pUtilities,
	const WCHAR** in_ppFilenameList,
	unsigned int in_uiFilenameListCount )
	: m_bEnableAddFiles( in_bEnableAddFiles )
	, m_bEnableCheckOutFiles( in_bEnableCheckOutFiles )
	, m_bNeedToCheckOutFiles( in_bCheckOutFiles )
	, m_bNeedToAddFiles( in_bAddFiles )
	, m_uiResult( 0 )
	, m_pUtilities( in_pUtilities )
	, m_ppFilenameList( in_ppFilenameList )
	, m_uiFilenameListCount( in_uiFilenameListCount )
{
}

// Destructor
CDlgPerforceOnNewFiles::~CDlgPerforceOnNewFiles()
{
}

void CDlgPerforceOnNewFiles::OnInitDialog( HWND in_hWnd )
{	
	EnableWindow( ::GetDlgItem( in_hWnd, IDC_CHECK_CHECKOUT ), m_bEnableCheckOutFiles ? TRUE : FALSE );
	EnableWindow( ::GetDlgItem( in_hWnd, IDC_CHECK_ADD ), m_bEnableAddFiles ? TRUE : FALSE );

	Button_SetCheck( ::GetDlgItem( in_hWnd, IDC_CHECK_CHECKOUT ), m_bNeedToCheckOutFiles? TRUE : FALSE );
	Button_SetCheck( ::GetDlgItem( in_hWnd, IDC_CHECK_ADD ), m_bNeedToAddFiles? TRUE : FALSE );

	if( m_uiFilenameListCount > 0 )
	{
		m_pUtilities->CreateFileStatusListControl( in_hWnd, IDC_FILELIST, m_ppFilenameList, m_uiFilenameListCount );
	}
}

bool CDlgPerforceOnNewFiles::OnBnClickedOK( HWND in_hWnd )
{
	m_bNeedToCheckOutFiles = Button_GetCheck( ::GetDlgItem( in_hWnd, IDC_CHECK_CHECKOUT ) ) == TRUE;
	m_bNeedToAddFiles = Button_GetCheck( ::GetDlgItem( in_hWnd, IDC_CHECK_ADD ) ) == TRUE;

	return false;
}

bool CDlgPerforceOnNewFiles::NeedToCheckOutFiles() const
{
	return m_bNeedToCheckOutFiles;
}

bool CDlgPerforceOnNewFiles::NeedToAddFiles() const
{
	return m_bNeedToAddFiles;
}

INT_PTR CDlgPerforceOnNewFiles::GetResult() const
{
	return m_uiResult;
}

// ISourceControlDialogBase
HINSTANCE CDlgPerforceOnNewFiles::GetResourceHandle() const
{    
	AFX_MANAGE_STATE( ::AfxGetStaticModuleState() );
	return ::AfxGetResourceHandle();
}

void CDlgPerforceOnNewFiles::GetDialog( UINT & out_uiDialogID ) const
{
	if( m_uiFilenameListCount > 0 )
		out_uiDialogID = IDD_PERFORCE_CHECKOUTFILES;
	else
		out_uiDialogID = IDD_PERFORCE_ONNEWFILES;
}

bool CDlgPerforceOnNewFiles::HasHelp() const
{
	return false;
}

bool CDlgPerforceOnNewFiles::Help( HWND in_hWnd ) const
{
	return false;
}

bool CDlgPerforceOnNewFiles::WindowProc( HWND in_hWnd, UINT in_message, WPARAM in_wParam, LPARAM in_lParam, LRESULT & out_lResult )
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
			{
				bool bPostClose = false;

				switch ( LOWORD( in_wParam ) )
				{
					case ID_OK:
						bReturn = OnBnClickedOK( in_hWnd );
						bPostClose = true;
						break;

					case ID_CANCEL:
						bPostClose = true;
						break;
				}

				if ( bPostClose )
				{
					m_uiResult = LOWORD( in_wParam );
					PostMessage( in_hWnd, WM_CLOSE, 0, 0 );
				}
				break;
			}
        }
    }

    return bReturn;
}