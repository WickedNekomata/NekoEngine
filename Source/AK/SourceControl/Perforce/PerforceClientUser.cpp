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
#include "p4/ErrorLog.h"
#include "Resource.h"
#include "p4/errornum.h"
#include "p4/MsgServer.h"
#include "SourceControlHelpers.h"

#include <shlwapi.h>
#include <atlbase.h>

#ifdef _DEBUG
	#define new DEBUG_NEW
#endif

using namespace AK;
using namespace Wwise;

namespace
{
	static const char* k_acFstatHeadAction = "headAction";
	static const char* k_acFstatHeadRev = "headRev";
	static const char* k_acFstatUnresolved = "unresolved";
	static const char* k_acFstatAction = "action";
	static const char* k_acFstatActionOwner = "actionOwner";
	static const char* k_acFstatHaveRev = "haveRev";
	static const char* k_acFstatMovedRev = "movedRev";
	static const char* k_acFstatOtherOpen = "otherOpen";
	static const char* k_acFstatOtherAction = "otherAction";
	static const char* k_acFstatClientFile = "clientFile";
	static const char* k_acFstatActionAdd = "add";
	static const char* k_acFstatActionEdit = "edit";
	static const char* k_acFstatActionDelete = "delete";
	static const char* k_acFstatActionBranch = "branch";
	static const char* k_acFstatActionMoveAdd = "move/add";
	static const char* k_acFstatActionMoveDelete = "move/delete";
	static const char* k_acFstatOurLock = "ourLock";
	static const char* k_acFstatOtherLock = "otherLock";
	static const char* k_acFstatMovedFile = "movedFile";
}

void PerforceSourceControl::Message( Error *in_pError )
{
	ErrorId* pErrorId = in_pError->GetId(0);
	const int iUniqueCode = pErrorId ? pErrorId->UniqueCode() : 0;

	if( ( iUniqueCode == MsgServer::BadPassword.UniqueCode() ||		// Invalid password
		  iUniqueCode == MsgServer::LoginExpired.UniqueCode() ||	// Login expired
		  iUniqueCode == MsgServer::BadPassword0.UniqueCode() ||	// Invalid password
		  iUniqueCode == MsgServer::BadPassword1.UniqueCode() )		// Invalid password
		&& ! m_bUserCancelledLogin )
	{
		m_bNeedLogin = true;

		// Don't show the error
		return;
	}

	if( in_pError->GetGeneric() == EV_CONFIG && 
		iUniqueCode == MsgServer::Unicode.UniqueCode() && 
		!m_bRetryUnicodeServer )
	{
		// Set the client in unicode
		const int utf8 = CharSetApi::UTF_8;
		int charset = CharSetApi::Lookup(m_pClient->GetCharset().Text());
		if (charset < 0) //charset env was not recognized
			charset = CharSetApi::NOCONV; 
		m_pClient->SetTrans(utf8, charset, utf8, utf8);
		m_pClientTag->SetTrans(utf8, charset, utf8, utf8);

		m_bRetryUnicodeServer = true;

		// Don't show the error
		return;
	}
	
	m_bRetryUnicodeServer = false;
	m_bNeedLogin = false;
	m_bUserCancelledLogin = false;

	StrBuf errorMsg;

	in_pError->Fmt( &errorMsg, EF_PLAIN );

	if( iUniqueCode != MsgServer::LoginNotRequired.UniqueCode() )
	{
		PrintText( 0, errorMsg.Text() );	
	}

	// Error handling
	if ( ( in_pError->GetSeverity() == E_FAILED ) || ( in_pError->GetSeverity() == E_FATAL ) )
	{
		if( m_operationResult == OperationResult_Succeed )
			m_operationResult = OperationResult_Failed;
		
		DisplayErrorMessageBox( in_pError );
	}
}

void PerforceSourceControl::OutputError( const char *in_pErrorBuffer )
{
	PrintText( 0, in_pErrorBuffer );
}

void PerforceSourceControl::OutputInfo( char in_cLevel, const char *in_pInfoBuffer )
{
	PrintText( in_cLevel, in_pInfoBuffer );
}

