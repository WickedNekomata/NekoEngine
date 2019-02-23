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
//////////////////////////////////////////////////////////////////////
//
// AkLpFilter.h
//
// AudioKinetic 2nd order IIR Butterworth low pass filter class
//
//////////////////////////////////////////////////////////////////////

#include "AkLpFilter.h"
#include <math.h>

#define PI					(3.1415926535897932384626433832795f)
#define ROOTTWO				(1.4142135623730950488016887242097f)

CAkLpFilter::CAkLpFilter( )
{
	// Initialize filter coefficients
	m_fB0 = 0.f;
	m_fB1 = 0.f;
	m_fB2 = 0.f;
	m_fA1 = 0.f;
	m_fA2 = 0.f;

	Reset();
}

CAkLpFilter::~CAkLpFilter( )
{

}

void CAkLpFilter::SetCoefs( AkReal32 in_fCutFreq, AkReal32 in_fSampRate )
{

	// Butterworth Low-pass filter computations (pre-computed using Bilinear transform)
	AkReal32 PiFcOSr = PI * in_fCutFreq / in_fSampRate;
	AkReal32 fTanPiFcSr = tanf(PiFcOSr);
	AkReal32 fIntVal = 1.f / fTanPiFcSr;
	AkReal32 fRootTwoxIntVal = ROOTTWO * fIntVal;
	AkReal32 fSqIntVal = fIntVal * fIntVal;

	m_fB0 = (1.f / ( 1.f + fRootTwoxIntVal + fSqIntVal));
	m_fB1 = m_fB0 + m_fB0;
	m_fB2 = m_fB0;
	m_fA1 = 2.f * ( 1.f - fSqIntVal) * m_fB0;
	m_fA2 = ( 1.f - fRootTwoxIntVal + fSqIntVal) * m_fB0;
}

void CAkLpFilter::Reset( )
{
	m_fIn1 = 0.f;
	m_fIn2 = 0.f;
	m_fOut1 = 0.f;
	m_fOut2 = 0.f;
}
