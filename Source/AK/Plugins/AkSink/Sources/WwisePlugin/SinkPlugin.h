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

#include <AK/Wwise/AudioPlugin.h>

class AkSinkPlugin
	: public AK::Wwise::DefaultAudioPluginImplementation
{
public:
	AkSinkPlugin();
	~AkSinkPlugin();

	// AK::Wwise::IPluginBase
	virtual void Destroy();

	// AK::Wwise::IAudioPlugin
	virtual void SetPluginPropertySet(AK::Wwise::IPluginPropertySet * in_pPSet);

	virtual HINSTANCE GetResourceHandle() const;
	virtual bool GetDialog(eDialog in_eDialog, UINT & out_uiDialogID, AK::Wwise::PopulateTableItem *& out_pTable) const;
	virtual bool WindowProc(eDialog in_eDialog, HWND in_hWnd, UINT in_message, WPARAM in_wParam, LPARAM in_lParam, LRESULT & out_lResult);

	virtual bool GetBankParameters(const GUID & in_guidPlatform, AK::Wwise::IWriteData* in_pDataWriter) const;

private:

	AK::Wwise::IPluginPropertySet * m_pPSet;
};
