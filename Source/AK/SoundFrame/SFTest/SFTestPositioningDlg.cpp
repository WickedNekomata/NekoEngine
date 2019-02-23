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
#include "SFTestPositioningDlg.h"

#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace
{
	static const double s_pi = 3.1415926535;
}

CSFTestPositioningDlg::GameObjectInfo::GameObjectInfo()
{
	::ZeroMemory( &m_position, sizeof( AkSoundPosition ) );

	// Listener 0 is active by default
	m_activeListener[0] = 0;
	m_uNumListeners = 1;

	// Default position and orientation for a Game object
	static const AkVector pos = { 0, 0, 1 };
	static const AkVector front = { 0, 0, -1 };
	static const AkVector top = { 0, 1, 0 };
	m_position.Set(pos, front, top);
}

CSFTestPositioningDlg::ListenerInfo::ListenerInfo()
	: m_bSpatialized( true )
	, m_fVolumeOffsetFrontLeft( 0.f )
	, m_fVolumeOffsetFrontRight( 0.f )
	, m_fVolumeOffsetCenter( 0.f )
	, m_fVolumeOffsetLfe( 0.f )
	, m_fVolumeOffsetRearLeft( 0.f )
	, m_fVolumeOffsetRearRight( 0.f )
{
	::ZeroMemory( &m_position, sizeof( AkListenerPosition ) );
	
	// Default position and orientation of the listener
	static const AkVector pos = { 0, 0, 0 };
	static const AkVector front = { 0, 0, 1 };
	static const AkVector top = { 0, 1, 0 };
	m_position.Set(pos, front, top);
}

CSFTestPositioningDlg::CSFTestPositioningDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSFTestPositioningDlg::IDD, pParent)
	, m_pSoundFrame( NULL )
{
	IdcListnerList[0] = IDC_L0_CHECK;
	IdcListnerList[1] = IDC_L1_CHECK;
	IdcListnerList[2] = IDC_L2_CHECK;
	IdcListnerList[3] = IDC_L3_CHECK;
	IdcListnerList[4] = IDC_L4_CHECK;
	IdcListnerList[5] = IDC_L5_CHECK;
	IdcListnerList[6] = IDC_L6_CHECK;
	IdcListnerList[7] = IDC_L7_CHECK;

	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CSFTestPositioningDlg::~CSFTestPositioningDlg()
{
}

void CSFTestPositioningDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_GAME_OBJECT_COMBO, m_gameObjectCombo);
	DDX_Control(pDX, IDC_LISTENER_COMBO, m_listenerCombo);
	DDX_Control(pDX, IDC_GAME_OBJECT_POSX_SLIDER, m_gameObjectPosX);
	DDX_Control(pDX, IDC_GAME_OBJECT_POSZ_SLIDER, m_gameObjectPosZ);
	DDX_Control(pDX, IDC_GAME_OBJECT_ANGLE_SLIDER, m_gameObjectOriAngle);
	DDX_Control(pDX, IDC_LISTENER_POSX_SLIDER, m_listenerPosX);
	DDX_Control(pDX, IDC_LISTENER_POSZ_SLIDER, m_listenerPosZ);
	DDX_Control(pDX, IDC_LISTENER_ANGLE_SLIDER, m_listenerOriAngle);
	DDX_Control(pDX, IDC_LEFT_EDIT, m_volumeLeftEdit);
	DDX_Control(pDX, IDC_RIGHT_EDIT, m_volumeRightEdit);
	DDX_Control(pDX, IDC_CENTER_EDIT, m_volumeCenterEdit);
	DDX_Control(pDX, IDC_LFE_EDIT, m_volumeLFEEdit);
	DDX_Control(pDX, IDC_REAR_LEFT_EDIT, m_volumeRearLeftEdit);
	DDX_Control(pDX, IDC_REAR_RIGHT_EDIT, m_volumeRearRightEdit);
}

