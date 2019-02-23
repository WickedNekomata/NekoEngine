/*******************************************************************************
The content of this file includes portions of the AUDIOKINETIC Wwise Technology
released in source code form as part of the SDK installer package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use this file in accordance with the end user license agreement provided 
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

  Version: v2017.2.6  Build: 6581
  Copyright (c) 2006-2018 Audiokinetic Inc.
*******************************************************************************/

#include "stdafx.h"
#include "SoundInputMgr.h"
#include "SoundInput.h"

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// SoundInputMgr implementation
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
SoundInputMgrBase& SoundInputMgr::Instance()
{
	static SoundInputMgr Singleton;
	return Singleton;
}

bool SoundInputMgr::Initialize()
{
	if (!SoundInputMgrBase::Initialize())
		return false;

	DetectInputDevices();
	return m_MicCount != 0;
}

void SoundInputMgr::Term()
{
	SoundInputMgrBase::Term();
}

// Detects / Register   Input devices
void SoundInputMgr::DetectInputDevices()
{	
	ClearDevices(); // Reset the count.
}