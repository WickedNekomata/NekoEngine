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

// ToggleControl.h
/// \file 
/// Contains the declaration of the Toggle Control type.

#pragma once

#include <vector>
#include "Control.h"


/// The ToggleControl class is a simple control that allows the user to select an option from a list.
/// The available options contain a string of text that is displayed to the user as well as an underlying
/// value which can be used by the programmer to associate an option with, for example, a unique identifier.
/// The control calls the delegate function whenever the selected option changes.
class ToggleControl : public Control
{
public:

	/// ToggleControl class contructor. Accepts a reference to the Page which contains the control.
	ToggleControl( 
		Page& in_pParentPage	///< - Pointer to the Page that contains the button
		);

	/// ToggleControl class destructor.
	virtual ~ToggleControl();

	/// Updates the button based on the given input. If the selected value changes, the delegate function is called.
	virtual void Update( 
		const UniversalGamepad& in_Gamepad	///< - The input to react to
		);

	/// Draws the button to the screen with the desired DrawStyle.
	/// \sa DrawStyle
	virtual void Draw( 
		DrawStyle in_eDrawStyle = DrawStyle_Control		///< - The style with which to draw the control (defaults to "DrawStyle_Control")
		);

	/// Called when the control goes into focus.
	virtual void OnFocus();

	/// Called when the control loses focus.
	virtual void OnLoseFocus();

	/// Adds an option to the list of options that can be chosen using the toggle control.
	void AddOption( 
		const char* in_szText,		///< - The string of text displayed to the user as the selected option
		void* in_pValue	= NULL			///< - An underlying numerical value associated with the particular option
		);

	/// Accessor to the index of the currently selected option.
	/// \return The index of the currently selected option.
	int SelectedIndex() const;

	/// Accessor to the display text of the currently selected option.
	/// \return The text displayed to the user of the currently selected option. If no options exist, returns an empty string.
	/// \note If this method is called while no options exist, it will return an empty string.
	const char* SelectedText() const;

	/// Accessor to the underlying value of the currently selected option.
	/// \return The underlying value of the currently selected option. If no options exist, returns NULL.
	/// \note If this method is called while no options exist, it will return a value of NULL.
	void* SelectedValue() const;

private:
	
	/// Represents a single option available in the control
	struct OptionNode
	{
		/// The text string displayed to the user.
		string text;

		/// The underlying value associated with the option.
		void* value;
	};

	/// Index of the currently selected option.
	int m_iSelectedIndex;

	/// Container to hold the options available to choose with the control.
	std::vector<OptionNode> m_Options;
};
