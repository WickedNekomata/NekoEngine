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

#include "SamplePlugin.h"
#include "DlgConfiguration.h"
#include "Resource.h"

using namespace AK;
using namespace Wwise;

// To use CRegKey
#include <atlbase.h>

namespace
{
	// Icon indexes in the ResourceIDPair list
	enum IconIndex
	{
		IconIndex_Icon1 = 0,
		IconIndex_Icon2,

		IconIndex_Count
	};

	// The list of all icons/tooltip
	const static UINT s_iconIDs[] = { IDI_ICON1,
												IDI_ICON2 };

	const static CString s_tooltipText[] = { _T("Icon 1"),
											_T("Icon 2") };
}

SamplePlugin::SamplePlugin()
{
	// Load icons
	m_icons = new HICON[IconIndex_Count];

	for ( unsigned int i=0 ; i<IconIndex_Count ; ++i )
	{
		m_icons[i] = ::LoadIcon( AfxGetStaticModuleState()->m_hCurrentResourceHandle, MAKEINTRESOURCE( s_iconIDs[i] ) );
	}
}

SamplePlugin::~SamplePlugin()
{
	delete[] m_icons;
}

void SamplePlugin::GetPluginInfo( PluginInfo& out_rPluginInfo )
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	// Plug-in name and version
	CString csName;
	csName = _T("Sample Plugin");

	out_rPluginInfo.m_bstrName = csName.AllocSysString();
	out_rPluginInfo.m_uiVersion = 1;

	// Functions availability
	out_rPluginInfo.m_bShowConfigDlgAvailable = true;
	out_rPluginInfo.m_dwUpdateCommandID = AK::Wwise::SourceControlConstant::s_dwInvalidOperationID;// Replace with your ID
	out_rPluginInfo.m_dwCommitCommandID = AK::Wwise::SourceControlConstant::s_dwInvalidOperationID;// Replace with your ID
	out_rPluginInfo.m_bStatusIconAvailable = true;
}

// ISourceControl
void SamplePlugin::Init( ISourceControlUtilities* in_pUtilities, bool in_bAutoAccept )
{
	// Save the 'Utilities' interface since the interface will always exist while the plug-in exists
	m_pSourceControlUtilities = in_pUtilities;

	//	About in_bAutoAccept:
	// Keep in_bAutoAccept as a member if the plug-in will request at some point to display UI for user confirmation.
	// For example, this will be true when runing in command-line mode.

	// Use AK::Wwise::ISourceControlUtilities::GetRegistryPath() to load plug-in configuration
	// from the registry.

	// CRegKey regKey;
	// regKey.Create( HKEY_CURRENT_USER, m_pSourceControlUtilities->GetRegistryPath() + L"SourceControlSample\\" );
	//
	// TCHAR szValue[MAX_PATH] = { 0 };
	// ULONG size = MAX_PATH;
	//
	// if( regKey.QueryStringValue( s_csRegKey, szValue, &size ) == ERROR_SUCCESS )
	// {
	//		// Do something with szValue
	// }
	//
	// regKey.Close();
}

void SamplePlugin::Term()
{
	// Use AK::Wwise::ISourceControlUtilities::GetRegistryPath() to save plug-in configuration
	// to the registry.

	// CRegKey regKey;
	// regKey.Create( HKEY_CURRENT_USER, m_pSourceControlUtilities->GetRegistryPath() + L"SourceControlSample\\" );
	// regKey.SetStringValue( s_csRegKey, L"Value" );
	// regKey.Close();
}

void SamplePlugin::Destroy()
{
	delete this;
}

bool SamplePlugin::ShowConfigDlg()
{
	DlgConfiguration configurationDialog( m_pSourceControlUtilities );

	// return true if the user presses 'OK'
	return m_pSourceControlUtilities->CreateModalCustomDialog( &configurationDialog ) == IDOK;
}

AK::Wwise::ISourceControl::OperationResult SamplePlugin::GetOperationList( OperationMenuType in_menuType, const StringList& in_rFilenameList, OperationList& out_rOperationList )
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	// Create operation items in static variables
	static bool s_bInitialized = false;
	static OperationListItem operation1Item = { OperationID_Operation1, true };
	static OperationListItem operation2Item = { OperationID_Operation2, true };

	switch ( in_menuType )
	{
		// Work Units tab contextual menu
		case OperationMenuType_WorkUnits:
			out_rOperationList.AddTail( operation1Item );
			out_rOperationList.AddTail( operation2Item );
			break;

		// Sources tab contextual menu
		case OperationMenuType_Sources:
			out_rOperationList.AddTail( operation1Item );
			break;

		// Project Explorer contextual menu
		case OperationMenuType_Explorer:
			out_rOperationList.AddTail( operation2Item );
			break;
	}

	return OperationResult_Succeed;
}

LPCWSTR SamplePlugin::GetOperationName( DWORD in_dwOperationID )
{
	if( in_dwOperationID == OperationID_Operation1 )
		return L"Operation1";
	else if( in_dwOperationID == OperationID_Operation2 )
		return L"Operation2";
	
	return L"";
}

