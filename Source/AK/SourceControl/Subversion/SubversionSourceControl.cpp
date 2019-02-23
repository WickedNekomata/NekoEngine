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
#include "DlgCommitMessage.h"
#include "DlgLockMessage.h"
#include "DlgRename.h"
#include "DlgOnCreateModify.h"
#include "DlgConfiguration.h"
#include "svn_time.h"
#include "svn_path.h"
#include "svn_utf.h"
#include "atlbase.h"
#include "OperationResults.h"
#include "SourceControlHelpers.h"

#include "Resource.h"

#include <vector>
#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace AK;
using namespace Wwise;

#define SVN_DATE_BUFFER 260

namespace
{
	enum IconsIndexes
	{
		IconsIndexes_Normal = 0,
		IconsIndexes_Modified,
		IconsIndexes_Added,

		IconsIndexes_Count
	};

	const static UINT k_tooltipIcons[] = { IDI_SUBVERSION_NORMAL,
											IDI_SUBVERSION_MODIFIED,
											IDI_SUBVERSION_ADD };

	const static CString k_tooltipText[] = { _T("Normal"),
											_T("Modified"),
											_T("Added") };

	static const CString s_operationNames[SubversionSourceControl::OperationIDCount] = { _T("Update"),
																						_T("Commit"),
																						_T("Get Lock"),
																						_T("Release Lock"),
																						_T("Add"),
																						_T("Delete"),
																						_T("Rename"),
																						_T("Move"),
																						_T("Revert"),
																						_T("Resolved"),
																						_T("Diff"),
																						_T("Show Log") };

	const static CString k_csRegFolder = L"Version 1\\";
	const static CString k_csRegKeyNewFilesAdd = L"NewFilesAdd";
	const static CString k_csRegKeyDiffToolPath = L"DiffToolPath";
	const static CString k_csUseAKWaveViewerForDiff = L"UseAKWaveViewerForDiff";
};

SubversionSourceControl::SubversionSourceControl()
	: m_pUtilities( NULL )
	, m_pGlobalPool( NULL )
	, m_pContext( NULL )
	, m_bOnNewFilesAdd( false )
	, m_bAutoAcceptMode( false )
	, m_bUseAKWavViewerToDiff ( true )
{
	// Load icons
	m_icons = new HICON[ IconsIndexes_Count ];

	for ( unsigned int i=0 ; i<IconsIndexes_Count ; ++i )
	{
		m_icons[i] = ::LoadIcon( AfxGetStaticModuleState()->m_hCurrentResourceHandle, MAKEINTRESOURCE( k_tooltipIcons[i] ) );
	}
}

SubversionSourceControl::~SubversionSourceControl()
{
    delete[] m_icons;
}

void SubversionSourceControl::GetPluginInfo( PluginInfo& out_rPluginInfo )
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	// Plug-in name and version
	CString csName;
	csName = _T("Subversion");

	out_rPluginInfo.m_bstrName = csName.AllocSysString();
	out_rPluginInfo.m_uiVersion = 1;

	// Functions availability
	out_rPluginInfo.m_bShowConfigDlgAvailable = true;
	out_rPluginInfo.m_dwUpdateCommandID = OperationID_Update;
	out_rPluginInfo.m_dwCommitCommandID = OperationID_Commit;
	out_rPluginInfo.m_dwRenameCommandID = OperationID_Rename;
	out_rPluginInfo.m_dwMoveCommandID = OperationID_Move;
	out_rPluginInfo.m_dwAddCommandID = OperationID_Add;
	out_rPluginInfo.m_dwDeleteCommandID = OperationID_Delete;
	out_rPluginInfo.m_dwRevertCommandID = OperationID_Revert;
	out_rPluginInfo.m_dwDiffCommandID = OperationID_Diff;
	out_rPluginInfo.m_dwCheckOutCommandID = AK::Wwise::SourceControlConstant::s_dwInvalidOperationID;
	out_rPluginInfo.m_dwRenameNoUICommandID = OperationID_RenameNoUI;
	out_rPluginInfo.m_dwMoveNoUICommandID = OperationID_MoveNoUI;
	out_rPluginInfo.m_dwDeleteNoUICommandID = OperationID_DeleteNoUI;
	out_rPluginInfo.m_bStatusIconAvailable = true;
}

// ISourceControl
void SubversionSourceControl::Init( ISourceControlUtilities* in_pUtilities, bool in_bAutoAccept )
{
	m_bAutoAcceptMode = in_bAutoAccept;

	m_pUtilities = in_pUtilities;

	// APR pool initialization.
	apr_initialize();
	m_pGlobalPool = svn_pool_create( NULL );

	svn_utf_initialize( m_pGlobalPool );

	// SVN Context initialization
	svn_error_clear(svn_client_create_context( &m_pContext, m_pGlobalPool ));

	m_pContext->cancel_func = (svn_cancel_func_t)CancelCallback;
	m_pContext->cancel_baton = m_pUtilities;
	m_pContext->notify_func2 = (svn_wc_notify_func2_t)NotifyCallback;
	m_pContext->notify_baton2 = this;
	m_pContext->log_msg_func2 = (svn_client_get_commit_log2_t)LogMessageCallback;
	m_pContext->log_msg_baton2 = "";
	m_pContext->notify_func = NULL;	// Deprecated callback
	m_pContext->notify_baton = NULL;
	m_pContext->log_msg_func = NULL;	// Deprecated callback
	m_pContext->log_msg_baton = NULL;
	m_pContext->progress_func = NULL; // Transferred bytes information is not used
	m_pContext->progress_baton = NULL;
	m_pContext->config = NULL; // ?

	// Set up authentication
	svn_auth_provider_object_t* pProvider = NULL;
	apr_array_header_t* pProviders = apr_array_make( m_pGlobalPool, 6, sizeof ( svn_auth_provider_object_t* ) );

	// The main disk-caching auth providers, for both
	// 'username/password' creds and 'username' creds.
	svn_client_get_windows_simple_provider( &pProvider, m_pGlobalPool );
	APR_ARRAY_PUSH( pProviders, svn_auth_provider_object_t* ) = pProvider;
	svn_auth_get_simple_provider(&pProvider, m_pGlobalPool);
	APR_ARRAY_PUSH(pProviders, svn_auth_provider_object_t *) = pProvider;
	svn_client_get_username_provider(&pProvider, m_pGlobalPool);
	APR_ARRAY_PUSH( pProviders, svn_auth_provider_object_t* ) = pProvider;


	// The server-cert, client-cert, and client-cert-password providers.
	svn_auth_get_platform_specific_provider(&pProvider, "windows", "ssl_server_trust", m_pGlobalPool);
	if (pProvider)
		APR_ARRAY_PUSH(pProviders, svn_auth_provider_object_t *) = pProvider;
	svn_auth_get_ssl_server_trust_file_provider(&pProvider, m_pGlobalPool);
	APR_ARRAY_PUSH(pProviders, svn_auth_provider_object_t *) = pProvider;
	svn_auth_get_ssl_client_cert_file_provider(&pProvider, m_pGlobalPool);
	APR_ARRAY_PUSH(pProviders, svn_auth_provider_object_t *) = pProvider;
	svn_auth_get_ssl_client_cert_pw_file_provider(&pProvider, m_pGlobalPool);
	APR_ARRAY_PUSH(pProviders, svn_auth_provider_object_t *) = pProvider;

	// Three prompting providers for server-certs, client-certs,
	// and client-cert-passphrases. 
	svn_auth_get_ssl_server_trust_prompt_provider(&pProvider, SslServerPromptCallback, NULL, m_pGlobalPool);
	APR_ARRAY_PUSH(pProviders, svn_auth_provider_object_t *) = pProvider;

	// Build the authentication baton
	svn_auth_open( &m_pContext->auth_baton, pProviders, m_pGlobalPool );

	// Load Subversion configuration
	DWORD bEnabled = FALSE;
	m_pUtilities->GetUserPreferenceDword( k_csRegFolder + k_csRegKeyNewFilesAdd, bEnabled );
	m_bOnNewFilesAdd = bEnabled ? true : false;

	TCHAR szValue[MAX_PATH] = { 0 };
	ULONG size = MAX_PATH;

	const TCHAR chInvalid = (TCHAR)-1;
	szValue[0] = chInvalid;
	m_pUtilities->GetUserPreferenceString( k_csRegKeyDiffToolPath, szValue, size );
	if( szValue[0] != chInvalid )
		m_csDiffToolPath = szValue;

	DWORD dwUseAKWavViewerToDiff = 1;
	m_pUtilities->GetUserPreferenceDword( k_csRegFolder + k_csUseAKWaveViewerForDiff, dwUseAKWavViewerToDiff );
	m_bUseAKWavViewerToDiff = dwUseAKWavViewerToDiff ? true : false;
}

svn_error_t* SubversionSourceControl::SslServerPromptCallback(svn_auth_cred_ssl_server_trust_t **cred_p, void *baton, const char *realm, apr_uint32_t failures, const svn_auth_ssl_server_cert_info_t *cert_info, svn_boolean_t may_save, apr_pool_t *pool)
{
	// Accept once the SSL server trust
	// Taken from SVNPrompt.cpp from TortoiseSVN
	*cred_p = (svn_auth_cred_ssl_server_trust_t*)apr_pcalloc(pool, sizeof(**cred_p));
	(*cred_p)->may_save = FALSE;

	return SVN_NO_ERROR;
}

