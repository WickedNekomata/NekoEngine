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
// AudioInput.cpp
//
// AudioInput Wwise plugin: Defines the initialization routines for the DLL.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AudioInput.h"
#include <AK/Wwise/Utilities.h>
#include "AudioInputPlugin.h"
#include "SoundInputMgr.h"

//
//	Note!
//
//		If this DLL is dynamically linked against the MFC
//		DLLs, any functions exported from this DLL which
//		call into MFC must have the AFX_MANAGE_STATE macro
//		added at the very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

#include <math.h>

// CAudioInputApp
BEGIN_MESSAGE_MAP(CAudioInputApp, CWinApp)
END_MESSAGE_MAP()

static const AkReal32 TWOPI			= 6.283185307179586476925286766559f;
static const AkReal32 FREQUENCY		= 55.0f;	// Frequency (A)
static const AkReal32 SAMPLERATE	= 48000.0f;	// NOTE: This might need to be located somewhere..

// CAudioInputApp construction
CAudioInputApp::CAudioInputApp()
{
}


// The one and only CAudioInputApp object
CAudioInputApp theApp;

// CAudioInputApp initialization
BOOL CAudioInputApp::InitInstance()
{
	AK::Wwise::RegisterWwisePlugin();
	CWinApp::InitInstance();	
	SoundInputMgr::Instance().Initialize();
	
	return TRUE;
}

// CAudioInputApp exit code
int CAudioInputApp::ExitInstance()
{
	SoundInputMgr::Instance().Term();
	return 0;
}

/////////////// DLL exports ///////////////////

// Plugin creation
AK::Wwise::IPluginBase* __stdcall AkCreatePlugin( unsigned short in_usCompanyID, unsigned short in_usPluginID )
{
	return new AudioInputPlugin;
}

/// Dummy assert hook for Wwise plug-ins using AKASSERT (cassert used by default).
DEFINEDUMMYASSERTHOOK;
DEFINE_PLUGIN_REGISTER_HOOK;