BEGIN_MESSAGE_MAP(CSFTestPositioningDlg, CDialog)
	ON_WM_DESTROY()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_CBN_SELCHANGE(IDC_GAME_OBJECT_COMBO, OnCbnSelchangeGameObjectCombo)
	ON_CBN_SELCHANGE(IDC_LISTENER_COMBO, OnCbnSelchangeListenerCombo)
	ON_BN_CLICKED(IDC_L0_CHECK, OnBnClickedListenerCheck)
	ON_BN_CLICKED(IDC_L1_CHECK, OnBnClickedListenerCheck)
	ON_BN_CLICKED(IDC_L2_CHECK, OnBnClickedListenerCheck)
	ON_BN_CLICKED(IDC_L3_CHECK, OnBnClickedListenerCheck)
	ON_BN_CLICKED(IDC_L4_CHECK, OnBnClickedListenerCheck)
	ON_BN_CLICKED(IDC_L5_CHECK, OnBnClickedListenerCheck)
	ON_BN_CLICKED(IDC_L6_CHECK, OnBnClickedListenerCheck)
	ON_BN_CLICKED(IDC_L7_CHECK, OnBnClickedListenerCheck)
	ON_BN_CLICKED(IDC_SPATIALIZATION_CHECK, OnBnClickedEnableSpatialization)
	ON_WM_HSCROLL()
	ON_EN_KILLFOCUS(IDC_LEFT_EDIT, OnKillFocusVolumeEdit)
	ON_EN_KILLFOCUS(IDC_RIGHT_EDIT, OnKillFocusVolumeEdit)
	ON_EN_KILLFOCUS(IDC_CENTER_EDIT, OnKillFocusVolumeEdit)
	ON_EN_KILLFOCUS(IDC_LFE_EDIT, OnKillFocusVolumeEdit)
	ON_EN_KILLFOCUS(IDC_REAR_LEFT_EDIT, OnKillFocusVolumeEdit)
	ON_EN_KILLFOCUS(IDC_REAR_RIGHT_EDIT, OnKillFocusVolumeEdit)
END_MESSAGE_MAP()

BOOL CSFTestPositioningDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	ASSERT( m_pSoundFrame );

	m_gameObjectPosX.SetRange(-100, 100, TRUE);
	m_gameObjectPosZ.SetRange(-100, 100, TRUE);
	m_gameObjectOriAngle.SetRange(0, 360, TRUE);

	m_listenerPosX.SetRange(-100, 100, TRUE);
	m_listenerPosZ.SetRange(-100, 100, TRUE);
	m_listenerOriAngle.SetRange(0, 360, TRUE);

	UpdateGameObjectList();

	m_listenerCombo.SetCurSel( 0 );
	OnCbnSelchangeListenerCombo();

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CSFTestPositioningDlg::OnDestroy()
{
	m_gameObjectCombo.ClearCombo();
}

HCURSOR CSFTestPositioningDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CSFTestPositioningDlg::Init( ISoundFrame * in_pSoundFrame )
{
	m_pSoundFrame = in_pSoundFrame;

	m_listenerInfoVector.insert( m_listenerInfoVector.begin(), 8, ListenerInfo() );
}

void CSFTestPositioningDlg::GetGameObjectPosition( AkGameObjectID in_gameObjectID, AkSoundPosition& out_rPosition )
{
	out_rPosition = m_objectInfoMap[in_gameObjectID].m_position;
}

void CSFTestPositioningDlg::UpdateGameObjectList()
{
	if( ::IsWindow( m_gameObjectCombo.GetSafeHwnd() ) )
	{
		AkGameObjectID selectedObject = AK_INVALID_GAME_OBJECT;
		int idx = m_gameObjectCombo.GetCurSel();
		if( idx != CB_ERR )
			selectedObject = m_gameObjectCombo.GetObject( idx )->GetID();

		m_gameObjectCombo.ClearCombo();

		IGameObjectList * pGameObjectList = NULL;
		// Get all Game objects registered in Wwise
		if ( m_pSoundFrame->GetGameObjectList( &pGameObjectList ) )
		{
			m_gameObjectCombo.AddObjects( pGameObjectList );
			pGameObjectList->Release();

			if( ! m_gameObjectCombo.SelectObject( selectedObject ) )
				m_gameObjectCombo.SetCurSel( 0 );
		}

		OnCbnSelchangeGameObjectCombo();
	}
}