void SubversionSourceControl::Term()
{
	// Save Subversion configuration
	m_pUtilities->SetUserPreferenceDword( k_csRegFolder + k_csRegKeyNewFilesAdd, m_bOnNewFilesAdd ? TRUE : FALSE );
	m_pUtilities->SetUserPreferenceString( k_csRegFolder + k_csRegKeyDiffToolPath, m_csDiffToolPath );
	m_pUtilities->SetUserPreferenceDword( k_csRegFolder + k_csUseAKWaveViewerForDiff, m_bUseAKWavViewerToDiff ? 1 : 0 );

	// Pool termination
	svn_pool_destroy( m_pGlobalPool );
    apr_terminate();

	m_pUtilities = NULL;
	m_pGlobalPool = NULL;
	m_pContext = NULL;
}

void SubversionSourceControl::Destroy()
{
	delete this;
}

bool SubversionSourceControl::ShowConfigDlg()
{
	bool bAcceptChanges = false;
	CDlgConfiguration configurationDlg( m_pUtilities, m_csDiffToolPath, m_bUseAKWavViewerToDiff );

	if ( m_pUtilities->CreateModalCustomDialog( &configurationDlg ) == IDOK )
	{
		bAcceptChanges = true;
		m_csDiffToolPath = configurationDlg.GetDiffToolPath();
		m_bUseAKWavViewerToDiff = configurationDlg.GetUseAKWavViewer();
	}

	return bAcceptChanges;
}

AK::Wwise::ISourceControl::OperationResult SubversionSourceControl::GetFilesForOperation( DWORD in_dwOperationID, const StringList& in_rFilenameList, StringList& out_rFilenameList, FilenameToStatusMap& out_rFileStatusMap )
{	
	// First get the file statuses
	SvnStatusList statusList;
	ISourceControl::OperationResult eResult = _GetFileStatus( in_rFilenameList, out_rFileStatusMap, INFINITE, &statusList );
	if ( eResult == OperationResult_Succeed )
	{
		// Fill list
		POSITION statusPos = statusList.GetHeadPosition();
		AK::Wwise::SourceControlContainers::AkPos filePos = in_rFilenameList.GetHeadPosition();
		for (  ; statusPos && filePos ; )
		{
			const SvnStatus& status = statusList.GetNext( statusPos );
			LPCWSTR szFile = in_rFilenameList.GetNext( filePos );

			if( IsOperationValid( OperationID_Commit, status, false ) )
			{
				// Found a committable file
				out_rFilenameList.AddTail( szFile );
			}
		}
	}

	return eResult;
}

ISourceControl::OperationResult SubversionSourceControl::GetOperationList( OperationMenuType in_menuType, const StringList& in_rFilenameList, OperationList& out_rOperationList )
{
	// Initialize operation list items
	static bool s_bInitialized = false;
	static OperationListItem s_operationsItems[OperationIDCount];

	if ( !s_bInitialized )
	{
		for ( unsigned int i=0 ; i<OperationIDCount ; ++i )
		{
			s_operationsItems[i].m_bEnabled = true;
			s_operationsItems[i].m_dwOperationID = i;
		}

		s_bInitialized = true;
	}

	// Update operation 'Enable' field
	SvnStatusList statusList;
	SvnPool subPool( m_pGlobalPool );
	for ( SourceControlContainers::AkPos filePos = in_rFilenameList.GetHeadPosition() ; filePos ; )
	{
		SvnStatus status;
		CString csFilename( in_rFilenameList.GetNext( filePos ) );

		subPool.Clear();

		svn_error_t* pError = GetSVNFileStatus( csFilename, status, m_pContext, subPool );
		svn_error_clear( pError );
		
		statusList.AddTail( status );
	}

	for ( unsigned int i=0 ; i<OperationIDCount ; ++i )
		s_operationsItems[i].m_bEnabled = IsOperationValid( (OperationID)i, statusList, ( in_rFilenameList.GetCount() > 1 ) ? true : false );

	// Fill the operation list
	switch( in_menuType )
	{
		case OperationMenuType_WorkUnits:
			out_rOperationList.AddTail( s_operationsItems[OperationID_Update] );
			out_rOperationList.AddTail( s_operationsItems[OperationID_Commit] );
			out_rOperationList.AddTail( s_operationsItems[OperationID_GetLock] );
			out_rOperationList.AddTail( s_operationsItems[OperationID_ReleaseLock] );
			out_rOperationList.AddTail( s_operationsItems[OperationID_Add] );
			out_rOperationList.AddTail( s_operationsItems[OperationID_Delete] );
			out_rOperationList.AddTail( s_operationsItems[OperationID_Rename] );
			out_rOperationList.AddTail( s_operationsItems[OperationID_Revert] );
			out_rOperationList.AddTail( s_operationsItems[OperationID_Move] );
			out_rOperationList.AddTail( s_operationsItems[OperationID_Resolved] );
			out_rOperationList.AddTail( s_operationsItems[OperationID_Diff] );
			out_rOperationList.AddTail( s_operationsItems[OperationID_ShowLog] );
			break;

		case OperationMenuType_Sources:
			out_rOperationList.AddTail( s_operationsItems[OperationID_Update] );
			out_rOperationList.AddTail( s_operationsItems[OperationID_Commit] );
			out_rOperationList.AddTail( s_operationsItems[OperationID_GetLock] );
			out_rOperationList.AddTail( s_operationsItems[OperationID_ReleaseLock] );
			out_rOperationList.AddTail( s_operationsItems[OperationID_Add] );
			out_rOperationList.AddTail( s_operationsItems[OperationID_Delete] );
			out_rOperationList.AddTail( s_operationsItems[OperationID_Rename] );
			out_rOperationList.AddTail( s_operationsItems[OperationID_Move] );
			out_rOperationList.AddTail( s_operationsItems[OperationID_Revert] );
			out_rOperationList.AddTail( s_operationsItems[OperationID_Diff] );
			out_rOperationList.AddTail( s_operationsItems[OperationID_ShowLog] );
			break;
			
		case OperationMenuType_Explorer:
			out_rOperationList.AddTail( s_operationsItems[OperationID_Commit] );
			out_rOperationList.AddTail( s_operationsItems[OperationID_GetLock] );
			out_rOperationList.AddTail( s_operationsItems[OperationID_ReleaseLock] );
			out_rOperationList.AddTail( s_operationsItems[OperationID_Add] );
			out_rOperationList.AddTail( s_operationsItems[OperationID_Revert] );
			out_rOperationList.AddTail( s_operationsItems[OperationID_Diff] );
			out_rOperationList.AddTail( s_operationsItems[OperationID_ShowLog] );
			break;
	}

	return OperationResult_Succeed;
}

LPCWSTR SubversionSourceControl::GetOperationName( DWORD in_dwOperationID )
{
	if( in_dwOperationID >= 0 && in_dwOperationID < OperationIDCount )
		return s_operationNames[in_dwOperationID];

	return L"";
}

DWORD SubversionSourceControl::GetOperationEffect( DWORD in_dwOperationID )
{
	DWORD dwRetVal = 0;

	switch( in_dwOperationID )
	{
	case OperationID_Update:
	case OperationID_Rename:
	case OperationID_Move:
	case OperationID_Revert:
	case OperationID_Delete:
		dwRetVal |= OperationEffect_LocalContentModification;
		break;

	case OperationID_Commit:
		dwRetVal |= OperationEffect_ServerContentModification;
		break;
	}

	return dwRetVal;
}

ISourceControl::OperationResult SubversionSourceControl::GetFileStatus( const StringList& in_rFilenameList, FilenameToStatusMap& out_rFileStatusMap, DWORD in_dwTimeoutMs )
{
	return _GetFileStatus( in_rFilenameList, out_rFileStatusMap, in_dwTimeoutMs, NULL );
}

ISourceControl::OperationResult SubversionSourceControl::_GetFileStatus( const StringList& in_rFilenameList, FilenameToStatusMap& out_rFileStatusMap, DWORD in_dwTimeoutMs, SvnStatusList* out_pStatuses )
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	OperationResult result = OperationResult_Succeed;

	// Load the 'non-versioned' status text
	CString csStatusNonVersioned;
	csStatusNonVersioned = _T("non-versioned");

	SvnPool subPool( m_pGlobalPool );

	for ( SourceControlContainers::AkPos filePos = in_rFilenameList.GetHeadPosition() ; filePos ; )
	{
		SvnStatus status;
		CString csFilename( in_rFilenameList.GetNext( filePos ) );

		subPool.Clear();

		svn_error_t* pError = GetSVNFileStatus( csFilename, status, m_pContext, subPool );
		svn_error_clear( pError );

		if( out_pStatuses )
			out_pStatuses->AddTail( status );

		CString csStatus = GetSVNStatusKindText( status.m_kind );
		if( status.m_bTreeConflict )
		{
			CString csTreeConflict;
			csTreeConflict = _T(", tree conflict");
			csStatus += csTreeConflict;
		}

		FilenameToStatusMapItem statusItem = { csStatus.AllocSysString(), status.m_csLockOwner.AllocSysString() };
		out_rFileStatusMap.SetAt( csFilename, statusItem );
	}

	return result;
}