DWORD SamplePlugin::GetOperationEffect( DWORD in_dwOperationID )
{
	DWORD dwRetVal = 0;

	switch( in_dwOperationID )
	{
	case OperationID_Operation1:
		dwRetVal |= OperationEffect_LocalContentModification;
		break;

	case OperationID_Operation2:
		dwRetVal |= OperationEffect_ServerContentModification;
		break;
	}

	return dwRetVal;
}

ISourceControl::OperationResult SamplePlugin::GetFileStatus( const StringList& in_rFilenameList, FilenameToStatusMap& out_rFileStatusMap, DWORD in_dwTimeoutMs )
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	// Fill the map with default status/owner values
	SourceControlContainers::AkPos filePos = in_rFilenameList.GetHeadPosition();

	CString csStatus, csOwner;
	csStatus = _T("normal status");
	csOwner = _T("no owner");

	while ( filePos )
	{
		FilenameToStatusMapItem defaultStatus;

		defaultStatus.m_bstrStatus = csStatus.AllocSysString();
		defaultStatus.m_bstrOwner = csOwner.AllocSysString();

		out_rFileStatusMap.SetAt( in_rFilenameList.GetAt( filePos ), defaultStatus );

		in_rFilenameList.GetNext( filePos );
	}

	return OperationResult_Succeed;
}

AK::Wwise::ISourceControl::OperationResult SamplePlugin::GetFilesForOperation( DWORD in_dwOperationID, const StringList& in_rFilenameList, StringList& out_rFilenameList, FilenameToStatusMap& out_rFileStatusMap )
{
	return GetFileStatus( in_rFilenameList, out_rFileStatusMap, INFINITE );
}

ISourceControl::OperationResult SamplePlugin::GetFileStatusIcons( const StringList& in_rFilenameList, FilenameToIconMap& out_rFileIconsMap, DWORD in_dwTimeoutMs )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// Fill the map alternating icons
	unsigned int uiIndex = 0;
	SourceControlContainers::AkPos filePos = in_rFilenameList.GetHeadPosition();

	while( filePos )
	{
		CString csToolTipText;
		csToolTipText = s_tooltipText[uiIndex];
		FilenameToIconMapItem iconItem = { m_icons[uiIndex], csToolTipText.AllocSysString() };

		out_rFileIconsMap.SetAt( in_rFilenameList.GetAt( filePos ), iconItem );

        if ( ++uiIndex == IconIndex_Count )
			uiIndex = 0;

		in_rFilenameList.GetNext( filePos );
	}

	return OperationResult_Succeed;
}

ISourceControl::OperationResult SamplePlugin::GetMissingFilesInDirectories( const StringList& in_rDirectoryList, StringList& out_rFilenameList )
{
	// There's no missing file here
	return OperationResult_NotImplemented;
}

AK::Wwise::ISourceControl::IOperationResult* SamplePlugin::DoOperation( 
	DWORD in_dwOperationID, 
	const StringList& in_rFilenameList,
	const StringList* in_pTargetFilenameList )
{
	// Dispatch operations
	switch ( in_dwOperationID )
	{
		case OperationID_Operation1:
			DoOperation1( in_rFilenameList );
			break;

		case OperationID_Operation2:
			DoOperation2( in_rFilenameList );
			break;
	}

	return NULL;
}

ISourceControl::OperationResult SamplePlugin::PreCreateOrModify( const StringList& in_rFilenameList, CreateOrModifyOperation in_eOperation, bool& out_rContinue )
{
	out_rContinue = true;

	return OperationResult_Succeed;
}

ISourceControl::OperationResult SamplePlugin::PostCreateOrModify( const StringList& in_rFilenameList, CreateOrModifyOperation in_eOperation, bool& out_rContinue )
{
	out_rContinue = true;

	return OperationResult_Succeed;
}

void SamplePlugin::DoOperation1( const StringList& in_rFilenameList )
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	ISourceControlOperationProgress* pProgressDialog = m_pSourceControlUtilities->GetProgressDialog();

	// Show the progress dialog
	pProgressDialog->ShowProgress();

	// Perform the operation here

	// Print a message
	CString csMessage;
	csMessage = _T("Operation 1 done.");

	pProgressDialog->AddLogMessage( csMessage );

	// Wait for the user to press 'Ok'
	pProgressDialog->OperationCompleted();
}

void SamplePlugin::DoOperation2( const StringList& in_rFilenameList )
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	ISourceControlOperationProgress* pProgressDialog = m_pSourceControlUtilities->GetProgressDialog();

	// Show the progress dialog
	pProgressDialog->ShowProgress();

	// Perform the operation here

	// Print a message
	CString csMessage;
	csMessage = _T("Operation 2 done.");

	pProgressDialog->AddLogMessage( csMessage );

	// Wait for the user to press 'Ok'
	pProgressDialog->OperationCompleted();
}