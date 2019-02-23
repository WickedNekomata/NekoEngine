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
#include "DlgPerforceDescription.h"
#include "DlgRename.h"
#include "DlgPerforceConfig.h"
#include "DlgPerforceOnNewFiles.h"
#include "OperationResults.h"
#include "Resource.h"
#include "SourceControlHelpers.h"

#include <atlbase.h>
#include <vector>
#include <algorithm>
#include <set>

#ifdef _DEBUG
	#define new DEBUG_NEW
#endif

using namespace AK;
using namespace Wwise;

namespace
{
	enum IconsIndexes
	{
		IconsIndexes_NotCheckOut = 0,
		IconsIndexes_CheckOut,
		IconsIndexes_Added,
		IconsIndexes_OtherCheckout,
		IconsIndexes_BothCheckout,

		IconsIndexes_Outdated_NotCheckOut,
		IconsIndexes_Outdated_CheckOut,
		IconsIndexes_Outdated_OtherCheckout,
		IconsIndexes_Outdated_BothCheckout,

		IconsIndexes_Count
	};

	const static UINT k_iconIDs[] = { IDI_PERFORCE_STATUS_NOT_CHECKED_OUT,
										IDI_PERFORCE_STATUS_CHECKED_OUT,
										IDI_PERFORCE_STATUS_ADDED,
										IDI_PERFORCE_STATUS_OTHER_CHECKOUT,
										IDI_PERFORCE_STATUS_BOTH_CHECKOUT,
										IDI_PERFORCE_STATUS_OUTDATED_NOT_CHECKED_OUT,
										IDI_PERFORCE_STATUS_OUTDATED_CHECKED_OUT,
										IDI_PERFORCE_STATUS_OUTDATED_OTHER_CHECKOUT,
										IDI_PERFORCE_STATUS_OUTDATED_BOTH_CHECKOUT};

	const static CString k_tooltipText[] = { _T("Not checked out"),
											_T("Checked out"),
											_T("Added"),
											_T("Checked out by someone else"),	
											_T("Checked out by you and someone else"),
											_T("Outdated, Not checked out"),
											_T("Outdated, Checked out"),
											_T("Outdated, Checked out by someone else"),	
											_T("Outdated, Checked out by you and someone else") };

	const static CString k_operationNames[PerforceSourceControl::OperationID_ContextMenuCount] = { 0 /*OperationID_None*/,
																		   _T("Get Latest Version"), 
																		   _T("Submit Changes"),
																		   _T("Check Out"),
																		   _T("Lock"),
																		   _T("Unlock"),
																		   _T("Mark for Add"),
																		   _T("Mark for Delete"),
																		   _T("Rename"),
																		   _T("Move"),
																		   _T("Revert Changes"),
																		   _T("Resolve"),
																		   _T("Diff"),
																		   _T("File History") };

	const static CString s_csPerforceInstallRegistry = L"SOFTWARE\\PERFORCE\\";

	const static int AK_PERFORCE_MAX_SERVER_NAME_LENGTH = MAX_PATH;

	void FolderAddBackslash( CString& io_rPath )
	{
		if ( ! io_rPath.IsEmpty() && io_rPath[ io_rPath.GetLength() - 1 ] != L'\\' )
		{
			io_rPath += L'\\';
		}
	}

	void GetFilesUnder( const CString& in_csDirectory, CStringList& io_files )
	{
		CString csDirectory( in_csDirectory );
		FolderAddBackslash( csDirectory );

		CFileFind fileFind;
		BOOL bIsFileAvailable = fileFind.FindFile( csDirectory + L"*" ) ;

		while( bIsFileAvailable )
		{
			bIsFileAvailable = fileFind.FindNextFile();

			if ( fileFind.IsDots() || fileFind.IsHidden() )
				continue;

			if ( fileFind.IsDirectory() )
			{
				GetFilesUnder( fileFind.GetFilePath(), io_files );
			}
			else if( fileFind.GetFileName().Right(5) != L".akpk" )	// Skip akpk files
			{
				io_files.AddTail( fileFind.GetFilePath() );
			}
		}
	}

	void ExpandDirectories( const CStringList& in_files, CStringList& out_files )
	{
		POSITION fileNamePos = in_files.GetHeadPosition();
		while ( fileNamePos )
		{
			CString csFilename = in_files.GetAt( fileNamePos );
			if( ::PathIsDirectory( csFilename ) )
			{
				// Need to get the files recursively
				GetFilesUnder( csFilename, out_files );
			}
			else
			{
				out_files.AddTail( csFilename );
			}

			in_files.GetNext( fileNamePos );
		}
	}

	struct NoCaseStringComp
	{
		// Sort in alpha order case incensitive
		bool operator()(const CString& in_csStrL, const CString& in_csStrR) const
		{
			return ( in_csStrR.CompareNoCase(in_csStrL) > 0 );
		}
	};
}

PerforceSourceControl::PerforceSourceControl()
	: m_pUtilities( NULL )
	, m_pMissingFiles( NULL )
	, m_pFilenameToStatusMap( NULL )
	, m_operationResult( OperationResult_Succeed )
	, m_bPrintTextToDialog( true )
	, m_bDismissNextErrorMessageBox( false )
	, m_bCancelOperation( false )
	, m_bFileDiff( false )
	, m_bFirstResolve( false )
	, m_bMergeFileChanged( false )
	, m_bOnNewFilesAdd( false )
	, m_bOnNewFilesCheckOut( false )
	, m_bOnNewFilesDoOperation( false )
	, m_bAutoAcceptMode( false )
	, m_bUserCancelledLogin( false )
	, m_bNeedLogin( false )
	, m_bRetryUnicodeServer( false )
	, m_pClient( NULL )
	, m_pClientTag( NULL )
	, m_pOperationDialog( NULL )
	, m_dwOperationBeginTime( 0 )
	, m_dwOperationTimeout( 0 )
{
	// Load icons
	m_icons = new HICON[ IconsIndexes_Count ];

	for ( unsigned int i=0 ; i<IconsIndexes_Count ; ++i )
	{
		m_icons[i] = ::LoadIcon( AfxGetStaticModuleState()->m_hCurrentResourceHandle, MAKEINTRESOURCE( k_iconIDs[i] ) );
	}
}

PerforceSourceControl::~PerforceSourceControl()
{
    delete[] m_icons;
}

