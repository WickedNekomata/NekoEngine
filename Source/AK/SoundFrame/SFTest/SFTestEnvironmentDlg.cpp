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
#include "SFTestEnvironmentDlg.h"

#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CSFTestAuxBusDlg::ObstructionOcclusionLevel::ObstructionOcclusionLevel()
	: m_fObstruction( 0.0f )
	, m_fOcclusion( 0.0f )
{}

CSFTestAuxBusDlg::GameObjectAuxBus::GameObjectAuxBus()
	: m_fDryLevel( 1.0f )
{
	for( int i = 0; i < s_iNumOfAuxBus; ++i )
	{
		m_sAuxBusValue[i].auxBusID = AK_INVALID_AUX_ID;
		m_sAuxBusValue[i].fControlValue = 0.0f;
	}
}

CSFTestAuxBusDlg::CSFTestAuxBusDlg(CWnd* pParent /*= NULL*/)
	: CDialog(CSFTestAuxBusDlg::IDD, pParent)
	, m_pSoundFrame( NULL )
	, m_pDropTarget( NULL )
{}
	
CSFTestAuxBusDlg::~CSFTestAuxBusDlg()
{}

void CSFTestAuxBusDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_ENVIRONMENT_LIST, m_AuxBusList);
	DDX_Control(pDX, IDC_ACTIVE_GAME_OBJECT_COMBO, m_activeGameObjectCombo);
	DDX_Control(pDX, IDC_ACTIVE_LISTENER_COMBO, m_activeListenerCombo);
	DDX_Control(pDX, IDC_OBSTRUCTION_LEVEL_SLIDER, m_obstructionLevelSlider);
	DDX_Control(pDX, IDC_OCCLUSION_LEVEL_SLIDER, m_occlusionLevelSlider);
	DDX_Control(pDX, IDC_GAME_OBJECT_COMBO, m_AuxBusGameObjectCombo);
	DDX_Control(pDX, IDC_ENVIRONMENT_1_COMBO, m_AuxBusCombo[0]);
	DDX_Control(pDX, IDC_ENVIRONMENT_2_COMBO, m_AuxBusCombo[1]);
	DDX_Control(pDX, IDC_ENVIRONMENT_3_COMBO, m_AuxBusCombo[2]);
	DDX_Control(pDX, IDC_ENVIRONMENT_4_COMBO, m_AuxBusCombo[3]);
	DDX_Control(pDX, IDC_ENVIRONMENT_1_LEVEL_SLIDER, m_AuxBusSlider[0]);
	DDX_Control(pDX, IDC_ENVIRONMENT_2_LEVEL_SLIDER, m_AuxBusSlider[1]);
	DDX_Control(pDX, IDC_ENVIRONMENT_3_LEVEL_SLIDER, m_AuxBusSlider[2]);
	DDX_Control(pDX, IDC_ENVIRONMENT_4_LEVEL_SLIDER, m_AuxBusSlider[3]);
	DDX_Control(pDX, IDC_ENVIRONMENT_1_LEVEL_STATIC, m_AuxBusStatic[0]);
	DDX_Control(pDX, IDC_ENVIRONMENT_2_LEVEL_STATIC, m_AuxBusStatic[1]);
	DDX_Control(pDX, IDC_ENVIRONMENT_3_LEVEL_STATIC, m_AuxBusStatic[2]);
	DDX_Control(pDX, IDC_ENVIRONMENT_4_LEVEL_STATIC, m_AuxBusStatic[3]);
	DDX_Control(pDX, IDC_GAME_OBJECT_DRY_LEVEL_SLIDER, m_dryLevelSlider);
}

