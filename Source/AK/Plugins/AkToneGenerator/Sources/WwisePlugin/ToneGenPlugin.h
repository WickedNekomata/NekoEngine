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
// ToneGenPlugin.h
//
// Tone generator Wwise plugin implementation.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <AK/Wwise/AudioPlugin.h>
 
// Tone generator property names
static LPCWSTR szStartFreq = L"StartFreq";
static LPCWSTR szStartFreqRandMin = L"StartFreqRandMin";
static LPCWSTR szStartFreqRandMax = L"StartFreqRandMax";
static LPCWSTR szSweepFreq = L"SweepFreq";
static LPCWSTR szSweepFreqType = L"SweepFreqType";
static LPCWSTR szStopFreq = L"StopFreq";
static LPCWSTR szStopFreqRandMin = L"StopFreqRandMin";
static LPCWSTR szStopFreqRandMax = L"StopFreqRandMax";
static LPCWSTR szWaveType = L"WaveType";
static LPCWSTR szWaveGain = L"WaveGain";
static LPCWSTR szDurMode = L"DurMode";
static LPCWSTR szFixDur = L"FixDur";
static LPCWSTR szAttackTime = L"AttackTime";
static LPCWSTR szDecayTime = L"DecayTime";
static LPCWSTR szSustainTime = L"SustainTime";
static LPCWSTR szSustainLevel = L"SustainLevel";
static LPCWSTR szReleaseTime = L"ReleaseTime";
static LPCWSTR szChannelMask = L"ChannelMask";

enum DurMode
{
	DurMode_Fixed = 0,
	DurMode_Envelope
};

class ToneGenPlugin : public AK::Wwise::DefaultAudioPluginImplementation
{
public:
	ToneGenPlugin(AkUInt16 in_idPlugin);
	~ToneGenPlugin();

	// AK::Wwise::IPluginBase
	// Implement the destruction of the Wwise source plugin.
	virtual void Destroy();

	// AK::Wwise::IAudioPlugin
	virtual void SetPluginPropertySet( AK::Wwise::IPluginPropertySet * in_pPSet );
	virtual void NotifyPropertyChanged( const GUID & in_guidPlatform, LPCWSTR in_szPropertyName );
	virtual HINSTANCE GetResourceHandle() const;
	virtual bool GetDialog( eDialog in_eDialog, UINT & out_uiDialogID, AK::Wwise::PopulateTableItem *& out_pTable ) const;
	virtual bool WindowProc( eDialog in_eDialog, HWND in_hWnd, UINT in_message, WPARAM in_wParam, LPARAM in_lParam, LRESULT & out_lResult );
    virtual bool GetBankParameters( const GUID & in_guidPlatform, AK::Wwise::IWriteData* in_pDataWriter ) const;
	virtual bool Help( HWND in_hWnd, eDialog in_eDialog, LPCWSTR in_szLanguageCode ) const;
	virtual bool GetSourceDuration( double& out_dblMinDuration, double& out_dblMaxDuration ) const;

private:

	// Window action controls
	// Enable/disable frequency sweeping controls
	void EnableFrequencySweeping( HWND in_hWnd, eDialog in_eDialog, bool in_bEnable );
	// Enable/disable envelope controls
	void EnableEnvelopeControls( HWND in_hWnd, eDialog in_eDialog, bool in_bEnable );
	// Enable/disable all frequency controls
	void EnableFrequencyControls( HWND in_hWnd, eDialog in_eDialog );

	// Enable a single dialog item
	static void EnableDlgItem( HWND in_hwndDialog, UINT in_uiDlgItem, BOOL in_bEnable );

	AK::Wwise::IPluginPropertySet * m_pPSet;

	HWND m_hwndPropView;
	HWND m_hwndObjPane;

	AkUInt16 m_idDialogBig;
	AkUInt16 m_idDialogSmall;
};
