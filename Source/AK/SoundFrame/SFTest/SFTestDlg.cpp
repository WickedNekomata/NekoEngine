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

// SFTestDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SFTestDlg.h"
#include "SFTestHelpers.h"

#include "SFAddGameObjectDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CSFTestDlg dialog

CSFTestDlg::CSFTestDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSFTestDlg::IDD, pParent)
	, m_pSoundFrame( NULL )
	, m_soundObjectDisplayed( AK_INVALID_UNIQUE_ID )
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_eventTarget.SetDlg( this );
	m_statesTarget.SetDlg( this );
	m_switchesTarget.SetDlg( this );
	m_gameParametersTarget.SetDlg( this );
	m_triggersTarget.SetDlg( this );
	m_AuxBusTarget.SetDlg( this );
	m_dialogueEventTarget.SetDlg( this );
	m_soundBankTarget.SetDlg( this );
}

CSFTestDlg::~CSFTestDlg()
{
	_ASSERT( m_pSoundFrame == NULL );
}

void CSFTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EVENTLIST, m_eventList);
	DDX_Control(pDX, IDC_ACTIONLIST, m_actionList);
	DDX_Control(pDX, IDC_STATE_GROUP_LIST, m_stateGroupList);
	DDX_Control(pDX, IDC_CURRENT_STATE_COMBO, m_currentStateCombo);
	DDX_Control(pDX, IDC_SWITCH_GROUP_LIST, m_switchGroupList);
	DDX_Control(pDX, IDC_CURRENT_SWITCH_COMBO, m_currentSwitchCombo);
	DDX_Control(pDX, IDC_GAME_OBJECTS_LIST, m_gameObjectListCtrl);
	DDX_Control(pDX, IDC_GAME_PARAMETER_LIST, m_gameParameterList);
	DDX_Control(pDX, IDC_CONVERSION_SETTINGS_LIST, m_conversionSettingsList);
	DDX_Control(pDX, IDC_GAME_PARAMETER_VALUE_SLIDER, m_gameParameterSlider);
	DDX_Control(pDX, IDC_TRIGGER_LIST, m_triggerList);
	DDX_Control(pDX, IDC_DIALOGUE_EVENT_LIST, m_dialogueEventList);
	DDX_Control(pDX, IDC_ARGUMENT_LIST, m_argumentList);
	DDX_Control(pDX, IDC_ARGUMENT_VALUE_LIST, m_argumentValueList);
	DDX_Control(pDX, IDC_EVENT_SOURCES_LIST, m_eventSourcesValueList);
	DDX_Control(pDX, IDC_DIALOGUE_EVENT_SOURCES_LIST, m_dialogueEventSourcesValueList);
	DDX_Control(pDX, IDC_SOUNDBANK_LIST, m_soundBankList);
}

BEGIN_MESSAGE_MAP(CSFTestDlg, CDialog)
	ON_WM_DESTROY()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_GAME_OBJECTS_LIST, OnLvnItemChangedGameObjectList)
	ON_LBN_SELCHANGE(IDC_EVENTLIST, OnLbnSelchangeEventlist)
	ON_LBN_SELCHANGE(IDC_ACTIONLIST, OnLbnSelchangeActionlist)
	ON_LBN_SELCHANGE(IDC_STATE_GROUP_LIST, OnLbnSelchangeStateGrouplist)
	ON_CBN_SELCHANGE(IDC_CURRENT_STATE_COMBO, OnCbnSelchangeCurrentStateCombo)
	ON_LBN_SELCHANGE(IDC_SWITCH_GROUP_LIST, OnLbnSelchangeSwitchGrouplist)
	ON_CBN_SELCHANGE(IDC_CURRENT_SWITCH_COMBO, OnCbnSelchangeCurrentSwitchCombo)
	ON_LBN_SELCHANGE(IDC_GAME_PARAMETER_LIST, OnLbnSelchangeGameParameterlist)
	ON_LBN_SELCHANGE(IDC_DIALOGUE_EVENT_LIST, OnLbnSelchangeDialogueEventlist)
	ON_LBN_SELCHANGE(IDC_ARGUMENT_LIST, OnLbnSelchangeArgumentlist)
	ON_LBN_SELCHANGE(IDC_TRIGGER_LIST, OnLbnSelchangeTriggerlist)
	ON_BN_CLICKED(IDC_PLAY, OnBnClickedPlay)
	ON_BN_CLICKED(IDC_STOP_ALL_BUTTON, OnBnClickedStopAll)
	ON_BN_CLICKED(IDC_GETALLEVENTS, OnBnClickedGetAllEvents)
	ON_BN_CLICKED(IDC_CLEAREVENTS, OnBnClickedClearEvents)
	ON_BN_CLICKED(IDC_GETALLSTATES, OnBnClickedGetAllStateGroups)
	ON_BN_CLICKED(IDC_CLEARSTATES, OnBnClickedClearStateGroups)
	ON_BN_CLICKED(IDC_GETALLSWITCHES, OnBnClickedGetAllSwitchGroups)
	ON_BN_CLICKED(IDC_CLEARSWITCHES, OnBnClickedClearSwitchGroups)
	ON_BN_CLICKED(IDC_SB_DEF_BROWSE_BUTTON, OnBnClickedSBDefBrowse)
	ON_BN_CLICKED(IDC_RESETGAMEPARAMETER, OnBnClickedResetGameParameter)
	ON_BN_CLICKED(IDC_GETALLGAMEPARAMETERS, OnBnClickedGetAllGameParameters)
	ON_BN_CLICKED(IDC_CLEARGAMEPARAMETERS, OnBnClickedClearGameParameters)
	ON_BN_CLICKED(IDC_GETALLTRIGGERS, OnBnClickedGetAllTriggers)
	ON_BN_CLICKED(IDC_CLEARTRIGGERS, OnBnClickedClearTriggers)
	ON_BN_CLICKED(IDC_POSTTRIGGER, OnBnClickedPostTrigger)
	ON_BN_CLICKED(IDC_GETALLSOUNDBANKS, OnBnClickedGetAllSoundBanks)
	ON_BN_CLICKED(IDC_CLEARSOUNDBANKS, OnBnClickedClearSoundBanks)
	ON_BN_CLICKED(IDC_GETALLDIALOGUEEVENTS, OnBnClickedGetAllDialogueEvents)
	ON_BN_CLICKED(IDC_CLEARDIALOGUEEVENTS, OnBnClickedClearDialogueEvents)	
	ON_BN_CLICKED(IDC_ADD_GAMEOBJECTS, OnBnClickedAddGameObject)
	ON_BN_CLICKED(IDC_REMOVE_GAMEOBJECTS, OnBnClickedRemoveGameObject)
	ON_BN_CLICKED(IDC_SB_DEF_FILE_PROCESS_BUTTON, OnBnClickedSBDefImport)
	ON_BN_CLICKED(IDC_GENERATE_SOUNDBANK_BUTTON, OnBnClickedGenerateSoundBank)
	ON_BN_CLICKED(IDC_PLAYBACK_MODE_CHECK, OnBnClickedPlayBackMode)
	ON_BN_CLICKED(IDC_EDIT_POSITION_BUTTON, OnBnClickedEditPosition)
	ON_BN_CLICKED(IDC_EDIT_ENVIRONMENT_BUTTON, OnBnClickedEditAuxBus)
	ON_BN_CLICKED(IDC_POST_MONITORING_MSG_BUTTON, OnBnClickedPostMonitoringMsg)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_CONVERT_EXTERNALSOURCES_BUTTON, &CSFTestDlg::OnBnClickedConvertExternalsourcesButton)
	ON_BN_CLICKED(IDC_ES_IN_DEF_BROWSE_BUTTON, &CSFTestDlg::OnBnClickedEsInDefBrowseButton)
	ON_BN_CLICKED(IDC_OUT_IN_DEF_BROWSE_BUTTON, &CSFTestDlg::OnBnClickedOutInDefBrowseButton)
	ON_BN_CLICKED(IDC_GETALLCONVERSIONSETTINGS, &CSFTestDlg::OnBnClickedGetallconversionsettings)
	ON_BN_CLICKED(IDC_CLEARCONVERSIONSETTINGS, &CSFTestDlg::OnBnClickedClearconversionsettings)
	ON_MESSAGE(WM_SF_SHOW, OnShowListItems)
