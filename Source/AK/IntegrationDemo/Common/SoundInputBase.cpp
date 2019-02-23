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

#include "stdafx.h"
#include "SoundInputBase.h"

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// SoundInputBase implementation
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
SoundInputBase::SoundInputBase()
	:m_playingID(AK_INVALID_PLAYING_ID)
{
}

SoundInputBase::~SoundInputBase() 
{
}

void SoundInputBase::SetPlayingID( AkPlayingID in_playingID )
{
	m_playingID = in_playingID;
}

AkPlayingID SoundInputBase::GetPlayingID()
{
	return m_playingID;
}