BEGIN_MESSAGE_MAP(CSFTestAuxBusDlg, CDialog)
	ON_WM_DESTROY()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_CBN_SELCHANGE(IDC_ACTIVE_GAME_OBJECT_COMBO, OnLbnSelchangeActiveGameObjectListenerCombo)
	ON_CBN_SELCHANGE(IDC_ACTIVE_LISTENER_COMBO, OnLbnSelchangeActiveGameObjectListenerCombo)
	ON_CBN_SELCHANGE(IDC_GAME_OBJECT_COMBO, OnLbnSelchangeAuxBusGameObjectCombo)
	ON_CBN_SELCHANGE(IDC_ENVIRONMENT_1_COMBO, OnGameObjectAuxBusChange)
	ON_CBN_SELCHANGE(IDC_ENVIRONMENT_2_COMBO, OnGameObjectAuxBusChange)
	ON_CBN_SELCHANGE(IDC_ENVIRONMENT_3_COMBO, OnGameObjectAuxBusChange)
	ON_CBN_SELCHANGE(IDC_ENVIRONMENT_4_COMBO, OnGameObjectAuxBusChange)
	ON_BN_CLICKED(IDC_GETALLENVIRONMENTS, OnBnClickedGetAllAuxBus)
	ON_BN_CLICKED(IDC_CLEARENVIRONMENTS, OnBnClickedClearAuxBus)
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_MESSAGE(WM_SF_SHOW, OnShowListItems)
END_MESSAGE_MAP()

BOOL CSFTestAuxBusDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	ASSERT( m_pSoundFrame && m_pDropTarget );

	m_pDropTarget->Register( &m_AuxBusList );
	
	m_obstructionLevelSlider.SetRange( 0, 100, TRUE );
	m_occlusionLevelSlider.SetRange( 0, 100, TRUE );
	m_dryLevelSlider.SetRange( 0, 100, TRUE );

	for(int i = 0; i < s_iNumOfAuxBus; ++i)
		m_AuxBusSlider[i].SetRange( 0, 100, TRUE );

	m_activeListenerCombo.SetCurSel( 0 );

	FillAuxBusCombo();

	UpdateGameObjectList();

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CSFTestAuxBusDlg::OnDestroy()
{
	m_pDropTarget->Revoke();

	m_AuxBusList.ClearList();
	m_activeGameObjectCombo.ClearCombo();
	m_AuxBusGameObjectCombo.ClearCombo();

	for(int i = 0; i < s_iNumOfAuxBus; ++i )
		m_AuxBusCombo[i].ClearCombo();
}

HCURSOR CSFTestAuxBusDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CSFTestAuxBusDlg::Init( ISoundFrame * in_pSoundFrame, CSFAuxBusDropTarget* in_pDropTarget )
{
	m_pSoundFrame = in_pSoundFrame;
	m_pDropTarget = in_pDropTarget;
}

void CSFTestAuxBusDlg::AddAuxBusToList( IAuxBusList * in_pAuxBusList )
{
	if( ::IsWindow( m_AuxBusList.GetSafeHwnd() ) )
		m_AuxBusList.AddObjects( in_pAuxBusList );
}

void CSFTestAuxBusDlg::OnAuxBusNotif( IClient::Notif in_eNotif, AkUniqueID in_AuxBusID )
{
	if( !::IsWindow( m_AuxBusList.GetSafeHwnd() ) )
		return;

	bool bHadAuxBus = false;
	bool bSelected = false;

	bool aWasSelected[s_iNumOfAuxBus] = {0};

	if ( in_eNotif == IClient::Notif_Removed || in_eNotif == IClient::Notif_Changed )
	{
		// Remove AuxBus from list
		bHadAuxBus = m_AuxBusList.RemoveObject( in_AuxBusID, bSelected );

		// Remove the AuxBus from the Combo box
		for(int i = 0; i < s_iNumOfAuxBus; ++i)
			m_AuxBusCombo[i].RemoveObject( in_AuxBusID, aWasSelected[i] );
	}

	if ( in_eNotif == IClient::Notif_Changed || in_eNotif == IClient::Notif_Added )
	{
		IAuxBus * pAuxBus = NULL;
		// Get the IAuxBus from its ID
		m_pSoundFrame->GetAuxBus( in_AuxBusID, &pAuxBus );

		if ( pAuxBus )
		{
			if ( bHadAuxBus && ( in_eNotif == IClient::Notif_Changed ) )
			{
				int idx = m_AuxBusList.AddObject( pAuxBus );

				if ( bSelected )
					m_AuxBusList.SetCurSel( idx );
			}

			for(int i = 0; i < s_iNumOfAuxBus; ++i)
			{
				int idx = m_AuxBusCombo[i].AddObject( pAuxBus );

				if ( aWasSelected[i] )
					m_AuxBusCombo[i].SetCurSel( idx );
			}

			// Release the AuxBus
			pAuxBus->Release();
		}
	}
}

