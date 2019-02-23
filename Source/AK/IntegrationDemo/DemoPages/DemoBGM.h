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

#pragma once

#include "Platform.h"
#include <AK/SoundEngine/Common/AkTypes.h>
#include <AK/SoundEngine/Common/AkCallback.h>
#include "Page.h"
#include <string>

/// Class representing the Non-recordable background music Demo page.  This pages demonstrates how to setup the background music
/// in your game and project to make sure it is not recorded by the DVR, thus compliying with the TCRs about licensed music.
class DemoBGMusic : public Page
{
public:

	/// DemoBGMusic class constructor
	DemoBGMusic( Menu& in_ParentMenu );

	/// Initializes the demo.
	/// \return True if successful and False otherwise.
	virtual bool Init();

	/// Releases resources used by the demo.
	virtual void Release();

	/// Override of the Page::Draw() method.
	virtual void Draw();
	void InitControls();
	
private:

	void Recordable_Pressed( void*, ControlEvent* );
	void NonRecordable_Pressed( void*, ControlEvent* );

	AkOutputDeviceID m_idBGMOutput;
	bool m_bPlayLicensed;
	bool m_bPlayCopyright;
};