void PerforceSourceControl::GetPluginInfo( PluginInfo& out_rPluginInfo )
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	// Plug-in name and version
	CString csName = _T("Perforce");

	out_rPluginInfo.m_bstrName = csName.AllocSysString();
	out_rPluginInfo.m_uiVersion = k_uiVersion;

	// Functions availability
	out_rPluginInfo.m_bShowConfigDlgAvailable = true;
	out_rPluginInfo.m_dwUpdateCommandID = OperationID_GetLatestVersion;
	out_rPluginInfo.m_dwCommitCommandID = OperationID_Submit;
	out_rPluginInfo.m_dwRenameCommandID = OperationID_Rename;
	out_rPluginInfo.m_dwMoveCommandID = OperationID_Move;
	out_rPluginInfo.m_dwAddCommandID = OperationID_Add;
	out_rPluginInfo.m_dwDeleteCommandID = OperationID_Delete;
	out_rPluginInfo.m_dwRevertCommandID = OperationID_Revert;
	out_rPluginInfo.m_dwDiffCommandID = OperationID_Diff;
	out_rPluginInfo.m_dwCheckOutCommandID = OperationID_CheckOut;
	out_rPluginInfo.m_dwRenameNoUICommandID = OperationID_RenameNoUI;
	out_rPluginInfo.m_dwMoveNoUICommandID = OperationID_MoveNoUI;
	out_rPluginInfo.m_dwDeleteNoUICommandID = OperationID_DeleteNoUI;
	out_rPluginInfo.m_bStatusIconAvailable = true;
}

bool PerforceSourceControl::IsAvalaible()
{
	bool bAvalaible = true;

	CRegKey regKey;
	
	if ( regKey.Open( HKEY_LOCAL_MACHINE, s_csPerforceInstallRegistry, KEY_READ ) != ERROR_SUCCESS )
	{
		if ( regKey.Open( HKEY_CURRENT_USER, s_csPerforceInstallRegistry, KEY_READ ) != ERROR_SUCCESS )
			bAvalaible = false;
	}

	return bAvalaible;
}

// ISourceControl
void PerforceSourceControl::Init( ISourceControlUtilities* in_pUtilities, bool in_bAutoAccept )
{
	m_bAutoAcceptMode = in_bAutoAccept;

	m_pUtilities = in_pUtilities;

	{
		DWORD bEnabled = FALSE;
		m_pUtilities->GetUserPreferenceDword( k_csRegFolder + k_csRegKeyNewFilesAdd, bEnabled );
		m_bOnNewFilesAdd = bEnabled ? true : false;
	}

	{
		DWORD bEnabled = FALSE;
		m_pUtilities->GetUserPreferenceDword( k_csRegFolder + k_csRegKeyNewFilesCheckOut, bEnabled );
		m_bOnNewFilesCheckOut = bEnabled ? true : false;
	}
}

void PerforceSourceControl::InitClientFromRegistry( ClientApi& io_client, AK::Wwise::ISourceControlUtilities* in_pUtilities )
{
	const TCHAR chInvalid = (TCHAR)-1;
	TCHAR szValue[MAX_PATH] = { 0 };
	ULONG size = MAX_PATH;

	in_pUtilities->GetUserPreferenceString( k_csRegFolder + k_csRegKeyClient, szValue, size );
	CW2A strClient( szValue );
	io_client.SetClient( strClient );

	szValue[0] = chInvalid;
	size = MAX_PATH;

	in_pUtilities->GetUserPreferenceString( k_csRegFolder + k_csRegKeyServer, szValue, size );
	if( szValue[0] != chInvalid )
	{
		CString csServer( szValue );
		szValue[0] = chInvalid;
		in_pUtilities->GetUserPreferenceString( k_csRegFolder + k_csRegKeyPort, szValue, size );
		if( szValue[0] != chInvalid )
		{
			// Format: "server:port"
			CW2A strServerAndPort( ServerAndPort( csServer, szValue ) );
			io_client.SetPort( strServerAndPort );
		}
	}
	else
	{
		// Backward compatibility: "Port" used to contain "server:port"...
		in_pUtilities->GetUserPreferenceString( k_csRegFolder + k_csRegKeyPort, szValue, size );
		if( szValue[0] != chInvalid )
		{
			// Format: "port"
			CW2A strServerAndPort( szValue );
			io_client.SetPort( strServerAndPort );
		}
	}

	szValue[0] = chInvalid;
	size = MAX_PATH;
	
	in_pUtilities->GetUserPreferenceString( k_csRegFolder + k_csRegKeyHost, szValue, size );
	if( szValue[0] != chInvalid )
	{
		CW2A strClient( szValue );
		io_client.SetHost( strClient );
	}

	szValue[0] = chInvalid;
	size = MAX_PATH;

	in_pUtilities->GetUserPreferenceString( k_csRegFolder + k_csRegKeyUser, szValue, size );
	if( szValue[0] != chInvalid )
	{
		CW2A strClient( szValue );
		io_client.SetUser( strClient );
	}

	// Enable Unicode servers
	//const int utf8 = CharSetApi::UTF_8;
	//int charset = CharSetApi::Lookup(io_client.GetCharset().Text());
	//if (charset < 0) //charset env was not recognized
	//	charset = CharSetApi::NOCONV; 
	//io_client.SetTrans(utf8, charset, utf8, utf8);
}

void PerforceSourceControl::Term()
{
	// Terminate the client: close the connection
	Error error;
	TermClient( error );

	m_pUtilities->SetUserPreferenceDword( k_csRegFolder + k_csRegKeyNewFilesAdd, m_bOnNewFilesAdd ? TRUE : FALSE );
	m_pUtilities->SetUserPreferenceDword( k_csRegFolder + k_csRegKeyNewFilesCheckOut, m_bOnNewFilesCheckOut ? TRUE : FALSE );
}

void PerforceSourceControl::Destroy()
{
	delete this;
}

bool PerforceSourceControl::ShowConfigDlg()
{
	CDlgPerforceConfig configDlg( m_pUtilities );

	LRESULT uiResult = m_pUtilities->CreateModalCustomDialog( &configDlg );

	return uiResult == IDOK;
}

