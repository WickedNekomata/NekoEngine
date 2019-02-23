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

#pragma once
#include "Control.h"


/// This control moves around in the screen using a joystick
class MovableChip : public Control
{
public:
	MovableChip(Page& in_pParentPage);
	virtual ~MovableChip();

	/// Updates the Control based on the input received.
	/// \sa UGBtnState
	virtual void Update( const UniversalGamepad& in_Gamepad	);

	/// Draws the Control to the screen.
	/// \sa DrawStyle
	virtual void Draw( DrawStyle in_eDrawStyle = DrawStyle_Control );

	void UseJoystick(UGThumbstick in_StickToUse);

	void GetPos(float &x, float &y) {x = m_x; y = m_y;}
	void SetPos(float x, float y) {m_x = x; m_y = y;}
	void SetMaxSpeed(float in_fSpeed) {m_fMaxSpeed = in_fSpeed;}
	void SetNonLinear() {m_bNonLinear = true;}

	int GetRightBottomMargin() const;

private:
	UGThumbstick m_Stick;
	float m_fMaxSpeed;
	float m_x;
	float m_y;
	bool m_bNonLinear;
};
