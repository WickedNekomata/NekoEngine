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

// DemoPositioningMenu.h
/// \file
/// Contains the declaration for the DemoDialogueMenu class.

#pragma once

#include "Page.h"

/// This class represents the Positioning Demos Menu page. This is simply a submenu to
/// link to various demonstrations involving dialogue.
class DemoPositioningMenu : public Page
{
public:

	/// DemoPositioningMenu class constructor
	DemoPositioningMenu(Menu& in_ParentMenu);

private:

	/// Initializes the controls on the page.
	virtual void InitControls();

	/// Delegate function for the "Localization Demo" button.
	void SinglePosition_Pressed( void* in_pSender, ControlEvent* in_pEvent );

	/// Delegate function for the "Dynamic Dialogue Demo" button.
	void Multiposition_Pressed( void* in_pSender, ControlEvent* in_pEvent );

	/// Delegate function for spatial audio demo button.
	void Scenario_Portals_Pressed(void* in_pSender, ControlEvent* in_pEvent);

	/// Delegate functions for 3D bus demos button.
	void Scenario_CoupledRoomsSimple_Pressed(void* in_pSender, ControlEvent* in_pEvent);
	void Scenario_CoupledRoomsWithFeedback_Pressed(void* in_pSender, ControlEvent* in_pEvent);
	void Scenario_3dSubmix_Pressed(void* in_pSender, ControlEvent* in_pEvent);
};