ISourceControl::OperationResult PerforceSourceControl::GetOperationList( OperationMenuType in_menuType, const StringList& in_rFilenameList, OperationList& out_rOperationList )
{
	static bool s_bInitialized = false;
	static OperationListItem s_operationItems[OperationID_ContextMenuCount];

	if ( !s_bInitialized  )
	{
		for ( unsigned int i=OperationID_None+1 ; i<OperationID_ContextMenuCount ; ++i )
		{
			s_operationItems[i].m_dwOperationID = i;
			s_operationItems[i].m_bEnabled = false;
		}

		s_bInitialized = true;
	}

	// Operations Enabled/Disabled
	OperationResult operationResult = OperationResult_Succeed;

	if ( in_rFilenameList.GetSize() )
	{
		operationResult = GetFileStatus( in_rFilenameList, NULL, true );
	}

	for ( unsigned int i=OperationID_None+1 ; i<OperationID_ContextMenuCount ; ++i )
		s_operationItems[i].m_bEnabled = operationResult == OperationResult_Succeed ? IsOperationValid( (OperationID)i, m_fileStatusMap ) : false;

	// Operation list construction
	switch( in_menuType )
	{
		case OperationMenuType_WorkUnits:
			{
				out_rOperationList.AddTail( s_operationItems[OperationID_GetLatestVersion] );
				out_rOperationList.AddTail( s_operationItems[OperationID_Submit] );
				out_rOperationList.AddTail( s_operationItems[OperationID_CheckOut] );
				out_rOperationList.AddTail( s_operationItems[OperationID_Lock] );
				out_rOperationList.AddTail( s_operationItems[OperationID_Unlock] );
				out_rOperationList.AddTail( s_operationItems[OperationID_Add] );
				out_rOperationList.AddTail( s_operationItems[OperationID_Delete] );
				out_rOperationList.AddTail( s_operationItems[OperationID_Rename] );
				out_rOperationList.AddTail( s_operationItems[OperationID_Move] );
				out_rOperationList.AddTail( s_operationItems[OperationID_Revert] );
				out_rOperationList.AddTail( s_operationItems[OperationID_Resolve] );
				out_rOperationList.AddTail( s_operationItems[OperationID_Diff] );
				out_rOperationList.AddTail( s_operationItems[OperationID_History] );
				break;
			}
			
		case OperationMenuType_Sources:
			{
				out_rOperationList.AddTail( s_operationItems[OperationID_GetLatestVersion] );
				out_rOperationList.AddTail( s_operationItems[OperationID_Submit] );
				out_rOperationList.AddTail( s_operationItems[OperationID_CheckOut] );
				out_rOperationList.AddTail( s_operationItems[OperationID_Lock] );
				out_rOperationList.AddTail( s_operationItems[OperationID_Unlock] );
				out_rOperationList.AddTail( s_operationItems[OperationID_Add] );
				out_rOperationList.AddTail( s_operationItems[OperationID_Delete] );
				out_rOperationList.AddTail( s_operationItems[OperationID_Rename] );
				out_rOperationList.AddTail( s_operationItems[OperationID_Move] );
				out_rOperationList.AddTail( s_operationItems[OperationID_Revert] );
				out_rOperationList.AddTail( s_operationItems[OperationID_Diff] );
				out_rOperationList.AddTail( s_operationItems[OperationID_History] );
				break;
			}
			
		case OperationMenuType_Explorer:
			{
				out_rOperationList.AddTail( s_operationItems[OperationID_Submit] );
				out_rOperationList.AddTail( s_operationItems[OperationID_CheckOut] );
				out_rOperationList.AddTail( s_operationItems[OperationID_Lock] );
				out_rOperationList.AddTail( s_operationItems[OperationID_Unlock] );
				out_rOperationList.AddTail( s_operationItems[OperationID_Add] );
				out_rOperationList.AddTail( s_operationItems[OperationID_Revert] );
				out_rOperationList.AddTail( s_operationItems[OperationID_Diff] );
				out_rOperationList.AddTail( s_operationItems[OperationID_History] );
				break;
			}
	}

	return operationResult;
}

LPCWSTR PerforceSourceControl::GetOperationName( DWORD in_dwOperationID )
{
	static bool s_bInitializedOperationNames = false;
	static CString s_operationNames[PerforceSourceControl::OperationID_ContextMenuCount];

	if ( !s_bInitializedOperationNames  )
	{
		for ( unsigned int i=PerforceSourceControl::OperationID_None+1 ; i<PerforceSourceControl::OperationID_ContextMenuCount ; ++i )
		{
			s_operationNames[i] = k_operationNames[i];
		}

		s_bInitializedOperationNames = true;
	}

	if( in_dwOperationID > OperationID_None && in_dwOperationID < OperationID_ContextMenuCount )
		return s_operationNames[in_dwOperationID];

	return L"";
}

DWORD PerforceSourceControl::GetOperationEffect( DWORD in_dwOperationID )
{
	DWORD dwRetVal = 0;

	switch( in_dwOperationID )
	{
	case OperationID_GetLatestVersion:
	case OperationID_Rename:
	case OperationID_Move:
	case OperationID_Revert:
	case OperationID_Delete:
		dwRetVal |= OperationEffect_LocalContentModification;
		break;

	case OperationID_Submit:
		dwRetVal |= OperationEffect_ServerContentModification;
		break;
	}

	return dwRetVal;
}

ISourceControl::OperationResult PerforceSourceControl::GetFileStatus( const StringList& in_rFilenameList, FilenameToStatusMap& out_rFileStatusMap, DWORD in_dwTimeoutMs )
{
	return GetFileStatus( in_rFilenameList, &out_rFileStatusMap, true, in_dwTimeoutMs );
}

ISourceControl::OperationResult PerforceSourceControl::GetFileStatus( 
	const StringList& in_rFilenameList, 
	FilenameToStatusMap* out_pFileStatusMap,
	bool in_bSkipDirectories,
	DWORD in_dwTimeoutMs )
{
	// Create a file name CStringList
	CStringList filenameList;
	for ( SourceControlContainers::AkPos pos = in_rFilenameList.GetHeadPosition() ; pos ; )
		filenameList.AddTail( in_rFilenameList.GetNext( pos ) );

	return GetFileStatus( filenameList, out_pFileStatusMap, in_bSkipDirectories, in_dwTimeoutMs );
}

ISourceControl::OperationResult PerforceSourceControl::GetFileStatus( 
	const CStringList& in_rFilenameList, 
	FilenameToStatusMap* out_pFileStatusMap,
	bool in_bSkipDirectories,
	DWORD in_dwTimeoutMs )
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	// Init parameters
	Operation operation( this, OperationID_GetFileStatus );
	m_pFilenameToStatusMap = out_pFileStatusMap;
	m_bPrintTextToDialog = false;

	// Convert filenames
	CStringList escapedFilenameList;
	GetValidPathList( &in_rFilenameList, &escapedFilenameList, in_bSkipDirectories );

	// Get the status of the files
	m_fileStatusMap.RemoveAll();
	OperationResult result = OperationResult_Succeed;
	
	if( escapedFilenameList.GetCount() )
		result = RunCommand("fstat", &escapedFilenameList, NULL, in_dwTimeoutMs );

	if ( result == OperationResult_Succeed )
	{
		// Unsetted values must be 'local only'
		POSITION fileListPos = in_rFilenameList.GetHeadPosition();

		CString csStatus = _T( "local only" );

		while ( fileListPos )
		{
			const CString csFileKey = FixFileMapKey( in_rFilenameList.GetAt( fileListPos ) );

			FileStatus fileStatus;
			bool bFound = m_fileStatusMap.Lookup( csFileKey, fileStatus ) == TRUE;

			if ( !bFound || ( fileStatus == FileStatus_NotAvailable ) )
			{
				if ( out_pFileStatusMap )
				{
					FilenameToStatusMapItem statusItem;
					if ( out_pFileStatusMap->Lookup( csFileKey, statusItem ) )
					{
						::SysFreeString( statusItem.m_bstrStatus );
						::SysFreeString( statusItem.m_bstrOwner );
					}
				
					FilenameToStatusMapItem defaultStatusItem = {0};
					if( ::PathIsDirectory( csFileKey ) )
						defaultStatusItem.m_bstrStatus = NULL;
					else
						defaultStatusItem.m_bstrStatus = csStatus.AllocSysString();

					out_pFileStatusMap->SetAt( csFileKey, defaultStatusItem );
				}
				
				m_fileStatusMap.SetAt( csFileKey, FileStatus_LocalOnly );
			}

			in_rFilenameList.GetNext( fileListPos );
		}
	}

	// Reset parameters
	m_pFilenameToStatusMap = NULL;
	m_bPrintTextToDialog = true;

	return result;
}