void PerforceSourceControl::OutputStat( StrDict* in_pStrDict )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	ASSERT( !m_currentOperation.empty() );
	if( m_currentOperation.empty() )
		return;

	OperationID operation = m_currentOperation.top();

	if( operation == OperationID_GetSubmitFiles )
	{
		StrPtr* pDepotFile = in_pStrDict->GetVar( "depotFile" );
		if( pDepotFile )
			m_submitFileList.AddTail( CString(CA2W( pDepotFile->Text(), CP_UTF8 )) );
		return;
	}
	else if( operation == OperationID_Workspaces )
	{
		// Save the root/client associations
		StrPtr* pClient = in_pStrDict->GetVar( "client" );
		StrPtr* pRoot = in_pStrDict->GetVar( "Root" );

		if( pClient && pRoot )
		{
			m_workspaces[CString(CA2W( pClient->Text(), CP_UTF8 ))] = 
				CString(CA2W( pRoot->Text(), CP_UTF8 ));
		}
		return;
	}
	else if( operation == OperationID_Opened && m_pMissingFiles )
	{
		// We are looking for missing files
		StrPtr* pAction = in_pStrDict->GetVar( k_acFstatAction );
		StrPtr* pClientFile = in_pStrDict->GetVar( k_acFstatClientFile );
		StrPtr* pClient = in_pStrDict->GetVar( "client" );

		if( pAction && pClientFile )
		{
			CString csAction( CA2W( pAction->Text(), CP_UTF8 ) );
			if( csAction == L"delete" ||
				csAction == L"move/delete" )
			{
				CString csClientFile( CA2W( pClientFile->Text(), CP_UTF8 ) );
				CString csClient( CA2W( pClient->Text(), CP_UTF8 ) );

				ClientToRootPath::iterator itFound = m_workspaces.find( csClient );
				if( itFound != m_workspaces.end() )
				{
					// Replace the client by the root path
					csClientFile.Replace( L"//" + csClient, itFound->second );

					csClientFile.Replace( _T('/'), _T('\\') );	
					csClientFile.Replace( _T("\\\\"), _T("\\") );	

					m_pMissingFiles->AddTail( csClientFile );
				}
			}
		}
		return;
	}

	if( operation == OperationID_LoginCheckExpired || operation == OperationID_Login)
		return;

	ASSERT( operation == OperationID_GetFileStatus );

	const CString csFilename = CString( CA2W( in_pStrDict->GetVar( k_acFstatClientFile )->Text(), CP_UTF8 ) );
	const CString csFileKey = FixFileMapKey( csFilename );

	// Get the Status/Owner text and the FileStatus value
	CString csStatus, csOwner;
	DWORD fileStatus = 0;
	
	// Status dictionary values
	StrPtr* pAction = in_pStrDict->GetVar( k_acFstatAction );
	StrPtr* pHaveRev = in_pStrDict->GetVar( k_acFstatHaveRev );
	StrPtr* pHeadRev = in_pStrDict->GetVar( k_acFstatHeadRev );
	StrPtr* pUnresolved = in_pStrDict->GetVar( k_acFstatUnresolved );
	StrPtr* pOurLock = in_pStrDict->GetVar( k_acFstatOurLock );
	
	// Get Unresolved status
	if ( pUnresolved )
	{
		CString csUnresolved;
		csUnresolved = _T("unresolved");

		AddEnumerationString( csStatus, csUnresolved );

		fileStatus |= FileStatus_Unresolved;
	}
	
	// Get the current action on the file
	if ( pAction )
	{
		AddEnumerationString( csStatus, CString( CA2W( pAction->Text(), CP_UTF8 ) ) );

		// Convert from text status to FileStatus value
		if ( ::strcmp( pAction->Text(), k_acFstatActionAdd ) == 0 )
		{
			fileStatus |= FileStatus_Added;
		}
		else if ( ::strcmp( pAction->Text(), k_acFstatActionDelete ) == 0 )
		{
			fileStatus |= FileStatus_Deleted;
		}
		else if ( ::strcmp( pAction->Text(), k_acFstatActionEdit ) == 0 )
		{
			fileStatus |= FileStatus_LocalEdit;
		}
		else if ( ::strcmp( pAction->Text(), k_acFstatActionBranch ) == 0 )
		{
			fileStatus |= FileStatus_Branch;
		}
		else if ( ::strcmp( pAction->Text(), k_acFstatActionMoveAdd ) == 0 )
		{
			fileStatus |= FileStatus_MoveAdd;
		}
		else if ( ::strcmp( pAction->Text(), k_acFstatActionMoveDelete ) == 0 )
		{
			fileStatus |= FileStatus_MoveDelete;
		}
	}

	// Get the current user locked status
	if ( pOurLock )
	{
		CString csLock;
		csLock = _T("lock");

		AddEnumerationString( csStatus, csLock );
		fileStatus |= FileStatus_LocalLock;
	}

	// Check if we have a version of the file
	if ( pHaveRev )
	{
		// If we have a movedRev, use it instead of the headRev
		StrPtr* pMovedRev = in_pStrDict->GetVar( k_acFstatMovedRev );
		if( pMovedRev )
			pHeadRev = pMovedRev;

		// Check if we have the latest version of the file
		if ( pHeadRev && ( atoi( pHeadRev->Text() ) > atoi( pHaveRev->Text() ) ) )
		{
			// There's a newer version of the file on the server
			CString csOutdated;
			csOutdated = _T("outdated");

			AddEnumerationString( csStatus, csOutdated );

			fileStatus |= FileStatus_Outdated;
		}
		
		DWORD dwFileAttributes = ::GetFileAttributes( csFilename );
		if ( dwFileAttributes == INVALID_FILE_ATTRIBUTES || ( dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) )
		{
			// The file doesn't exist locally, or is obstructed by a folder with the same name
			CString csMissing;
			csMissing = _T("missing");

			AddEnumerationString( csStatus, csMissing );

			fileStatus |= FileStatus_LocalMissing;
		}

		// Set the default "normal" text only if we have no other status text displayed
		if ( csStatus.IsEmpty() )
		{
			csStatus = _T("normal");
		}

		// If we are here, the file is on the server
		fileStatus |= FileStatus_OnServer;
	}

	// Owner
	StrPtr* pOwner = in_pStrDict->GetVar( k_acFstatActionOwner );
	StrPtr* pOtherOpen = in_pStrDict->GetVar( k_acFstatOtherOpen );

	// We own the file
	if ( pOwner )
	{
		AddEnumerationString( csOwner, CString( CA2W( pOwner->Text(), CP_UTF8  ) ) );
	}

	// Add other owner adding the action in parenthesis
	if ( pOtherOpen )
	{
		unsigned int uiOtherOpenCount = pOtherOpen->Atoi( pOtherOpen->Text() );

		// Each owner have a difference set of variables
		for ( unsigned int i=0 ; i<uiOtherOpenCount ; ++i )
		{
			CString csOtherOpen( k_acFstatOtherOpen );
			CString csOtherAction( k_acFstatOtherAction );
			CString csOtherLock( k_acFstatOtherLock );
			char acNumber[8];

			// Add the current otherOwner number at the end of the variables
			_itoa( i, acNumber, 10 );

			CString csNumber( acNumber );

			csOtherOpen += csNumber;
			csOtherAction += csNumber;
			csOtherLock += csNumber;

			CW2A strOtherOpen( csOtherOpen, CP_UTF8  );
			CW2A strOtherAction( csOtherAction, CP_UTF8  );
			CW2A strOtherLock( csOtherLock, CP_UTF8  );
			StrPtr* pOtherOpenN = in_pStrDict->GetVar( static_cast<const char*>( strOtherOpen ) );
			StrPtr* pOtherActionN = in_pStrDict->GetVar( static_cast<const char*>( strOtherAction ) );
			StrPtr* pOtherLockN = in_pStrDict->GetVar( static_cast<const char*>( strOtherLock ) );

			if ( pOtherActionN )
			{
				// Add the action taken by the other owner
				CString csOwnerText;
				csOwnerText.Format( L"%s(%s", CString( CA2W( pOtherOpenN->Text(), CP_UTF8 ) ), CString( pOtherActionN->Text() ) );

				if (strcmp(pOtherActionN->Text(), "edit") == 0)
					fileStatus |= FileStatus_OtherCheckout;

				// Add a "lock" action text if he locked the file
				if ( pOtherLockN )
				{
					CString csLock;
					csLock = _T("lock");

					csOwnerText += CString( L", " ) + csLock;

					fileStatus |= FileStatus_OtherLock;
				}

				// Close the parenthesis and add it the the list of user
				csOwnerText += CString( L')' );

				AddEnumerationString( csOwner, csOwnerText );
			}
		}
	}

	// Set the new Status/Owner text and the new FileStatus value
	FilenameToStatusMapItem filenameToStatusMapItem = { csStatus.AllocSysString(), csOwner.AllocSysString() };

	m_fileStatusMap.SetAt( csFileKey, (FileStatus)fileStatus );

	if ( m_pFilenameToStatusMap )
		m_pFilenameToStatusMap->SetAt( csFileKey, filenameToStatusMapItem );
}

