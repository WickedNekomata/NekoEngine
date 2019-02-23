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

// MultiplayerPage.h
/// \file
/// Contains the declaration of the MutliplayerPage abstract base class.

#pragma once

#include "Page.h"

/// The abstract MultiplayerPage class is a specialized version of the Page base class that
/// is designed to support up to 4 players who each have their own controls. Like the Page
/// class, basic Update and Draw methods are supplied which can be overriden if desired.
class MultiplayerPage : public Page
{
public:

	/// MultiplayerPage class constructor.
	MultiplayerPage( 
		Menu& in_ParentMenu,		///< - Reference to the Menu object that the Page belongs to
		string in_strPageTitle		///< - The Page's title
		);

	/// Initializes the Page.
	/// \return True if initialization succeeded and False otherwise.
	/// \warning Child classes that override this method MUST make a call to this method in their override.
	/// \note If returning false, remember to call SetErrorMessage() otherwise a blank error message page will be displayed.
	virtual bool Init();

	/// Releases all resources used by the page.
	/// \warning Child classes that override this method should make a call to this method in order to avoid memory leaks.
	virtual void Release();

	/// Updates the page based on input and updates the currently selected control.
	/// \return False if the page needs to be closed (if an error occured during processing, for example) and True otherwise.
	/// \note This method can be overriden in subtypes to tailor how the page reacts.
	/// \note If returning false due to an error, remember to call SetErrorMessage() first otherwise no error message will be displayed.
	virtual bool Update();
	
	/// Arranges the controls and draws them as well as the page title to the screen.
	/// \note This method can be overriden in subtypes to tailor how the page looks.
	virtual void Draw();

protected:

	/// Function which will initialize controls in subtypes of Page.
	virtual void InitControls() = 0;

	/// Repositions the controls to fit the page.
	void ResetControlPositions();

	/// Called when a player is connected.
	virtual void OnPlayerConnect( int in_iPlayerIndex ) = 0;

	/// Called when a player disconnects.
	virtual void OnPlayerDisconnect( int in_iPlayerIndex ) = 0;

	/// The maximum number of players supported at one time.
	static const int MAX_PLAYERS = 4;

	/// The selected control index for each player.
	AkUInt8 m_iSelectedIndex[MAX_PLAYERS];

	/// Containers for the controls found on the page.
	std::vector<Control*> m_Controls[MAX_PLAYERS];

	/// Whether each player is connected and playing or not.
	bool m_bPlayerConnected[MAX_PLAYERS];

private:

	/// Sets the controls on the page to their default position based on the width and height of the menu.
	void PositionControls();

	/// Set to 'true' if the controls on the page need to have their locations rearranged.
	bool m_bResetControlPositions;
};