AK::Wwise::ISourceControl::OperationResult PerforceSourceControl::GetFilesForOperation( DWORD in_dwOperationID, const StringList& in_rFilenameList, StringList& out_rFilenameList, FilenameToStatusMap& out_rFileStatusMap )
{	
	// First get the file statuses
	ISourceControl::OperationResult eResult = GetFileStatus( in_rFilenameList, out_rFileStatusMap, INFINITE );
	if ( eResult == OperationResult_Succeed )
	{
		// Get files that are not checked out
		SourceControlContainers::AkPos filePos = in_rFilenameList.GetHeadPosition();
		for ( ;filePos; in_rFilenameList.GetNext( filePos ) )
		{
			LPCWSTR czFilename = in_rFilenameList.GetAt( filePos );

			FileStatus fileStatus;
			VERIFY( m_fileStatusMap.Lookup( FixFileMapKey( czFilename ), fileStatus ) );

			if( IsOperationValid( OperationID_Submit, fileStatus, czFilename, false ) )
			{
				// Found a committable file
				out_rFilenameList.AddTail( czFilename );
			}
		}
	}

	return eResult;
}

ISourceControl::OperationResult PerforceSourceControl::GetFileStatusIcons( const StringList& in_rFilenameList, FilenameToIconMap& out_rFileIconsMap, DWORD in_dwTimeoutMs )
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	OperationResult result = OperationResult_Succeed;

	// Get the status of the files
	result = GetFileStatus( in_rFilenameList, NULL, false, in_dwTimeoutMs );

	if ( result == OperationResult_Succeed )
	{
		SourceControlContainers::AkPos filePos = in_rFilenameList.GetHeadPosition();

		// Fill the "FileToIcon" map
		while ( filePos )
		{
			const CString csFileKey = FixFileMapKey( in_rFilenameList.GetAt( filePos ) );

			FileStatus fileStatus = FileStatus_NotAvailable;
			
			if ( m_fileStatusMap.Lookup( csFileKey, fileStatus ) )
			{
				HICON hIcon = NULL;
				CString csTooltipText;

				if ( (fileStatus & FileStatus_OtherCheckout) && (fileStatus & FileStatus_LocalEdit) && (fileStatus & FileStatus_Outdated) )
				{
					hIcon = m_icons[IconsIndexes_Outdated_BothCheckout];
					csTooltipText = k_tooltipText[IconsIndexes_Outdated_BothCheckout];
				}
				else if ( (fileStatus & FileStatus_OtherCheckout) && (fileStatus & FileStatus_LocalEdit) )
				{
					hIcon = m_icons[IconsIndexes_BothCheckout];
					csTooltipText = k_tooltipText[IconsIndexes_BothCheckout];
				}
				else if ( (fileStatus & FileStatus_OtherCheckout) && (fileStatus & FileStatus_Outdated) )
				{
					hIcon = m_icons[IconsIndexes_Outdated_OtherCheckout];
					csTooltipText = k_tooltipText[IconsIndexes_Outdated_OtherCheckout];
				}
				else if ( fileStatus & FileStatus_OtherCheckout )
				{
					hIcon = m_icons[IconsIndexes_OtherCheckout];
					csTooltipText = k_tooltipText[IconsIndexes_OtherCheckout];
				}
				else if ( (fileStatus & FileStatus_LocalEdit) && (fileStatus & FileStatus_Outdated) )
				{
					hIcon = m_icons[IconsIndexes_Outdated_CheckOut];
					csTooltipText = k_tooltipText[IconsIndexes_Outdated_CheckOut];
				}
				else if ( fileStatus & FileStatus_LocalEdit )
				{
					hIcon = m_icons[IconsIndexes_CheckOut];
					csTooltipText = k_tooltipText[IconsIndexes_CheckOut];
				}
				else if ( (fileStatus & FileStatus_OnServer) && (fileStatus & FileStatus_Outdated) )
				{
					hIcon = m_icons[IconsIndexes_Outdated_NotCheckOut];
					csTooltipText = k_tooltipText[IconsIndexes_Outdated_NotCheckOut];
				}
				else if ( fileStatus & FileStatus_Added || fileStatus & FileStatus_MoveAdd )
				{
					hIcon = m_icons[IconsIndexes_Added];
					csTooltipText = k_tooltipText[IconsIndexes_Added];
				}
				else if ( fileStatus & FileStatus_OnServer )
				{
					hIcon = m_icons[IconsIndexes_NotCheckOut];
					csTooltipText = k_tooltipText[IconsIndexes_NotCheckOut];
				}

				FilenameToIconMapItem iconItem = { hIcon, csTooltipText.AllocSysString() };
				out_rFileIconsMap.SetAt( csFileKey, iconItem );
			}

			in_rFilenameList.GetNext( filePos );
		}
	}

	return result;
}

ISourceControl::OperationResult PerforceSourceControl::UpdateWorkspaces()
{
	// Obtain the workspace root
	Operation operation( this, OperationID_Workspaces );
	
	m_workspaces.clear();
	return RunCommand("workspaces", NULL, NULL);
}

ISourceControl::OperationResult PerforceSourceControl::GetMissingFilesInDirectories( const StringList& in_rDirectoryList, StringList& out_rFilenameList )
{
	// Init parameters
	m_bPrintTextToDialog = false;

	CStringList localFilenameList;
	GetValidPathList( &in_rDirectoryList, &localFilenameList, false );

	OperationResult result = UpdateWorkspaces();

	if( result == OperationResult_Succeed )
	{
		// Obtain the deleted files
		Operation operation( this, OperationID_Opened );
		
		m_pMissingFiles = &out_rFilenameList;

		// Argument used to get missing files from the 'have' list
		CStringList argList;
		RunCommand("opened", &localFilenameList, &argList);

		m_pMissingFiles = NULL;
	}

	// Reset parameters
	m_bPrintTextToDialog = true;

	return result;
}

