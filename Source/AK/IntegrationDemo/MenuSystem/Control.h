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

// Control.h
/// \file 
/// Contains the declaration for the Control abstract base class.

#pragma once

#include <string>
#include "Drawing.h"
#include "UniversalInput.h"
using std::string;

class Page;


/// Structure to hold details about the event when a Control is triggered.
struct ControlEvent
{
	int iPlayerIndex;	///< - 1-based index of the player who caused the event to fire.
};



/// Defines a pointer to a Page class member function.
typedef void(Page::*PageMFP)( void*, ControlEvent* );


/// The Control class is the abstract base class of all the UI Controls used by the Menu system.
class Control
{
public:

	/// Constructor for the Control class. Accepts a reference to the Page object to which the control belongs.
	Control( 
		Page& in_pParentPage	///< - Pointer to the Page that the control belongs to
		);

	/// Destructor for the Control class.
	virtual ~Control(){};
	
	/// Updates the Control based on the input received.
	/// \sa UGBtnState
	virtual void Update( 
		const UniversalGamepad& in_Gamepad	///< - The input to react to
		) = 0;
	
	/// Draws the Control to the screen.
	/// \sa DrawStyle
	virtual void Draw( 
		DrawStyle in_eDrawStyle = DrawStyle_Control		///< - Drawstyle to use when drawing the Control
		) = 0;

	/// Called when the control goes into focus.
	virtual void OnFocus(){};

	/// Called when the control loses focus.
	virtual void OnLoseFocus(){};

	/// Sets the Control's delegate function to be called when the Control is activated.
	void SetDelegate( 
		PageMFP in_pDelegateFunction	///< - Pointer to a the delegated Page Class Member-function
		);
	
	/// Calls the delegated function, if one is set.
	void CallDelegate( 
		ControlEvent* in_pEvent			///< - Pointer to the generated event that will be passed to the delegate function
		) const;
	
	/// Sets the Control's label string.
	void SetLabel( 
		const char* in_szLabel			///< - Text to set as the Control's label
		);

	/// Sets the Control's X and Y position on the screen
	void SetPosition( 
		int in_iXPos,				///< - The X Position
		int in_iYPos				///< - The Y Position
		);

	void GetPosition(
		int& out_iXPos,
		int& out_iYPos )
	{
		out_iXPos =  m_iXPos;
		out_iYPos =  m_iYPos;
	}

	virtual bool OnPointerEvent( PointerEventType /*in_eType*/, int /*in_x*/, int /*in_y*/ ) { return true; }

protected:

	/// Pointer to the member-function delegated to the Control.
	PageMFP m_pDelegateFunc;

	/// Pointer to the Page on which the Control lies.
	Page* m_pParentPage;

	/// The Control's label text.
	string m_szLabel;

	/// The X Position of the Control.
	int m_iXPos;

	/// The Y Position of the Control.
	int m_iYPos;

private:

	/// Default Control class constructor. Private to remind the user to call the public
	/// Control constructor that accepts a pointer to a Page object as a parameter.
	Control(){};
};
