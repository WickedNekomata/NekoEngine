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

#pragma once

#include "resource.h"

// Small dialog that will get the new Game object ID and name
class SFAddGameObjectDialog : public CDialog
{
public:
	SFAddGameObjectDialog(CWnd* pParent = NULL);	// standard constructor
	~SFAddGameObjectDialog();

	enum { IDD = IDD_ADD_GAME_OBJECT_DIALOG };

	UINT32 GetGameObjectID() const;
	const CString& GetGameObjectName() const;

protected:
	virtual void OnOK( );

private:

	UINT32 m_uiGameObjectID;
	CString m_csGameObjectName;
};