AK::Wwise::ISourceControl::IOperationResult* PerforceSourceControl::DoOperation( DWORD in_dwOperationID, const StringList& in_rFilenameList, const StringList* in_pTargetFilenameList )
{
	switch( in_dwOperationID )
	{
	case OperationID_RenameNoUI:
	case OperationID_MoveNoUI:
		return MoveNoUI( in_dwOperationID, in_rFilenameList, in_pTargetFilenameList );
	}

	// Create a file name CStringList
	CStringList filenameList, targetFilenameList;
	for ( SourceControlContainers::AkPos pos = in_rFilenameList.GetHeadPosition() ; pos ; )
		filenameList.AddTail( in_rFilenameList.GetNext( pos ) );

	return DoOperation( in_dwOperationID, filenameList );
}

AK::Wwise::ISourceControl::IOperationResult* PerforceSourceControl::DoOperation( DWORD in_dwOperationID, const CStringList& in_rFilenameList )
{
	AK::Wwise::ISourceControl::IOperationResult* pResult = NULL;

	// Get a valid filename list (correct directory path)
	CStringList escapedFilenameList;
	GetValidPathList( &in_rFilenameList, &escapedFilenameList );

	// Init parameters
	Operation operation( this, static_cast<OperationID>( in_dwOperationID ) );


	// Do the operation
	switch ( in_dwOperationID )
	{
		case OperationID_GetLatestVersion:
			GetLatestVersion( escapedFilenameList );
			break;
			
		case OperationID_Submit:
			Submit( escapedFilenameList );
			break;
			
		case OperationID_CheckOut:
			CheckOut( escapedFilenameList );
			break;
			
		case OperationID_Lock:
			Lock( escapedFilenameList );
			break;
			
		case OperationID_Unlock:
			Unlock( escapedFilenameList );
			break;
			
		case OperationID_Add:
			// The Add operation must use the original filename list to force the use of wildcards
			Add( in_rFilenameList );
			break;
			
		case OperationID_Delete:
		case OperationID_DeleteNoUI:
			// The Delete operation must use the original filename list to manage local file name (will be converted later)
			pResult = Delete( in_rFilenameList, in_dwOperationID == OperationID_Delete );
			break;
			
		case OperationID_Rename:
			// The Rename operation must use the original filename list to manage local file name (will be converted later)
			pResult = Rename( in_rFilenameList );
			break;

		case OperationID_Move:
			// The Move operation must use the original filename list to manage local file name (will be converted later)
			pResult = Move( in_rFilenameList );
			break;

		case OperationID_Revert:
			Revert( escapedFilenameList );
			break;
			
		case OperationID_Resolve:
			Resolve( escapedFilenameList );
			break;
			
		case OperationID_Diff:
			Diff( escapedFilenameList );
			break;
			
		case OperationID_History:
			History( escapedFilenameList );
			break;
	}

	return pResult;
}

AK::Wwise::ISourceControl::IOperationResult* PerforceSourceControl::MoveNoUI( DWORD in_dwOperationID, const StringList& in_rFilenameList, const StringList* in_pTargetFilenameList )
{
	FileOperationResult* pResult = NULL;

	ASSERT( !in_pTargetFilenameList || in_rFilenameList.GetCount() == in_pTargetFilenameList->GetCount() );

	// Init parameters
	Operation operation( this, static_cast<OperationID>( in_dwOperationID ) );

	// Do the operation
	if( (in_dwOperationID == OperationID_MoveNoUI || in_dwOperationID == OperationID_RenameNoUI ) &&
		in_pTargetFilenameList )
	{
		m_pUtilities->GetProgressDialog()->ShowProgress();

		pResult = new FileOperationResult();

		// Get file status once
		CStringList statusFileList;
		SourceControlContainers::AkPos srcPos = in_rFilenameList.GetHeadPosition();
		SourceControlContainers::AkPos dstPos = in_pTargetFilenameList->GetHeadPosition();
		for( ; srcPos ; )
		{
			statusFileList.AddTail( in_rFilenameList.GetNext( srcPos ) );
			statusFileList.AddTail( in_pTargetFilenameList->GetNext( dstPos ) );
		}

		bool bCanProceed = (GetFileStatus( statusFileList, NULL ) == OperationResult_Succeed);

		if( bCanProceed )
		{
			SourceControlContainers::AkPos srcPos = in_rFilenameList.GetHeadPosition();
			SourceControlContainers::AkPos dstPos = in_pTargetFilenameList->GetHeadPosition();
			for( ; srcPos ; )
			{
				CString csSource = in_rFilenameList.GetNext( srcPos );
				CString csTarget = in_pTargetFilenameList->GetNext( dstPos );

				Move( csSource, csTarget, pResult );
			}
		}

		m_pUtilities->GetProgressDialog()->OperationCompleted();
	}

	return pResult;
}

void PerforceSourceControl::GetLatestVersion( const CStringList& in_rFilenameList )
{
	m_pUtilities->GetProgressDialog()->ShowProgress();

	RunCommand("sync", &in_rFilenameList, NULL);

	m_pUtilities->GetProgressDialog()->OperationCompleted();
}

void PerforceSourceControl::Submit( const CStringList& in_rFilenameList )
{
	// Convert file name from client to depot format
	m_submitFileList.RemoveAll();
	Operation operation( this, OperationID_GetSubmitFiles );

	OperationResult result = RunCommand("where", &in_rFilenameList, NULL);

	if ( result == OperationResult_Succeed )
	{
		CDlgPerforceDescription descriptionDlg( m_pUtilities );
		INT_PTR dialogResult = m_pUtilities->CreateModalCustomDialog( &descriptionDlg );

		if ( dialogResult == IDOK )
		{
			{
				Operation operation( this, OperationID_Reopen );

				// Ensure all items are in the default change list, unless the submit will fail
				CStringList escapedFilenameList;
				GetValidPathList( &in_rFilenameList, &escapedFilenameList );

				CStringList argList;
				argList.AddTail( CString( L"-c" ) );
				argList.AddTail( CString( L"default" ) );
				result = RunCommand("reopen", &escapedFilenameList, &argList);
			}

			// Submit changes
			CStringList argList;
			Operation operation( this, OperationID_Submit );
			m_csDescription = descriptionDlg.GetDescription();
			
			m_pUtilities->GetProgressDialog()->ShowProgress();

			// Create argument list
			argList.AddTail( CString( L"-i" ) );

			if ( descriptionDlg.CheckOutAfterSubmit() )
			{
				argList.AddTail( CString( L"-r" ) );
			}

			ASSERT( in_rFilenameList.GetSize() == m_submitFileList.GetSize() );

			result = RunCommand("submit", NULL, &argList);
			
			m_pUtilities->GetProgressDialog()->OperationCompleted();
		}
	}
}

