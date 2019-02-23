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
////////////////////////////////////////////////////////////////////////
// SourceControlHelpers.cpp
//
// SourceControlHelpers implementation file
//
// Author:  brodrigue
// Version: 1.0
//
////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "SourceControlHelpers.h"

#include <AK/Wwise/SourceControl/ISourceControlUtilities.h>
#include <algorithm>
#include <shlwapi.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace
{
	bool FindNoCase( const std::vector<CString>& in_vector, const CString& in_tofind )
	{
		for( std::vector<CString>::const_iterator it = in_vector.begin(); it != in_vector.end(); it++)
		{
			if( in_tofind.CompareNoCase(*it) == 0 )
				return true;
		}

		return false;
	}

	CString FolderAddBackslash( const CString& in_csPath )
	{
		if ( ! in_csPath.IsEmpty() && in_csPath[ in_csPath.GetLength() - 1 ] != L'\\' )
		{
			return in_csPath + L'\\';
		}
		return in_csPath;
	}
}

bool SourceControlHelpers::CanProceedWithMove( 
	const CStringList& in_rFilenameList, 
	const CString& in_csDestinationDir,
	std::vector<CString>& out_rNewPaths,
	AK::Wwise::ISourceControlUtilities* in_pUtilities,
	LPCTSTR in_pszErrorAlreadyExist,
	LPCTSTR in_pszErrorNameConflict )
{
	bool bCanProceed = true;

	// Split the destination dir
	wchar_t szDstDrive[MAX_PATH] = {0}, szDstDir[MAX_PATH] = {0}, szDstName[MAX_PATH] = {0}, szDstExt[MAX_PATH] = {0};
	_wsplitpath_s( FolderAddBackslash( in_csDestinationDir ), szDstDrive, szDstDir, szDstName, szDstExt );

	// Ensure the move is a valid operation
	for( POSITION position = in_rFilenameList.GetHeadPosition(); position; in_rFilenameList.GetNext(position) )
	{
		CString csFilename = in_rFilenameList.GetAt(position);

		wchar_t szSrcDrive[MAX_PATH] = {0}, szSrcDir[MAX_PATH] = {0}, szSrcName[MAX_PATH] = {0}, szSrcExt[MAX_PATH] = {0};
		_wsplitpath_s( csFilename, szSrcDrive, szSrcDir, szSrcName, szSrcExt );

		wchar_t szNewPath[MAX_PATH] = {0};
		_wmakepath_s( szNewPath, szDstDrive, szDstDir, szSrcName, szSrcExt );

		if( PathFileExists(szNewPath) )
		{
			// Can't move: a file with the same name exist at destination
			CString csMsg;
			csMsg.FormatMessage( in_pszErrorAlreadyExist, szNewPath );
			in_pUtilities->GetProgressDialog()->AddLogMessage( csMsg );

			bCanProceed = false;
			break;
		}
		else if( FindNoCase( out_rNewPaths, szNewPath ) )
		{
			// Can't move: two files with the same name moved same destination
			CString csMsg;
			csMsg.FormatMessage( in_pszErrorNameConflict, szNewPath );
			in_pUtilities->GetProgressDialog()->AddLogMessage( csMsg );

			bCanProceed = false;
			break;
		}

		out_rNewPaths.push_back( szNewPath );
	}

	return bCanProceed;
}

CString SourceControlHelpers::GetLastErrorString()
{
	CString csMessage;
	DWORD err = GetLastError();
	if( err )
	{
		TCHAR* lpMsgBuf;
		if( ::FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			err,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPWSTR)&lpMsgBuf,
			0, NULL ) )
		{
			csMessage = lpMsgBuf;
			LocalFree(lpMsgBuf);
		}
	}
	if( csMessage.IsEmpty() )
	{
		csMessage = L"Unknown";
	}

	CString csFinal;
	csFinal.Format( _T("Error %d: '%s'"), err, (LPCTSTR)csMessage );
	return csFinal;
}

// Taken from ATL's AtlWaitWithMessageLoop implementation, with the Message type changed
bool SourceControlHelpers::WaitWithPaintLoop(HANDLE hEvent)
{
	DWORD dwRet;
	MSG msg;

	while(1)
	{
		dwRet = MsgWaitForMultipleObjects(1, &hEvent, FALSE,INFINITE, QS_PAINT);

		if (dwRet == WAIT_OBJECT_0)
			return TRUE; // The event was signaled

		if (dwRet != WAIT_OBJECT_0 + 1)
			break; // Something else happened

		// There is one or more window message available. Dispatch them
		while(PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			// Don't process more message if the event is signaled, stop right away!
			if (WaitForSingleObject(hEvent, 0) == WAIT_OBJECT_0)
				return true; // Event is now signaled.
		}
	}
	return false;
}