void PerforceSourceControl::InputData( StrBuf* in_pBuffer, Error* in_pError )
{
	ASSERT( !m_currentOperation.empty() && m_currentOperation.top() == OperationID_Submit );

	// Constant fields
	in_pBuffer->Append( "Change:\tnew\n" );
	in_pBuffer->Append( "Description:\n\t" );

	// Description
	m_csDescription.Replace( L"\n", L"\n\t" );
	CW2A strDescription( m_csDescription, CP_UTF8 );
	in_pBuffer->Append( static_cast<char*>( strDescription ) );
	in_pBuffer->Append( "\n" );

	// Create file list
	POSITION filePos = m_submitFileList.GetHeadPosition();
	ASSERT( filePos && "There must be at least one file to submit." );

	in_pBuffer->Append( "Files:" );

	while ( filePos )
	{
		CString csFilename = m_submitFileList.GetAt( filePos );

		// Convert and append filename
		CW2A strFilename( csFilename, CP_UTF8 );
		char* acFilename = static_cast<char*>( strFilename );

		in_pBuffer->Append( "\n\t" );
		in_pBuffer->Append( acFilename );

		m_submitFileList.GetNext( filePos );
	}
}

void PerforceSourceControl::Prompt( const StrPtr& in_rMessage, StrBuf& out_rUserResponse, int in_iNoEcho, Error* in_pError )
{
	if( !m_currentOperation.empty() && m_currentOperation.top() == OperationID_Resolve )
	{
		if ( m_bFirstResolve )
		{
			m_bFirstResolve = false;
			out_rUserResponse = "m";
		}
		else
		{
			AFX_MANAGE_STATE( AfxGetStaticModuleState() );

			CString csCaption;
			CString csMessage;

			csCaption = _T("Perforce plug-in");
			csMessage = _T("Replace the current file with the merged result file?");

			if ( m_bMergeFileChanged && m_pUtilities->MessageBox( NULL, csMessage, csCaption, MB_YESNO | MB_ICONQUESTION ) == IDYES )
			{
				out_rUserResponse = "a";
				m_bFirstResolve = true; // Restore flag to resolve a second conflict if necessary
			}
			else
			{
				out_rUserResponse = "s";
			}
		}

		m_bMergeFileChanged = false;
	}
	else if( !m_currentOperation.empty() && m_currentOperation.top() == OperationID_Login )
	{
		AFX_MANAGE_STATE( AfxGetStaticModuleState() );

		CString csCaption, csMessage;

		csCaption = _T("Perforce Login");
		csMessage = in_rMessage.Value();

		CString csInput;
		m_bUserCancelledLogin = m_pUtilities->PromptMessage( NULL, csMessage, csCaption, csInput.GetBufferSetLength( 256 ), 256, true ) != IDOK;

		if( ! m_bUserCancelledLogin )
		{
			USES_CONVERSION;
			out_rUserResponse = W2A((LPCTSTR)csInput);
		}
	}
}

