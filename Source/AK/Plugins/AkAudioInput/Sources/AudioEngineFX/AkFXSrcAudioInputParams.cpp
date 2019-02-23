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
// AkFXSrcAudioInputParams.cpp
//
// Allows for audio source to come from an external input.
//////////////////////////////////////////////////////////////////////

#include "AkFXSrcAudioInputParams.h"
#include <AK/Tools/Common/AkBankReadHelpers.h>
#if defined(AK_APPLE)
#include <string.h>
#endif

// Constructor.
CAkFxSrcAudioInputParams::CAkFxSrcAudioInputParams()
{

}

// Destructor.
CAkFxSrcAudioInputParams::~CAkFxSrcAudioInputParams()
{

}

// Copy constructor.
CAkFxSrcAudioInputParams::CAkFxSrcAudioInputParams( const CAkFxSrcAudioInputParams & in_rCopy )
{
	m_Params = in_rCopy.m_Params;	 
}

// Create parameters node duplicate.
AK::IAkPluginParam * CAkFxSrcAudioInputParams::Clone( AK::IAkPluginMemAlloc * in_pAllocator )
{
	return AK_PLUGIN_NEW( in_pAllocator, CAkFxSrcAudioInputParams(*this) );
}

// Shared parameters initialization.
AKRESULT CAkFxSrcAudioInputParams::Init( AK::IAkPluginMemAlloc *	in_pAllocator,								   
										 const void *				in_pParamsBlock, 
										 AkUInt32					in_ulBlockSize 
                                 )
{
    if ( in_ulBlockSize == 0)
    {
		// Init with default values if we got invalid parameter block.
        m_Params.fGain  = 0.f; // Gain (in dB FS)
        return AK_Success;
    }

    return SetParamsBlock( in_pParamsBlock, in_ulBlockSize );
}

// Parameter node termination.
AKRESULT CAkFxSrcAudioInputParams::Term( AK::IAkPluginMemAlloc * in_pAllocator )
{
	AK_PLUGIN_DELETE( in_pAllocator, this );
    return AK_Success;
}

// Set all shared parameters at once.
AKRESULT CAkFxSrcAudioInputParams::SetParamsBlock( const void * in_pParamsBlock, 
												   AkUInt32 in_ulBlockSize
                                           )
{
	AKRESULT eResult = AK_Success;
	AkUInt8 * pParamsBlock = (AkUInt8 *)in_pParamsBlock;
	m_Params.fGain = READBANKDATA( AkReal32, pParamsBlock, in_ulBlockSize );
	CHECKBANKDATASIZE( in_ulBlockSize, eResult );
    return eResult;
}

// Update single parameter.
AKRESULT CAkFxSrcAudioInputParams::SetParam( AkPluginParamID	in_ParamID,
											 const void *		in_pValue, 
											 AkUInt32			in_ulParamSize
                                     )
{
	// Consistency check.
	if ( in_pValue == NULL )
		return AK_InvalidParameter;

    // Set parameter value.
    switch ( in_ParamID )
    {
	case AK_SRCAUDIOINPUT_FXPARAM_GAIN_ID:
	{
		AkReal32 fValue = *reinterpret_cast<const AkReal32*>(in_pValue);
		m_Params.fGain = AkClamp( fValue, -96.3f, 0.f );
		break;
	}
	default:
		return AK_InvalidParameter;
	}

    return AK_Success;
}