void CSFTestPositioningDlg::OnCbnSelchangeGameObjectCombo()
{
	int idx = m_gameObjectCombo.GetCurSel();
	if( idx != CB_ERR )
	{
		IGameObject* pGameObject = m_gameObjectCombo.GetObject( idx );
		const GameObjectInfo& rObjectInfo = m_objectInfoMap[pGameObject->GetID()];

		m_gameObjectPosX.SetPos( (int)rObjectInfo.m_position.Position().X );
		m_gameObjectPosZ.SetPos( (int)rObjectInfo.m_position.Position().Z );

		m_gameObjectOriAngle.SetPos( CalcAngle( rObjectInfo.m_position.OrientationFront().X, rObjectInfo.m_position.OrientationFront().Z ) );

		CString csTemp;
		csTemp.Format( _T("%.0f"), rObjectInfo.m_position.Position().X );
		SetDlgItemText( IDC_GAME_OBJECT_POSX_STATIC, csTemp );

		csTemp.Format( _T("%.0f"), rObjectInfo.m_position.Position().Z );
		SetDlgItemText( IDC_GAME_OBJECT_POSZ_STATIC, csTemp );

		csTemp.Format( _T("%.2f"), rObjectInfo.m_position.OrientationFront().X );
		SetDlgItemText( IDC_GAME_OBJECT_ORIX_STATIC, csTemp );

		csTemp.Format( _T("%.2f"), rObjectInfo.m_position.OrientationFront().Z );
		SetDlgItemText( IDC_GAME_OBJECT_ORIZ_STATIC, csTemp );

		for (AkUInt32 i = 0; i < MAX_NUM_LISTENER_CHECKBOX; i++)
		{
			CheckDlgButton(IdcListnerList[i], BST_UNCHECKED );
		}

		for (AkUInt32 i = 0; i < rObjectInfo.m_uNumListeners; i++)
		{
			CheckDlgButton(IdcListnerList[rObjectInfo.m_activeListener[i]], BST_CHECKED);
		}
	}
	else
	{
		m_gameObjectPosX.SetPos( 0 );
		m_gameObjectPosZ.SetPos( 0 );
		m_gameObjectOriAngle.SetPos( 0 );
		SetDlgItemText( IDC_GAME_OBJECT_POSX_STATIC, _T("") );
		SetDlgItemText( IDC_GAME_OBJECT_POSZ_STATIC, _T("") );
		SetDlgItemText( IDC_GAME_OBJECT_ORIX_STATIC, _T("") );
		SetDlgItemText( IDC_GAME_OBJECT_ORIZ_STATIC, _T("") );

		CheckDlgButton( IDC_L0_CHECK, BST_UNCHECKED );
		CheckDlgButton( IDC_L1_CHECK, BST_UNCHECKED );
		CheckDlgButton( IDC_L2_CHECK, BST_UNCHECKED );
		CheckDlgButton( IDC_L3_CHECK, BST_UNCHECKED );
		CheckDlgButton( IDC_L4_CHECK, BST_UNCHECKED );
		CheckDlgButton( IDC_L5_CHECK, BST_UNCHECKED );
		CheckDlgButton( IDC_L6_CHECK, BST_UNCHECKED );
		CheckDlgButton( IDC_L7_CHECK, BST_UNCHECKED );

	}
}

