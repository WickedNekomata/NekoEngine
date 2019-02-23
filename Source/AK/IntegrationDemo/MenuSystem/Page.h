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

// Page.h
/// \file 
/// Contains the declaration of the Page abstract base class.

#pragma once

#include <string>
#include <vector>
#include <AK/SoundEngine/Common/AkTypes.h>
#include "Control.h"
#include "Drawing.h"
#include "UniversalInput.h"
#include "MenuControls.h"
using std::string;

class Menu;

/// The abstract Page class is the base class for all the individual pages of the Menu system.
/// It supplies basic Update and Draw methods for its subtypes, though these methods can be
/// overriden if desired to tailor the way the page looks and reacts to input.
class Page
{
public:
	/// Page class constructor.
	Page( 
		Menu& in_ParentMenu,		///< - Reference to the Menu object that the Page belongs to	
		string in_strPageTitle		///< - The Page's title
		);

	/// Page class destructor.
	virtual ~Page(){};

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
	
	/// \return true to stay on page, false to go back.
	virtual bool OnPointerEvent( PointerEventType in_eType, int in_x, int in_y );

	/// Arranges the controls and draws them as well as the page title to the screen.
	/// \note This method can be overriden in subtypes to tailor how the page looks.
	virtual void Draw();

	/// Returns whether an error has occured during the Page's processing or not.
	/// \return True if an error has occured and false otherwise.
	bool ErrorOccured();

	/// Returns the contents of the error message.
	/// \return The content of the error message (which is an empty string if no error occured).
	const char* RetrieveErrorMsg();

	/// Sets the Page's title.
	void SetTitle( const char* in_szTitle );

	/// Returns a constant pointer to the page's parent Menu object.
	const Menu* ParentMenu();

protected:

	/// Function which will initialize controls in subtypes of Page.
	virtual void InitControls() = 0;

	/// Sets the Page's error 
	virtual void SetErrorMessage( 
		const char* in_szError 
		);

	/// Sets file loading error
	void SetLoadFileErrorMessage( const char* fileName );

	/// Repositions the controls to fit the page.
	void ResetControlPositions();

	/// Displays a Help screen based on the contents of m_szHelp.
	void DisplayHelp();
	
	/// Index of the currently selected control.
	int m_iSelectedIndex;

	/// The page's title text.
	string m_szPageTitle;

	/// Error message of the previous error that occurred.
	string m_szError;

	/// String of text to display when the user asks for help.
	/// \note This string can be modified to provide custom help for your page.
	string m_szHelp;

	/// Pointer to the Menu object which contains the page.
	Menu* m_pParentMenu;

	/// Container for the controls found on the page.
	std::vector<Control*> m_Controls;

private:

	/// Hidden Page class contructor to remind the user to call the proper constructor.
	Page();

	/// Calculate spacing between controls
	void GetControlSpacing( int & out_iTopControl, int & out_iControlsDist );

	/// Sets the controls on the page to their default position based on the width and height of the menu.
	void PositionControls();

	/// Set to 'true' if the controls on the page need to have their locations rearranged.
	bool m_bResetControlPositions;
};
