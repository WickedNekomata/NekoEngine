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
// SinePlugin.cpp
//
// Sine tone Wwise plugin implementation.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "SinePlugin.h"
#include "TopicAlias.h"

using namespace AK;
using namespace Wwise;

// Delay property names
static LPCWSTR szSineFrequency = L"SineFrequency";
static LPCWSTR szSineGain = L"SineGain";
static LPCWSTR szSineDuration = L"SineDuration";
static LPCWSTR szChannelMask = L"ChannelMask";

// Constructor
SinePlugin::SinePlugin()
	: m_pPSet( NULL )
{
}

// Destructor
SinePlugin::~SinePlugin()
{
}

// Implement the destruction of the Wwise source plugin.
void SinePlugin::Destroy()
{
	delete this;
}

// Set internal values of the property set (allow persistence)
void SinePlugin::SetPluginPropertySet( IPluginPropertySet * in_pPSet )
{
	m_pPSet = in_pPSet;
}

// Get access to UI resource handle.
HINSTANCE SinePlugin::GetResourceHandle() const
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );
	return AfxGetStaticModuleState()->m_hCurrentResourceHandle;
}

// Determine what dialog just get called and set the property names to UI control binding populated table.
bool SinePlugin::GetDialog( eDialog in_eDialog, UINT & out_uiDialogID, PopulateTableItem *& out_pTable ) const
{
	switch ( in_eDialog )
	{
	case SettingsDialog:
		out_uiDialogID = IDD_SINEPLUGIN_BIG;
		out_pTable = NULL;
		return true;

	case ContentsEditorDialog:
		out_uiDialogID = IDD_SINEPLUGIN_SMALL;
		out_pTable = NULL;
		return true;
	}

	return false;
}

// Store current plugin settings into banks.
bool SinePlugin::GetBankParameters( const GUID & in_guidPlatform, AK::Wwise::IWriteData* in_pDataWriter ) const
{
	CComVariant varProp;

	m_pPSet->GetValue( in_guidPlatform, szSineFrequency, varProp );
	in_pDataWriter->WriteReal32( varProp.fltVal );

	m_pPSet->GetValue( in_guidPlatform, szSineGain, varProp );
	in_pDataWriter->WriteReal32( varProp.fltVal );

	m_pPSet->GetValue( in_guidPlatform, szSineDuration, varProp );
	in_pDataWriter->WriteReal32( varProp.fltVal );

	m_pPSet->GetValue( in_guidPlatform, szChannelMask, varProp );
	in_pDataWriter->WriteInt32( varProp.intVal );

	return true;
}

// Implement online help when the user clicks on the "?" icon .
bool SinePlugin::Help( HWND in_hWnd, eDialog in_eDialog, LPCWSTR in_szLanguageCode ) const
{
	AFX_MANAGE_STATE( ::AfxGetStaticModuleState() ) ;

	DWORD dwTopic = ONLINEHELP::Sine_Properties;
	if ( in_eDialog == AK::Wwise::IAudioPlugin::ContentsEditorDialog )
		dwTopic = ONLINEHELP::Sound_Object_Sine;

	::SendMessage( in_hWnd, WM_AK_PRIVATE_SHOW_HELP_TOPIC, dwTopic, 0 );

	return true;
}

bool SinePlugin::GetSourceDuration( double& out_dblMinDuration, double& out_dblMaxDuration ) const
{
	AKASSERT( m_pPSet );

	if( m_pPSet == nullptr || m_pPSet->PropertyHasRTPC( szSineDuration ) )
	{
		out_dblMinDuration = 0.f;
		out_dblMaxDuration = FLT_MAX;
		return false;
	}

	CComVariant varProp;
	m_pPSet->GetValue( m_pPSet->GetCurrentPlatform(), szSineDuration, varProp );
	out_dblMinDuration = varProp.fltVal;
	out_dblMaxDuration = varProp.fltVal;

	return true;
}
