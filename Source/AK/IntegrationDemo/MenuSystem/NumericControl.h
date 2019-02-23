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

// NumericControl.h
/// \file 
/// Contains the declaration of the Numeric Control type.

#pragma once

#include <AK/SoundEngine/Common/AkTypes.h>
#include "Control.h"


/// The NumericControl class is a numeric control on a page in the Menu system.
/// This control allows the user to view and modify a numeric value within an established range.
/// The function delegated to a control of this type is called when the control's value changes.
class NumericControl : public Control
{
public:
	
	/// NumericControl class constructor. Accepts a reference to the Page which contains the control.
	NumericControl( 
		Page& in_pParentPage 
		);

	/// Updates the button based on the given input. If the control's value changes, the delegate function is called.
	virtual void Update( 
		const UniversalGamepad& in_Gamepad		///< - The input to react to
		);

	/// Draws the control to the screen with the desired DrawStyle.
	/// \sa DrawStyle
	virtual void Draw( 
		DrawStyle in_eDrawStyle = DrawStyle_Control		///< - The style with which to draw the control (defaults to "DrawStyle_Control")
		);

	virtual bool OnPointerEvent( PointerEventType in_eType, int in_x, int in_y );

	/// Called when the control goes into focus.
	virtual void OnFocus();

	/// Called when the control loses focus.
	virtual void OnLoseFocus();

	/// Accessor to get the maximum value of the Numeric control.
	AkReal64 GetMaxValue() const;

	/// Accessor to get the minimum value of the Numeric control.
	AkReal64 GetMinValue() const;

	/// Accessor to get the value by which the Numeric control increments/decrements.
	AkReal64 GetIncrement() const;

	/// Accessor to get the current value of the Numeric control.
	AkReal64 GetValue() const;

	/// Accessor to set the maximum value of the Numeric control.
	/// \note If the control's value is out of the new range set, the value will be adjusted.
	/// \warning This method does not take into account the control's minimum value. Be careful not to assign a maximum value less than the minimum as this can result in unintended behavior.
	void SetMaxValue( 
		AkReal64 in_dblMax		///< - Value to set the control's maximum value to
		);

	/// Accessor to set the minimum value of the Numeric control.
	/// \note If the control's value is out of the new range set, the value will be adjusted.
	/// \warning This method does not take into account the control's maximum value. Be careful not to assign a minmum value greater than the maximum as this can result in unintended behavior.
	void SetMinValue( 
		AkReal64 in_dblMin		///< - Value to set the control's minimum value to
		);

	/// Used to set a different initial increment/decrement from the continuous increment.
	/// \note If no initial increment is set, the control will simply use the regular increment for the first increment.
	void SetInitialIncrement(
		AkReal64 in_dblInitialIncrement	/// - Value to set the control's initial increment to
		);

	/// Accessor to set the value by which the Numeric control increments/decrements.
	void SetIncrement( 
		AkReal64 in_dblIncrement	///< - Value to set the control's increment to
		);

	/// Accessor to set the current value of the Numeric control.
	/// \note If the value set is out of the range of the control's minimum and maximum value, the value set will be adjusted to be in range.
	void SetValue( 
		AkReal64 in_dblVal		///< - Value to set the control's current value to
		);

	/// Sets the control's hysteresis time. The hysteresis time creates a delay between when the
	/// control is triggered and when continuous key repeats (due to a button being held down)
	/// kicks in.
	void SetHysteresisTime( 
		AkInt32 in_iHTime		///< - Time until key repeat kicks in, in milliseconds
		);

private:

	/// Calculates the amount of time left (in frames) until hysteresis kicks in.
	void CalcHysteresisTimeLeft();

	/// Hysteresis time for the control's key repeat to kick in, in milliseconds.
	AkInt32 m_iHTime;

	/// The number of frames left until hysteresis kicks in.
	AkInt32 m_iHTimeLeft;

	/// Whether the control is in hysteresis or not.
	bool m_bInHysteresis;

	/// The index of the player who has control over this control. A user gains sole control
	/// over the numeric slider when he first changes the value and loses control when letting
	/// go of the left/right button. A value of -1 means no user has control.
	int m_iPlayerInControl;

	/// Maximum value of the Numeric control.
	AkReal64 m_dblMax;

	/// Minimum value of the Numeric control.
	AkReal64 m_dblMin;

	/// Value by which the control's value increments or decrements.
	AkReal64 m_dblIncrement;

	/// Optional value set by the user to have an initial increment different from the continuous increment.
	AkReal64 m_dblInitialIncrement;

	/// The current value of the Numeric control.
	AkReal64 m_dblValue;
};