void PerforceSourceControl::Diff( FileSys* in_pTheirsFile, FileSys* in_pYoursFile, int in_iDoPage, char* in_acDiffFlags, Error* in_pError )
{
	bool bSucceed = true;
	CString csCommand;
	CString csTheirsFile;
	CString csYoursFile = CString( CA2W( in_pYoursFile->Path()->Text(), CP_UTF8 ) );

	CString csNewName = GetFilename( CString( CA2W(in_pYoursFile->Path()->Text(), CP_UTF8 ) ) );

	// Move temporary file
	if ( MoveToTemporaryFolder( CString( CA2W( in_pTheirsFile->Path()->Text(), CP_UTF8 ) ), csNewName + CString( L"#HEAD" ), csTheirsFile ) )
	{
		CW2A strTheirsFile( csTheirsFile, CP_UTF8 );
		in_pTheirsFile->Set( strTheirsFile );
	}
	else
	{
		bSucceed = false;
	}

	if ( bSucceed )
	{
        // Get the diff tool from user preferences
		TCHAR szDiffCommand[MAX_PATH] = {0};
		ULONG uSize = ARRAYSIZE( szDiffCommand );
		m_pUtilities->GetUserPreferenceString( k_csRegFolder + k_csRegKeyDiffCommand, szDiffCommand, uSize );

		DWORD dwUseAKWaveViewerForDiff = 1;
		m_pUtilities->GetUserPreferenceDword( k_csRegFolder + k_csUseAKWaveViewerForDiff, dwUseAKWaveViewerForDiff );

		CString csCommand( szDiffCommand );
        
        // If no diff tool found, use P4Merge
		if( csCommand.IsEmpty() )
			csCommand = L"p4Merge %1 %2";

		if( dwUseAKWaveViewerForDiff && CString( ::PathFindExtension( csYoursFile )).CompareNoCase( L".wav" ) == 0 ) 
		{
			// Use Ak WaveViewer
			TCHAR szPath[MAX_PATH] = {0};
			::GetModuleFileName( NULL, szPath, MAX_PATH );

			::PathRemoveFileSpec( szPath );
			
			TCHAR szWaveViewer[MAX_PATH] = {0};
			::PathCombine( szWaveViewer, szPath, L"WaveViewer.exe" );

			csCommand = szWaveViewer;
			csCommand += L" -noakd -diff %1 %2";
		}

		csCommand.Replace( L"%1", L"\"" + csTheirsFile + L"\"" );
		csCommand.Replace( L"%2", L"\"" + csYoursFile + L"\"" );

		Error error;
		CW2A strCommand( csCommand, CP_UTF8 );

		RunCmd( strCommand, NULL, NULL, NULL, NULL, NULL, NULL, &error );
	}
	else
	{
		AFX_MANAGE_STATE( AfxGetStaticModuleState() );

		CString csCaption;
		CString csMessage;

		csCaption = _T("Perforce plug-in");
		csMessage = _T("An error occurred launching diff tool");

		m_pUtilities->MessageBox( NULL, csMessage, csCaption, MB_OK | MB_ICONERROR );
	}

	m_bFileDiff = true;
}