void CSFTestAuxBusDlg::UpdateGameObjectList()
{
	if( ::IsWindow( m_activeGameObjectCombo.GetSafeHwnd() ) 
		&& ::IsWindow( m_AuxBusGameObjectCombo.GetSafeHwnd() ))
	{
		AkGameObjectID selectedActiveObject = AK_INVALID_GAME_OBJECT;
		AkGameObjectID selectedAuxBusObject = AK_INVALID_GAME_OBJECT;

		int idx = m_activeGameObjectCombo.GetCurSel();
		if( idx != CB_ERR )
			selectedActiveObject = m_activeGameObjectCombo.GetObject( idx )->GetID();

		idx = m_AuxBusGameObjectCombo.GetCurSel();
		if( idx != CB_ERR )
			selectedAuxBusObject = m_AuxBusGameObjectCombo.GetObject( idx )->GetID();

		m_activeGameObjectCombo.ClearCombo();
		m_AuxBusGameObjectCombo.ClearCombo();

		IGameObjectList * pGameObjectList = NULL;
		// Get all game object registered in Wwise
		if ( m_pSoundFrame->GetGameObjectList( &pGameObjectList ) )
		{
			m_activeGameObjectCombo.AddObjects( pGameObjectList );
			m_AuxBusGameObjectCombo.AddObjects( pGameObjectList );
			pGameObjectList->Release();

			if( ! m_activeGameObjectCombo.SelectObject( selectedActiveObject ) )
				m_activeGameObjectCombo.SetCurSel( 0 );

			if( ! m_AuxBusGameObjectCombo.SelectObject( selectedAuxBusObject ) )
				m_AuxBusGameObjectCombo.SetCurSel( 0 );
		}

		OnLbnSelchangeActiveGameObjectListenerCombo();
		OnLbnSelchangeAuxBusGameObjectCombo();
	}
}

void CSFTestAuxBusDlg::OnLbnSelchangeActiveGameObjectListenerCombo()
{
	int idxGO = m_activeGameObjectCombo.GetCurSel();
	int iListenerIndex = m_activeListenerCombo.GetCurSel();
	if( idxGO != CB_ERR && iListenerIndex != CB_ERR )
	{
		IGameObject* pGameObject = m_activeGameObjectCombo.GetObject( idxGO );
		const ObstructionOcclusionLevel& rObsOccLevel = m_obstructionOcclusionMap[ GameObjectListenerPair( pGameObject->GetID(), iListenerIndex ) ];

		m_obstructionLevelSlider.SetPos( (int)(rObsOccLevel.m_fObstruction * 100.0f) );
		m_occlusionLevelSlider.SetPos( (int)(rObsOccLevel.m_fOcclusion * 100.0f) );

		CString csTemp;
		csTemp.Format( _T("%.2f"), rObsOccLevel.m_fObstruction );
		SetDlgItemText( IDC_OBSTRUCTION_LEVEL_STATIC, csTemp );

		csTemp.Format( _T("%.2f"), rObsOccLevel.m_fOcclusion );
		SetDlgItemText( IDC_OCCLUSION_LEVEL_STATIC, csTemp );
	}
	else
	{
		m_obstructionLevelSlider.SetPos( 0 );
		m_occlusionLevelSlider.SetPos( 0 );

		SetDlgItemText( IDC_OBSTRUCTION_LEVEL_STATIC, _T("") );
		SetDlgItemText( IDC_OCCLUSION_LEVEL_STATIC, _T("") );
	}
}

