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
#include "SFTestDropTarget.h"
#include "SFTestDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CSFDropTargetBase

CSFDropTargetBase::CSFDropTargetBase()
	:	m_pDlg( NULL )
{}

void CSFDropTargetBase::SetDlg( CSFTestDlg * in_pDlg )
{
	m_pDlg = in_pDlg;
}

DROPEFFECT CSFDropTargetBase::OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject,
	DWORD dwKeyState, CPoint point)
{
	return OnDragOver( pWnd, pDataObject, dwKeyState, point );
}

DROPEFFECT CSFDropTargetBase::OnDragOver(CWnd* pWnd, COleDataObject* pDataObject,
	DWORD dwKeyState, CPoint point)
{
	DROPEFFECT effect = DROPEFFECT_NONE;

	if ( m_pDlg )
	{
		// Ask SoundFrame the type of drag from from Wwise.

		ISoundFrame * pSoundFrame = m_pDlg->GetSoundFrame();
		if ( pSoundFrame )
		{
			// Check if it match the type wanted by this drop target.
			effect = ( pSoundFrame->GetDnDType( pDataObject->GetIDataObject( FALSE ) ) == GetType() ) ? DROPEFFECT_MOVE : DROPEFFECT_NONE;
		}
	}

	return effect;
}

// CSFEventDropTarget

ISoundFrame::DnDType CSFEventDropTarget::GetType()
{
	return ISoundFrame::TypeEvent;
}

BOOL CSFEventDropTarget::OnDrop(CWnd* pWnd, COleDataObject* pDataObject,
	DROPEFFECT dropEffect, CPoint point)
{
	BOOL result = FALSE;

	if ( m_pDlg )
	{
		// Ask SoundFrame to process Event drag from Wwise.

		ISoundFrame * pSoundFrame = m_pDlg->GetSoundFrame();
		if ( pSoundFrame )
		{
			IEventList * pEventList = NULL;

			result = pSoundFrame->ProcessEventDnD( pDataObject->GetIDataObject( FALSE ), &pEventList );
			if ( result )
			{
				// Add dragged events to our list.

                m_pDlg->AddEventsToList( pEventList );
				pEventList->Release();
			}
		}
	}

	return result;
}

// CSFStateGroupDropTarget

ISoundFrame::DnDType CSFStateGroupDropTarget::GetType()
{
	return ISoundFrame::TypeStates;
}

BOOL CSFStateGroupDropTarget::OnDrop(CWnd* pWnd, COleDataObject* pDataObject,
	DROPEFFECT dropEffect, CPoint point)
{
	BOOL result = FALSE;

	if ( m_pDlg )
	{
		// Ask SoundFrame to process States drag from Wwise.

		ISoundFrame * pSoundFrame = m_pDlg->GetSoundFrame();
		if ( pSoundFrame )
		{
			IStateGroupList * pStateGroupList = NULL;

			result = pSoundFrame->ProcessStateGroupDnD( pDataObject->GetIDataObject( FALSE ), &pStateGroupList );
			if ( result )
			{
				// Add dragged State Groups to our list.

				m_pDlg->AddStateGroupToList( pStateGroupList );
				pStateGroupList->Release();
			}
		}
	}

	return result;
}

// CSFSwitchGroupDropTarget

ISoundFrame::DnDType CSFSwitchGroupDropTarget::GetType()
{
	return ISoundFrame::TypeSwitches;
}

BOOL CSFSwitchGroupDropTarget::OnDrop(CWnd* pWnd, COleDataObject* pDataObject,
	DROPEFFECT dropEffect, CPoint point)
{
	BOOL result = FALSE;

	if ( m_pDlg )
	{
		// Ask SoundFrame to process Switches drag from Wwise.

		ISoundFrame * pSoundFrame = m_pDlg->GetSoundFrame();
		if ( pSoundFrame )
		{
			ISwitchGroupList * pSwitchGroupList = NULL;

			result = pSoundFrame->ProcessSwitchGroupDnD( pDataObject->GetIDataObject( FALSE ), &pSwitchGroupList );
			if ( result )
			{
				// Add dragged Switch Groups to our list.

				m_pDlg->AddSwitchGroupToList( pSwitchGroupList );
				pSwitchGroupList->Release();
			}
		}
	}

	return result;
}

// CSFGameParameterDropTarget

ISoundFrame::DnDType CSFGameParameterDropTarget::GetType()
{
	return ISoundFrame::TypeGameParameters;
}

