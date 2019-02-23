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

#include "SubversionSourceControl.h"

namespace
{
	// SubversionSourceControl - {BF84CFDB-5BB1-4eaa-9253-469A1EFA1B68}
	static const GUID k_guidSubversionSourceControlID = 
	{ 0xbf84cfdb, 0x5bb1, 0x4eaa, { 0x92, 0x53, 0x46, 0x9a, 0x1e, 0xfa, 0x1b, 0x68 } };


};

void __stdcall AkSourceControlGetPluginIDList ( AK::Wwise::ISourceControl::PluginIDList& out_rPluginIDList )
{
	out_rPluginIDList.AddTail( k_guidSubversionSourceControlID );
}

void __stdcall AkSourceControlGetPluginInfo ( const GUID& in_rguidPluginID, AK::Wwise::ISourceControl::PluginInfo& out_rPluginInfo )
{
	if ( in_rguidPluginID == k_guidSubversionSourceControlID )
	{
		SubversionSourceControl::GetPluginInfo( out_rPluginInfo );
	}
}

AK::Wwise::ISourceControl* __stdcall AkSourceControlCreatePlugin ( const GUID& in_rguidPluginID )
{
	AK::Wwise::ISourceControl* pSourceControl = NULL;

	if ( in_rguidPluginID == k_guidSubversionSourceControlID )
	{
		pSourceControl =  new SubversionSourceControl();
	}

	return pSourceControl;
}
