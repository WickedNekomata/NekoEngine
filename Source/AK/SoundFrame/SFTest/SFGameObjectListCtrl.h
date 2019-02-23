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

#include <AK/SoundFrame/SFObjects.h>
#include "afxwin.h"

using namespace AK;
using namespace SoundFrame;

class CSFTestPositioningDlg;

// List control that is used to display Game Object
// It keep a reference to all IGameObject that is displayed in it.
class SFGameObjectListCtrl : public CListCtrl
{
public:
	SFGameObjectListCtrl();

	void Init( CSFTestPositioningDlg* in_pPositionDlg );

	void ClearList();

	IGameObject* GetObject( int in_idx );
	IGameObject* GetSelectedObject();
	int AddObject( IGameObject * in_pObject );
	int AddGlobalObject();
	void AddObjects( IGameObjectList * in_pObjectList );

	// Return true if the object was at least once in the list
	bool RemoveObject( AkGameObjectID in_uiGameObject );

private:

	CSFTestPositioningDlg* m_pPositionDlg;
};