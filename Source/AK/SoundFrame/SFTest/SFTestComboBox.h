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

#include "afxwin.h"

template<class TObject, class TList, class TIDType>
class SFTestComboBox : public CComboBox
{
public:
	void ClearCombo()
	{
		int cObjects = GetCount();
		for ( int i = 0; i < cObjects; i++ )
		{
			TObject * pObject = (TObject *) GetItemDataPtr( i );
			if ( pObject )
				pObject->Release();
		}

		ResetContent();
	}

	TObject* GetObject( int in_idx )
	{
		return (TObject*) GetItemDataPtr( in_idx );
	}

	int AddObject( TObject * in_pObject )
	{
		int idx = AddString( in_pObject->GetName() );
		SetItemDataPtr( idx, in_pObject );

		in_pObject->AddRef();

		return idx;
	}

	void AddObjects( TList * in_pObjectList )
	{
		while ( TObject * pObject = in_pObjectList->Next() )
		{
			AddObject( pObject );
		}

		in_pObjectList->Reset();
	}

	bool SelectObject( const TIDType& in_ObjectID )
	{
		int cObjects = GetCount();
		for ( int i = 0; i < cObjects; i++ )
		{
			TObject * pObject = (TObject *) GetItemDataPtr( i );
			if ( pObject && pObject->GetID() == in_ObjectID )
			{
				SetCurSel( i );
				return true;
			}
		}

		return false;
	}

	// Return true if the object was at least once in the list
	bool RemoveObject( const TIDType& in_ObjectID, bool& out_bWasSelected )
	{
		bool retVal = false;
		out_bWasSelected = false;

		int cObjects = GetCount();
		int curSel = GetCurSel();
		for ( int i = (cObjects - 1); i >= 0; --i )
		{
			TObject * pObject = (TObject *) GetItemDataPtr( i );
			if ( pObject && pObject->GetID() == in_ObjectID )
			{
				out_bWasSelected = out_bWasSelected || curSel == i;
				retVal = true;

				pObject->Release();
				DeleteString( i );
			}
		}

		return retVal;
	}
};