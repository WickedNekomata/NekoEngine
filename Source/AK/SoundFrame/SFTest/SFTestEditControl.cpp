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

#include "stdafx.h"
#include "SFTestEditControl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace
{
	static const double s_MaxValue = 0.0;
	static const double s_MinValue = -96.3;
}

CSFTestVolumeEdit::CSFTestVolumeEdit()
:	m_dblVolume( 0.0 )
{}

double CSFTestVolumeEdit::GetVolume()
{
	ValidateAndUpdate();

	return m_dblVolume;
}
	
void CSFTestVolumeEdit::SetVolume( const double& in_rVolume )
{
	m_dblVolume = in_rVolume;

	UpdateText();
}

BEGIN_MESSAGE_MAP(CSFTestVolumeEdit, CEdit)
    ON_CONTROL_REFLECT_EX(EN_KILLFOCUS, OnEnKillfocus)
    ON_WM_CHAR()
END_MESSAGE_MAP()

BOOL CSFTestVolumeEdit::OnEnKillfocus()
{
	ValidateAndUpdate();
	
	return FALSE;
}
	
void CSFTestVolumeEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if ( nChar == VK_RETURN )
    {
		// For this to work, the Edit control must be created with Multiline and Want return style
		// In our case, those styles are set directly in the resources.
        if( GetParent() )
            GetParent()->SetFocus();

        SetSel(0, -1);
    }
    else if ( IsValidChar(nChar) )
	{
		// Let it pass.
		__super::OnChar(nChar, nRepCnt, nFlags);
	}
}

void CSFTestVolumeEdit::UpdateText()
{
	CString csText;
	csText.Format( _T("%.1f"), m_dblVolume );

	SetWindowText( csText );
}

bool CSFTestVolumeEdit::ValidateAndUpdate()
{
	double oldValue = m_dblVolume;

	CString csText;
	GetWindowText( csText );

	m_dblVolume = _tstof( csText );

	// Clamp the value
	m_dblVolume = min( m_dblVolume, s_MaxValue );
	m_dblVolume = max( m_dblVolume, s_MinValue );

	UpdateText();

	return oldValue != m_dblVolume;
}

bool CSFTestVolumeEdit::IsValidChar( UINT in_nChar )
{
	return isdigit(in_nChar) || in_nChar == _T('-') || in_nChar == _T('\b') || in_nChar == _T('\t') || in_nChar == _T('.');
}