END_MESSAGE_MAP()


// CSFTestDlg message handlers

BOOL CSFTestDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_ListenerName[0] = L"Listener0";
	m_ListenerName[1] = L"Listener1";
	m_ListenerName[2] = L"Listener2";
	m_ListenerName[3] = L"Listener3";
	m_ListenerName[4] = L"Listener4";
	m_ListenerName[5] = L"Listener5";
	m_ListenerName[6] = L"Listener6";
	m_ListenerName[7] = L"Listener7";

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	SoundFrame::Create( this, &m_pSoundFrame );
	_ASSERT( m_pSoundFrame );

	m_pSoundFrame->Connect();

	m_eventTarget.Register( &m_eventList );
	m_statesTarget.Register( &m_stateGroupList );
	m_switchesTarget.Register( &m_switchGroupList );
	m_gameParametersTarget.Register( &m_gameParameterList );
	m_triggersTarget.Register( &m_triggerList );
	m_dialogueEventTarget.Register( &m_dialogueEventList );
	m_soundBankTarget.Register( &m_soundBankList );

	m_positioningDialog.Init( m_pSoundFrame );
	m_AuxBusDialog.Init( m_pSoundFrame, &m_AuxBusTarget );

	m_gameObjectListCtrl.Init( &m_positioningDialog );
		
	LoadProject();

	GetGameObjectList();
	OnLbnSelchangeActionlist();
	OnLbnSelchangeGameParameterlist();

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CSFTestDlg::OnDestroy()
{
	m_eventTarget.Revoke();
	m_statesTarget.Revoke();
	m_switchesTarget.Revoke();
	m_gameParametersTarget.Revoke();
	m_triggersTarget.Revoke();
	m_dialogueEventTarget.Revoke();
	m_soundBankTarget.Revoke();

	SaveProject();

    ClearEventList();
	ClearDialogueEventList();
	ClearStateGroupList();
	ClearSwitchGroupList();
	ClearGameObjectList();
	ClearGameParameterList();
	ClearTriggerList();
	ClearArgumentList();
	ClearArgumentValueList();
	ClearEventSourcesValueList();
	ClearDialogueEventSourcesValueList();
	ClearConversionSettingsList();
	ClearSoundBankList();
	
	// Release the Sound Frame
	if ( m_pSoundFrame )
	{
		m_pSoundFrame->Release();
		m_pSoundFrame = NULL;
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSFTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CSFTestDlg::OnConnect( bool bConnect )
{
	GetDlgItem( IDC_CONNECT )->SetWindowText( bConnect ? _T( "Connected" ) : _T( "Disconnected" ) );

	SetProjectTitle( m_pSoundFrame->GetCurrentProjectName() );

	GetGameObjectList();

	for (AkUInt32 i = 0; i < CSFTestPositioningDlg::MAX_NUM_LISTENER_CHECKBOX; i++)
	{
		m_pSoundFrame->RegisterGameObject(i, m_ListenerName[i]);
	}

	m_pSoundFrame->SetListeners(IGameObject::s_WwiseGameObject, &IGameObject::s_WwiseGameObject, 1);
}

void CSFTestDlg::OnEventNotif( Notif in_eNotif, AkUniqueID in_eventID )
{
	if ( in_eNotif == Notif_Added ) // This app doesn't care about newly created events
		return;

	bool bHadEvent = false;
	bool bSelected = false;

	if ( in_eNotif == Notif_Removed || in_eNotif == Notif_Changed )
	{
		// Remove Event from list
		bHadEvent = m_eventList.RemoveObject( in_eventID, bSelected );
	}

	if ( in_eNotif == Notif_Push || ( bHadEvent && ( in_eNotif == Notif_Changed ) ) )
	{
		IEvent * pEvent = NULL;
		// Get the IEvent from its ID
		m_pSoundFrame->GetEvent( in_eventID, &pEvent );

		if ( pEvent )
		{
			int idx = m_eventList.AddObject( pEvent );

			if ( bSelected )
				m_eventList.SetCurSel( idx );

			// Release the event
			pEvent->Release();
		}
	}

	if( bHadEvent )
		OnLbnSelchangeEventlist();
}

void CSFTestDlg::OnDialogueEventNotif( Notif in_eNotif, AkUniqueID in_dialogueEventID )
{
	if ( in_eNotif == Notif_Added ) // This app doesn't care about newly created Dialogue Events
		return;

	bool bHadDialogueEvent = false;
	bool bSelected = false;

	if ( in_eNotif == Notif_Removed || in_eNotif == Notif_Changed )
	{
		// Remove Dialogue Event from list
		bHadDialogueEvent = m_dialogueEventList.RemoveObject( in_dialogueEventID, bSelected );
	}

	if ( bHadDialogueEvent && ( in_eNotif == Notif_Changed ) )
	{
		IDialogueEvent * pDialogueEvent = NULL;
		// Get the IDialogueEvent from its ID
		m_pSoundFrame->GetDialogueEvent( in_dialogueEventID, &pDialogueEvent );

		if ( pDialogueEvent )
		{
			int idx = m_dialogueEventList.AddObject( pDialogueEvent );

			if ( bSelected )
				m_dialogueEventList.SetCurSel( idx );

			// Release the Dialogue Event
			pDialogueEvent->Release();
		}
	}

	if( bHadDialogueEvent )
		OnLbnSelchangeDialogueEventlist();
}

void CSFTestDlg::OnSoundObjectNotif( Notif in_eNotif, AkUniqueID in_soundObjectID )
{
    if( in_soundObjectID == m_soundObjectDisplayed )
		OnLbnSelchangeActionlist();
}

void CSFTestDlg::OnStatesNotif( Notif in_eNotif, AkUniqueID in_stateGroupID	)
{
	if ( in_eNotif == Notif_Added ) // This app doesn't care about newly created State Group.
		return;

	bool bHadStateGroup = false;
	bool bSelected = false;

	if ( in_eNotif == Notif_Removed || in_eNotif == Notif_Changed )
	{
		// Remove IStateGroup from list
		bHadStateGroup = m_stateGroupList.RemoveObject( in_stateGroupID, bSelected );
	}

	if ( bHadStateGroup && ( in_eNotif == Notif_Changed ) )
	{
		IStateGroup * pStateGroup = NULL;
		// Get the State Group from its ID
		m_pSoundFrame->GetStateGroup( in_stateGroupID, &pStateGroup );

		if ( pStateGroup )
		{
			int idx = m_stateGroupList.AddObject( pStateGroup );

			if ( bSelected )
				m_stateGroupList.SetCurSel( idx );

			// Release the State Group
			pStateGroup->Release();
		}
	}

	if( bHadStateGroup )
		OnLbnSelchangeStateGrouplist();
}

void CSFTestDlg::OnSwitchesNotif( Notif in_eNotif, AkUniqueID in_switchGroupID )
{
	if ( in_eNotif == Notif_Added ) // This app doesn't care about newly created Switch Group.
		return;

	bool bHadSwitchGroup = false;
	bool bSelected = false;

	if ( in_eNotif == Notif_Removed || in_eNotif == Notif_Changed )
	{
		// Remove Switch Group from list
		bHadSwitchGroup = m_switchGroupList.RemoveObject( in_switchGroupID, bSelected );
	}

	if ( bHadSwitchGroup && ( in_eNotif == Notif_Changed ) )
	{
		ISwitchGroup * pSwitchGroup = NULL;
		// Get the ISwitchGroup from its ID
		m_pSoundFrame->GetSwitchGroup( in_switchGroupID, &pSwitchGroup );

		if ( pSwitchGroup )
		{
			int idx = m_switchGroupList.AddObject( pSwitchGroup );

			if ( bSelected )
				m_switchGroupList.SetCurSel( idx );

			// Release the Switch Group
			pSwitchGroup->Release();
		}
	}

	if( bHadSwitchGroup || in_eNotif == Notif_Reset )
		OnLbnSelchangeSwitchGrouplist();
}

void CSFTestDlg::OnGameObjectsNotif( Notif in_eNotif, AkGameObjectID in_gameObjectID )
{
	switch( in_eNotif )
	{
	case Notif_Removed:
		m_gameObjectListCtrl.RemoveObject( in_gameObjectID );
		break;
	case Notif_Added:
		GetGameObjectList();
		break;
	case Notif_Reset:
		ClearGameObjectList();
		break;
	};

	m_positioningDialog.UpdateGameObjectList();
	m_AuxBusDialog.UpdateGameObjectList();
}

void CSFTestDlg::OnGameParametersNotif( Notif in_eNotif, AkUniqueID in_gameParameterID )
{
	if ( in_eNotif == Notif_Added ) // This app doesn't care about newly created Game Parameter.
		return;

	bool bHadGameParameter = false;
	bool bSelected = false;

	if ( in_eNotif == Notif_Removed || in_eNotif == Notif_Changed )
	{
		// Remove Game Parameter from list
		bHadGameParameter = m_gameParameterList.RemoveObject( in_gameParameterID, bSelected );
	}

	if ( bHadGameParameter && ( in_eNotif == Notif_Changed ) )
	{
		IGameParameter * pGameParameter = NULL;
		// Get the IGameParameter from its ID
		m_pSoundFrame->GetGameParameter( in_gameParameterID, &pGameParameter );

		if ( pGameParameter )
		{
			int idx = m_gameParameterList.AddObject( pGameParameter );

			if ( bSelected )
				m_gameParameterList.SetCurSel( idx );

			// Release the Game Parameter
			pGameParameter->Release();
		}
	}

	if( bHadGameParameter )
		OnLbnSelchangeGameParameterlist();
}

void CSFTestDlg::OnTriggersNotif( Notif in_eNotif, AkUniqueID in_triggerID )
{
	if ( in_eNotif == Notif_Added ) // This app doesn't care about newly created Trigger.
		return;

	bool bHadGameParameter = false;
	bool bSelected = false;

	if ( in_eNotif == Notif_Removed || in_eNotif == Notif_Changed )
	{
		// Remove Trigger from list
		bHadGameParameter = m_triggerList.RemoveObject( in_triggerID, bSelected );
	}

	if ( bHadGameParameter && ( in_eNotif == Notif_Changed ) )
	{
		ITrigger * pTrigger = NULL;
		// Get the ITrigger from its ID
		m_pSoundFrame->GetTrigger( in_triggerID, &pTrigger );

		if ( pTrigger )
		{
			int idx = m_triggerList.AddObject( pTrigger );

			if ( bSelected )
				m_triggerList.SetCurSel( idx );

			// Release the Trigger
			pTrigger->Release();
		}
	}
}

void CSFTestDlg::OnArgumentsNotif( Notif in_eNotif, AkUniqueID in_argumentID )
{
	// If a new argument was added, it is surely not referenced by the Dialogue Event that is currently
	// selected in the list. When that argument will be used by a Dialogue Event, we will receive a 
	// Notif_Changed notification for that Dialogue Event.
	if ( in_eNotif == Notif_Added )
		return;

	bool bSelected = false;
	bool bHadArgument = false;

	if ( in_eNotif == Notif_Removed || in_eNotif == Notif_Changed )
	{
		// Remove Game Parameter from list
		bHadArgument = m_argumentList.RemoveObject( in_argumentID, bSelected );
	}

	if ( bHadArgument && ( in_eNotif == Notif_Changed ) )
	{
		IArgument * pArgument = NULL;
		m_pSoundFrame->GetArgument( in_argumentID, &pArgument );

		if ( pArgument )
		{
			int idx = m_argumentList.AddObject( pArgument );

			if ( bSelected )
				m_argumentList.SetCurSel( idx );

			// Release the Argument
			pArgument->Release();
		}
	}

	if( bHadArgument )
		OnLbnSelchangeArgumentlist();
}

void CSFTestDlg::OnAuxBusNotif( Notif in_eNotif, AkUniqueID in_AuxBusID )
{
	m_AuxBusDialog.OnAuxBusNotif( in_eNotif, in_AuxBusID );
}

void CSFTestDlg::OnSoundBankNotif( Notif in_eNotif, AkUniqueID in_objID )
{
	if ( in_eNotif == Notif_Added ) // This app doesn't care about newly created sound banks.
		return;

	bool bHadObj = false;
	bool bSelected = false;

	if ( in_eNotif == Notif_Removed || in_eNotif == Notif_Changed )
	{
		// Remove object from list
		bHadObj = m_soundBankList.RemoveObject( in_objID, bSelected );
	}

	if ( bHadObj && ( in_eNotif == Notif_Changed ) )
	{
		ISoundBank * pObj = NULL;
		// Get the ITrigger from its ID
		m_pSoundFrame->GetSoundBank( in_objID, &pObj );

		if ( pObj )
		{
			int idx = m_soundBankList.AddObject( pObj );

			if ( bSelected )
				m_soundBankList.SetCurSel( idx );

			// Release the Trigger
			pObj->Release();
		}
	}
}

ISoundFrame * CSFTestDlg::GetSoundFrame()
{
	 return m_pSoundFrame;
}

void CSFTestDlg::AddEventsToList( IEventList * in_pEventList )
{
	m_eventList.AddObjects( in_pEventList );
}

void CSFTestDlg::AddStateGroupToList( IStateGroupList * in_pStateGroupList )
{
	m_stateGroupList.AddObjects( in_pStateGroupList );
}

void CSFTestDlg::AddSwitchGroupToList( ISwitchGroupList * in_pSwitchGroupList )
{
	m_switchGroupList.AddObjects( in_pSwitchGroupList );
}

void CSFTestDlg::AddGameParameterToList( IGameParameterList * in_pGameParameterList )
{
	m_gameParameterList.AddObjects( in_pGameParameterList );
}

void CSFTestDlg::AddConversionSettingsToList( IConversionSettingsList * in_pConversionSettingsList )
{
	m_conversionSettingsList.AddObjects( in_pConversionSettingsList );
}

void CSFTestDlg::AddTriggerToList( ITriggerList * in_pTriggerList )
{
	m_triggerList.AddObjects( in_pTriggerList );
}

void CSFTestDlg::AddAuxBusToList( IAuxBusList * in_pAuxBusList )
{
	m_AuxBusDialog.AddAuxBusToList( in_pAuxBusList );
}

void CSFTestDlg::AddDialogueEventToList( IDialogueEventList * in_pDialogueEventList )
{
	m_dialogueEventList.AddObjects( in_pDialogueEventList );
}

void CSFTestDlg::AddSoundBankToList( ISoundBankList * in_pSoundBankList )
{
	m_soundBankList.AddObjects( in_pSoundBankList );
}

void CSFTestDlg::OnLbnSelchangeEventlist()
{
	ClearActionList();

	int iSel = m_eventList.GetCurSel();
	if ( iSel == LB_ERR )
		return;

	IEvent * pEvent = m_eventList.GetObject(iSel);

	// Show event's actions in action list.

    IActionList * pActionList = pEvent->GetActionList();
	if ( pActionList )
	{
		m_actionList.AddObjects( pActionList );
		m_actionList.SetCurSel( 0 );
	}

    // Original list
	ClearEventSourcesValueList();
	IOriginalFileList * pOriginalList = NULL;
	if ( m_pSoundFrame->GetEventOriginalFileList(pEvent->GetName(), &pOriginalList ) && pOriginalList )
	{
		m_eventSourcesValueList.AddObjects( pOriginalList );
		pOriginalList->Release();
	}

	OnLbnSelchangeActionlist();

	UpdateExplorerPath(pEvent, NULL);
}

void CSFTestDlg::OnLbnSelchangeActionlist()
{
	AkUniqueID oldSoundObject = m_soundObjectDisplayed;

	ISoundObject * pSoundObject = NULL;
	m_soundObjectDisplayed = AK_INVALID_UNIQUE_ID;

	int iSel = m_actionList.GetCurSel();
	if ( iSel != LB_ERR )
	{
		IAction * pAction = m_actionList.GetObject(iSel);

		AkUniqueID actionSoundObjectID = pAction->GetSoundObjectID();

		// Get the ISoundObject if there one associated with the action.
		if( actionSoundObjectID != AK_INVALID_UNIQUE_ID )
			m_pSoundFrame->GetSoundObject( actionSoundObjectID, &pSoundObject );
	}

	CString csMax;
	if( pSoundObject )
	{
		m_soundObjectDisplayed = pSoundObject->GetID();

		csMax.Format( _T("%.2f"), pSoundObject->AttenuationMaxDistance() );
	}

	SetDlgItemText( IDC_SOUND_NAME_STATIC, pSoundObject? pSoundObject->GetName() : _T("") );
	SetDlgItemText( IDC_HAS_ATTENUATION_STATIC, pSoundObject? ( pSoundObject->HasAttenuation()? _T("Yes") : _T("No") ) : _T("") );
	SetDlgItemText( IDC_ATTENUATION_MAX_STATIC, csMax );

	if( m_soundObjectDisplayed != oldSoundObject )
	{
		// Stop listening for attenuation change on the Old Sound Object
		m_pSoundFrame->ListenAttenuation( NULL, 0 );

		// If we have a new Sound object listen for its radius changes
		if( m_soundObjectDisplayed != AK_INVALID_UNIQUE_ID )
			m_pSoundFrame->ListenAttenuation( &m_soundObjectDisplayed, 1 );
	}

	// release the Sound Object
	if( pSoundObject )
		pSoundObject->Release();
}

void CSFTestDlg::OnLbnSelchangeStateGrouplist()
{
	m_currentStateCombo.ClearCombo();

	int iSel = m_stateGroupList.GetCurSel();
	if ( iSel == LB_ERR )
		return;

	IStateGroup * pStateGroup = m_stateGroupList.GetObject(iSel);

	// Show States in the Combo box.

    IStateList * pStateList = pStateGroup->GetStateList();
	if ( pStateList )
	{
		m_currentStateCombo.AddObjects( pStateList );

		IState* pState = NULL;
		// Get the Current State for this State Group

		if( UseString() )
			m_pSoundFrame->GetCurrentState( pStateGroup->GetName(), &pState );
		else
			m_pSoundFrame->GetCurrentState( pStateGroup->GetID(), &pState );

		if( pState )
			m_currentStateCombo.SelectObject( pState->GetID() );
		else
			m_currentStateCombo.SetCurSel( 0 );

		if( pState )
			pState->Release();

		UpdateExplorerPath(pStateGroup, NULL);
	}	
}

void CSFTestDlg::OnCbnSelchangeCurrentStateCombo()
{
	int iSGSel = m_stateGroupList.GetCurSel();
	int iStateSel = m_currentStateCombo.GetCurSel();
	if ( iSGSel == LB_ERR || iStateSel == CB_ERR )
		return;

	IStateGroup* pStateGroup = m_stateGroupList.GetObject( iSGSel );
	IState* pState = m_currentStateCombo.GetObject( iStateSel );

	// Change the current State for this State Group
	if( pStateGroup && pState )
	{
		if( UseString() )
			m_pSoundFrame->SetCurrentState( pStateGroup->GetName(), pState->GetName() );
		else
			m_pSoundFrame->SetCurrentState( pStateGroup->GetID(), pState->GetID() );

		UpdateExplorerPath(pState, pStateGroup);
	}
}

void CSFTestDlg::OnLbnSelchangeSwitchGrouplist()
{
	m_currentSwitchCombo.ClearCombo();

	int iSel = m_switchGroupList.GetCurSel();
	if ( iSel == LB_ERR )
		return;

	ISwitchGroup * pSwitchGroup = m_switchGroupList.GetObject(iSel);

	// Show Switches in the Combo box.

    ISwitchList * pSwitchList = pSwitchGroup->GetSwitchList();
	if ( pSwitchList )
	{
		m_currentSwitchCombo.AddObjects( pSwitchList );

		ISwitch* pSwitch = NULL;
		// Get the Current Switch on the selected Game Object
		if( UseString() )
			m_pSoundFrame->GetCurrentSwitch( pSwitchGroup->GetName(), &pSwitch, GetCurrentGameObject() );
		else
			m_pSoundFrame->GetCurrentSwitch( pSwitchGroup->GetID(), &pSwitch, GetCurrentGameObject() );

		if( pSwitch )
			m_currentSwitchCombo.SelectObject( pSwitch->GetID() );
		else
			m_currentSwitchCombo.SetCurSel( 0 );

		if( pSwitch )
			pSwitch->Release();

		UpdateExplorerPath(pSwitchGroup, NULL);
	}
}

void CSFTestDlg::OnCbnSelchangeCurrentSwitchCombo()
{
	int iSGSel = m_switchGroupList.GetCurSel();
	int iSwitchSel = m_currentSwitchCombo.GetCurSel();
	if ( iSGSel == LB_ERR || iSwitchSel == CB_ERR )
		return;

	ISwitchGroup* pSwitchGroup = m_switchGroupList.GetObject( iSGSel );
	ISwitch* pSwitch = m_currentSwitchCombo.GetObject( iSwitchSel );

	// Change the current Switch for the selected Game object
	if( pSwitchGroup && pSwitch )
	{
		if( UseString() )
			m_pSoundFrame->SetCurrentSwitch( pSwitchGroup->GetName(), pSwitch->GetName(), GetCurrentGameObject() );
		else
			m_pSoundFrame->SetCurrentSwitch( pSwitchGroup->GetID(), pSwitch->GetID(), GetCurrentGameObject() );

		UpdateExplorerPath(pSwitch, pSwitchGroup);
	}
}

void CSFTestDlg::OnLvnItemChangedGameObjectList(NMHDR* pNMHDR, LRESULT* pResult)
{
	// Update the current switch for the selected game object
	OnLbnSelchangeSwitchGrouplist();

	// Update the Game Parameter Value
	OnLbnSelchangeGameParameterlist();
}

void CSFTestDlg::OnLbnSelchangeGameParameterlist()
{
	int iSel = m_gameParameterList.GetCurSel();
	if ( iSel != LB_ERR )
	{
		IGameParameter * pGameParameter = m_gameParameterList.GetObject(iSel);

		CString csMin, csMax, csValue;
		csMin.Format( _T("%.0f"), pGameParameter->RangeMin() );
		csMax.Format( _T("%.0f"), pGameParameter->RangeMax() );

		SetDlgItemText( IDC_GAME_PARAMETER_MIN_STATIC, csMin );
		SetDlgItemText( IDC_GAME_PARAMETER_MAX_STATIC, csMax );

		int iSliderPos = m_gameParameterSlider.GetPos();

		// Look in our maps to find the stored Game parameter value on the current Game object
		GameObjectToRTPCValueMap::iterator it = m_gameParametersValuesMap[pGameParameter->GetID()].find( GetCurrentGameObject() );
		GameObjectToRTPCValueMap::iterator itEnd = m_gameParametersValuesMap[pGameParameter->GetID()].end();
		if( it != itEnd )
			iSliderPos = (int)it->second;
		else
			iSliderPos = (int)pGameParameter->Default();

		m_gameParameterSlider.SetRange( (int)pGameParameter->RangeMin(), (int)pGameParameter->RangeMax(), TRUE );
		m_gameParameterSlider.SetPos( iSliderPos );
		m_gameParameterSlider.EnableWindow( TRUE );

		csValue.Format( _T("%d"), m_gameParameterSlider.GetPos() );
		SetDlgItemText( IDC_GAME_PARAMETER_VALUE_STATIC, csValue );

		// update the map value in case the value was clamped by the slider
		if( it != itEnd )
			it->second = m_gameParameterSlider.GetPos();

		UpdateExplorerPath(pGameParameter, NULL);
	}
	else
	{
		SetDlgItemText( IDC_GAME_PARAMETER_MIN_STATIC, _T("") );
		SetDlgItemText( IDC_GAME_PARAMETER_MAX_STATIC, _T("") );
		SetDlgItemText( IDC_GAME_PARAMETER_VALUE_STATIC, _T("") );

		m_gameParameterSlider.EnableWindow( FALSE );
	}
}

void CSFTestDlg::OnLbnSelchangeDialogueEventlist()
{
	ClearArgumentList();

	IDialogueEvent * pDialogueEvent = GetSelectedDialogueEvent();
	if ( pDialogueEvent == NULL )
		return;
	
	// Show dialog event's arguments in argument list.

    IArgumentList * pArgumentList = pDialogueEvent->GetArgumentList();
	if ( pArgumentList )
	{
		m_argumentList.AddObjects( pArgumentList );
		m_argumentList.SetCurSel( 0 );
	}

    // Original list
	ClearDialogueEventSourcesValueList();
	IOriginalFileList * pOriginalList = NULL;
	if ( m_pSoundFrame->GetDialogueEventOriginalFileList(pDialogueEvent->GetName(), &pOriginalList ) && pOriginalList )
	{
		m_dialogueEventSourcesValueList.AddObjects( pOriginalList );
		pOriginalList->Release();
	}

	OnLbnSelchangeArgumentlist();

	UpdateExplorerPath(pDialogueEvent, NULL);
}

void CSFTestDlg::OnLbnSelchangeArgumentlist()
{
	ClearArgumentValueList();

	int iSel = m_argumentList.GetCurSel();
	if ( iSel == LB_ERR )
		return;

	IArgument * pArgument = m_argumentList.GetObject(iSel);

	// Show dialog event's arguments in argument list.

    IArgumentValueList * pArgumentValueList = pArgument->GetArgumentValueList();
	if ( pArgumentValueList )
	{
		m_argumentValueList.AddObjects( pArgumentValueList );
	}

	UpdateExplorerPath(pArgument, NULL);
}

void CSFTestDlg::OnBnClickedPlay()
{
	int iSel = m_eventList.GetCurSel();
	if ( iSel == LB_ERR )
		return;

	IEvent * pEvent = m_eventList.GetObject( iSel );

	AkGameObjectID iGameObjectID = GetCurrentGameObject();

	// Launch a play for the selected Event on the current Game Object
	if( UseString() )
	{
		CString csEventName = pEvent->GetName();
		LPCWSTR strEventName = csEventName.GetBuffer();

		m_pSoundFrame->PlayEvents( &strEventName, 1, iGameObjectID );

		csEventName.ReleaseBuffer();
	}
	else
	{
		AkUniqueID eventID = pEvent->GetID();
		m_pSoundFrame->PlayEvents( &eventID, 1, iGameObjectID );
	}
}

void CSFTestDlg::OnBnClickedStopAll()
{
	CheckDlgButton(IDC_PLAYBACK_MODE_CHECK, BST_UNCHECKED);
	// Set the playback mode to false to stop all playing events
	m_pSoundFrame->SetPlayBackMode( false );
}

void CSFTestDlg::OnBnClickedGetAllEvents()
{
	GetEventList();
}

void CSFTestDlg::OnBnClickedClearEvents()
{
	ClearEventList();
}

void CSFTestDlg::OnBnClickedGetAllDialogueEvents()
{
	GetDialogueEventList();
}

void CSFTestDlg::OnBnClickedClearDialogueEvents()
{
	ClearDialogueEventList();
}

void CSFTestDlg::OnBnClickedGetAllStateGroups()
{
	GetStateGroupList();
}

void CSFTestDlg::OnBnClickedClearStateGroups()
{
	ClearStateGroupList();
}

void CSFTestDlg::OnBnClickedGetAllSwitchGroups()
{
	GetSwitchGroupList();
}

void CSFTestDlg::OnBnClickedClearSwitchGroups()
{
	ClearSwitchGroupList();
}

void CSFTestDlg::OnBnClickedResetGameParameter()
{
	ResetGameParameter();
}

void CSFTestDlg::OnBnClickedGetAllGameParameters()
{
	GetGameParameterList();
}

void CSFTestDlg::OnBnClickedClearGameParameters()
{
	ClearGameParameterList();
}

void CSFTestDlg::OnBnClickedGetallconversionsettings()
{
	GetConversionSettingsList();
}

void CSFTestDlg::OnBnClickedClearconversionsettings()
{
	ClearConversionSettingsList();
}

void CSFTestDlg::OnBnClickedGetAllTriggers()
{
	GetTriggerList();
}

void CSFTestDlg::OnBnClickedClearTriggers()
{
	ClearTriggerList();
}

void CSFTestDlg::OnBnClickedPostTrigger()
{
	int iSel = m_triggerList.GetCurSel();
	if ( iSel == LB_ERR )
		return;

	ITrigger * pTrigger = m_triggerList.GetObject( iSel );

	// Post the trigger on the selected Game Object
	if( UseString() )
		m_pSoundFrame->PostTrigger( pTrigger->GetName(), GetCurrentGameObject() );
	else
		m_pSoundFrame->PostTrigger( pTrigger->GetID(), GetCurrentGameObject() );
}

void CSFTestDlg::OnBnClickedGetAllSoundBanks()
{
	GetSoundBankList();
}

void CSFTestDlg::OnBnClickedClearSoundBanks()
{
	ClearSoundBankList();
}

void CSFTestDlg::OnBnClickedAddGameObject()
{
	SFAddGameObjectDialog dialog;
	if( dialog.DoModal() == IDOK )
	{
		AkGameObjectID uGameObjectID = dialog.GetGameObjectID();

		// Register a new Game object
		m_pSoundFrame->RegisterGameObject( dialog.GetGameObjectID(), dialog.GetGameObjectName() );

		// No need to update the UI the notification handler will do it

		AkGameObjectID listenerID = 0;
		m_pSoundFrame->SetListeners(uGameObjectID, &listenerID, 1);
	}
}

void CSFTestDlg::OnBnClickedRemoveGameObject()
{
	IGameObject* pSelectedGameObject = m_gameObjectListCtrl.GetSelectedObject();
	// Unregister the selected Game Object
	if( pSelectedGameObject )
		m_pSoundFrame->UnregisterGameObject( pSelectedGameObject->GetID() );

	// No need to update the UI the notification handler will do it
}

void CSFTestDlg::OnBnClickedSBDefBrowse()
{
	CFileDialog fileDialog( TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR, 
								_T("Text Files (*.txt)|*.txt|All Files (*.*)|*.*||") );

	if( fileDialog.DoModal() == IDOK )
		SetDlgItemText( IDC_SB_DEF_FILE_EDIT, fileDialog.GetPathName() );
}

void CSFTestDlg::OnBnClickedSBDefImport()
{
	WCHAR szDefFileBuffer[MAX_PATH] = {0};
	LPCWSTR strDefFile = szDefFileBuffer;

	// Import the Sound Bank definition file
	if(	GetDlgItemText( IDC_SB_DEF_FILE_EDIT, szDefFileBuffer, MAX_PATH ) != 0 )
		m_pSoundFrame->ProcessDefinitionFiles( &strDefFile, 1 );
}

void CSFTestDlg::OnBnClickedGenerateSoundBank()
{
	WCHAR szBankNameBuffer[MAX_PATH] = {0};
	LPCWSTR strBankNameList[MAX_PATH]; 
	LPCWSTR strPlatform = _T("Windows");
	LPCWSTR strLanguage = _T("English(US)");

	GetDlgItemText( IDC_SOUNDBANK_NAME_EDIT, szBankNameBuffer, MAX_PATH );

	WCHAR* strBankNames;
	WCHAR token[] = L" ";
	WCHAR *remainingBankNames = NULL;

	strBankNames = wcstok_s(szBankNameBuffer, token, &remainingBankNames);
	int bankCount = 0;
	while (strBankNames != NULL)
	{
		strBankNameList[bankCount] = strBankNames;
		bankCount++;

		strBankNames = wcstok_s(NULL, token, &remainingBankNames);
	}

	// Generate the Sound bank in Wwise
	bool bResult = m_pSoundFrame->GenerateSoundBanks(strBankNameList, bankCount, &strPlatform, 1, &strLanguage, 1);
	if ( !bResult )
		MessageBox( L"SoundBank Generation Failed." );
}

void CSFTestDlg::OnBnClickedEsInDefBrowseButton()
{
	CFileDialog fileDialog( TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR, 
								_T("Text Files (*.wsources)|*.wsources|All Files (*.*)|*.*||") );

	if( fileDialog.DoModal() == IDOK )
		SetDlgItemText( IDC_ES_IN_DEF_FILE_EDIT, fileDialog.GetPathName() );
}

void CSFTestDlg::OnBnClickedOutInDefBrowseButton()
{
	CFileDialog folderDialog( TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR, 
								_T("All Files (*.*)|*.*||") );

	if( folderDialog.DoModal() == IDOK )
		SetDlgItemText( IDC_ES_OUT_DEF_FILE_EDIT, folderDialog.GetFolderPath() );
}


void CSFTestDlg::OnBnClickedConvertExternalsourcesButton()
{
	WCHAR szExternalSourceInputNameBuffer[MAX_PATH] = {0};
	WCHAR szExternalSourceOutputNameBuffer[MAX_PATH] = {0};

	LPCWSTR strPlatform = _T("Windows");
	LPCWSTR strExternalSourceInputFile = szExternalSourceInputNameBuffer;
	LPCWSTR strExternalSourceOutputFile = szExternalSourceOutputNameBuffer;

	int cSourceInputFile = GetDlgItemText( IDC_ES_IN_DEF_FILE_EDIT, szExternalSourceInputNameBuffer, MAX_PATH ) > 0 ? 1 : 0; // Is there an item or not?
	int cSourceOutputFile = GetDlgItemText( IDC_ES_OUT_DEF_FILE_EDIT, szExternalSourceOutputNameBuffer, MAX_PATH ) > 0 ? 1 : 0; // Is there an item or not?
	
	// Generate the Sound bank in Wwise
	bool bResult = m_pSoundFrame->ConvertExternalSources( &strPlatform, 1,
										   &strExternalSourceInputFile, cSourceInputFile,
										   &strExternalSourceOutputFile, cSourceOutputFile);
	if ( !bResult )
		MessageBox( L"External Sources Conversion Failed." );
	
}

void CSFTestDlg::OnBnClickedPlayBackMode()
{
	// Change the playback mode in Wwise. Settings the playback mode to false will stop all playing events.
	m_pSoundFrame->SetPlayBackMode( IsDlgButtonChecked(IDC_PLAYBACK_MODE_CHECK)? true : false );
}

void CSFTestDlg::OnBnClickedEditPosition()
{
	m_positioningDialog.DoModal();

	GetGameObjectList();
}

void CSFTestDlg::OnBnClickedEditAuxBus()
{
	m_AuxBusDialog.DoModal();
}

void CSFTestDlg::OnBnClickedPostMonitoringMsg()
{
	CString csMessage;
	GetDlgItemText( IDC_POST_MONITORING_MSG_EDIT, csMessage );

	// Post a monitoring message in Wwise.
	if( ! csMessage.IsEmpty() )
		m_pSoundFrame->PostMsgMonitor( csMessage );
}

void CSFTestDlg::OnHScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar )
{
	if( (CSliderCtrl*)pScrollBar == &m_gameParameterSlider )
	{
		IGameParameter* pGameParameter = m_gameParameterList.GetObject( m_gameParameterList.GetCurSel() );

		if( pGameParameter )
		{
			AkGameObjectID gameObjectID = GetCurrentGameObject( IGameObject::s_InvalidGameObject );
			// Set the Game parameter value on the current Game object
			if( UseString() )
				m_pSoundFrame->SetRTPCValue( pGameParameter->GetName(), (AkRtpcValue)m_gameParameterSlider.GetPos(), gameObjectID );
			else
				m_pSoundFrame->SetRTPCValue( pGameParameter->GetID(), (AkRtpcValue)m_gameParameterSlider.GetPos(), gameObjectID );

			// Cache the value we just in our map, since we can't get it for the Sound Frame
			m_gameParametersValuesMap[pGameParameter->GetID()][gameObjectID] = (double)m_gameParameterSlider.GetPos();
		}

		CString csValue;
		csValue.Format( _T("%d"), m_gameParameterSlider.GetPos() );
		SetDlgItemText( IDC_GAME_PARAMETER_VALUE_STATIC, csValue );
	}
}

LRESULT CSFTestDlg::OnShowListItems( WPARAM in_wParam, LPARAM in_lParam )
{
	std::vector<GUID> * pGuids = (std::vector<GUID> *) in_lParam;
	m_pSoundFrame->ShowWwiseObject( &pGuids->front(), pGuids->size(), (AK::SoundFrame::ISoundFrame::ShowLocation) in_wParam );
	
	return TRUE;
}

void CSFTestDlg::ResetGameParameter()
{
	IGameParameter* pGameParameter = m_gameParameterList.GetObject( m_gameParameterList.GetCurSel() );
	if( pGameParameter )
	{
		AkGameObjectID gameObjectID = GetCurrentGameObject( IGameObject::s_InvalidGameObject );

		// Set the Game parameter value on the current Game object
		if( UseString() )
			m_pSoundFrame->ResetRTPCValue( pGameParameter->GetName(), gameObjectID );
		else
			m_pSoundFrame->ResetRTPCValue( pGameParameter->GetID(), gameObjectID );

		int iSliderPos = (int) pGameParameter->Default();
		m_gameParametersValuesMap[pGameParameter->GetID()][gameObjectID] = iSliderPos;
		m_gameParameterSlider.SetPos( iSliderPos );

		CString csValue;
		csValue.Format( _T("%d"), iSliderPos );
		SetDlgItemText( IDC_GAME_PARAMETER_VALUE_STATIC, csValue );
	}
}

void CSFTestDlg::GetEventList()
{
	ClearEventList();

	IEventList * pEventList = NULL;
	// Get the list of all Events created in the current Wwise project.
	if ( m_pSoundFrame->GetEventList( &pEventList ) )
	{
		AddEventsToList( pEventList );
		pEventList->Release();
	}
}

void CSFTestDlg::GetDialogueEventList()
{
	ClearDialogueEventList();

	IDialogueEventList * pDialogueEventList = NULL;
	// Get the list of all Trigger created in the current Wwise project.
	if ( m_pSoundFrame->GetDialogueEventList( &pDialogueEventList ) )
	{
		AddDialogueEventToList( pDialogueEventList );
		pDialogueEventList->Release();
	}
}

void CSFTestDlg::GetStateGroupList()
{
	ClearStateGroupList();

	IStateGroupList * pStateGroupList = NULL;
	// Get the list of all State Groups created in the current Wwise project.
	if ( m_pSoundFrame->GetStateGroupList( &pStateGroupList ) )
	{
		AddStateGroupToList( pStateGroupList );
		pStateGroupList->Release();
	}
}

void CSFTestDlg::GetSwitchGroupList()
{
	ClearSwitchGroupList();

	ISwitchGroupList * pSwitchGroupList = NULL;
	// Get the list of all Switch Groups created in the current Wwise project.
	if ( m_pSoundFrame->GetSwitchGroupList( &pSwitchGroupList ) )
	{
		AddSwitchGroupToList( pSwitchGroupList );
		pSwitchGroupList->Release();
	}
}

void CSFTestDlg::GetGameObjectList()
{
	m_gameObjectListCtrl.ClearList();

	m_gameObjectListCtrl.AddGlobalObject();
	IGameObjectList * pGameObjectList = NULL;
	// Get the list of all Game objects created in the current Wwise project.
	if ( m_pSoundFrame->GetGameObjectList( &pGameObjectList ) )
	{
		m_gameObjectListCtrl.AddObjects( pGameObjectList );
		pGameObjectList->Release();
	}

	OnLvnItemChangedGameObjectList( 0, 0 );
}

void CSFTestDlg::GetGameParameterList()
{
	ClearGameParameterList();

	IGameParameterList * pGameParameterList = NULL;
	// Get the list of all Game Parameters created in the current Wwise project.
	if ( m_pSoundFrame->GetGameParameterList( &pGameParameterList ) )
	{
		AddGameParameterToList( pGameParameterList );
		pGameParameterList->Release();
	}
}

void CSFTestDlg::GetConversionSettingsList()
{
	ClearConversionSettingsList();

	IConversionSettingsList * pConversionSettingsList = NULL;
	// Get the list of all Game Parameters created in the current Wwise project.
	if ( m_pSoundFrame->GetConversionSettingsList( &pConversionSettingsList ) )
	{
		AddConversionSettingsToList( pConversionSettingsList );
		pConversionSettingsList->Release();
	}
}

void CSFTestDlg::GetSoundBankList()
{
	ClearSoundBankList();

	ISoundBankList * pSoundBankList = NULL;
	if ( m_pSoundFrame->GetSoundBankList( &pSoundBankList ) )
	{
		AddSoundBankToList( pSoundBankList );
		pSoundBankList->Release();
	}
}

void CSFTestDlg::GetTriggerList()
{
	ClearTriggerList();

	ITriggerList * pTriggerList = NULL;
	// Get the list of all Trigger created in the current Wwise project.
	if ( m_pSoundFrame->GetTriggerList( &pTriggerList ) )
	{
		AddTriggerToList( pTriggerList );
		pTriggerList->Release();
	}
}

void CSFTestDlg::ClearEventList()
{
	ClearActionList();

	m_eventList.ClearList();
}

void CSFTestDlg::ClearDialogueEventList()
{
	ClearArgumentList();

	m_dialogueEventList.ClearList();
}

void CSFTestDlg::ClearActionList()
{
	m_actionList.ClearList();

	OnLbnSelchangeActionlist();
}

void CSFTestDlg::ClearStateGroupList()
{
	m_currentStateCombo.ClearCombo();

	m_stateGroupList.ClearList();
}

void CSFTestDlg::ClearSwitchGroupList()
{
	m_currentSwitchCombo.ClearCombo();

	m_switchGroupList.ClearList();
}

void CSFTestDlg::ClearGameObjectList()
{
	m_gameObjectListCtrl.ClearList();

	OnLvnItemChangedGameObjectList( 0, 0 );
}

void CSFTestDlg::ClearGameParameterList()
{
	m_gameParameterList.ClearList();

	OnLbnSelchangeGameParameterlist();
}

void CSFTestDlg::ClearConversionSettingsList()
{
	m_conversionSettingsList.ClearList();
	// OnLbnSelConversionSettingslist();
}

void CSFTestDlg::ClearTriggerList()
{
	m_triggerList.ClearList();
}

void CSFTestDlg::ClearArgumentList()
{
	ClearArgumentValueList();

	m_argumentList.ClearList();
}

void CSFTestDlg::ClearArgumentValueList()
{
	m_argumentValueList.ClearList();
}

void CSFTestDlg::ClearEventSourcesValueList()
{
	m_eventSourcesValueList.ClearList();
}

void CSFTestDlg::ClearDialogueEventSourcesValueList()
{
	m_dialogueEventSourcesValueList.ClearList();
}

void CSFTestDlg::ClearSoundBankList()
{
	m_soundBankList.ClearList();
}

void CSFTestDlg::SetProjectTitle( const WCHAR * in_szNewProject )
{
	CString csProject = _T("Project: ");

	if ( in_szNewProject && wcslen( in_szNewProject ) )
		csProject += in_szNewProject;
	else
		csProject += _T("None");

	GetDlgItem( IDC_PROJECTNAME )->SetWindowText( csProject );
}

void CSFTestDlg::LoadProject()
{
	FILE * file = fopen( "SFTest.dat", "rb" );
	if ( !file )
		return;

	// This an example of how you can load Sound Frame objects from a file.

	ReadBytesOnFILE bytes( file );

	long cEvents = bytes.Read<long>();
	for ( long i = 0; i < cEvents; i++ )
	{
		IEvent * pEvent = IEvent::From( &bytes );
		if ( !pEvent ) 
			break; // couldn't load an event -- abort

		m_eventList.AddObject( pEvent );

		pEvent->Release();
	}

	fclose( file );
}

void CSFTestDlg::SaveProject()
{
	FILE * file = fopen( "SFTest.dat", "wb" );
	if ( !file )
		return;

	// This an example of how you can save Sound Frame objects in a file.

	WriteBytesOnFILE bytes( file );

	long cEvents = m_eventList.GetCount();
	bytes.Write<long>( cEvents );
	for ( long i = 0; i < cEvents; i++ )
	{
		IEvent * pEvent = m_eventList.GetObject( i );
		pEvent->To( &bytes );
	}

	fclose( file );
}

bool CSFTestDlg::UseString()
{
	return IsDlgButtonChecked( IDC_STRING_MODE_CHECK ) == BST_CHECKED;
}

AkGameObjectID CSFTestDlg::GetCurrentGameObject( AkGameObjectID in_defaultGameObject )
{
	// Return the selected Game Object in the list or the global Game object if nothing is selected.
	AkGameObjectID iGameObjectID = in_defaultGameObject;

	IGameObject* pGameObject = m_gameObjectListCtrl.GetSelectedObject();
	if( pGameObject )
		iGameObjectID = pGameObject->GetID();

	return iGameObjectID;
}

IDialogueEvent * CSFTestDlg::GetSelectedDialogueEvent()
{
	int iSel = m_dialogueEventList.GetCurSel();
	if ( iSel == LB_ERR )
		return NULL;

	return m_dialogueEventList.GetObject(iSel);
}

void CSFTestDlg::UpdateExplorerPath( ISFObject *in_pObject, ISFObject* in_pParent )
{
	WCHAR szPath[1024];
	m_pSoundFrame->GetWwiseObjectPath(in_pObject, in_pParent, szPath, 1024);

	SetDlgItemText(IDC_EXPLORER_PATH, szPath);
}

void CSFTestDlg::OnLbnSelchangeTriggerlist()
{
	int iSel = m_triggerList.GetCurSel();
	if ( iSel != LB_ERR )
		UpdateExplorerPath(m_triggerList.GetObject(iSel), NULL);
}
