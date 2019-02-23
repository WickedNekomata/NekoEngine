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

#include "afxwin.h"

// Edit control class that validate the input characters to prevent invalid characters
class CSFTestVolumeEdit : public CEdit
{
public:
	CSFTestVolumeEdit();

	double GetVolume();
	void SetVolume( const double& in_rVolume );

protected:

	DECLARE_MESSAGE_MAP()

	afx_msg BOOL OnEnKillfocus();
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);

private:
	void UpdateText();
	bool ValidateAndUpdate();

	bool IsValidChar( UINT in_nChar );

	double m_dblVolume;
};