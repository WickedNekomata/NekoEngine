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
#include "SFGameObjectListCtrl.h"
#include "SFTestPositioningDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

SFGameObjectListCtrl::SFGameObjectListCtrl()
	: m_pPositionDlg( NULL )
{}

void SFGameObjectListCtrl::Init( CSFTestPositioningDlg* in_pPositionDlg )
{
	m_pPositionDlg = in_pPositionDlg;

	InsertColumn( 0, L"ID", LVCFMT_LEFT, 30 );
	InsertColumn( 1, L"Name", LVCFMT_LEFT, 150 );
	InsertColumn( 2, L"Pos.X", LVCFMT_LEFT, 45 );
	InsertColumn( 3, L"Pos.Z", LVCFMT_LEFT, 45 );
	InsertColumn( 4, L"Ori.X", LVCFMT_LEFT, 45 );
	InsertColumn( 5, L"Ori.Z", LVCFMT_LEFT, 45 );

	SetExtendedStyle( GetExtendedStyle() | LVS_EX_FULLROWSELECT );
}

void SFGameObjectListCtrl::ClearList()
{
	int cObjects = GetItemCount();
	for ( int i = 0; i < cObjects; i++ )
	{
		IGameObject * pObject = (IGameObject *) GetItemData( i );
		if ( pObject )
			pObject->Release();
	}

	DeleteAllItems();
}

IGameObject* SFGameObjectListCtrl::GetObject( int in_idx )
{
	return (IGameObject*) GetItemData( in_idx );
}

IGameObject* SFGameObjectListCtrl::GetSelectedObject()
{
	IGameObject* pReturn = NULL;

	POSITION pos = GetFirstSelectedItemPosition();
	if ( pos )
	{
		pReturn = GetObject( GetNextSelectedItem(pos) );
	}
	return pReturn;
}

int SFGameObjectListCtrl::AddObject( IGameObject * in_pObject )
{
	CString csTemp;
	csTemp.Format( _T("%d"), in_pObject->GetID() );

	int idx = InsertItem( GetItemCount(), csTemp );
	SetItemText( idx, 1, in_pObject->GetName() );

	static const AkVector pos = { 0, 0, 0 };
	static const AkVector front = { 0, 0, -1 };
	static const AkVector top = { 0, 1, 0 };
	AkSoundPosition gameObjectPosition;
	gameObjectPosition.Set(pos, front, top);
	
	m_pPositionDlg->GetGameObjectPosition( in_pObject->GetID(), gameObjectPosition );

	csTemp.Format( _T("%.0f"), gameObjectPosition.Position().X );
	SetItemText( idx, 2, csTemp );
	csTemp.Format( _T("%.0f"), gameObjectPosition.Position().Z );
	SetItemText( idx, 3, csTemp );
	csTemp.Format( _T("%.2f"), gameObjectPosition.OrientationFront().X );
	SetItemText( idx, 4, csTemp );
	csTemp.Format( _T("%.2f"), gameObjectPosition.OrientationFront().Z );
	SetItemText( idx, 5, csTemp );

	SetItemData( idx, (DWORD_PTR)in_pObject );

	in_pObject->AddRef();

	return idx;
}

int SFGameObjectListCtrl::AddGlobalObject()
{
	int idx = InsertItem( GetItemCount(), _T("-") );// no ID for global object
	SetItemText( idx, 1, _T("Global") );

	SetItemText( idx, 2, _T("none") );
	SetItemText( idx, 3, _T("none") );
	SetItemText( idx, 4, _T("none") );
	SetItemText( idx, 5, _T("none") );

	SetItemData( idx, NULL );

	return idx;
}

void SFGameObjectListCtrl::AddObjects( IGameObjectList * in_pObjectList )
{
	while ( IGameObject * pObject = in_pObjectList->Next() )
	{
		AddObject( pObject );
	}
}

bool SFGameObjectListCtrl::RemoveObject( AkGameObjectID in_uiGameObject )
{
	bool retVal = false;

	int cObjects = GetItemCount();
	for ( int i = (cObjects - 1); i >= 0; --i )
	{
		IGameObject* pObject = GetObject( i );
		if ( pObject && pObject->GetID() == in_uiGameObject )
		{
			pObject->Release();
			DeleteItem( i );

			retVal = true;
		}
	}

	return retVal;
}