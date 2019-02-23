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

// BaseMenuPage.h
/// \file 
/// Defines the Base page that is loaded by the menu system when it is created.

#pragma once

#include "Page.h"


/// This class defines the first page that the Menu system automatically loads
/// when the Menu class is instantiated.
class BaseMenuPage : public Page
{
public:

	/// BaseMenuPage class constructor
	BaseMenuPage( Menu& in_ParentMenu );

	/// Initializes the demo.
	/// \return True if successful and False otherwise.
	virtual bool Init();

	/// Releases resources used by the demo.
	virtual void Release();

	/// Override of the parent's update method.
	virtual bool Update();

private:

	/// Initializes the various components on the page.
	virtual void InitControls();

	/// Delegate function for the "Footsteps Demo" button.
	void OpenFootstepsDemoPage( void* in_pSender, ControlEvent* in_pEvent );

	/// Delegate function for the "RTPC Demo" button.
	void OpenRTPCDemoPage( void* in_pSender, ControlEvent* in_pEvent );

	/// Delegate function for the "Dialogue Demos" button.
	void OpenDialogueDemosPage( void* in_pSender, ControlEvent* in_pEvent );

	/// Delegate function for the "Markers Demo" button.
	void OpenMarkersDemoPage( void* in_pSender, ControlEvent* in_pEvent );

	/// Delegate function for the "Music Callbacks Demo" button.
	void OpenMusicCallbacksDemoPage( void* in_pSender, ControlEvent* in_pEvent );

	/// Delegate function for the " Interactive Music Demo" button.
	void OpenInteractiveMusicDemoPage( void* in_pSender, ControlEvent* in_pEvent );

	/// Delegate function for the " MIDI API Demo" button.
	void OpenMIDIMetronomeDemoPage( void* in_pSender, ControlEvent* in_pEvent );

	/// Delegate function for the "Motion Demo" button.
	void OpenMotionDemoPage( void* in_pSender, ControlEvent* in_pEvent );

	void OpenMicrophoneDemoPage( void* in_pSender, ControlEvent* in_pEvent );

	/// Delegate function for the "Positioning Demo" button.
	void OpenPositioningDemoPage( void* in_pSender, ControlEvent* in_pEvent );

	/// Delegate function for the "External Sources Demo" button.
	void OpenExternalSourcesDemoPage( void* in_pSender, ControlEvent* in_pEvent );

	/// Delegate function for the "BGM Demo" button.
	void OpenBGMDemoPage( void* in_pSender, ControlEvent* in_pEvent );

	/// Delegate function for the "Exit" button.
	void ExitButton_OnPress( void* in_pSender, ControlEvent* in_pEvent );
};