ISourceControl::OperationResult SubversionSourceControl::GetFileStatusIcons( const StringList& in_rFilenameList, FilenameToIconMap& out_rFileIconsMap, DWORD in_dwTimeoutMs )
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	OperationResult result = OperationResult_Succeed;

	SvnPool subPool( m_pGlobalPool );

	for ( SourceControlContainers::AkPos filePos = in_rFilenameList.GetHeadPosition() ; filePos ; )
	{
		SvnStatus status;
		CString csFilename( in_rFilenameList.GetNext( filePos ) );

		subPool.Clear();

		svn_error_t* pError = GetSVNFileStatus( csFilename, status, m_pContext, subPool );
		svn_error_clear( pError );

		CString csToolTip;
		FilenameToIconMapItem iconItem = { NULL, NULL };

		switch( status.m_kind )
		{
			case svn_wc_status_added:
				csToolTip = k_tooltipText[ IconsIndexes_Added ];
				iconItem.m_hIcon = m_icons[ IconsIndexes_Added ];
				iconItem.m_bstrToolTip = csToolTip.AllocSysString();
				break;

			case svn_wc_status_normal:
				csToolTip = k_tooltipText[ IconsIndexes_Normal ];
				iconItem.m_hIcon = m_icons[ IconsIndexes_Normal ];
				iconItem.m_bstrToolTip = csToolTip.AllocSysString();
				break;

			case svn_wc_status_conflicted:
			case svn_wc_status_modified:
				csToolTip = k_tooltipText[ IconsIndexes_Modified ];
				iconItem.m_hIcon = m_icons[ IconsIndexes_Modified ];
				iconItem.m_bstrToolTip = csToolTip.AllocSysString();
				break;
		}

		out_rFileIconsMap.SetAt( csFilename, iconItem );
	}

	return result;
}

ISourceControl::OperationResult SubversionSourceControl::GetMissingFilesInDirectories( const StringList& in_rDirectoryList, StringList& out_rFilenameList )
{
	OperationResult result = OperationResult_Succeed;

	for ( SourceControlContainers::AkPos filePos = in_rDirectoryList.GetHeadPosition() ; filePos ; )
	{
		CString csDirectory( in_rDirectoryList.GetNext( filePos ) );

		svn_opt_revision_t revOptions = { svn_opt_revision_working, 0 };

		// Disable any global callback
		m_pContext->notify_func2 = NULL;
		m_pContext->log_msg_func2 = NULL;

		const char* pFile = GetSVNPath( csDirectory, m_pGlobalPool );

		svn_opt_revision_t rev;
	    rev.kind = svn_opt_revision_working; 

        svn_error_t* pError = svn_client_status5 (NULL,
                                m_pContext,
                                pFile,
                                &rev,
                                svn_depth_infinity,
                                TRUE,					// get all
                                FALSE,					// update
                                TRUE,					// no_ignore,
                                FALSE,					// ignore_externals
                                TRUE,					// depth as sticky
                                NULL,					// changelists
                                GetMissingFilesCallback,// callback,
                                &out_rFilenameList,		// baton,
                                m_pGlobalPool);

		svn_error_clear( pError );

		// Restore the previous callbacks
		m_pContext->notify_func2 = (svn_wc_notify_func2_t)NotifyCallback;
		m_pContext->log_msg_func2 = (svn_client_get_commit_log2_t)LogMessageCallback;
	}

	return result;
}

AK::Wwise::ISourceControl::IOperationResult* SubversionSourceControl::DoOperation( 
	DWORD in_dwOperationID, 
	const StringList& in_rFilenameList,
	const StringList* in_pTargetFilenameList )
{
	if( in_dwOperationID == OperationID_RenameNoUI ||
		in_dwOperationID == OperationID_MoveNoUI )
	{
		return MoveNoUI( in_dwOperationID, in_rFilenameList, in_pTargetFilenameList );
	}

	CStringList filenameList;

	for ( SourceControlContainers::AkPos filePos=in_rFilenameList.GetHeadPosition() ; filePos ; )
		filenameList.AddTail( in_rFilenameList.GetNext( filePos ) );

    return DoOperation( in_dwOperationID, filenameList );
}

AK::Wwise::ISourceControl::IOperationResult* SubversionSourceControl::DoOperation( 
	DWORD in_dwOperationID, 
	const CStringList& in_rFilenameList )
{
	AK::Wwise::ISourceControl::IOperationResult* pResult = NULL;

	switch( in_dwOperationID )
	{
		case OperationID_Update:
			Update( in_rFilenameList );
			break;
			
		case OperationID_Commit:
			Commit( in_rFilenameList );
			break;
			
		case OperationID_GetLock:
			GetLock( in_rFilenameList );
			break;
			
		case OperationID_ReleaseLock:
			ReleaseLock( in_rFilenameList );
			break;
			
		case OperationID_Add:
			Add( in_rFilenameList );
			break;
			
		case OperationID_Delete:
		case OperationID_DeleteNoUI:
			pResult = Delete( in_rFilenameList, in_dwOperationID == OperationID_Delete );
			break;
			
		case OperationID_Rename:
			pResult = Rename( in_rFilenameList );
			break;

		case OperationID_Move:
			pResult = Move( in_rFilenameList );
			break;

		case OperationID_Revert:
			Revert( in_rFilenameList );
			break;
			
		case OperationID_Resolved:
			Resolved( in_rFilenameList );
			break;
			
		case OperationID_Diff:
			Diff( in_rFilenameList );
			break;
	
		case OperationID_ShowLog:
			ShowLog( in_rFilenameList );
			break;
	}

	return pResult;
}

void SubversionSourceControl::Update( const CStringList& in_rFilenameList )
{
	m_pUtilities->GetProgressDialog()->ShowProgress();

	// Update files
	SvnPool subPool( m_pGlobalPool );
	svn_opt_revision_t revOptions = { svn_opt_revision_head, 0 };

	svn_error_t* pError = svn_client_update2( NULL, GetSVNPathArray( in_rFilenameList, subPool ), &revOptions, TRUE, FALSE, m_pContext, subPool );

	ManageSVNError( pError );

	m_pUtilities->GetProgressDialog()->OperationCompleted();
}

void RemoveCarriageReturn( CString& in_csWide )
{
	in_csWide.Remove( '\r' );
}

void CStringToUTF8( const CString& in_csWide, CStringA& out_csUtf8 )
{
	int size = in_csWide.GetLength() + 1;
	char* pBuffer = out_csUtf8.GetBuffer( size * 4 );
    int len = WideCharToMultiByte(CP_UTF8, 0, (const wchar_t*)in_csWide, size, pBuffer, 4*size, 0, NULL);
	out_csUtf8.ReleaseBuffer( len );
}

void SubversionSourceControl::Commit( const CStringList& in_rFilenameList )
{
	CDlgCommitMessage dlgMessage( m_pUtilities );

	if ( m_pUtilities->CreateModalCustomDialog( &dlgMessage ) == IDOK )
	{
		m_pUtilities->GetProgressDialog()->ShowProgress();

		// Commit files
		SvnPool subPool( m_pGlobalPool );
		svn_commit_info_t *pCommitInfo = svn_create_commit_info( subPool );

		CString csMsg = dlgMessage.GetMessage();
		RemoveCarriageReturn( csMsg );
		CStringA csUtf8;
		CStringToUTF8( csMsg, csUtf8 );
		m_pContext->log_msg_baton2 = apr_pstrdup( subPool, csUtf8 );

		svn_error_t* pError = svn_client_commit4(			
			&pCommitInfo,									// svn_commit_info_t **commit_info_p,
			GetSVNPathArray( in_rFilenameList, subPool ),	// const apr_array_header_t *targets,
			svn_depth_infinity,								// svn_depth_t depth,
			dlgMessage.KeepLocks(),							// svn_boolean_t keep_locks,
			FALSE,											// svn_boolean_t keep_changelists,
			NULL,											// const apr_array_header_t *changelists,
			NULL,											// const apr_hash_t *revprop_table,
			m_pContext,										// svn_client_ctx_t *ctx,
			subPool );										// apr_pool_t *pool);

		ManageSVNError( pError );
		m_pContext->log_msg_baton2 = "";

		// If the commit operation succeed, show the "Completed" message.
		if ( pCommitInfo && SVN_IS_VALID_REVNUM( pCommitInfo->revision ) )
		{
			svn_wc_notify_t notifyInfo = {0};
			notifyInfo.action = svn_wc_notify_update_completed;
			notifyInfo.revision = pCommitInfo->revision;
			NotifyCallback( this, &notifyInfo, subPool );
		}

		m_pUtilities->GetProgressDialog()->OperationCompleted();
	}
}