void CSFTestPositioningDlg::OnCbnSelchangeListenerCombo()
{
	int idx = m_listenerCombo.GetCurSel();
	if( idx != CB_ERR )
	{
		const ListenerInfo& rListenerInfo = m_listenerInfoVector[idx];

		m_listenerPosX.SetPos( (int)rListenerInfo.m_position.Position().X );
		m_listenerPosZ.SetPos( (int)rListenerInfo.m_position.Position().Z );

		m_listenerOriAngle.SetPos( CalcAngle( rListenerInfo.m_position.OrientationFront().X, rListenerInfo.m_position.OrientationFront().Z ) );

		CString csTemp;
		csTemp.Format( _T("%.0f"), rListenerInfo.m_position.Position().X );
		SetDlgItemText( IDC_LISTENER_POSX_STATIC, csTemp );

		csTemp.Format( _T("%.0f"), rListenerInfo.m_position.Position().Z );
		SetDlgItemText( IDC_LISTENER_POSZ_STATIC, csTemp );

		csTemp.Format( _T("%.2f"), rListenerInfo.m_position.OrientationFront().X );
		SetDlgItemText( IDC_LISTENER_ORIX_STATIC, csTemp );

		csTemp.Format( _T("%.2f"), rListenerInfo.m_position.OrientationFront().Z );
		SetDlgItemText( IDC_LISTENER_ORIZ_STATIC, csTemp );

		CheckDlgButton( IDC_SPATIALIZATION_CHECK, rListenerInfo.m_bSpatialized? BST_CHECKED : BST_UNCHECKED );

		m_volumeLeftEdit.SetVolume( rListenerInfo.m_fVolumeOffsetFrontLeft );
		m_volumeRightEdit.SetVolume( rListenerInfo.m_fVolumeOffsetFrontRight );
		m_volumeCenterEdit.SetVolume( rListenerInfo.m_fVolumeOffsetCenter );
		m_volumeLFEEdit.SetVolume( rListenerInfo.m_fVolumeOffsetLfe );
		m_volumeRearLeftEdit.SetVolume( rListenerInfo.m_fVolumeOffsetRearLeft );
		m_volumeRearRightEdit.SetVolume( rListenerInfo.m_fVolumeOffsetRearRight );
	}
	else
	{
		m_listenerPosX.SetPos( 0 );
		m_listenerPosZ.SetPos( 0 );
		m_listenerOriAngle.SetPos( 0 );
		SetDlgItemText( IDC_LISTENER_POSX_STATIC, _T("") );
		SetDlgItemText( IDC_LISTENER_POSZ_STATIC, _T("") );
		SetDlgItemText( IDC_LISTENER_ORIX_STATIC, _T("") );
		SetDlgItemText( IDC_LISTENER_ORIZ_STATIC, _T("") );

		CheckDlgButton( IDC_SPATIALIZATION_CHECK, BST_UNCHECKED );

		m_volumeLeftEdit.SetVolume( 0.0 );
		m_volumeRightEdit.SetVolume( 0.0 );
		m_volumeCenterEdit.SetVolume( 0.0 );
		m_volumeLFEEdit.SetVolume( 0.0 );
		m_volumeRearLeftEdit.SetVolume( 0.0 );
		m_volumeRearRightEdit.SetVolume( 0.0 );
	}
}

void CSFTestPositioningDlg::OnBnClickedListenerCheck()
{
	int idx = m_gameObjectCombo.GetCurSel();
	if( idx != CB_ERR )
	{
		IGameObject* pGameObject = m_gameObjectCombo.GetObject( idx );
		GameObjectInfo& rObjectInfo = m_objectInfoMap[pGameObject->GetID()];

		AkUInt32 idx = 0;
		for (AkUInt32 i = 0; i < MAX_NUM_LISTENER_CHECKBOX; i++) {
			if (IsDlgButtonChecked(IdcListnerList[i]) == BST_CHECKED) {
				rObjectInfo.m_activeListener[idx] = i;
				++idx;
			}
		}

		rObjectInfo.m_uNumListeners = idx;

		// Set the active listener array for the selected Game Object
		m_pSoundFrame->SetListeners( pGameObject->GetID(), rObjectInfo.m_activeListener, rObjectInfo.m_uNumListeners);
	}
}

