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
// AkSinkPlugin.cpp  
//
#include "stdafx.h"
#include "SinkPlugin.h"
#include ".\Help\TopicAlias.h"
#include <AK/Plugin/AkSinkFactory.h>

using namespace AK;
using namespace Wwise;

// Property names
static LPCWSTR szSampleProp = L"DirectGain";

AK_BEGIN_POPULATE_TABLE(AkSinkPlugin)
AK_END_POPULATE_TABLE()

AkSinkPlugin::AkSinkPlugin() : m_pPSet(NULL)
{
}

AkSinkPlugin::~AkSinkPlugin()
{
}

void AkSinkPlugin::Destroy()
{
	delete this;
}

void AkSinkPlugin::SetPluginPropertySet(IPluginPropertySet * in_pPSet)
{
	m_pPSet = in_pPSet;
}

HINSTANCE AkSinkPlugin::GetResourceHandle() const
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return AfxGetStaticModuleState()->m_hCurrentResourceHandle;
}

bool AkSinkPlugin::GetDialog(eDialog in_eDialog, UINT & out_uiDialogID, PopulateTableItem *& out_pTable) const
{
	// This sample does not provide UI, it will simply dispay properties in the default property editor.
	return false;
}

bool AkSinkPlugin::WindowProc(eDialog in_eDialog, HWND in_hWnd, UINT in_message, WPARAM in_wParam, LPARAM in_lParam, LRESULT & out_lResult)
{
	out_lResult = 0;
	return false;
}

bool AkSinkPlugin::GetBankParameters(const GUID & in_guidPlatform, AK::Wwise::IWriteData* in_pDataWriter) const
{
	// Sample code to add one float property.

	CComVariant varProp;
	m_pPSet->GetValue(in_guidPlatform, szSampleProp, varProp);
	in_pDataWriter->WriteReal32(varProp.fltVal);

	return true;
}