void SubversionSourceControl::GetLock( const CStringList& in_rFilenameList )
{
	CDlgLockMessage dlgMessage( m_pUtilities );

	if ( m_pUtilities->CreateModalCustomDialog( &dlgMessage ) == IDOK )
	{
		m_pUtilities->GetProgressDialog()->ShowProgress();

		SvnPool subPool( m_pGlobalPool );
		svn_error_t* pError = svn_client_lock( GetSVNPathArray( in_rFilenameList, subPool ), CStringA( dlgMessage.GetMessage() ), dlgMessage.StealLocks(), m_pContext, subPool );

		ManageSVNError( pError );

		m_pUtilities->GetProgressDialog()->OperationCompleted();
	}
}

void SubversionSourceControl::ReleaseLock( const CStringList& in_rFilenameList )
{
	m_pUtilities->GetProgressDialog()->ShowProgress();

	SvnPool subPool( m_pGlobalPool );
	svn_error_t* pError = svn_client_unlock( GetSVNPathArray( in_rFilenameList, subPool ), FALSE, m_pContext, subPool );

	ManageSVNError( pError );

	m_pUtilities->GetProgressDialog()->OperationCompleted();
}

void SubversionSourceControl::Add( const CStringList& in_rFilenameList )
{
	m_pUtilities->GetProgressDialog()->ShowProgress();

	SvnPool subPool( m_pGlobalPool );

	bool bError = false;

	for ( POSITION filePos=in_rFilenameList.GetHeadPosition() ; filePos ; )
	{
		CString csFilename( in_rFilenameList.GetNext( filePos ) );
		
		// Get the status of the file
		subPool.Clear();

		SvnStatus status;
		svn_error_t* pError = GetSVNFileStatus( csFilename, status, m_pContext, subPool );
		svn_error_clear( pError );

		// Only  try to add when the file is NOT under source control
		if ( ( status.m_kind == svn_wc_status_unversioned ) || ( status.m_kind == svn_wc_status_none ) )
		{
			subPool.Clear();
			svn_error_t* pError = svn_client_add3( GetSVNPath( csFilename, subPool ), FALSE, FALSE, TRUE, m_pContext, subPool );
			bError = bError || pError!=NULL;
			ManageSVNError( pError );	
		}
	}

	m_pUtilities->GetProgressDialog()->OperationCompleted( bError );
}

AK::Wwise::ISourceControl::IOperationResult* SubversionSourceControl::Delete( const CStringList& in_rFilenameList, bool in_bShowConfirmation )
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	if( in_bShowConfirmation )
	{
		CString csCaption;
		CString csMessage;

		csCaption = _T("Delete confirmation");
		csMessage = _T("Are you sure you want to delete the selected file(s)?");

		if( m_pUtilities->MessageBox( NULL, csMessage, csCaption, MB_YESNO ) != IDYES )
			return NULL;
	}

	FileOperationResult* pResult = new FileOperationResult();

	m_pUtilities->GetProgressDialog()->ShowProgress();

	CStringList localOnlyFilenameList;
	CStringList serverFilenameList;
	SvnPool subPool( m_pGlobalPool );

	// Build local only and server filename list
	for ( POSITION fileNamePos=in_rFilenameList.GetHeadPosition() ; fileNamePos ; )
	{
		SvnStatus status;
		CString csFilename( in_rFilenameList.GetNext( fileNamePos ) );

		// Get the status of the file
		subPool.Clear();

		svn_error_t* pError = GetSVNFileStatus( csFilename, status, m_pContext, subPool );
		svn_error_clear( pError );

		if ( ( status.m_kind == svn_wc_status_unversioned ) || ( status.m_kind == svn_wc_status_none ) )
		{
			localOnlyFilenameList.AddTail( csFilename );
		}
		else
		{
			serverFilenameList.AddTail( csFilename );
		}
	}

	// Delete files on server
	if ( serverFilenameList.GetSize() )
	{
		svn_commit_info_t* pCommitInfo = svn_create_commit_info( subPool );

		svn_error_t* pError = svn_client_delete2 ( &pCommitInfo, GetSVNPathArray( serverFilenameList, subPool ), FALSE, m_pContext, subPool );
		ManageSVNError( pError );

		if ( pCommitInfo && SVN_IS_VALID_REVNUM( pCommitInfo->revision ) )
		{
			svn_wc_notify_t notifyInfo = {0};
			notifyInfo.action = svn_wc_notify_update_completed;
			notifyInfo.revision = pCommitInfo->revision;
			NotifyCallback( this, &notifyInfo, subPool );

			for ( POSITION filePos = serverFilenameList.GetHeadPosition() ; filePos ; )
			{
				CString csFilename = serverFilenameList.GetNext( filePos );
				pResult->AddFile( csFilename );
			}
		}
	}

	// Delete local files
	if ( localOnlyFilenameList.GetSize() )
	{
		CString csMessage;

		for ( POSITION filePos=localOnlyFilenameList.GetHeadPosition() ; filePos ; )
		{
			CString csFilename = localOnlyFilenameList.GetNext( filePos );

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
		}
	}

	m_pUtilities->GetProgressDialog()->OperationCompleted();

	return pResult;
}

AK::Wwise::ISourceControl::IOperationResult* SubversionSourceControl::Rename( const CStringList& in_rFilenameList )
{
	FileOperationResult* pResult = NULL;

	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	ASSERT( in_rFilenameList.GetSize() == 1 );

	CString csOldFilename = in_rFilenameList.GetHead();

	// Get the folder and the initial name
	CDlgRename renameDialog( csOldFilename, m_pUtilities );
	INT_PTR dialogResult = m_pUtilities->CreateModalCustomDialog( &renameDialog );

	if ( dialogResult == IDOK )
	{
		m_pUtilities->GetProgressDialog()->ShowProgress();

		// Get the status of the old file
		SvnPool subPool( m_pGlobalPool );
		SvnStatus oldFileStatus;

		svn_error_t* pError = GetSVNFileStatus( csOldFilename, oldFileStatus, m_pContext, subPool );
		svn_error_clear( pError );

		if ( ( oldFileStatus.m_kind == svn_wc_status_unversioned ) || ( oldFileStatus.m_kind == svn_wc_status_none ) )
		{
			CString csMessage;

			if ( ::MoveFile( csOldFilename, renameDialog.GetNewFilename() ) == FALSE )
			{
				csMessage.Format( _T("Error renaming local file %s"), csOldFilename );
			}
			else
			{
				csMessage.Format( _T("Successfully renamed local file %s to %s"), csOldFilename, renameDialog.GetNewFilename() );
				
				pResult = new FileOperationResult();
				pResult->AddMovedFile( csOldFilename, renameDialog.GetNewFilename() );
			}

			m_pUtilities->GetProgressDialog()->AddLogMessage( csMessage );
		}
		else
		{
			SvnPool subPool( m_pGlobalPool );
			SvnStatus newFileStatus;

			svn_error_t* pError = GetSVNFileStatus( renameDialog.GetNewFilename(), newFileStatus, m_pContext, subPool );
			svn_error_clear( pError );

			if ( ( ( newFileStatus.m_kind != svn_wc_status_unversioned ) && 
				   ( newFileStatus.m_kind != svn_wc_status_none ) )
				 || ::PathFileExists( renameDialog.GetNewFilename() ) )
			{
				CString csMessage;

				csMessage.Format( _T("Can't rename %s to %s, file already exists"), csOldFilename, renameDialog.GetNewFilename() );

				m_pUtilities->GetProgressDialog()->AddLogMessage( csMessage );
			}
			else
			{
				svn_commit_info_t *pCommitInfo = svn_create_commit_info( subPool );

				svn_error_t* pError = svn_client_move3( &pCommitInfo, GetSVNPath( csOldFilename, subPool ), GetSVNPath( renameDialog.GetNewFilename(), subPool ), FALSE, m_pContext, subPool );
				ManageSVNError( pError );

				if( ! pError )
				{
					pResult = new FileOperationResult();
					pResult->AddMovedFile( csOldFilename, renameDialog.GetNewFilename() );
				}

				// If the commit operation succeed, show the "Completed" message.
				if ( pCommitInfo && SVN_IS_VALID_REVNUM( pCommitInfo->revision ) )
				{
					svn_wc_notify_t notifyInfo = {0};
					notifyInfo.action = svn_wc_notify_update_completed;
					notifyInfo.revision = pCommitInfo->revision;
					NotifyCallback( this, &notifyInfo, subPool );
				}
			}
		}

		m_pUtilities->GetProgressDialog()->OperationCompleted();
	}

	return pResult;
}

