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

// DemoPositioningMenu.cpp
/// \file 
/// Defines the methods declared in DemoPositioningMenu.h.

#include "stdafx.h"

#include <AK/SoundEngine/Common/AkSoundEngine.h>    // Sound engine

#include "Menu.h"
#include "DemoPositioningMenu.h"
#include "DemoPositioning.h"
#include "Demo3dBus.h"
#include "DemoSpatialAudio.h"


/////////////////////////////////////////////////////////////////////
// DemoDialogueMenu Public Methods
/////////////////////////////////////////////////////////////////////

DemoPositioningMenu::DemoPositioningMenu(Menu& in_ParentMenu) :Page(in_ParentMenu, "Positioning Demos Menu")
{

}


/////////////////////////////////////////////////////////////////////
// DemoPositioningMenu Private Methods
/////////////////////////////////////////////////////////////////////

void DemoPositioningMenu::InitControls()
{
	ButtonControl* newBtn;

	// Add button to open the localization demo page
	newBtn = new ButtonControl( *this );
	newBtn->SetLabel( "Position Demo" );
	newBtn->SetDelegate((PageMFP)&DemoPositioningMenu::SinglePosition_Pressed);
	m_Controls.push_back( newBtn );

	// Add button to open the dynamic dialogue demo page
	newBtn = new ButtonControl( *this );
	newBtn->SetLabel( "Multi-Position Demo" );
	newBtn->SetDelegate((PageMFP)&DemoPositioningMenu::Multiposition_Pressed);
	m_Controls.push_back(newBtn);

	newBtn = new ButtonControl(*this);
	newBtn->SetLabel("Spatial Audio - Portals");
	newBtn->SetDelegate((PageMFP)&DemoPositioningMenu::Scenario_Portals_Pressed);
	m_Controls.push_back(newBtn);

	newBtn = new ButtonControl(*this);
	newBtn->SetLabel("3d Bus - Clustering/3D Submix");
	newBtn->SetDelegate((PageMFP)&DemoPositioningMenu::Scenario_3dSubmix_Pressed);
	m_Controls.push_back(newBtn);

	newBtn = new ButtonControl(*this);
	newBtn->SetLabel("3d Bus - 3D Portal and Standard Room");
	newBtn->SetDelegate((PageMFP)&DemoPositioningMenu::Scenario_CoupledRoomsSimple_Pressed);
	m_Controls.push_back(newBtn);

	newBtn = new ButtonControl(*this);
	newBtn->SetLabel("3d Bus - 2X 3D Portals");
	newBtn->SetDelegate((PageMFP)&DemoPositioningMenu::Scenario_CoupledRoomsWithFeedback_Pressed);
	m_Controls.push_back(newBtn);
}

void DemoPositioningMenu::SinglePosition_Pressed(void*, ControlEvent*)
{
	DemoPositioning* pg = new DemoPositioning(*m_pParentMenu, false);
	m_pParentMenu->StackPage( pg );
}

void DemoPositioningMenu::Multiposition_Pressed(void*, ControlEvent*)
{
	DemoPositioning* pg = new DemoPositioning(*m_pParentMenu, true);
	m_pParentMenu->StackPage( pg );
}

void DemoPositioningMenu::Scenario_Portals_Pressed(void* in_pSender, ControlEvent* in_pEvent)
{
	DemoSpatialAudio* pg = new DemoSpatialAudio(*m_pParentMenu, Scenario_Portals);
	m_pParentMenu->StackPage(pg);
}

void DemoPositioningMenu::Scenario_CoupledRoomsSimple_Pressed(void* in_pSender, ControlEvent* in_pEvent)
{
	Demo3dBus* pg = new Demo3dBus(*m_pParentMenu, Scenario_CoupledRoomsSimple);
	m_pParentMenu->StackPage(pg);
}

void DemoPositioningMenu::Scenario_CoupledRoomsWithFeedback_Pressed(void* in_pSender, ControlEvent* in_pEvent)
{
	Demo3dBus* pg = new Demo3dBus(*m_pParentMenu, Scenario_CoupledRoomsWithFeedback);
	m_pParentMenu->StackPage(pg);
}

void DemoPositioningMenu::Scenario_3dSubmix_Pressed(void* in_pSender, ControlEvent* in_pEvent)
{
	Demo3dBus* pg = new Demo3dBus(*m_pParentMenu, Scenario_3dSubmix);
	m_pParentMenu->StackPage(pg);
}

