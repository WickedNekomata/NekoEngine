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

// ButtonControl.h
/// \file 
/// Contains the declaration of the Button Control type.

#pragma once

#include "Control.h"


/// This class represents a simple button control on a page in the Menu system.
/// The button can be activated to trigger its delegate function.
class ButtonControl : public Control
{
public:
	
	/// ButtonControl class constructor. Accepts a reference to the Page which contains the control.
	ButtonControl( 
		Page& in_pParentPage	///< - Pointer to the Page that contains the button
		);

	/// Updates the button based on the given input. If the button is activated, calls the delegate function.
	virtual void Update( 
		const UniversalGamepad& in_Gamepad	///< - The input to react to
		);

	/// Draws the button to the screen with the desired DrawStyle.
	/// \sa DrawStyle
	virtual void Draw( 
		DrawStyle in_eDrawStyle = DrawStyle_Control		///< - The style with which to draw the button (defaults to "DrawStyle_Control")
		);

	/// Called when the control goes into focus.
	virtual void OnFocus();

	/// Called when the control loses focus.
	virtual void OnLoseFocus();

	virtual bool OnPointerEvent( PointerEventType in_eType, int in_x, int in_y );
};
