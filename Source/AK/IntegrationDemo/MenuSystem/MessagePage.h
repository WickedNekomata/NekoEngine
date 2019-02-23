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

// MessagePage.h
/// \file
/// Contains the declaration for the MessagePage class.

#pragma once

#include "Page.h"

class MessagePage : public Page
{
public:

	/// MessagePage class constructor.
	MessagePage(
		Menu& in_pParentMenu	///< - Pointer to the Menu that the page belongs to
		);

	/// Override of the Parent's Update() method.
	virtual bool Update();

	/// Override of the Parent's Draw() method.
	virtual void Draw();

	/// Sets the error message to display.
	virtual void SetMessage( const char* in_szMessage, DrawStyle in_eDrawStyle = DrawStyle_Text );

private:

	/// Initializes the controls on the page.
	virtual void InitControls();

	/// The error message to be displayed.
	std::string m_szMessage;

	/// The message's style (text, error, etc..)
	DrawStyle m_MsgDrawStyle;
};