void PerforceSourceControl::CheckOut( const CStringList& in_rFilenameList )
{
	m_pUtilities->GetProgressDialog()->ShowProgress();

	RunCommand("edit", &in_rFilenameList, NULL);

	m_pUtilities->GetProgressDialog()->OperationCompleted( m_operationResult != OperationResult_Succeed );
}

void PerforceSourceControl::Lock( const CStringList& in_rFilenameList )
{
	m_pUtilities->GetProgressDialog()->ShowProgress();

	RunCommand("lock", &in_rFilenameList, NULL);

	m_pUtilities->GetProgressDialog()->OperationCompleted();
}

void PerforceSourceControl::Unlock( const CStringList& in_rFilenameList )
{
	m_pUtilities->GetProgressDialog()->ShowProgress();

	RunCommand("unlock", &in_rFilenameList, NULL);

	m_pUtilities->GetProgressDialog()->OperationCompleted();
}

void PerforceSourceControl::Add( const CStringList& in_rOriginalFilenameList )
{
	CStringList argList;
	argList.AddTail( L"-f" );	// Force wildcards characters

	// Expand directories to files under it
	CStringList files;
	ExpandDirectories( in_rOriginalFilenameList, files );

	if( files.GetCount() > 0 )
	{
		m_pUtilities->GetProgressDialog()->ShowProgress();

		RunCommand("add", &files, &argList );

		m_pUtilities->GetProgressDialog()->OperationCompleted( m_operationResult != OperationResult_Succeed );
	}
}

AK::Wwise::ISourceControl::IOperationResult* PerforceSourceControl::Delete( const CStringList& in_rOriginalFilenameList, bool in_bShowConfirmation )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if( in_bShowConfirmation )
	{
		CString csCaption;
		CString csMessage;

		csCaption = _T( "Delete confirmation" );
		csMessage = _T( "Are you sure you want to delete the selected file(s)?" );

		if( m_pUtilities->MessageBox( NULL, csMessage, csCaption, MB_YESNO ) != IDYES )
			return NULL;
	}

	FileOperationResult* pResult = NULL;

	m_pUtilities->GetProgressDialog()->ShowProgress();

	// Get the latest status of the files
	OperationResult result = GetFileStatus( in_rOriginalFilenameList, NULL );

	if ( result == OperationResult_Succeed )
	{
		CStringList localOnlyFilenameList;
		CStringList serverFilenameList;

		// Build local only and server filename list
		POSITION fileNamePos = in_rOriginalFilenameList.GetHeadPosition();

		// Split local files and server files
		while ( fileNamePos )
		{
			CString csFilename = in_rOriginalFilenameList.GetAt( fileNamePos );

			FileStatus fileStatus;
			VERIFY( m_fileStatusMap.Lookup( FixFileMapKey( csFilename ), fileStatus ) );

			// Do not continue if any of the files is opened on Perforce
			if( (fileStatus & FileStatus_Added) || 
				(fileStatus & FileStatus_Deleted) ||
				(fileStatus & FileStatus_LocalEdit) ||
				(fileStatus & FileStatus_MoveAdd) ||
				(fileStatus & FileStatus_MoveDelete))
			{
				CString csMessage;
				csMessage.FormatMessage( _T("File '%1' is opened.  Can not delete a file that is currently opened."), csFilename );
				m_pUtilities->GetProgressDialog()->AddLogMessage( csMessage );
				result = OperationResult_Failed;
			}

			if( ::PathIsDirectory( csFilename ) )
			{
                localOnlyFilenameList.AddTail( csFilename );

				FolderAddBackslash( csFilename );
				csFilename += L"...";
				serverFilenameList.AddTail( EscapeWildcards( csFilename ) );
			}
			else if ( fileStatus & FileStatus_LocalOnly )
			{
				localOnlyFilenameList.AddTail( csFilename );
			}
			else
			{
				serverFilenameList.AddTail( EscapeWildcards( csFilename ) );
			}

			in_rOriginalFilenameList.GetNext( fileNamePos );
		}

		if ( result == OperationResult_Succeed )
		{
			pResult = new FileOperationResult();

			// Delete files on server
			if ( serverFilenameList.GetSize() )
			{
				OperationResult result = RunCommand("delete", &serverFilenameList, NULL);
				if( result == OperationResult_Succeed )
				{
					// Save results
					for ( POSITION filePos = serverFilenameList.GetHeadPosition() ; filePos ; )
					{
						// Verify if file exist
						CString csFilename = serverFilenameList.GetNext( filePos );
						if( !::PathFileExists( csFilename ) )
							pResult->AddFile( csFilename );
					}
				}
			}

			// Delete local files
			if ( localOnlyFilenameList.GetSize() )
			{
				CString csMessage;
				POSITION filePos = localOnlyFilenameList.GetHeadPosition();

				while ( filePos )
				{
					CString csFilename = localOnlyFilenameList.GetAt( filePos );

					bool bSuccess = false;
					if( ::PathIsDirectory( csFilename ) )
					{
						bSuccess = ::RemoveDirectory( csFilename ) != FALSE;
					}
					else
					{
						bSuccess = ::DeleteFile( csFilename ) != FALSE;
					}

					if ( bSuccess )
					{
						csMessage.Format( _T("Successfully deleted local file %s"), csFilename );
						pResult->AddFile( csFilename );
					}
					else
					{
						csMessage.Format( _T("Error deleting local file %s (%s)"), csFilename, SourceControlHelpers::GetLastErrorString() );
					}

					m_pUtilities->GetProgressDialog()->AddLogMessage( csMessage );

					localOnlyFilenameList.GetNext( filePos );
				}
			}
		}

		m_pUtilities->GetProgressDialog()->OperationCompleted();
	}

	return pResult;
}

AK::Wwise::ISourceControl::IOperationResult* PerforceSourceControl::Rename( 
	const CStringList& in_rOriginalFilenameList )
{
	FileOperationResult* pResult = NULL;

	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	ASSERT( in_rOriginalFilenameList.GetSize() == 1 );

	CString csOldFilename = in_rOriginalFilenameList.GetHead();

	// Get the folder and the initial name
	CDlgRename renameDialog( csOldFilename, m_pUtilities );
	INT_PTR dialogResult = m_pUtilities->CreateModalCustomDialog( &renameDialog );

	if ( dialogResult == IDOK )
	{
		m_pUtilities->GetProgressDialog()->ShowProgress();

		pResult = new FileOperationResult();

		// Get the status of the files
		CStringList filenameList;
		filenameList.AddTail( csOldFilename );
		filenameList.AddTail( renameDialog.GetNewFilename() );

		OperationResult result = GetFileStatus( filenameList, NULL, false );

		if ( result == OperationResult_Succeed )
		{
			Move( csOldFilename, renameDialog.GetNewFilename(), pResult );
		}
		
		m_pUtilities->GetProgressDialog()->OperationCompleted();
	}

	return pResult;
}