void CSFTestPositioningDlg::SetListenerSpatialization( AkUInt32 in_uiIndex, CSFTestPositioningDlg::ListenerInfo & in_rListenerInfo )
{
	// Enable or disable Listener spatialization
	AkUInt32 uVectorSize = AK::SpeakerVolumes::Vector::GetRequiredSize( 6 );	// 5.1 channels
	AkReal32 * pVolumes = (AkReal32 *)_alloca( uVectorSize );
	// Channel order is defined by bit ordering in the channel mask, with the exception of the LFE which is always shuffled at the end.
	pVolumes[ 0 ] = in_rListenerInfo.m_fVolumeOffsetFrontLeft;
	pVolumes[ 1 ] = in_rListenerInfo.m_fVolumeOffsetFrontRight;
	pVolumes[ 2 ] = in_rListenerInfo.m_fVolumeOffsetCenter;
	pVolumes[ 3 ] = in_rListenerInfo.m_fVolumeOffsetRearLeft;
	pVolumes[ 4 ] = in_rListenerInfo.m_fVolumeOffsetRearRight;
	pVolumes[ 5 ] = in_rListenerInfo.m_fVolumeOffsetLfe;
	m_pSoundFrame->SetListenerSpatialization( in_uiIndex, in_rListenerInfo.m_bSpatialized, AkChannelConfig( 6, AK_SPEAKER_SETUP_5_1 ), in_rListenerInfo.m_bSpatialized? NULL : pVolumes );
}

void CSFTestPositioningDlg::OnBnClickedEnableSpatialization()
{
	int idx = m_listenerCombo.GetCurSel();
	if( idx != CB_ERR )
	{
		ListenerInfo& rListenerInfo = m_listenerInfoVector[idx];

		rListenerInfo.m_bSpatialized = ( IsDlgButtonChecked( IDC_SPATIALIZATION_CHECK ) == BST_CHECKED )? true : false;

		SetListenerSpatialization( idx, rListenerInfo );
	}
}

void CSFTestPositioningDlg::OnHScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar )
{
	if( pScrollBar->m_hWnd == m_gameObjectPosX.m_hWnd
			|| pScrollBar->m_hWnd == m_gameObjectPosZ.m_hWnd
			|| pScrollBar->m_hWnd == m_gameObjectOriAngle.m_hWnd )
	{
		OnGameObjectPosOriChanged();
	}
	else if( pScrollBar->m_hWnd == m_listenerPosX.m_hWnd
				|| pScrollBar->m_hWnd == m_listenerPosZ.m_hWnd
				|| pScrollBar->m_hWnd == m_listenerOriAngle.m_hWnd )
	{
		OnListenerPosOriChanged();
	}
}

void CSFTestPositioningDlg::OnKillFocusVolumeEdit()
{
	int idx = m_listenerCombo.GetCurSel();
	if( idx != CB_ERR )
	{
		ListenerInfo& rListenerInfo = m_listenerInfoVector[idx];

		rListenerInfo.m_fVolumeOffsetFrontLeft = (AkReal32) m_volumeLeftEdit.GetVolume();
		rListenerInfo.m_fVolumeOffsetFrontRight = (AkReal32) m_volumeRightEdit.GetVolume();
		rListenerInfo.m_fVolumeOffsetCenter = (AkReal32) m_volumeCenterEdit.GetVolume();
		rListenerInfo.m_fVolumeOffsetLfe = (AkReal32) m_volumeLFEEdit.GetVolume();
		rListenerInfo.m_fVolumeOffsetRearLeft = (AkReal32) m_volumeRearLeftEdit.GetVolume();
		rListenerInfo.m_fVolumeOffsetRearRight = (AkReal32) m_volumeRearRightEdit.GetVolume();

		bool bIsChecked = ( IsDlgButtonChecked( IDC_SPATIALIZATION_CHECK ) == BST_CHECKED )? true : false;

		if( !bIsChecked || !rListenerInfo.m_bSpatialized )
		{
			rListenerInfo.m_bSpatialized = bIsChecked;

			// Set the Volume offset on each channel for the Selected Listener (Only if spacialization is false)
			SetListenerSpatialization( idx, rListenerInfo );
		}
	}
}

