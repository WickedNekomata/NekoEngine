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
// DlgRename.h
//
// Header file for the CDlgRename dialog, used to let the user
// choose a new name for a file.
//
///////////////////////////////////////////////////////////////////////

#pragma once

#include <AK/Wwise/SourceControl/ISourceControlDialogBase.h>
#include <AK/Wwise/SourceControl/ISourceControlUtilities.h>

using namespace AK;
using namespace Wwise;

// CDlgRename dialog
class CDlgRename
	: public ISourceControlDialogBase
{
public:
	// Constructor
	CDlgRename( const CString& in_csFilename, ISourceControlUtilities* in_pUtilities );

	// Destructor
	virtual ~CDlgRename();

	// ISourceControlDialogBase
	virtual HINSTANCE GetResourceHandle() const;
	virtual void GetDialog( UINT & out_uiDialogID ) const;
	virtual bool HasHelp() const;
	virtual bool Help( HWND in_hWnd ) const;
	virtual bool WindowProc( HWND in_hWnd, UINT in_message, WPARAM in_wParam, LPARAM in_lParam, LRESULT & out_lResult );

	CString GetNewFilename() const;   
private:

	// Overrides
	virtual void OnInitDialog( HWND in_hWnd );

	// Message handlers
	bool OnBnClickedOk( HWND in_hWnd );
	bool OnEnChangeFileName( HWND in_hWnd );

	// Data
	CString m_csExtension;
	CString m_csName;
	CString m_csFolder;
	ISourceControlUtilities* m_pUtilities;
};
