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

#include "Perforce.h"

namespace
{
	// Perforce - {F56BDC98-5067-4719-86A4-83CD509C54B7}
	static const GUID k_guidPerforceSourceControlID = 
	{ 0xf56bdc98, 0x5067, 0x4719, { 0x86, 0xa4, 0x83, 0xcd, 0x50, 0x9c, 0x54, 0xb7 } };
};

void __stdcall AkSourceControlGetPluginIDList ( AK::Wwise::ISourceControl::PluginIDList& out_rPluginIDList )
{
	if ( PerforceSourceControl::IsAvalaible() )
		out_rPluginIDList.AddTail( k_guidPerforceSourceControlID );
}

void __stdcall AkSourceControlGetPluginInfo ( const GUID& in_rguidPluginID, AK::Wwise::ISourceControl::PluginInfo& out_rPluginInfo )
{
	if ( in_rguidPluginID == k_guidPerforceSourceControlID )
	{
		PerforceSourceControl::GetPluginInfo( out_rPluginInfo );
	}
}

AK::Wwise::ISourceControl* __stdcall AkSourceControlCreatePlugin ( const GUID& in_rguidPluginID )
{
	AK::Wwise::ISourceControl* pSourceControl = NULL;

	if ( in_rguidPluginID == k_guidPerforceSourceControlID )
	{
		pSourceControl =  new PerforceSourceControl();
	}

	return pSourceControl;
}