AK::Wwise::ISourceControl::IOperationResult* SubversionSourceControl::MoveNoUI( DWORD in_dwOperationID, const StringList& in_rFilenameList, const StringList* in_pTargetFilenameList )
{
    ASSERT( in_pTargetFilenameList );
    if( ! in_pTargetFilenameList )
        return NULL;

	ASSERT( in_pTargetFilenameList->GetCount() == in_rFilenameList.GetCount() );
	if( in_pTargetFilenameList->GetCount() != in_rFilenameList.GetCount() )
		return NULL;

	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	m_pUtilities->GetProgressDialog()->ShowProgress();

	// Create the move results
	FileOperationResult* pResult = new FileOperationResult();

	// Move each file
	AK::Wwise::SourceControlContainers::AkPos positionSrc = in_rFilenameList.GetHeadPosition();
	AK::Wwise::SourceControlContainers::AkPos positionDst = in_pTargetFilenameList->GetHeadPosition();
	for( ; positionSrc; in_rFilenameList.GetNext(positionSrc), in_pTargetFilenameList->GetNext(positionDst))
	{
		Move( in_rFilenameList.GetAt(positionSrc), in_pTargetFilenameList->GetAt(positionDst), pResult );
	}

	m_pUtilities->GetProgressDialog()->OperationCompleted();

	return pResult;
}

AK::Wwise::ISourceControl::IOperationResult* SubversionSourceControl::Move( const CStringList& in_rFilenameList )
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

	// Let the user decide the destination
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
			// Obtain the status of the destination directory
			SvnPool subPool( m_pGlobalPool );
			SvnStatus directoryDirStatus;

			svn_error_t* pError = GetSVNFileStatus( szDestinationDir, directoryDirStatus, m_pContext, subPool );
			svn_error_clear( pError );

			if( directoryDirStatus.m_kind == svn_wc_status_unversioned )
			{
				// If the destination is a non-working directory, do a "svn add" on it for convenience
				svn_error_t* pError = svn_client_add4( GetSVNPath( szDestinationDir, subPool ), svn_depth_empty, FALSE, FALSE, TRUE, m_pContext, subPool );
				ManageSVNError( pError );	

				// Stop when errors occurs
				bCanProceed = (pError == NULL);
			}
		}

		if( bCanProceed )
		{
			ASSERT( newPaths.size() == in_rFilenameList.GetCount() );

			// Create the move results
			pResult = new FileOperationResult();

			// Move each file
			unsigned int i = 0;
			for( POSITION position = in_rFilenameList.GetHeadPosition(); 
				position; 
				in_rFilenameList.GetNext(position), ++i )
			{
				Move( in_rFilenameList.GetAt(position), newPaths[i], pResult );
			}
		}

		m_pUtilities->GetProgressDialog()->OperationCompleted();
	}

	return pResult;
}

void SubversionSourceControl::Move( const CString& in_csFrom, const CString& in_csTo, FileOperationResult* io_pResult )
{
	// Get the status of the old file
	SvnPool subPool( m_pGlobalPool );
	SvnStatus oldFileStatus;

	svn_error_t* pError = GetSVNFileStatus( in_csFrom, oldFileStatus, m_pContext, subPool );
	svn_error_clear( pError );

	if ( ( oldFileStatus.m_kind == svn_wc_status_unversioned ) || ( oldFileStatus.m_kind == svn_wc_status_none ) )
	{
		// We are moving a non-versioned file, just do the file system operation
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
		// Do the SVN operation
		SvnPool subPool( m_pGlobalPool );
		SvnStatus newFileStatus;

		svn_error_t* pError = GetSVNFileStatus( in_csTo, newFileStatus, m_pContext, subPool );
		svn_error_clear( pError );

		// Verify the file is in SVN
		if ( ( ( newFileStatus.m_kind != svn_wc_status_unversioned ) && 
			   ( newFileStatus.m_kind != svn_wc_status_none ) )
			 || ::PathFileExists( in_csTo ) )
		{
			CString csMessage;
			csMessage.Format( _T("Can't move %s to %s, file already exists"), in_csFrom, in_csTo );

			m_pUtilities->GetProgressDialog()->AddLogMessage( csMessage );
		}
		else
		{
			// Do a move operation
			svn_commit_info_t *pCommitInfo = svn_create_commit_info( subPool );

			svn_error_t* pError = svn_client_move3( 
				&pCommitInfo, GetSVNPath( in_csFrom, subPool ), GetSVNPath( in_csTo, subPool ), FALSE, m_pContext, subPool );
			ManageSVNError( pError );

			if( ! pError )
			{
				io_pResult->AddMovedFile( in_csFrom, in_csTo );
			}

			// If the move operation succeed, show the "Completed" message.
			if ( pCommitInfo && SVN_IS_VALID_REVNUM( pCommitInfo->revision ) )
			{
				svn_wc_notify_t notifyInfo = {0};
				notifyInfo.action = svn_wc_notify_update_completed;
				notifyInfo.revision = pCommitInfo->revision;
				NotifyCallback( this, &notifyInfo, subPool );
			}
		}
	}
}

void SubversionSourceControl::Revert( const CStringList& in_rFilenameList )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CString csCaption, csMessage;
	csCaption = _T("Subversion");
	csMessage = _T("Are you sure you want to revert the selected item(s)? You will lose all changes since the last update.");

	if( m_pUtilities->MessageBox( NULL, csMessage, csCaption, MB_YESNO ) == IDYES )
	{
		m_pUtilities->GetProgressDialog()->ShowProgress();

		SvnPool subPool( m_pGlobalPool );
		svn_error_t* pError = svn_client_revert( GetSVNPathArray( in_rFilenameList, subPool ), FALSE, m_pContext, subPool );
		ManageSVNError( pError );

		m_pUtilities->GetProgressDialog()->OperationCompleted();
	}
}

void SubversionSourceControl::Resolved( const CStringList& in_rFilenameList )
{
	m_pUtilities->GetProgressDialog()->ShowProgress();

	SvnPool subPool( m_pGlobalPool );

	for ( POSITION filePos=in_rFilenameList.GetHeadPosition() ; filePos ; )
	{
		CString csFilename( in_rFilenameList.GetNext( filePos ) );
		
		subPool.Clear();
		svn_error_t* pError = svn_client_resolved( GetSVNPath( csFilename, subPool ), FALSE, m_pContext, subPool );
		ManageSVNError( pError );
	}

	m_pUtilities->GetProgressDialog()->OperationCompleted();
}

void SubversionSourceControl::Diff( const CStringList& in_rFilenameList )
{
	ASSERT( in_rFilenameList.GetSize() == 1 );

	CString csFilename = in_rFilenameList.GetHead();
	bool bUseWavViewer = m_bUseAKWavViewerToDiff && CString( ::PathFindExtension( csFilename )).CompareNoCase( L".wav" ) == 0;

	if ( !m_csDiffToolPath.IsEmpty() || bUseWavViewer )
	{
		const char* szPristinePath = NULL;

		SvnPool subPool( m_pGlobalPool );

		svn_error_t* pError = svn_wc_get_pristine_copy_path( svn_path_internal_style( GetSVNPath( csFilename, subPool ), subPool ), &szPristinePath, subPool );
		ManageSVNError( pError );

		if ( !pError && szPristinePath )
		{
			CString csExePath = m_csDiffToolPath;
			CString csExeArgs;

			if( bUseWavViewer ) 
			{
				// Use Ak WaveViewer
				TCHAR szPath[MAX_PATH] = {0};
				::GetModuleFileName( NULL, szPath, MAX_PATH );

				::PathRemoveFileSpec( szPath );
				
				TCHAR szWaveViewer[MAX_PATH] = {0};
				::PathCombine( szWaveViewer, szPath, L"WaveViewer.exe" );

				csExePath = szWaveViewer;
				csExeArgs = L" -noakd -diff";
			}

			CString csCommand = L'\"' + csExePath + L'\"' + csExeArgs;

			csCommand += L" \"" + CString( szPristinePath ) + L'\"';
			csCommand += L" \"" + csFilename + L'\"';

			RunCommand( csCommand );
		}
	}
	else
	{
		AFX_MANAGE_STATE(AfxGetStaticModuleState());

		CString csMessage;
		CString csCaption;

		csMessage = _T("No diff tool specified");
		csCaption = _T("Subversion plug-in");

		m_pUtilities->MessageBox( NULL, csMessage, csCaption, MB_OK | MB_ICONERROR );
	}
}

void SubversionSourceControl::ShowLog( const CStringList& in_rFilenameList )
{
	ASSERT( in_rFilenameList.GetSize() == 1 );

	m_pUtilities->GetProgressDialog()->ShowProgress();

	SvnPool subPool( m_pGlobalPool );
	svn_opt_revision_t revOptionsFirst = { svn_opt_revision_head, 0 };
	svn_opt_revision_t revOptionsLast = { svn_opt_revision_number, 0 };

	svn_error_t* pError = svn_client_log2( GetSVNPathArray( in_rFilenameList, subPool ), &revOptionsFirst, &revOptionsLast, 100, TRUE, FALSE, (svn_log_message_receiver_t)ShowLogCallback, m_pUtilities, m_pContext, subPool );
	ManageSVNError( pError );

	m_pUtilities->GetProgressDialog()->OperationCompleted();
}

