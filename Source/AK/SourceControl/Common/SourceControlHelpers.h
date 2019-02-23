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
// SourceControlHelpers.h
//
// SourceControlHelpers header file
//
// Author:  brodrigue
// Version: 1.0
//
////////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>

namespace AK{ namespace Wwise{ class ISourceControlUtilities; } }

namespace SourceControlHelpers
{
	bool CanProceedWithMove( 
		const CStringList& in_rFilenameList, 
		const CString& in_csDestinationDir,
		std::vector<CString>& out_rNewPaths,
		AK::Wwise::ISourceControlUtilities* in_pUtilities,
		LPCTSTR in_pszErrorAlreadyExist,
		LPCTSTR in_pszErrorNameConflict );

	CString GetLastErrorString();

	bool WaitWithPaintLoop(HANDLE hEvent);
};
