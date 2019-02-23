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
// AudioInputPlugin.cpp
//
// Audio Input Wwise plugin implementation.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "AudioInputPlugin.h"
#include "Help\TopicAlias.h"
#include <AK\Plugin\AkAudioInputSourceFactory.h>
#include "SoundInput.h"

#define _USE_MATH_DEFINES
#include <math.h>

// Microphone Input
using namespace AK;
using namespace Wwise;

void AudioInputPlugin::GetFormatCallbackFunc(
    AkPlayingID		in_playingID,   ///< Playing ID (same that was returned from the PostEvent call or from the PlayAudioInput call.
    AkAudioFormat&  io_AudioFormat  ///< Already filled format, modify it if required.
    )
{
}

void AudioInputPlugin::ExecuteCallbackFunc(
    AkPlayingID		in_playingID,  ///< Playing ID (same that was returned from the PostEvent call or from the PlayAudioInput call.
    AkAudioBuffer*	io_pBufferOut  ///< Buffer to fill
    )
{
	AkSampleType * pSamples = io_pBufferOut->GetChannel( 0 );

	for ( int i = 0; i < io_pBufferOut->MaxFrames(); ++i )
		pSamples[ i ] = (float) sin( (double) i / 256.0 * M_PI * 2.0 );

	io_pBufferOut->eState = AK_DataReady;
	io_pBufferOut->uValidFrames = io_pBufferOut->MaxFrames();
}

// Constructor
AudioInputPlugin::AudioInputPlugin()
	: m_pPSet( NULL )
{
	m_Input.InputOn();
}

// Destructor
AudioInputPlugin::~AudioInputPlugin()
{
	m_Input.InputOff();
}

// Implement the destruction of the Wwise source plugin.
void AudioInputPlugin::Destroy()
{
	delete this;
}

// Set internal values of the property set (allow persistence)
void AudioInputPlugin::SetPluginPropertySet( IPluginPropertySet * in_pPSet )
{
	m_pPSet = in_pPSet;
}

// Get access to UI resource handle.
HINSTANCE AudioInputPlugin::GetResourceHandle() const
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );
	return AfxGetStaticModuleState()->m_hCurrentResourceHandle;
}

// Determine what dialog just get called and set the property names to UI control binding populated table.
bool AudioInputPlugin::GetDialog( eDialog in_eDialog, UINT & out_uiDialogID, PopulateTableItem *& out_pTable ) const
{
	switch ( in_eDialog )
	{
	case SettingsDialog:
		out_uiDialogID = IDD_AUDIOINPUTPLUGIN_BIG;
		out_pTable = NULL;
		return true;

	case ContentsEditorDialog:
		out_uiDialogID = IDD_AUDIOINPUTPLUGIN_SMALL;
		out_pTable = NULL;
		return true;
	}

	return false;
}

// Store current plugin settings into banks when asked to.
bool AudioInputPlugin::GetBankParameters( const GUID & in_guidPlatform, AK::Wwise::IWriteData* in_pDataWriter ) const
{
	CComVariant varProp;

	m_pPSet->GetValue( in_guidPlatform, szAudioInputGain, varProp );
	in_pDataWriter->WriteReal32( varProp.fltVal );

	return true;
}

// Implement online help when the user clicks on the "?" icon .
bool AudioInputPlugin::Help( HWND in_hWnd, eDialog in_eDialog, LPCWSTR in_szLanguageCode ) const
{
	AFX_MANAGE_STATE( ::AfxGetStaticModuleState() ) ;

	DWORD dwTopic = ONLINEHELP::Audio_Input_Property;
	if ( in_eDialog == AK::Wwise::IAudioPlugin::ContentsEditorDialog )
		dwTopic = ONLINEHELP::Audio_Input_Contents;

	::SendMessage( in_hWnd, WM_AK_PRIVATE_SHOW_HELP_TOPIC, dwTopic, 0 );

	return true;
}