ISourceControl::OperationResult SubversionSourceControl::PreCreateOrModify( const StringList& in_rFilenameList, CreateOrModifyOperation in_eOperation, bool& out_rContinue )
{
	out_rContinue = true;

	m_filesToAdd.RemoveAll();

	if ( in_eOperation & CreateOrModifyOperation_Create )
	{
		SvnPool subPool( m_pGlobalPool );

		for ( SourceControlContainers::AkPos filePos = in_rFilenameList.GetHeadPosition() ; filePos ; )
		{
			SvnStatus status;
			CString csFilename( in_rFilenameList.GetNext( filePos ) );

			subPool.Clear();
			svn_error_t* pError = GetSVNFileStatus( csFilename, status, m_pContext, subPool );

			// The 'none' status is used because the file doesn't exist at this time
			if ( status.m_kind == svn_wc_status_none )
				m_filesToAdd.AddTail( csFilename );

			svn_error_clear( pError );
		}

		if ( !m_filesToAdd.IsEmpty() )
		{
			if( m_bAutoAcceptMode )
			{
				m_bOnNewFilesAdd = true;
			}
			else
			{
				CDlgOnCreateModify dlgCreateModify( m_bOnNewFilesAdd );

				m_pUtilities->CreateModalCustomDialog( &dlgCreateModify );
				INT_PTR iResult = dlgCreateModify.GetResult();

				if ( iResult == ID_CANCEL )
				{
					out_rContinue = false;
				}
				else if ( iResult == ID_YES )
				{
					m_bOnNewFilesAdd = dlgCreateModify.NeedToAddFiles();
				}
				else
				{
					m_filesToAdd.RemoveAll();
				}
			}
		}
	}

	return OperationResult_Succeed;
}

ISourceControl::OperationResult SubversionSourceControl::PostCreateOrModify( const StringList& in_rFilenameList, CreateOrModifyOperation in_eOperation, bool& out_rContinue )
{
	out_rContinue = true;

	if ( ( in_eOperation & CreateOrModifyOperation_Create ) && m_bOnNewFilesAdd && !m_filesToAdd.IsEmpty() )
	{
		DoOperation( OperationID_Add, m_filesToAdd );
	}

	return OperationResult_Succeed;
}

//
// Utility functions
//
CString SubversionSourceControl::GetSVNStatusKindText( svn_wc_status_kind in_status )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CString csStatus;

	switch( in_status )
	{
		case svn_wc_status_none:
			csStatus = _T("none");
			break;

		case svn_wc_status_unversioned:
			csStatus = _T("non-versioned");
			break;

		case svn_wc_status_normal:
			csStatus = _T("normal");
			break;

		case svn_wc_status_added:
			csStatus = _T("added");
			break;

		case svn_wc_status_missing:
			csStatus = _T("missing");
			break;

		case svn_wc_status_deleted:
			csStatus = _T("deleted");
			break;

		case svn_wc_status_replaced:
			csStatus = _T("replaced");
			break;

		case svn_wc_status_modified:
			csStatus = _T("modified");
			break;

		case svn_wc_status_merged:
			csStatus = _T("merged");
			break;

		case svn_wc_status_conflicted:
			csStatus = _T("conflicted");
			break;

		case svn_wc_status_ignored:
			csStatus = _T("ignored");
			break;

		case svn_wc_status_obstructed:
			csStatus = _T("obstructed");
			break;

		case svn_wc_status_external:
			csStatus = _T("external");
			break;

		case svn_wc_status_incomplete:
			csStatus = _T("incomplete");
			break;

		default:
			ASSERT( !"Unknown Subversion status" );
			break;
	};

	return csStatus;
}

CString SubversionSourceControl::GetPathFromSVN( const char* in_pSVNPath, apr_pool_t* in_pPool )
{
	CString csWinPath;

	if ( in_pSVNPath )
	{
		int iMbSize = (int)_mbstrlen( in_pSVNPath );
		wchar_t * pWideChar = csWinPath.GetBufferSetLength( iMbSize );
		int len = ::MultiByteToWideChar( CP_UTF8, 0, in_pSVNPath, iMbSize, pWideChar, iMbSize );

		csWinPath.ReleaseBuffer( len );
		csWinPath.Replace( L'/', L'\\' );
	}

	return csWinPath;
}

const char* SubversionSourceControl::GetSVNPath( const CString& in_rPath, apr_pool_t* in_pPool )
{
	CString csSVNPath( in_rPath );

	csSVNPath.Replace( '\\', '/' );
	csSVNPath.TrimRight( '/' );

	CStringA csUtf8;
	CStringToUTF8( csSVNPath, csUtf8 );

	return apr_pstrdup( in_pPool, csUtf8 );
}

apr_array_header_t* SubversionSourceControl::GetSVNPathArray( const StringList& in_rPathList, apr_pool_t* in_pPool )
{
	CStringList pathList;

	for ( SourceControlContainers::AkPos pathPos=in_rPathList.GetHeadPosition() ; pathPos ; )
		pathList.AddTail( in_rPathList.GetNext( pathPos ) ); 

	return GetSVNPathArray( pathList, in_pPool );
}

apr_array_header_t* SubversionSourceControl::GetSVNPathArray( const CStringList& in_rPathList, apr_pool_t* in_pPool )
{
	apr_array_header_t *targets = apr_array_make( in_pPool, (unsigned int)in_rPathList.GetCount(), sizeof(const char *) );

	for ( POSITION pos = in_rPathList.GetHeadPosition() ; pos ; )
		APR_ARRAY_PUSH( targets, const char * ) = apr_pstrdup( in_pPool, GetSVNPath( in_rPathList.GetNext( pos ), in_pPool ) );

	return targets;
}

CString SubversionSourceControl::GetSVNDate( const apr_time_t& in_rAprTime )
{
	SYSTEMTIME sysTime;
	apr_time_exp_t explodedTime = {0};
	wchar_t timeBuffer[SVN_DATE_BUFFER] = {0};
	wchar_t datebuffer[SVN_DATE_BUFFER] = {0};

	LCID locale = MAKELCID( LOCALE_USER_DEFAULT, SORT_DEFAULT );

	apr_time_exp_lt ( &explodedTime, in_rAprTime );
	
	sysTime.wDay = (WORD)explodedTime.tm_mday;
	sysTime.wDayOfWeek = (WORD)explodedTime.tm_wday;
	sysTime.wHour = (WORD)explodedTime.tm_hour;
	sysTime.wMilliseconds = (WORD)( explodedTime.tm_usec / 1000 );
	sysTime.wMinute = (WORD)explodedTime.tm_min;
	sysTime.wMonth = (WORD)explodedTime.tm_mon + 1;
	sysTime.wSecond = (WORD)explodedTime.tm_sec;
	sysTime.wYear = (WORD)explodedTime.tm_year + 1900;

	GetDateFormat( locale, DATE_LONGDATE, &sysTime, NULL, datebuffer, SVN_DATE_BUFFER );
	GetTimeFormat( locale, 0, &sysTime, NULL, timeBuffer, SVN_DATE_BUFFER );

	return CString( timeBuffer ) + L", " + CString( datebuffer );
}

svn_error_t* SubversionSourceControl::GetSVNFileStatus( const CString& in_csFilename, SvnStatus& out_rStatus, svn_client_ctx_t* in_pContext, apr_pool_t* in_pPool )
{
	svn_opt_revision_t revOptions = { svn_opt_revision_working, 0 };

	out_rStatus.m_kind = svn_wc_status_none;
	out_rStatus.m_bLocked = false;
	out_rStatus.m_csLockOwner = CString();
	out_rStatus.m_bTreeConflict = false;

	return svn_client_status5(
		NULL,												// svn_revnum_t *result_rev,
		in_pContext,										// svn_client_ctx_t *ctx,
		GetSVNPath( in_csFilename, in_pPool ),				// const char *path,
		&revOptions,										// const svn_opt_revision_t *revision,
		svn_depth_empty,									// svn_depth_t depth,
		TRUE,												// svn_boolean_t get_all,
		FALSE,												// svn_boolean_t update,
		TRUE,												// svn_boolean_t no_ignore,
		FALSE,												// svn_boolean_t ignore_externals,
		TRUE,												// svn_boolean_t depth_as_sticky,
		NULL,												// const apr_array_header_t *changelists,
		(svn_client_status_func_t)GetFileStatusCallback,	// svn_client_status_func_t status_func,
		&out_rStatus,										// void *status_baton,
		in_pPool );											// apr_pool_t *scratch_pool);
}

bool SubversionSourceControl::IsOperationValid( OperationID in_operationID, const SvnStatusList& in_rStatusList, bool in_bMultipleSelection )
{
	bool bValid = in_rStatusList.IsEmpty() ? false : true;

	for ( POSITION statusPos=in_rStatusList.GetHeadPosition() ; bValid && statusPos ; )
	{
		const SvnStatus& status = in_rStatusList.GetNext( statusPos );

		bValid = IsOperationValid( in_operationID, status, in_bMultipleSelection );
	}

	return bValid;
}

