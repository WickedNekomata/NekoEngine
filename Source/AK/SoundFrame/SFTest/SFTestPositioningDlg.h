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

#include "SFTest.h"
#include "SFTestComboBox.h"
#include "SFTestEditControl.h"

#include <bitset>
#include <map>
#include <vector>

using namespace AK;
using namespace SoundFrame;

// Dialog used to test the positioning function in the Sound frame
class CSFTestPositioningDlg 
	: public CDialog
{
public:

	static const int MAX_NUM_LISTENER_CHECKBOX = 8;
	int IdcListnerList[MAX_NUM_LISTENER_CHECKBOX];

	CSFTestPositioningDlg(CWnd* pParent = NULL);	// standard constructor
	~CSFTestPositioningDlg();

	enum { IDD = IDD_EDIT_POSITION_DIALOG };

	void Init( ISoundFrame * in_pSoundFrame );

	void GetGameObjectPosition( AkGameObjectID in_gameObjectID, AkSoundPosition& out_rPosition );

	void UpdateGameObjectList();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnCbnSelchangeGameObjectCombo();
	afx_msg void OnCbnSelchangeListenerCombo();
	afx_msg void OnBnClickedListenerCheck();
	afx_msg void OnBnClickedEnableSpatialization();
	afx_msg void OnHScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar );
	afx_msg void OnKillFocusVolumeEdit();

	DECLARE_MESSAGE_MAP()

private:

	void OnGameObjectPosOriChanged();
	void OnListenerPosOriChanged();
	
	// Return the angle specified from the vector in degree (range from [0 to 360[)
	int CalcAngle( const double& in_dblX, const double& in_dblZ );

	struct GameObjectInfo
	{
		// Default constructor
		GameObjectInfo();
		
		AkSoundPosition m_position;
		AkGameObjectID m_activeListener[8];
		AkUInt32 m_uNumListeners;
	};

	struct ListenerInfo
	{
		// Default constructor
		ListenerInfo();

		AkListenerPosition m_position;
		bool m_bSpatialized;
		AkReal32 m_fVolumeOffsetFrontLeft;
		AkReal32 m_fVolumeOffsetFrontRight;
		AkReal32 m_fVolumeOffsetCenter;
		AkReal32 m_fVolumeOffsetRearLeft;
		AkReal32 m_fVolumeOffsetRearRight;
		AkReal32 m_fVolumeOffsetLfe;
	};

	void SetListenerSpatialization( AkUInt32 in_uiIndex, ListenerInfo & in_rListenerInfo );

	typedef std::map<AkGameObjectID, GameObjectInfo> GameObjectInfoMap;
	typedef std::vector<ListenerInfo> ListenerInfoVector;

	SFTestComboBox<IGameObject, IGameObjectList, AkGameObjectID> m_gameObjectCombo;
	CComboBox m_listenerCombo;

	CSliderCtrl m_gameObjectPosX;
	CSliderCtrl m_gameObjectPosZ;
	CSliderCtrl m_gameObjectOriAngle;

	CSliderCtrl m_listenerPosX;
	CSliderCtrl m_listenerPosZ;
	CSliderCtrl m_listenerOriAngle;

	CSFTestVolumeEdit m_volumeLeftEdit;
	CSFTestVolumeEdit m_volumeRightEdit;
	CSFTestVolumeEdit m_volumeCenterEdit;
	CSFTestVolumeEdit m_volumeLFEEdit;
	CSFTestVolumeEdit m_volumeRearLeftEdit;
	CSFTestVolumeEdit m_volumeRearRightEdit;

	ISoundFrame * m_pSoundFrame;

	// Maps to remember the value we set since they can't be gotten from the Sound Frame
	GameObjectInfoMap m_objectInfoMap;
	ListenerInfoVector m_listenerInfoVector;
};