void CSFTestPositioningDlg::OnGameObjectPosOriChanged()
{
	int idx = m_gameObjectCombo.GetCurSel();
	if( idx != CB_ERR )
	{
		IGameObject* pGameObject = m_gameObjectCombo.GetObject( idx );
		GameObjectInfo& rObjectInfo = m_objectInfoMap[pGameObject->GetID()];

		AkVector pos = { (AkReal32)m_gameObjectPosX.GetPos(), 0, (AkReal32)m_gameObjectPosZ.GetPos() };
		AkVector front = { (AkReal32)cos((double)m_gameObjectOriAngle.GetPos() * (2 * s_pi) / 360.0), 0, (AkReal32)sin((double)m_gameObjectOriAngle.GetPos() * (2 * s_pi) / 360.0) };
		AkVector top = { 0, 1, 0 };
		rObjectInfo.m_position.Set(pos, front, top);

		CString csTemp;
		csTemp.Format( _T("%.0f"), rObjectInfo.m_position.Position().X );
		SetDlgItemText( IDC_GAME_OBJECT_POSX_STATIC, csTemp );

		csTemp.Format( _T("%.0f"), rObjectInfo.m_position.Position().Z );
		SetDlgItemText( IDC_GAME_OBJECT_POSZ_STATIC, csTemp );

		csTemp.Format( _T("%.2f"), rObjectInfo.m_position.OrientationFront().X );
		SetDlgItemText( IDC_GAME_OBJECT_ORIX_STATIC, csTemp );

		csTemp.Format( _T("%.2f"), rObjectInfo.m_position.OrientationFront().Z );
		SetDlgItemText( IDC_GAME_OBJECT_ORIZ_STATIC, csTemp );

		// Set the game object position
		m_pSoundFrame->SetPosition( pGameObject->GetID(), rObjectInfo.m_position );
	}
}

void CSFTestPositioningDlg::OnListenerPosOriChanged()
{
	int idx = m_listenerCombo.GetCurSel();
	if (idx != CB_ERR)
	{
		ListenerInfo& rListenerInfo = m_listenerInfoVector[idx];

		AkVector pos = { (AkReal32)m_listenerPosX.GetPos(), 0, (AkReal32)m_listenerPosZ.GetPos() };
		AkVector front = { (AkReal32)cos((double)m_listenerOriAngle.GetPos() * (2 * s_pi) / 360.0), 0, (AkReal32)sin((double)m_listenerOriAngle.GetPos() * (2 * s_pi) / 360.0) };
		AkVector top = { 0, 1, 0 };
		rListenerInfo.m_position.Set(pos, front, top);

		CString csTemp;
		csTemp.Format( _T("%.0f"), rListenerInfo.m_position.Position().X );
		SetDlgItemText( IDC_LISTENER_POSX_STATIC, csTemp );

		csTemp.Format( _T("%.0f"), rListenerInfo.m_position.Position().Z );
		SetDlgItemText( IDC_LISTENER_POSZ_STATIC, csTemp );

		csTemp.Format( _T("%.2f"), rListenerInfo.m_position.OrientationFront().X );
		SetDlgItemText( IDC_LISTENER_ORIX_STATIC, csTemp );

		csTemp.Format( _T("%.2f"), rListenerInfo.m_position.OrientationFront().Z );
		SetDlgItemText( IDC_LISTENER_ORIZ_STATIC, csTemp );

		// Set the listener position
		m_pSoundFrame->SetPosition(AkGameObjectID(idx), rListenerInfo.m_position);
	}
}

int CSFTestPositioningDlg::CalcAngle( const double& in_dblX, const double& in_dblZ )
{
	// Calc the angle of the orientation in the X - Z plane
	int angle = (int)( atan2( in_dblZ, in_dblX ) * ( 360.0 / ( 2 * s_pi ) ) ) ;
	if( angle < 0 )
		angle += 360;

	return angle;
}