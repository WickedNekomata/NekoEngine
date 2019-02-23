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
#include "SFAddGameObjectDialog.h"

#include <AK/SoundFrame/SFObjects.h>

using namespace AK;
using namespace SoundFrame;

SFAddGameObjectDialog::SFAddGameObjectDialog( CWnd* pParent )
:	CDialog( SFAddGameObjectDialog::IDD, pParent )
,	m_uiGameObjectID( IGameObject::s_InvalidGameObject )
{
}
	
SFAddGameObjectDialog::~SFAddGameObjectDialog()
{
}

UINT32 SFAddGameObjectDialog::GetGameObjectID() const
{
	return m_uiGameObjectID;
}
	
const CString& SFAddGameObjectDialog::GetGameObjectName() const
{
	return m_csGameObjectName;
}

void SFAddGameObjectDialog::OnOK( )
{
	CString csGameObjectID;
	GetDlgItemText( IDC_GAME_OBJECT_ID_EDIT, csGameObjectID );

	GetDlgItemText( IDC_GAME_OBJECT_NAME_EDIT, m_csGameObjectName );

	m_uiGameObjectID = _ttol( csGameObjectID );

	if( csGameObjectID.IsEmpty() || m_uiGameObjectID == IGameObject::s_InvalidGameObject )
	{
		MessageBox( L"Enter a valid Game Object ID." );
		return;
	}

	CDialog::OnOK(); 
}