bool SubversionSourceControl::IsOperationValid( OperationID in_operationID, const SvnStatus& in_rStatus, bool in_bMultipleSelection )
{
	bool bValid = false;

	switch( in_rStatus.m_kind )
	{
		case svn_wc_status_ignored:
		case svn_wc_status_none:
			bValid = false;
			break;

		case svn_wc_status_unversioned:
			if ( ( in_operationID == OperationID_Add ) ||
				 ( in_operationID == OperationID_Delete ) ||
				 ( in_operationID == OperationID_Rename ) ||
				 ( in_operationID == OperationID_Move ))
			{
				bValid = true;
			}
			break;

		case svn_wc_status_normal:
			if ( ( in_operationID == OperationID_Update ) ||
				 ( in_operationID == OperationID_Delete ) ||
				 ( in_operationID == OperationID_Rename ) ||
				 ( in_operationID == OperationID_Move ) ||
				 ( in_operationID == OperationID_ShowLog ) ||
				 ( in_operationID == OperationID_GetLock ) ||
				 ( in_operationID == OperationID_ReleaseLock ) )
			{
				bValid = true;
			}
			break;

		case svn_wc_status_added:
			if ( ( in_operationID == OperationID_Commit ) ||
				 ( in_operationID == OperationID_Revert ) ||
				 ( in_operationID == OperationID_Rename ) )
			{
				bValid = true;
			}
			break;

		case svn_wc_status_missing:
			if ( ( in_operationID == OperationID_Delete ) ||
				 ( in_operationID == OperationID_Revert ) )
			{
				bValid = true;
			}
			break;

		case svn_wc_status_deleted:
			if ( ( in_operationID == OperationID_ShowLog ) ||
				 ( in_operationID == OperationID_Commit ) ||
				 ( in_operationID == OperationID_Revert ) )
			{
				bValid = true;
			}
			break;

		case svn_wc_status_modified:
			if ( ( in_operationID == OperationID_Update ) ||
				 ( in_operationID == OperationID_Revert ) ||
				 ( in_operationID == OperationID_Commit ) ||
				 ( in_operationID == OperationID_Rename ) ||
				 ( in_operationID == OperationID_Move ) ||
				 ( in_operationID == OperationID_Delete ) ||
				 ( in_operationID == OperationID_ShowLog ) ||
				 ( in_operationID == OperationID_Diff ) ||
				 ( in_operationID == OperationID_GetLock ) ||
				 ( in_operationID == OperationID_ReleaseLock ) )
			{
				bValid = true;
			}
			break;

		case svn_wc_status_conflicted:
			if ( ( in_operationID == OperationID_Update ) ||
				 ( in_operationID == OperationID_Revert ) ||
				 ( in_operationID == OperationID_Resolved ) ||
				 ( in_operationID == OperationID_Rename ) ||
				 ( in_operationID == OperationID_Move ) ||
				 ( in_operationID == OperationID_Delete ) ||
				 ( in_operationID == OperationID_ShowLog ) ||
				 ( in_operationID == OperationID_Diff ) ||
				 ( in_operationID == OperationID_GetLock ) ||
				 ( in_operationID == OperationID_ReleaseLock ) )
			{
				bValid = true;
			}
			break;

		case svn_wc_status_replaced:
		case svn_wc_status_merged:
		case svn_wc_status_obstructed:
			ASSERT( !"Not managed Subversion status" );
			TRACE( "Not mamaged Subversion status: %d\n", in_rStatus.m_kind );
			break;
	}

	// Disable single selection operations when MultipleOperation is set
	if ( in_bMultipleSelection &&
		 ( ( in_operationID == OperationID_Rename ) ||
		   ( in_operationID == OperationID_Diff ) ||
		   ( in_operationID == OperationID_ShowLog ) ) )
	{
		bValid = false;
	}

    // Disable GetLock/ReleaseLock operations when the 'locked' flag is not compatible with the operation
	if ( ( ( in_operationID == OperationID_GetLock ) && in_rStatus.m_bLocked ) ||
		 ( ( in_operationID == OperationID_ReleaseLock ) && !in_rStatus.m_bLocked ) )
	{
		bValid = false;
	}

	return bValid;
}

void SubversionSourceControl::RunCommand( const CString& in_rCommand )
{
	STARTUPINFO startupInfo;
	PROCESS_INFORMATION processInfo;

	::ZeroMemory( &startupInfo, sizeof(startupInfo) );
	startupInfo.cb = sizeof(startupInfo);
	::ZeroMemory( &processInfo, sizeof(processInfo) );

	CW2W acCommand( in_rCommand );

	if ( ::CreateProcess(NULL, acCommand, NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInfo) )
	{
		SourceControlHelpers::WaitWithPaintLoop( processInfo.hProcess );

		::CloseHandle( processInfo.hProcess );
		::CloseHandle( processInfo.hThread );
	}
	else
	{
		AFX_MANAGE_STATE(AfxGetStaticModuleState());

		CString csMessage;
		CString csCaption;

		csMessage.Format( _T("Could not run the following command:\n%s"), in_rCommand );
		csCaption = _T("Subversion plug-in");

		m_pUtilities->MessageBox( NULL, csMessage, csCaption, MB_OK | MB_ICONERROR );
		
	}
}

void SubversionSourceControl::ManageSVNError( svn_error_t* in_pError, SVNErrorDestination in_destination /*= SVNErrorDestination_Progress*/, bool in_bClearError )
{
	if ( in_pError )
	{
		switch( in_destination )
		{
			case SVNErrorDestination_Progress:
				{
					svn_error_t* pChildError = in_pError;

					while ( pChildError )
					{
						m_pUtilities->GetProgressDialog()->AddLogMessage( CString( pChildError->message ) );
						pChildError = pChildError->child;
					}

					break;
				}

			case SVNErrorDestination_MessageBox:
				{
					AFX_MANAGE_STATE(AfxGetStaticModuleState());

					CString csCaption;
					CString csMessage( in_pError->message );
					csCaption = _T("Subversion plug-in");

					svn_error_t* pChildError = in_pError->child;

					while ( pChildError )
					{
						csMessage += L"\n" + CString( pChildError->message );
						pChildError = pChildError->child;
					}
					if( m_bAutoAcceptMode )
					{

						// In auto accept mode, we don't want to display message boxes.
						// Not actually in use, since we only modify and never create from command line at this point.
						// If at some ponit it becomes possible to create files from command line, this should be logged somewhere.
					}
					else
					{
						m_pUtilities->MessageBox( NULL, csMessage, csCaption, MB_OK | MB_ICONERROR );
					}

					break;
				}
		}

		if( in_bClearError )
			svn_error_clear( in_pError );
	}
}

svn_error_t* SubversionSourceControl::CancelCallback( AK::Wwise::ISourceControlUtilities* in_pUtilities )
{
	ASSERT( in_pUtilities );

	ISourceControlOperationProgress* pProgress = in_pUtilities->GetProgressDialog();

	if ( pProgress && pProgress->IsCanceled() )
	{
		AFX_MANAGE_STATE( AfxGetStaticModuleState() );

		CStringA csCanceled;
		csCanceled = _T("Operation canceled.");
		return svn_error_create( SVN_ERR_CANCELLED, NULL, csCanceled );
	}

	return SVN_NO_ERROR;
}

svn_error_t* SubversionSourceControl::LogMessageCallback( const char** in_pLogMsg, const char** in_pTmpFile, const apr_array_header_t* in_pArrayHeader, const char* in_pMessage, apr_pool_t* in_pPool )
{
	ASSERT( in_pMessage );

	*in_pTmpFile = NULL;
	*in_pLogMsg = apr_pstrdup( in_pPool, in_pMessage );

	return SVN_NO_ERROR;
}