void PerforceSourceControl::Merge( FileSys* in_pBaseFile, FileSys* in_pTheirsFile, FileSys* in_pYoursFile, FileSys* in_pResultFile, Error* in_pError )
{
	bool bSucceed = true;

	CString csBaseFilename;
	CString csTheirsFilename;
	CString csResultFilename;

	CString csNewName = GetFilename( CString( CA2W( in_pYoursFile->Path()->Text(), CP_UTF8 ) ) );

	// Move Base
	if ( MoveToTemporaryFolder( CString( CA2W( in_pBaseFile->Path()->Text(), CP_UTF8 ) ), csNewName + CString( L"#BASE" ), csBaseFilename ) )
	{
		CW2A strBaseFilename( csBaseFilename, CP_UTF8 );
		in_pBaseFile->Set( strBaseFilename );
	}
	else
	{
		bSucceed = false;
	}

	// Move Theirs
	if ( bSucceed )
	{
		if ( MoveToTemporaryFolder( CString( CA2W( in_pTheirsFile->Path()->Text(), CP_UTF8 ) ), csNewName + CString( L"#THEIRS" ), csTheirsFilename ) )
		{
			CW2A strTheirsFilename( csTheirsFilename, CP_UTF8 );
			in_pTheirsFile->Set( strTheirsFilename );
		}
		else
		{
			bSucceed = false;
		}
	}

	// Move Result
	if ( bSucceed )
	{
		if ( MoveToTemporaryFolder( CString( CA2W( in_pResultFile->Path()->Text(), CP_UTF8 ) ), csNewName + CString( L"#MERGE" ), csResultFilename ) )
		{
			CW2A strResultFilename( csResultFilename, CP_UTF8 );
			in_pResultFile->Set( strResultFilename );
		}
		else
		{
			bSucceed = false;
		}
	}

	if ( bSucceed )
	{
		CString csCommand;
		CString csYoursFilename = CString( CA2W( in_pYoursFile->Path()->Text(), CP_UTF8 ) );

		if ( bSucceed )
		{
			// Get the file time
			FILETIME creationTime, accessTime, writeTime;
			HANDLE hFile = CreateFile(csResultFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			::GetFileTime( hFile, &creationTime, &accessTime, &writeTime );
			CloseHandle( hFile );
			
			// Run the merge tool
			csCommand.Format( L"p4Merge \"%s\" \"%s\" \"%s\" \"%s\"", csBaseFilename, csTheirsFilename, csYoursFilename, csResultFilename );

			Error error;
			CW2A strCommand( csCommand, CP_UTF8 );

			RunCmd( strCommand, NULL, NULL, NULL, NULL, NULL, NULL, &error );

			// Get the post file time to see if the file was saved
			FILETIME postCreationTime, postAccessTime, postWriteTime;
			hFile = CreateFile(csResultFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			::GetFileTime( hFile, &postCreationTime, &postAccessTime, &postWriteTime );
			CloseHandle( hFile );

			if ( ( postWriteTime.dwHighDateTime != writeTime.dwHighDateTime ) || ( postWriteTime.dwLowDateTime != writeTime.dwLowDateTime ) )
			{
				m_bMergeFileChanged = true;
			}
		}
	}

	if ( !bSucceed )
	{
		AFX_MANAGE_STATE( AfxGetStaticModuleState() );

		CString csCaption;
		CString csMessage;

		csCaption = _T("Perforce plug-in");
		csMessage = _T("An error occurred launching merge tool");

		m_pUtilities->MessageBox( NULL, csMessage, csCaption, MB_OK | MB_ICONERROR );
	}
}

void PerforceSourceControl::RunCmd( const char* in_pCommand, const char* in_pArg1, const char* in_pArg2, const char* in_pArg3, const char* in_pArg4, 
									const char* in_pArg5, const char* in_pPager, Error* in_pError )
{
	STARTUPINFO startupInfo;
	PROCESS_INFORMATION processInfo;

	::ZeroMemory( &startupInfo, sizeof(startupInfo) );
	startupInfo.cb = sizeof(startupInfo);
	::ZeroMemory( &processInfo, sizeof(processInfo) );

	CString csCommand( in_pCommand );
	if( in_pArg1 ) csCommand += CString( L" " ) + CString(in_pArg1);
	if( in_pArg2 ) csCommand += CString( L" " ) + CString(in_pArg2);
	if( in_pArg3 ) csCommand += CString( L" " ) + CString(in_pArg3);
	if( in_pArg4 ) csCommand += CString( L" " ) + CString(in_pArg4);
	if( in_pArg5 ) csCommand += CString( L" " ) + CString(in_pArg5);

	CW2W acCommand( csCommand );

	if ( ::CreateProcess(NULL, acCommand, NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInfo) )
	{
		SourceControlHelpers::WaitWithPaintLoop( processInfo.hProcess );

		::CloseHandle( processInfo.hProcess );
		::CloseHandle( processInfo.hThread );
	}
}


void ProcessMessages( HWND in_hWnd )
{
	MSG msg = {0};

	// There is one or more window message available. Dispatch them
	while(PeekMessage(&msg,in_hWnd,NULL,NULL,PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

int PerforceSourceControl::IsAlive()
{
	int iReturn = 1;

	DWORD dwElapsed = GetTickCount() - m_dwOperationBeginTime;
	
	if( dwElapsed > m_dwOperationTimeout )
	{
		iReturn = 0;
		m_operationResult = OperationResult_TimedOut;
	}
	else if( m_pOperationDialog == NULL && dwElapsed > 3000 && IsMainThread() && !m_currentOperation.empty() && !m_bAutoAcceptMode )
	{
		// Show Cancel when the operation takes more than 3 seconds
		AFX_MANAGE_STATE( AfxGetStaticModuleState() );
		m_pOperationDialog = new OperationDialog( this );
	}

	// Cancel operation if it times-out

	if( m_pOperationDialog )
		ProcessMessages( m_pOperationDialog->m_hWnd );

	if ( m_bCancelOperation || ( m_pUtilities && m_pUtilities->GetProgressDialog() && m_pUtilities->GetProgressDialog()->IsCanceled() ) )
	{
		iReturn = 0;
	}

	return iReturn;
}