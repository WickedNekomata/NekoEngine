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
// OperationResults.h
//
// OperationResults header file
//
// Author:  brodrigue
// Version: 1.0
//
////////////////////////////////////////////////////////////////////////

#pragma once

#include <AK/Wwise/SourceControl/ISourceControl.h>
#include <vector>


// Stores results of the move
class FileOperationResult : public AK::Wwise::ISourceControl::IFileOperationResult
{
public:
	FileOperationResult();
	virtual ~FileOperationResult();
	
	virtual AK::Wwise::ISourceControl::OperationResult GetOperationResult();
	virtual void Destroy();

	void SetOperationResult( AK::Wwise::ISourceControl::OperationResult in_eResult );

	// Get a moved file
	virtual void GetMovedFile( unsigned int in_uiIndex, LPWSTR out_szFrom, LPWSTR out_szTo, unsigned int in_uiArraySize );
	virtual void GetFile( unsigned int in_uiIndex, LPWSTR out_szPath, unsigned int in_uiArraySize );
	
	// Get the number of moved file
	virtual unsigned int GetFileCount();

	// Add a file successfully moved
	void AddMovedFile( const CString& in_csFrom, const CString& in_csTo );

	// Add a file successfully moved
	void AddFile( const CString& in_csPath );

private:

	struct FileInfo
	{
		CString csFrom;
		CString csTo;
	};

	std::vector<FileInfo> m_files;

	AK::Wwise::ISourceControl::OperationResult m_eResult;
};
