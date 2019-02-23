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
#include "resource.h"
#include "DelayPlugin.h"
#include ".\Help\TopicAlias.h"
#include <AK\Plugin\AkDelayFXFactory.h>
#include <AK/Tools/Common/AkAssert.h>

using namespace AK;
using namespace Wwise;

// Delay property names
static LPCWSTR szDelayTime = L"DelayTime";
static LPCWSTR szFeedbackEnabled = L"FeedbackEnabled";
static LPCWSTR szFeedback = L"Feedback";
static LPCWSTR szWetDryMix = L"WetDryMix";
static LPCWSTR szOutputLevel = L"OutputLevel";
static LPCWSTR szProcessLFE = L"ProcessLFE";

// Bind non static text UI controls to properties for property view
AK_BEGIN_POPULATE_TABLE(DelayProp)
	AK_POP_ITEM(IDC_CHECK_FEEDBACKENABLED, szFeedbackEnabled)
	AK_POP_ITEM(IDC_CHECK_PROCESSLFE, szProcessLFE)
AK_END_POPULATE_TABLE()

// Constructor
DelayPlugin::DelayPlugin()
	: m_pPSet( NULL ), m_hwndPropView( NULL )
{
}

// Destructor
DelayPlugin::~DelayPlugin()
{
}

// Implement the destruction of the Wwise source plugin.
void DelayPlugin::Destroy()
{
	delete this;
}

// Set internal values of the property set (allow persistence)
void DelayPlugin::SetPluginPropertySet( IPluginPropertySet * in_pPSet )
{
	m_pPSet = in_pPSet;
}

// Take necessary action on property changes. 
// Note: user also has the option of catching appropriate message in WindowProc function.
void DelayPlugin::NotifyPropertyChanged( const GUID & in_guidPlatform, LPCWSTR in_szPropertyName )
{
	if ( !wcscmp( in_szPropertyName, szFeedbackEnabled ) )
	{
		if ( m_hwndPropView ) 
			EnableFeedback( );
	}
}

// Get access to UI resource handle.
HINSTANCE DelayPlugin::GetResourceHandle() const
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );
	return AfxGetStaticModuleState()->m_hCurrentResourceHandle;
}

// Set the property names to UI control binding populated table.
bool DelayPlugin::GetDialog( eDialog in_eDialog, UINT & out_uiDialogID, PopulateTableItem *& out_pTable ) const
{
	AKASSERT( in_eDialog == SettingsDialog );

	out_uiDialogID = IDD_DELAY_BIG;
	out_pTable = DelayProp;

	return true;
}

// Standard window function, user can intercept what ever message that is of interest to him to implement UI behavior.
bool DelayPlugin::WindowProc( eDialog in_eDialog, HWND in_hWnd, UINT in_message, WPARAM in_wParam, LPARAM in_lParam, LRESULT & out_lResult )
{
	switch ( in_message )
	{
	case WM_INITDIALOG:
		m_hwndPropView = in_hWnd;
		EnableFeedback( );
		break;
	case WM_DESTROY:
		m_hwndPropView = NULL;
		break;
	}

	out_lResult = 0;
	return false;
}

// Store current plugin settings into banks when asked to.
bool DelayPlugin::GetBankParameters( const GUID & in_guidPlatform, AK::Wwise::IWriteData* in_pDataWriter ) const
{
	CComVariant varProp;

	// Pack parameters in bank 
	// IMPORTANT NOTE: they need to be written and read on the AudioEngine side in the same order
	m_pPSet->GetValue( in_guidPlatform, szDelayTime, varProp );
	in_pDataWriter->WriteReal32( varProp.fltVal );
	m_pPSet->GetValue( in_guidPlatform, szFeedback, varProp );
	in_pDataWriter->WriteReal32( varProp.fltVal );
	m_pPSet->GetValue( in_guidPlatform, szWetDryMix, varProp );
	in_pDataWriter->WriteReal32( varProp.fltVal );
	m_pPSet->GetValue( in_guidPlatform, szOutputLevel, varProp );
	in_pDataWriter->WriteReal32( varProp.fltVal );
	m_pPSet->GetValue( in_guidPlatform, szFeedbackEnabled, varProp );
	in_pDataWriter->WriteBool( varProp.boolVal != 0 );
	m_pPSet->GetValue( in_guidPlatform, szProcessLFE, varProp );
	in_pDataWriter->WriteBool( varProp.boolVal != 0 );

    return true;
}

// Implement online help when the user clicks on the "?" icon .
bool DelayPlugin::Help( HWND in_hWnd, eDialog in_eDialog, LPCWSTR in_szLanguageCode ) const
{
	AFX_MANAGE_STATE( ::AfxGetStaticModuleState() ) ;

	if ( in_eDialog == AK::Wwise::IAudioPlugin::SettingsDialog )
		::SendMessage( in_hWnd, WM_AK_PRIVATE_SHOW_HELP_TOPIC, ONLINEHELP::Delay_Properties, 0 );
	else
		return false;

	return true;
}

// Enable/disable feedback slider based on feedback enable checkbox
void DelayPlugin::EnableFeedback( )
{
	CComVariant varProp;
	m_pPSet->GetValue( m_pPSet->GetCurrentPlatform(), szFeedbackEnabled, varProp );
	HWND hwndItem = GetDlgItem( m_hwndPropView, IDC_RANGE_FEEDBACK );
	AKASSERT( hwndItem );
	::EnableWindow( hwndItem, varProp.boolVal );
	hwndItem = GetDlgItem( m_hwndPropView, IDC_STATIC_FEEDBACK );
	AKASSERT( hwndItem );
	::EnableWindow( hwndItem, varProp.boolVal );
}