AK::Wwise::ISourceControl::IOperationResult* PerforceSourceControl::Move( const CStringList& in_rFilenameList )
{
	FileOperationResult* pResult = NULL;

	AFX_MANAGE_STATE( AfxGetStaticModuleState() );
	
	// Wwise makes sure the move operation is done only on sources of the same language
	// Find the language root
	wchar_t szRootPath[MAX_PATH] = {0};
	m_pUtilities->GetMoveRootPath( in_rFilenameList.GetHead(), szRootPath, ARRAYSIZE(szRootPath) );

	CString csRoot( szRootPath );

	CString csPrompt;
	csPrompt = _T("Choose the destination folder for the Move operation.");

	wchar_t szDestinationDir[MAX_PATH] = {0};
	if ( m_pUtilities->ShowBrowseForFolderDialog(
			csPrompt, 
			szDestinationDir,
			ARRAYSIZE(szDestinationDir),
			csRoot ) )
	{
		CString csMsg;
		m_pUtilities->GetProgressDialog()->ShowProgress();

		std::vector<CString> newPaths;
		bool bCanProceed = SourceControlHelpers::CanProceedWithMove( in_rFilenameList, szDestinationDir, newPaths, m_pUtilities,
			_T("Error: The following file already exist in the destination directory: %1"),
			_T("Error: Multiple files with the same name can't be moved at the same destination (%1).") );

		if( bCanProceed )
		{
			// Get file status once
			CStringList statusFileList;
			unsigned int i = 0;
			for( POSITION position = in_rFilenameList.GetHeadPosition(); position; in_rFilenameList.GetNext(position), ++i )
			{
				statusFileList.AddTail( in_rFilenameList.GetAt(position) );
				statusFileList.AddTail( newPaths[i] );
			}

			bCanProceed = (GetFileStatus( statusFileList, NULL ) == OperationResult_Succeed);
		}

		if( bCanProceed )
		{
			ASSERT( newPaths.size() == in_rFilenameList.GetCount() );

			// Create the move results
			pResult = new FileOperationResult();

			int i = 0;
			for( POSITION position = in_rFilenameList.GetHeadPosition(); position; in_rFilenameList.GetNext(position), ++i )
			{
				Move( in_rFilenameList.GetAt(position), newPaths[i], pResult );
			}
		}

		m_pUtilities->GetProgressDialog()->OperationCompleted();
	}

	return pResult;
}

void PerforceSourceControl::Move( 
	const CString& in_csFrom, 
	const CString& in_csTo, 
	FileOperationResult* io_pResult )
{
	// Get the fixed version of the file name
	const CString csFixedFrom = FixFileMapKey( in_csFrom );
	const CString csFixedTo = FixFileMapKey( in_csTo );

	// Local or server file renaming
	FileStatus fileStatusFrom;
	VERIFY( m_fileStatusMap.Lookup( csFixedFrom, fileStatusFrom ) );

	bool bIsDirectory = ::PathIsDirectory(in_csFrom) != 0;

	// When m_fileStatusMap.GetCount() == 2, it means only the source and destination were found in the status map
	// When we move/rename a directory, it means there are no files in the depot.
	if ( (fileStatusFrom & FileStatus_LocalOnly) && (!bIsDirectory || m_fileStatusMap.GetCount() == 2) )
	{
		// File is not in repository yet, move it
		CString csMessage;

		if ( ::MoveFile( in_csFrom, in_csTo ) )
		{
			csMessage.Format( _T("Successfully moved local file %s to %s"), (LPCTSTR)in_csFrom, (LPCTSTR)in_csTo );
			io_pResult->AddMovedFile( in_csFrom, in_csTo );
		}
		else
		{
			csMessage.Format( _T("Error moving local file %s (%s)"), (LPCTSTR)in_csFrom, SourceControlHelpers::GetLastErrorString() );
		}

		m_pUtilities->GetProgressDialog()->AddLogMessage( csMessage );
	}
	else
	{			
		// We must reinit parameters since GetFileStatus modify them
		Operation operation( this, OperationID_Move );

		FileStatus fileStatusTo;
		VERIFY( m_fileStatusMap.Lookup( csFixedTo, fileStatusTo ) );

		if ( !( fileStatusTo & FileStatus_LocalOnly ) || ::PathFileExists( in_csTo ) )
		{
			// Failed, name conflict
			CString csMsg;
			csMsg.Format( _T("Can't rename %s to %s, file already exists"), in_csFrom, in_csTo );

			m_pUtilities->GetProgressDialog()->AddLogMessage( csMsg );
		}
		else
		{
			CStringList filenameList;
			filenameList.AddTail( in_csFrom );
			filenameList.AddTail( in_csTo );

			CStringList escapedFilenameList;
			GetValidPathList( &filenameList, &escapedFilenameList );

			// Run "edit" first on the source
			CStringList escapedEditFilenameList;
			escapedEditFilenameList.AddTail( escapedFilenameList.GetHead() );
			RunCommand("edit", &escapedEditFilenameList, NULL);

			OperationResult result = RunCommand("move", &escapedFilenameList, NULL);

			if ( result == OperationResult_Succeed && ::PathFileExists( in_csTo ) )
			{
				io_pResult->AddMovedFile( in_csFrom, in_csTo );
			}
		}
	}
}

void PerforceSourceControl::Revert( const CStringList& in_rFilenameList )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CString csCaption, csMessage;
	csCaption = _T("Perforce plug-in");
	csMessage = _T("Are you sure you want to revert the selected item(s)? You will lose all changes since the last update.");

	if( m_pUtilities->MessageBox( NULL, csMessage, csCaption, MB_YESNO ) == IDYES )
	{
		m_pUtilities->GetProgressDialog()->ShowProgress();

		RunCommand("revert", &in_rFilenameList, NULL);

		m_pUtilities->GetProgressDialog()->OperationCompleted();
	}
}

void PerforceSourceControl::Resolve( const CStringList& in_rFilenameList )
{
	m_pUtilities->GetProgressDialog()->ShowProgress();

	m_bFirstResolve = true;

	CStringList argList;
	argList.AddTail( CString( L"-o" ) );
	argList.AddTail( CString( L"-t" ) );

	RunCommand("resolve", &in_rFilenameList, &argList);

	m_pUtilities->GetProgressDialog()->OperationCompleted();
}

void PerforceSourceControl::Diff( const CStringList& in_rFilenameList )
{
	m_bFileDiff = false;

	CStringList argList;

	RunCommand("diff", &in_rFilenameList, &argList);

	// If the ClientUser diff function has not been called, files are identical
	if ( !m_bFileDiff )
	{
		AFX_MANAGE_STATE(AfxGetStaticModuleState());

		CString csCaption;
		CString csMessage;

		csCaption = _T("Perforce plug-in");
		csMessage = _T("Files are identical");

		m_pUtilities->MessageBox( NULL, csMessage, csCaption, MB_OK | MB_ICONINFORMATION );
	}
}

