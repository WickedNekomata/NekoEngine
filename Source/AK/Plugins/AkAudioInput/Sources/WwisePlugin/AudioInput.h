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
//////////////////////////////////////////////////////////////////////
//
// AudioInput.h
//
// Audio Input Wwise plugin: Main header file for the Audio Input DLL.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

// CAudioInputApp
// See AudioInput.cpp for the implementation of this class
//

class CAudioInputApp : public CWinApp
{
public:
	CAudioInputApp();

// Overrides
public:
	virtual BOOL InitInstance();
	virtual int  ExitInstance(); 

	DECLARE_MESSAGE_MAP()
};