void CSFTestAuxBusDlg::OnLbnSelchangeAuxBusGameObjectCombo()
{
	int idx = m_AuxBusGameObjectCombo.GetCurSel();
	if( idx != CB_ERR )
	{
		IGameObject* pGameObject = m_AuxBusGameObjectCombo.GetObject( idx );
		const GameObjectAuxBus& rGameObjectAuxBus = m_gameObjectAuxBusMap[ pGameObject->GetID() ];

		CString csTemp;

		for(int i = 0; i < s_iNumOfAuxBus; ++i)
		{
			if( rGameObjectAuxBus.m_sAuxBusValue[i].auxBusID != AK_INVALID_AUX_ID )
			{
				m_AuxBusCombo[i].SelectObject( rGameObjectAuxBus.m_sAuxBusValue[i].auxBusID );

				m_AuxBusSlider[i].SetPos( (int)(rGameObjectAuxBus.m_sAuxBusValue[i].fControlValue * 100.0f) );
				
				csTemp.Format( _T("%.2f"), rGameObjectAuxBus.m_sAuxBusValue[i].fControlValue );
				m_AuxBusStatic[i].SetWindowText( csTemp );
			}
			else
			{
				m_AuxBusCombo[i].SetCurSel( 0 );
				m_AuxBusSlider[i].SetPos( 0 );
				m_AuxBusStatic[i].SetWindowText( _T("") );
			}
		}

		m_dryLevelSlider.SetPos( (int)(rGameObjectAuxBus.m_fDryLevel * 100.0f) );

		csTemp.Format( _T("%.2f"), rGameObjectAuxBus.m_fDryLevel );
		SetDlgItemText( IDC_GAME_OBJECT_DRY_LEVEL_STATIC, csTemp );
	}
	else
	{
		for(int i = 0; i < s_iNumOfAuxBus; ++i)
		{
			m_AuxBusCombo[i].SetCurSel( 0 );
			m_AuxBusSlider[i].SetPos( 0 );
			m_AuxBusStatic[i].SetWindowText( _T("") );
		}

		m_dryLevelSlider.SetPos(0);
		SetDlgItemText( IDC_GAME_OBJECT_DRY_LEVEL_STATIC, _T("") );
	}
}

void CSFTestAuxBusDlg::OnBnClickedGetAllAuxBus()
{
	m_AuxBusList.ClearList();

	IAuxBusList * pAuxBusList = NULL;
	// Get all AuxBus created in the current Wwise project
	if ( m_pSoundFrame->GetAuxBusList( &pAuxBusList ) )
	{
		AddAuxBusToList( pAuxBusList );
		pAuxBusList->Release();
	}
}

void CSFTestAuxBusDlg::OnBnClickedClearAuxBus()
{
	m_AuxBusList.ClearList();
}

void CSFTestAuxBusDlg::OnGameObjectAuxBusChange()
{
	int idx = m_AuxBusGameObjectCombo.GetCurSel();
	if( idx != CB_ERR )
	{
		IGameObject* pGameObject = m_AuxBusGameObjectCombo.GetObject( idx );
		GameObjectAuxBus& rGameObjectAuxBus = m_gameObjectAuxBusMap[ pGameObject->GetID() ];

		std::vector<AkAuxSendValue> auxBusToSet;

		for( int i = 0; i < s_iNumOfAuxBus; ++i )
		{
			IAuxBus* pEnv = NULL;
			int envIdx = m_AuxBusCombo[i].GetCurSel();
			if( envIdx != CB_ERR )
				pEnv = m_AuxBusCombo[i].GetObject( envIdx );

			if( pEnv )
			{
				rGameObjectAuxBus.m_sAuxBusValue[i].auxBusID = pEnv->GetID();
				rGameObjectAuxBus.m_sAuxBusValue[i].fControlValue = (AkReal32)m_AuxBusSlider[i].GetPos() / 100.0f;

				auxBusToSet.push_back( rGameObjectAuxBus.m_sAuxBusValue[i] );

				CString csValue;
				csValue.Format( _T("%.2f"), rGameObjectAuxBus.m_sAuxBusValue[i].fControlValue );
				m_AuxBusStatic[i].SetWindowText( csValue );
			}
			else
			{
				rGameObjectAuxBus.m_sAuxBusValue[i].auxBusID = AK_INVALID_AUX_ID;
				rGameObjectAuxBus.m_sAuxBusValue[i].fControlValue = 0.0f;

				m_AuxBusSlider[i].SetPos( 0 );
				m_AuxBusStatic[i].SetWindowText( _T("") );
			}
		}

		// Set the AuxBus for the Selected Game Object
		m_pSoundFrame->SetGameObjectAuxSendValues( pGameObject->GetID(), auxBusToSet.empty() ? NULL : &(auxBusToSet[0]), (AkUInt32)auxBusToSet.size() );
	}
}

