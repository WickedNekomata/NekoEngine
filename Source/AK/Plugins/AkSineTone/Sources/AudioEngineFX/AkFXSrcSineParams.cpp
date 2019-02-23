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
// AkFXSrcSineParams.cpp
//
// Implements a simple sine tone source.
//
//////////////////////////////////////////////////////////////////////

#include "AkFXSrcSineParams.h"
#include <AK/Tools/Common/AkBankReadHelpers.h>

// Constructor.
CAkFxSrcSineParams::CAkFxSrcSineParams()
{

}

// Destructor.
CAkFxSrcSineParams::~CAkFxSrcSineParams()
{

}

// Copy constructor.
CAkFxSrcSineParams::CAkFxSrcSineParams( const CAkFxSrcSineParams & in_rCopy )
{
	*static_cast<AkSineFXParams*>(this) = static_cast<AkSineFXParams>(in_rCopy);	
	m_ParamChangeHandler.SetAllParamChanges();
}

// Create parameter node duplicate.
AK::IAkPluginParam * CAkFxSrcSineParams::Clone( AK::IAkPluginMemAlloc * in_pAllocator )
{
	return AK_PLUGIN_NEW( in_pAllocator, CAkFxSrcSineParams(*this) );
}

// Parameter node initialization.
AKRESULT CAkFxSrcSineParams::Init( 
								  AK::IAkPluginMemAlloc *	/*in_pAllocator*/,								   
								  const void *				in_pParamsBlock, 
								  AkUInt32					in_ulBlockSize 
								  )
{
    if ( in_ulBlockSize == 0)
    {
		// Init default parameters.
        fFrequency = 440.f;        // Frequency (in Hertz).
		fGain = AK_DBTOLIN( -12.f );             // Gain (in dB FS)
        fDuration = 1.0f;          // Duration if finite (secs). 
		uChannelMask = AK_SPEAKER_SETUP_MONO;
		m_ParamChangeHandler.SetAllParamChanges();
        return AK_Success;
    }
    return SetParamsBlock( in_pParamsBlock, in_ulBlockSize );
}

// Parameter node termination.
AKRESULT CAkFxSrcSineParams::Term( AK::IAkPluginMemAlloc * in_pAllocator )
{
	AK_PLUGIN_DELETE( in_pAllocator, this );
    return AK_Success;
}

// Set all shared parameters at once.
AKRESULT CAkFxSrcSineParams::SetParamsBlock( 
	const void * in_pParamsBlock, 
	AkUInt32 in_ulBlockSize
	)
{
	AKRESULT eResult = AK_Success;
	AkUInt8 * pParamsBlock = (AkUInt8 *)in_pParamsBlock;
	fFrequency = READBANKDATA( AkReal32, pParamsBlock, in_ulBlockSize );
	fGain = AK_DBTOLIN( READBANKDATA( AkReal32, pParamsBlock, in_ulBlockSize ) );
	fDuration = READBANKDATA( AkReal32, pParamsBlock, in_ulBlockSize );
	uChannelMask = READBANKDATA( AkUInt32, pParamsBlock, in_ulBlockSize );
	CHECKBANKDATASIZE( in_ulBlockSize, eResult );
	m_ParamChangeHandler.SetAllParamChanges();
    return eResult;
}

// Update single parameter.
AKRESULT CAkFxSrcSineParams::SetParam( 
									  AkPluginParamID	in_ParamID,
									  const void *		in_pValue, 
									  AkUInt32			/*in_ulParamSize*/
									  )
{
    // Set parameter value.
    switch ( in_ParamID )
    {
	case AK_SINE_FXPARAM_FREQ_ID:
		fFrequency = *(AkReal32*)in_pValue;
		m_ParamChangeHandler.SetParamChange( AK_SINE_FXPARAM_FREQ_ID );
		break;
	case AK_SINE_FXPARAM_GAIN_ID:
	{
		AkReal32 fValue = *(AkReal32*)in_pValue;
		fValue = AkClamp( fValue, -96.3f, 0.f );
		fGain = AK_DBTOLIN( fValue );
		m_ParamChangeHandler.SetParamChange( AK_SINE_FXPARAM_GAIN_ID );
		break;
	}
	case AK_SINE_FXPARAM_DURATION_ID:
	{
		AkReal32 fValue = *(AkReal32*)in_pValue;
		fDuration = AkClamp( fValue, 0.f, 3600.f );
		m_ParamChangeHandler.SetParamChange( AK_SINE_FXPARAM_DURATION_ID );
		break;
	}
	case AK_SINE_FXPARAM_CHANNELMASK_ID:
		uChannelMask = (AkChannelMask)*(AkUInt32*)in_pValue;
		m_ParamChangeHandler.SetParamChange( AK_SINE_FXPARAM_CHANNELMASK_ID );
		break;
	default:
		return AK_InvalidParameter;
	}

    return AK_Success;
}