BOOL CSFGameParameterDropTarget::OnDrop(CWnd* pWnd, COleDataObject* pDataObject,
	DROPEFFECT dropEffect, CPoint point)
{
	BOOL result = FALSE;

	if ( m_pDlg )
	{
		// Ask SoundFrame to process GameParameters drag from Wwise.

		ISoundFrame * pSoundFrame = m_pDlg->GetSoundFrame();
		if ( pSoundFrame )
		{
			IGameParameterList * pGameParameterList = NULL;

			result = pSoundFrame->ProcessGameParameterDnD( pDataObject->GetIDataObject( FALSE ), &pGameParameterList );
			if ( result )
			{
				// Add dragged Game Parameters to our list.

				m_pDlg->AddGameParameterToList( pGameParameterList );
				pGameParameterList->Release();
			}
		}
	}

	return result;
}

// CSFTriggerDropTarget

ISoundFrame::DnDType CSFTriggerDropTarget::GetType()
{
	return ISoundFrame::TypeTriggers;
}

BOOL CSFTriggerDropTarget::OnDrop(CWnd* pWnd, COleDataObject* pDataObject,
	DROPEFFECT dropEffect, CPoint point)
{
	BOOL result = FALSE;

	if ( m_pDlg )
	{
		// Ask SoundFrame to process Triggers drag from Wwise.

		ISoundFrame * pSoundFrame = m_pDlg->GetSoundFrame();
		if ( pSoundFrame )
		{
			ITriggerList * pTriggerList = NULL;

			result = pSoundFrame->ProcessTriggerDnD( pDataObject->GetIDataObject( FALSE ), &pTriggerList );
			if ( result )
			{
				// Add dragged Triggers to our list.

				m_pDlg->AddTriggerToList( pTriggerList );
				pTriggerList->Release();
			}
		}
	}

	return result;
}

// CSFAuxBusDropTarget

ISoundFrame::DnDType CSFAuxBusDropTarget::GetType()
{
	return ISoundFrame::TypeAuxBus;
}

BOOL CSFAuxBusDropTarget::OnDrop(CWnd* pWnd, COleDataObject* pDataObject,
	DROPEFFECT dropEffect, CPoint point)
{
	BOOL result = FALSE;

	if ( m_pDlg )
	{
		// Ask SoundFrame to process Triggers drag from Wwise.

		ISoundFrame * pSoundFrame = m_pDlg->GetSoundFrame();
		if ( pSoundFrame )
		{
			IAuxBusList * pAuxBusList = NULL;

			result = pSoundFrame->ProcessAuxBusDnD( pDataObject->GetIDataObject( FALSE ), &pAuxBusList );
			if ( result )
			{
				// Add dragged Triggers to our list.

				m_pDlg->AddAuxBusToList( pAuxBusList );
				pAuxBusList->Release();
			}
		}
	}

	return result;
}

// CSFDialogueEventDropTarget

ISoundFrame::DnDType CSFDialogueEventDropTarget::GetType()
{
	return ISoundFrame::TypeDialogueEvent;
}

BOOL CSFDialogueEventDropTarget::OnDrop(CWnd* pWnd, COleDataObject* pDataObject,
	DROPEFFECT dropEffect, CPoint point)
{
	BOOL result = FALSE;

	if ( m_pDlg )
	{
		// Ask SoundFrame to process Dialogue event drag from Wwise.

		ISoundFrame * pSoundFrame = m_pDlg->GetSoundFrame();
		if ( pSoundFrame )
		{
			IDialogueEventList * pDialogueEventList = NULL;

			result = pSoundFrame->ProcessDialogueEventDnD( pDataObject->GetIDataObject( FALSE ), &pDialogueEventList );
			if ( result )
			{
				// Add dragged Dialogue Events to our list.

				m_pDlg->AddDialogueEventToList( pDialogueEventList );
				pDialogueEventList->Release();
			}
		}
	}

	return result;
}

// CSFSoundBankDropTarget

ISoundFrame::DnDType CSFSoundBankDropTarget::GetType()
{
	return ISoundFrame::TypeSoundBank;
}

BOOL CSFSoundBankDropTarget::OnDrop(CWnd* pWnd, COleDataObject* pDataObject,
	DROPEFFECT dropEffect, CPoint point)
{
	BOOL result = FALSE;

	if ( m_pDlg )
	{
		// Ask SoundFrame to process Dialogue event drag from Wwise.

		ISoundFrame * pSoundFrame = m_pDlg->GetSoundFrame();
		if ( pSoundFrame )
		{
			ISoundBankList * pSoundBankList = NULL;

			result = pSoundFrame->ProcessSoundBankDnD( pDataObject->GetIDataObject( FALSE ), &pSoundBankList );
			if ( result )
			{
				// Add dragged Dialogue Events to our list.

				m_pDlg->AddSoundBankToList( pSoundBankList );
				pSoundBankList->Release();
			}
		}
	}

	return result;
}