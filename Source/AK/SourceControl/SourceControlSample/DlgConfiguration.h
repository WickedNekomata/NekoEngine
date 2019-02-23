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

#ifndef _CONFIGURATION_DIALOG_H
#define _CONFIGURATION_DIALOG_H

#include "AK/Wwise/SourceControl/ISourceControl.h"

using namespace AK;
using namespace Wwise;

class DlgConfiguration
	: public ISourceControlDialogBase
{
public:
	DlgConfiguration( ISourceControlUtilities* in_pUtilities );

	// ISourceControlDialogBase implementation
	virtual HINSTANCE GetResourceHandle() const;
	virtual void GetDialog( UINT & out_uiDialogID ) const;
	virtual bool Help( HWND in_hWnd	) const;
	virtual bool HasHelp() const;
	virtual bool WindowProc( HWND in_hWnd, UINT in_message,	WPARAM in_wParam, LPARAM in_lParam,	LRESULT & out_lResult );

private:
	bool OnInitDialog( HWND in_hWnd );
	bool OnOK( HWND in_hWnd );
	
	ISourceControlUtilities* m_pSourceControlUtilities;
};

#endif // _CONFIGURATION_DIALOG_H