void PerforceSourceControl::History( const CStringList& in_rFilenameList )
{
	ASSERT( in_rFilenameList.GetSize() == 1 ); // This has never been tested with multiple files, no guarantee it will work!

	m_pUtilities->GetProgressDialog()->ShowProgress();

	RunCommand("filelog", &in_rFilenameList, NULL);

	m_pUtilities->GetProgressDialog()->OperationCompleted();
}

ISourceControl::OperationResult PerforceSourceControl::PreCreateOrModify( const StringList& in_rFilenameList, CreateOrModifyOperation in_eOperation, bool& out_rContinue )
{
	CStringList notCheckedOutFiles;
	std::vector< LPCWSTR > notCheckedOutFilesList;
	OperationResult result = OperationResult_Succeed;

	ASSERT( in_eOperation );
	
	// Get the status of the files to know if we need to check out files
	if ( in_eOperation & CreateOrModifyOperation_Modify )
	{
		result = GetFileStatus( in_rFilenameList, NULL );

		if ( result == OperationResult_Succeed )
		{
			// Get files that are not checked out
			SourceControlContainers::AkPos filePos = in_rFilenameList.GetHeadPosition();

			while ( filePos )
			{
				LPCWSTR czFilename = in_rFilenameList.GetAt( filePos );

				FileStatus fileStatus;
				VERIFY( m_fileStatusMap.Lookup( FixFileMapKey( czFilename ), fileStatus ) );

				if ( !( fileStatus & FileStatus_LocalEdit ) && 
					( fileStatus & FileStatus_OnServer ) )
				{
					notCheckedOutFiles.AddTail( czFilename );
					notCheckedOutFilesList.push_back( czFilename );
				}

				in_rFilenameList.GetNext( filePos );
			}
		}
	}

	bool bHasFilesToCheckOut = notCheckedOutFiles.GetSize() ? true : false;
	bool bHasFilesToAdd = ( in_eOperation & CreateOrModifyOperation_Create ) ? true : false;
	
	// Show the CheckOut/Add dialog only if operations can be done on files
	if ( bHasFilesToCheckOut || bHasFilesToAdd )
	{
		if( m_bAutoAcceptMode )
		{
			m_bOnNewFilesDoOperation = true;
			out_rContinue = true;
			m_bOnNewFilesAdd = bHasFilesToAdd;
			m_bOnNewFilesCheckOut = bHasFilesToCheckOut;
		}
		else
		{
			CDlgPerforceOnNewFiles newFiles( 
				bHasFilesToAdd, 
				bHasFilesToCheckOut, 
				m_bOnNewFilesAdd, 
				m_bOnNewFilesCheckOut, 
				m_pUtilities, 
				notCheckedOutFilesList.empty() ? NULL : &(notCheckedOutFilesList[0]),
				(unsigned int)notCheckedOutFilesList.size() );

			m_pUtilities->CreateModalCustomDialog( &newFiles );
			INT_PTR uiResult = newFiles.GetResult();

			m_bOnNewFilesDoOperation = uiResult == ID_OK;
			out_rContinue = uiResult != ID_CANCEL;

			if( out_rContinue )
			{
				m_bOnNewFilesAdd = newFiles.NeedToAddFiles();
				m_bOnNewFilesCheckOut = newFiles.NeedToCheckOutFiles();
			}
		}
	}
	else
	{
		m_bOnNewFilesDoOperation = false;
		out_rContinue = true;
	}

	// We need to check out files
	if( m_bOnNewFilesDoOperation && m_bOnNewFilesCheckOut && bHasFilesToCheckOut )
	{
		DoOperation( OperationID_CheckOut, notCheckedOutFiles );
	}

	return result;
}

ISourceControl::OperationResult PerforceSourceControl::PostCreateOrModify( const StringList& in_rFilenameList, CreateOrModifyOperation in_eOperation, bool& out_rContinue )
{
	out_rContinue = true;

	if ( m_bOnNewFilesDoOperation && ( in_eOperation & CreateOrModifyOperation_Create )  )
	{
		// Add files
		if ( m_bOnNewFilesAdd )
		{
			OperationResult result = GetFileStatus( in_rFilenameList, NULL );

			if ( result == OperationResult_Succeed )
			{
				// Get files that need to be added
				CStringList toAddFiles;
				SourceControlContainers::AkPos filePos = in_rFilenameList.GetHeadPosition();

				while ( filePos )
				{
					const CString csFilename = in_rFilenameList.GetAt( filePos );

					FileStatus fileStatus;

					VERIFY( m_fileStatusMap.Lookup( FixFileMapKey( csFilename ), fileStatus ) );

					if ( fileStatus & FileStatus_LocalOnly )
					{
						toAddFiles.AddTail( csFilename );
					}

					in_rFilenameList.GetNext( filePos );
				}

				if ( toAddFiles.GetSize() )
				{
					DoOperation( OperationID_Add, toAddFiles );
				}
			}
		}
	}

	return OperationResult_Succeed;
}

bool PerforceSourceControl::ExtractServerAndPort( LPCTSTR in_szString, CString& out_csServer, CString& out_csPort )
{
	bool bResult = false;

	UINT unPort = 0;

	// Note: in_szString might be in the "server:port" format, or just "port" (for local server)

	const int nScanCount = _stscanf_s( in_szString, _T("%[^:]:%d"), out_csServer.GetBuffer( AK_PERFORCE_MAX_SERVER_NAME_LENGTH ), AK_PERFORCE_MAX_SERVER_NAME_LENGTH, &unPort );
	out_csServer.ReleaseBuffer();

	if ( nScanCount == 2 )
	{
		// Format: "server:port"
		out_csPort.Format( _T("%d"), unPort );
		bResult = true;
	}
	else if ( _stscanf_s( in_szString, _T("%d"), &unPort ) == 1 )
	{
		// Format: "port" (no server)
		out_csPort.Format( _T("%d"), unPort );
		out_csServer.Empty();
		bResult = true;
	}

	return bResult;
}

CString PerforceSourceControl::ServerAndPort( LPCTSTR in_szServer, LPCTSTR in_szPort )
{
	ASSERT( in_szServer && in_szServer );

	CString csServerAndPort;

	if ( _tcslen( in_szServer ) > 0 )
	{
		// Format: "server:port"
		csServerAndPort.Format( _T("%s:%s"), in_szServer, in_szPort );
	}
	else
	{
		// Format: "port" (no server)
		csServerAndPort = in_szPort;
	}

	return csServerAndPort;
}