void SubversionSourceControl::NotifyCallback( SubversionSourceControl* in_pSourceControl, const svn_wc_notify_t* in_pNotifyInfo, apr_pool_t* in_pPool )
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	ASSERT( in_pSourceControl && in_pNotifyInfo );

	if ( in_pSourceControl && in_pNotifyInfo )
	{
		CString csNotifyText;
		CString csPath = GetPathFromSVN( in_pNotifyInfo->path, in_pPool );

		switch( in_pNotifyInfo->action )
		{
			// Adding a path to revision control.
			case svn_wc_notify_add:
			case svn_wc_notify_update_add:
				csNotifyText.Format( _T("Added: %s"), csPath );
				break;

			// Copying a versioned path.
			case svn_wc_notify_copy:
				csNotifyText.Format( _T("Copied: %s"), csPath );
				break;
			  
			// Deleting a versioned path.
			case svn_wc_notify_delete:
			case svn_wc_notify_update_delete:
				csNotifyText.Format( _T("Deleted: %s"), csPath );
				break;

			// Restoring a missing path from the pristine text-base.
			case svn_wc_notify_restore:
				csNotifyText.Format( _T("Restored: %s"), csPath );
				break;
			  
			// Reverting a modified path.
			case svn_wc_notify_revert:
				csNotifyText.Format( _T("Reverted: %s"), csPath );
				break;

			// A revert operation has failed.
			case svn_wc_notify_failed_revert:
				csNotifyText.Format( _T("Failed revert: %s"), csPath );
				break;

			// Resolving a conflict.
			case svn_wc_notify_resolved:
				csNotifyText.Format( _T("Resolved: %s"), csPath );
				break;

			// Skipping a path.
			case svn_wc_notify_skip:
				csNotifyText.Format( _T("Skipped: %s"), csPath );
				break;

			// Got any other action in an update.
			case svn_wc_notify_update_update:
				// if this is an inoperative dir change, don't show the nofification.
				// an inoperative dir change is when a directory gets updated without
				// any real change in either text or properties.
				if ( ( in_pNotifyInfo->kind == svn_node_dir ) && ( ( in_pNotifyInfo->prop_state == svn_wc_notify_state_inapplicable )	|| 
																   ( in_pNotifyInfo->prop_state == svn_wc_notify_state_unknown ) || 
																   ( in_pNotifyInfo->prop_state == svn_wc_notify_state_unchanged ) ) )
				{
					break;
				}

				if ( ( in_pNotifyInfo->content_state == svn_wc_notify_state_conflicted ) || 
					 ( in_pNotifyInfo->prop_state == svn_wc_notify_state_conflicted ) )
				{
					csNotifyText.Format( _T("Conflicted: %s"), csPath );
				}
				else if ( ( in_pNotifyInfo->content_state == svn_wc_notify_state_merged) || 
					      ( in_pNotifyInfo->prop_state == svn_wc_notify_state_merged ) )
				{
					csNotifyText.Format( _T("Merged: %s"), csPath );
				}
				else
				{
					csNotifyText.Format( _T("Updated: %s"), csPath );
				}
				break;

			// The last notification in an update (including updates of externals).
			case svn_wc_notify_update_completed:
				csNotifyText.Format( _T("Completed at revision: %d."), in_pNotifyInfo->revision );
				break;

			// Updating an external module.
			case svn_wc_notify_update_external:
				csNotifyText.Format( _T("External: %s"), csPath );
				break;

			// The last notification in a status (including status on externals).
			case svn_wc_notify_status_completed:
			case svn_wc_notify_status_external:
				csNotifyText.Format( _T("Status") );
				break;

			// Committing a modification.
			case svn_wc_notify_commit_modified:
				csNotifyText.Format( _T("Modified: %s"), csPath );
				break;
			  
			// Committing an addition.
			case svn_wc_notify_commit_added:
				csNotifyText.Format( _T("Adding %s"), csPath );
				break;

			// Committing a deletion.
			case svn_wc_notify_commit_deleted:
				csNotifyText.Format( _T("Deleting %s"), csPath );
				break;

			// Committing a replacement.
			case svn_wc_notify_commit_replaced:
				csNotifyText.Format( _T("Replaced: %s"), csPath );
				break;

			// Transmitting post-fix text-delta data for a file.
			case svn_wc_notify_commit_postfix_txdelta:
				csNotifyText.Format( _T("Sending content: %s"), csPath );
				break;

			// Processed a single revision's blame.
			case svn_wc_notify_blame_revision:
				csNotifyText.Format( _T("Blame revision."), csPath );
				break;

			// Locking a path. @since New in 1.2.
			case svn_wc_notify_locked:
				if ( in_pNotifyInfo->lock && in_pNotifyInfo->lock->owner )
					csNotifyText.Format( _T("Locked by '%s': %s."), CString( in_pNotifyInfo->lock->owner ), csPath );
				break;

			// Unlocking a path. @since New in 1.2.
			case svn_wc_notify_unlocked:
				csNotifyText.Format( _T("Unlocked: %s"), csPath );
				break;

			// Failed to lock a path. @since New in 1.2.
			case svn_wc_notify_failed_lock:
				csNotifyText.Format( _T("Lock failed: %s"), csPath );
				break;

			// Failed to unlock a path. @since New in 1.2.
			case svn_wc_notify_failed_unlock:
				csNotifyText.Format( _T("Unlock failed: %s"), csPath );
				break;
		}

		if ( !csNotifyText.IsEmpty() )
			in_pSourceControl->m_pUtilities->GetProgressDialog()->AddLogMessage( csNotifyText );

		in_pSourceControl->ManageSVNError( in_pNotifyInfo->err, SVNErrorDestination_Progress, false );
	}
}

//
// Callbacks
//
svn_error_t* SubversionSourceControl::GetFileStatusCallback(SvnStatus* io_pStatus, const char * in_pPath, const svn_client_status_t * in_pSvnStatus, apr_pool_t * pool)
{
	ASSERT( io_pStatus && in_pSvnStatus && in_pPath );

	io_pStatus->m_kind = in_pSvnStatus->node_status;

	if( in_pSvnStatus->conflicted )
	{
		io_pStatus->m_bTreeConflict = true;
	}

	if ( in_pSvnStatus->lock && in_pSvnStatus->lock->owner )
	{
		io_pStatus->m_bLocked = true;
		io_pStatus->m_csLockOwner = CString( in_pSvnStatus->lock->owner );

		if ( in_pSvnStatus->lock->comment && in_pSvnStatus->lock->comment[0] )
		{
			CString csComment( in_pSvnStatus->lock->comment );
			csComment.Replace( L'\n', L' ' );
			csComment.Replace( L'\r', L'' );

			io_pStatus->m_csLockOwner += L'(' + csComment + L')';
		}
	}
	else
	{
		io_pStatus->m_bLocked = false;
	}

	return SVN_NO_ERROR;
}

svn_error_t * SubversionSourceControl::GetMissingFilesCallback(void * in_baton, const char * in_pPath, const svn_client_status_t * in_pSvnStatus, apr_pool_t * /*pool*/)
{
	AK::Wwise::ISourceControl::StringList* in_pMissingFileList = (AK::Wwise::ISourceControl::StringList*)in_baton;
	ASSERT( in_pSvnStatus && in_pMissingFileList && in_pPath );

	if ( in_pSvnStatus && in_pMissingFileList && in_pPath )
	{
		if( ( in_pSvnStatus->node_status == svn_wc_status_deleted ) || 
			( in_pSvnStatus->node_status == svn_wc_status_missing ) )
		{
			SvnPool localPool( NULL );
			CString csPath( GetPathFromSVN( in_pPath, localPool ) );

			if ( !::PathIsDirectory( csPath ) )
			{
				in_pMissingFileList->AddTail( csPath );
			}
		}
	}
	return SVN_NO_ERROR;
}

svn_error_t* SubversionSourceControl::ShowLogCallback( AK::Wwise::ISourceControlUtilities* in_pUtilities, apr_hash_t* in_paths, svn_revnum_t in_revision, const char* in_pAuthor, const char* in_pDate, const char* in_pMessage, apr_pool_t* in_pPool )
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	ASSERT( in_pUtilities && in_pPool );

	CString csAuthor;
	CString csDate;
	CString csMessage;
	CString csActions;

	// Get the author
	if ( in_pAuthor )
		csAuthor = CString( in_pAuthor );
	else
		csAuthor = _T("(no author)");

	svn_error_t* pError = NULL;

	// Format the the date
	if ( in_pDate && in_pDate[0] )
	{
		apr_time_t aprTime = 0;
		pError = svn_time_from_cstring( &aprTime, in_pDate, in_pPool );
		
		if ( !pError )
			csDate = GetSVNDate( aprTime );
	}
	else
	{
		csDate = _T("(no date)");
	}

	if ( !pError )
	{
		// Format the message
		if ( in_pMessage )
		{
			csMessage = CString( in_pMessage );
			csMessage.Replace( L'\n', L' ' );
			csMessage.Replace( L'\r', L'' );
		}
		else
		{
			csMessage = _T("(no message)");
		}

		// Get the actions
		if ( in_paths )
		{
			bool bAdded = false;
			bool bDeleted = false;
			bool bModified = false;
			bool bReplaced = false;

			apr_hash_index_t *pHashIndex = NULL;
			svn_log_changed_path_t* logItem = NULL;

			// Get all actions for the specified revision
			for ( pHashIndex=apr_hash_first( in_pPool, in_paths ) ; pHashIndex ; pHashIndex=apr_hash_next( pHashIndex ) )
			{
				apr_hash_this( pHashIndex, NULL, NULL, (void**)&logItem);

				if ( logItem )
				{
					switch( logItem->action )
					{
						case 'A':
							bAdded = true;
							break;
							
						case 'D':
							bDeleted = true;
							break;
							
						case 'M':
							bModified = true;
							break;
							
						case 'R':
							bReplaced = true;
							break;
					}
				}
			}

			// Format actions name
			if ( bAdded )
				csActions = GetSVNStatusKindText( svn_wc_status_added );

			if ( bDeleted )
				csActions += ( csActions.IsEmpty() ? L"" : L", " ) + GetSVNStatusKindText( svn_wc_status_deleted );

			if ( bModified )
				csActions += ( csActions.IsEmpty() ? L"" : L", " ) + GetSVNStatusKindText( svn_wc_status_modified );

			if ( bReplaced )
				csActions += ( csActions.IsEmpty() ? L"" : L", " ) + GetSVNStatusKindText( svn_wc_status_replaced );
		}

		if ( csActions.IsEmpty() )
		{
			csActions = _T("(no action)");
		}
	}

	if ( !pError )
	{
		CString csLog;
		csLog.Format( _T("Revision: %d, Actions: '%s', Author: '%s', Date: '%s', Message: '%s'"), in_revision, csActions, csAuthor, csDate, csMessage );
		
		in_pUtilities->GetProgressDialog()->AddLogMessage( csLog );
	}
	
	return pError;
}