void CSFTestAuxBusDlg::OnHScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar )
{
	if( pScrollBar->m_hWnd == m_obstructionLevelSlider.m_hWnd
				|| pScrollBar->m_hWnd == m_occlusionLevelSlider.m_hWnd )
	{
		OnObstructionOcclusionChanged();
	}
	else if( pScrollBar->m_hWnd == m_dryLevelSlider.m_hWnd )
	{
		OnDryLevelChanged();
	}
	else
	{
		for( int i = 0; i < s_iNumOfAuxBus; ++i )
		{
			if( pScrollBar->m_hWnd == m_AuxBusSlider[i].m_hWnd )
			{
				OnGameObjectAuxBusChange();
				break;
			}
		}
	}
}

LRESULT CSFTestAuxBusDlg::OnShowListItems( WPARAM in_wParam, LPARAM in_lParam )
{
	std::vector<GUID> * pGuids = (std::vector<GUID> *) in_lParam;
	m_pSoundFrame->ShowWwiseObject( &pGuids->front(), pGuids->size(), (AK::SoundFrame::ISoundFrame::ShowLocation) in_wParam );
	
	return TRUE;
}

void CSFTestAuxBusDlg::OnObstructionOcclusionChanged()
{
	int idxGO = m_activeGameObjectCombo.GetCurSel();
	int iListenerIndex = m_activeListenerCombo.GetCurSel();
	if( idxGO != CB_ERR && iListenerIndex != CB_ERR )
	{
		IGameObject* pGameObject = m_activeGameObjectCombo.GetObject( idxGO );
		ObstructionOcclusionLevel& rObsOccLevel = m_obstructionOcclusionMap[ GameObjectListenerPair( pGameObject->GetID(), iListenerIndex ) ];

		// Keep the value in the map
		rObsOccLevel.m_fObstruction = (AkReal32)m_obstructionLevelSlider.GetPos() / 100.0f;
		rObsOccLevel.m_fOcclusion = (AkReal32)m_occlusionLevelSlider.GetPos() / 100.0f;

		CString csTemp;
		csTemp.Format( _T("%.2f"), rObsOccLevel.m_fObstruction );
		SetDlgItemText( IDC_OBSTRUCTION_LEVEL_STATIC, csTemp );

		csTemp.Format( _T("%.2f"), rObsOccLevel.m_fOcclusion );
		SetDlgItemText( IDC_OCCLUSION_LEVEL_STATIC, csTemp );

		// Change the obstruction and occlusion level between the Selected Game object and the Selected Listener.
		m_pSoundFrame->SetObjectObstructionAndOcclusion( pGameObject->GetID(), iListenerIndex, rObsOccLevel.m_fObstruction, rObsOccLevel.m_fOcclusion );
	}
}

void CSFTestAuxBusDlg::OnDryLevelChanged()
{
	int idx = m_AuxBusGameObjectCombo.GetCurSel();
	if( idx != CB_ERR )
	{
		IGameObject* pGameObject = m_AuxBusGameObjectCombo.GetObject( idx );
		GameObjectAuxBus& rGameObjectAuxBus = m_gameObjectAuxBusMap[ pGameObject->GetID() ];

		// Keep the value in the map
		rGameObjectAuxBus.m_fDryLevel = (AkReal32)m_dryLevelSlider.GetPos() / 100.0f;

		CString csTemp;
		csTemp.Format( _T("%.2f"), rGameObjectAuxBus.m_fDryLevel );
		SetDlgItemText( IDC_GAME_OBJECT_DRY_LEVEL_STATIC, csTemp );

		// Set the Game object Dry level
		m_pSoundFrame->SetGameObjectOutputBusVolume( pGameObject->GetID(), AK_INVALID_GAME_OBJECT, rGameObjectAuxBus.m_fDryLevel );
	}
}

void CSFTestAuxBusDlg::FillAuxBusCombo()
{
	IAuxBusList * pAuxBusList = NULL;
	if ( m_pSoundFrame->GetAuxBusList( &pAuxBusList ) )
	{
		for(int i = 0; i < s_iNumOfAuxBus; ++i)
		{
			m_AuxBusCombo[i].ClearCombo();

			// Add the none AuxBus
			// Make sure that sorting is off in the combo box since we assume that the None element is
			// at the index 0.
			int idx = m_AuxBusCombo[i].AddString( _T("None") );
			m_AuxBusCombo[i].SetItemDataPtr( idx, NULL );

			m_AuxBusCombo[i].AddObjects( pAuxBusList );
		}

		pAuxBusList->Release();
	}
}