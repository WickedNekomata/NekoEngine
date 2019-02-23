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
// Demo3dBus.h
/// \file
/// Contains the declaration for the Demo3dBus class.

#pragma once

#include "Page.h"
#include "MovableChip.h"

enum Demo3dBus_Scenario
{
	Scenario_CoupledRoomsSimple,
	Scenario_CoupledRoomsWithFeedback,
	Scenario_3dSubmix
};

class Demo3dBus : public Page
{
public:

	/// Demo3dBus class constructor.
	Demo3dBus(
		Menu& in_pParentMenu,	///< - Pointer to the Menu that the page belongs to
		Demo3dBus_Scenario in_scenario
		);

	/// Override of the Parent's Init() method.
	virtual bool Init();

	/// Override of the Parent's Release() method.
	virtual void Release();

	/// Override of the Parent's Update() method.
	virtual bool Update();

	/// Override of the Parent's Draw() method.
	virtual void Draw();

	virtual bool OnPointerEvent( PointerEventType in_eType, int in_x, int in_y );

private:

	/// Initializes the controls on the page.
	virtual void InitControls();
	void InitControls_CoupledRoomsSimple();
	void InitControls_CoupledRoomsWithFeedback();
	void UpdateGameObjPos(MovableChip* in_pChip, AkGameObjectID in_GameObjectId);

	void Init_3dSubmix();
	void Init_CoupledRoomsWithFeedback();
	void Init_CoupledRoomsSimple();

	// Helpers

	float PixelsToAKPos_X(float in_X);
	float PixelsToAKPos_Y(float in_y);

	// Members

	MovableChip *m_pEmitterChip;
	MovableChip *m_pListenerChip;
	MovableChip *m_pRoom1Chip;
	MovableChip *m_pRoom2Chip;

	float m_fGameObjectX;
	float m_fGameObjectZ;
	float m_fWidth;
	float m_fHeight;
	bool m_bLooping;
	AkUInt8 m_uRepeat;

	Demo3dBus_Scenario m_eScenario;
};
