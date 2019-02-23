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
#include "afxole.h"

using namespace AK;
using namespace SoundFrame;

class CSFTestDlg;

// Classes in this file manage the Drop target for the different type of Sound Frame object

// Base class for all Drop Target
class CSFDropTargetBase
	: public COleDropTarget
{
public:
	CSFDropTargetBase();

	void SetDlg( CSFTestDlg * in_pDlg );

	virtual DROPEFFECT OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual DROPEFFECT OnDragOver(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);

protected:
	// Return the type of object the drop target accept.
	virtual ISoundFrame::DnDType GetType() = 0;

	CSFTestDlg * m_pDlg;
};

// Event Drop target
class CSFEventDropTarget
	: public CSFDropTargetBase
{
public:
	virtual ISoundFrame::DnDType GetType();

	virtual BOOL       OnDrop(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);
};

// State Group Drop target
class CSFStateGroupDropTarget
	: public CSFDropTargetBase
{
public:
	virtual ISoundFrame::DnDType GetType();

	virtual BOOL       OnDrop(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);
};

// Switch Group Drop target
class CSFSwitchGroupDropTarget
	: public CSFDropTargetBase
{
public:
	virtual ISoundFrame::DnDType GetType();

	virtual BOOL       OnDrop(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);
};

// Game Parameter Drop Target
class CSFGameParameterDropTarget
	: public CSFDropTargetBase
{
public:
	virtual ISoundFrame::DnDType GetType();

	virtual BOOL       OnDrop(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);
};

// Trigger Drop Target
class CSFTriggerDropTarget
	: public CSFDropTargetBase
{
public:
	virtual ISoundFrame::DnDType GetType();

	virtual BOOL       OnDrop(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);
};

// AuxBus Drop Target
class CSFAuxBusDropTarget
	: public CSFDropTargetBase
{
public:
	virtual ISoundFrame::DnDType GetType();

	virtual BOOL       OnDrop(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);
};

// Dialogue Event Drop Target
class CSFDialogueEventDropTarget
	: public CSFDropTargetBase
{
public:
	virtual ISoundFrame::DnDType GetType();

	virtual BOOL       OnDrop(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);
};

// SoundBank Drop Target
class CSFSoundBankDropTarget
	: public CSFDropTargetBase
{
public:
	virtual ISoundFrame::DnDType GetType();

	virtual BOOL       OnDrop(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);
};
