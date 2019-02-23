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

#include <AK/SoundFrame/SF.h>

#define WM_SF_SHOW (WM_APP+0x1)

#define BEGIN_TEMPLATE_MESSAGE_MAP2(theClass, type_namea, type_nameb, baseClass)\
	PTM_WARNING_DISABLE														\
	template < typename type_namea, typename type_nameb >					\
	const AFX_MSGMAP* theClass< typename type_namea, typename type_nameb >::GetMessageMap() const			\
		{ return GetThisMessageMap(); }										\
	template < typename type_namea, typename type_nameb >											\
	const AFX_MSGMAP* PASCAL theClass< typename type_namea, typename type_nameb >::GetThisMessageMap()		\
	{																		\
		typedef theClass< typename type_namea, typename type_nameb > ThisClass;							\
		typedef baseClass TheBaseClass;										\
		static const AFX_MSGMAP_ENTRY _messageEntries[] =					\
		{

template<typename TObject>
void SFTestListBox_HandleContextMenu( CListBox * in_pList, CPoint in_point )
{
	CMenu menu;
	menu.CreatePopupMenu();
    menu.AppendMenu( MF_STRING, AK::SoundFrame::ISoundFrame::ShowLocation_Editor, _T( "Edit") );
    menu.AppendMenu( MF_STRING, AK::SoundFrame::ISoundFrame::ShowLocation_MultiEditor, _T( "Edit with Multi-Editor") );
    menu.AppendMenu( MF_STRING, AK::SoundFrame::ISoundFrame::ShowLocation_ListView, _T( "Show in List View") );
    menu.AppendMenu( MF_STRING, AK::SoundFrame::ISoundFrame::ShowLocation_ReferenceView, _T( "Find all references") );
    menu.AppendMenu( MF_STRING, AK::SoundFrame::ISoundFrame::ShowLocation_SchematicView, _T( "Show in Schematic View") );
    menu.AppendMenu( MF_STRING, AK::SoundFrame::ISoundFrame::ShowLocation_ProjectExplorer0, _T( "Find in Project Explorer (No Sync Group)") );
    menu.AppendMenu( MF_STRING, AK::SoundFrame::ISoundFrame::ShowLocation_ProjectExplorer1, _T( "Find in Project Explorer (Sync Group 1)") );
    menu.AppendMenu( MF_STRING, AK::SoundFrame::ISoundFrame::ShowLocation_ProjectExplorer2, _T( "Find in Project Explorer (Sync Group 2)") );
    menu.AppendMenu( MF_STRING, AK::SoundFrame::ISoundFrame::ShowLocation_ProjectExplorer3, _T( "Find in Project Explorer (Sync Group 3)") );
    menu.AppendMenu( MF_STRING, AK::SoundFrame::ISoundFrame::ShowLocation_ProjectExplorer4, _T( "Find in Project Explorer (Sync Group 4)") );
    menu.AppendMenu( MF_STRING, AK::SoundFrame::ISoundFrame::ShowLocation_Transport, _T( "Show in Transport Control") );

	int iChoice = menu.TrackPopupMenu(TPM_NONOTIFY|TPM_RETURNCMD|TPM_LEFTALIGN, 
										in_point.x, in_point.y, in_pList);

	if ( iChoice != 0 )
	{
		std::vector<GUID> guids;

		CPoint ptClient = in_point;
		in_pList->ScreenToClient( &ptClient );

		BOOL bOutside = TRUE;
		int i = in_pList->ItemFromPoint( ptClient, bOutside );
		if ( !bOutside )
		{
			TObject * pObject = (TObject *) in_pList->GetItemDataPtr( i );
			GUID guidObject = pObject->GetGUID();
			guids.push_back( guidObject );
		}

		if ( !guids.empty() )
			in_pList->GetParent()->SendMessage( WM_SF_SHOW, iChoice, (LPARAM) &guids );
	}
}

// these object types do not expose guids
template<> inline void SFTestListBox_HandleContextMenu<AK::SoundFrame::IAction>( CListBox * in_pList, CPoint in_point ) {}
template<> inline void SFTestListBox_HandleContextMenu<AK::SoundFrame::IOriginalFile>( CListBox * in_pList, CPoint in_point ) {}

// List box that will contain Sound Frame Objects
// It keeps a referece to all objects in the list.
template<class TObject, class TList>
class SFTestListBox : public CListBox
{
public:
	void ClearList()
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
		if (in_idx >= 0 )
			return (TObject*) GetItemDataPtr( in_idx );
		else
			return NULL;
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

	// Return true if the object was at least once in the list
	bool RemoveObject( AkUniqueID in_ObjectID, bool& out_bWasSelected )
	{
		bool retVal = false;
		out_bWasSelected = false;

		int cObjects = GetCount();
		for ( int i = (cObjects - 1); i >= 0; --i )
		{
			TObject * pObject = (TObject *) GetItemDataPtr( i );
			if ( pObject && pObject->GetID() == in_ObjectID )
			{
				out_bWasSelected = out_bWasSelected || GetSel( i ) != FALSE;
				retVal = true;

				pObject->Release();
				DeleteString( i );
			}
		}

		return retVal;
	}

protected:
	DECLARE_MESSAGE_MAP()

	afx_msg void OnContextMenu( CWnd*, CPoint in_point )
	{
		SFTestListBox_HandleContextMenu<TObject>( this, in_point );
	}
};

BEGIN_TEMPLATE_MESSAGE_MAP2(SFTestListBox, TObject, TList, CListBox)
	ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()
