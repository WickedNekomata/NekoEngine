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
// AkToneGenParams.cpp
//
// Shared parameter implementation for tone generator.
//
//////////////////////////////////////////////////////////////////////

#include "AkToneGenParams.h"
#include <AK/Tools/Common/AkBankReadHelpers.h>


// Constructor.
CAkToneGenParams::CAkToneGenParams()
{

}

// Copy constructor.
CAkToneGenParams::CAkToneGenParams( const CAkToneGenParams & in_rCopy )
{
	m_Params = in_rCopy.m_Params;	 
}

// Destructor.
CAkToneGenParams::~CAkToneGenParams()
{

}

// Create parameter node duplicate.
AK::IAkPluginParam * CAkToneGenParams::Clone( AK::IAkPluginMemAlloc * in_pAllocator )
{
	return AK_PLUGIN_NEW( in_pAllocator, CAkToneGenParams(*this) );	 
}

// Parameters node initialization.
AKRESULT CAkToneGenParams::Init( AK::IAkPluginMemAlloc *	/*in_pAllocator*/,								 
								 const void *				in_pParamsBlock, 
								 AkUInt32					in_uBlockSize 
								)
{
	if ( in_uBlockSize == 0)
	{
		// Init with default values if we got invalid parameter block.
		// RTPCed
		m_Params.fGain = 0.f;
		m_Params.fStartFreq = 440.f;
		m_Params.fStopFreq = 0.f;
		// Static		
		m_Params.staticParams.fStartFreqRandMin = 0.f;
		m_Params.staticParams.fStartFreqRandMax = 0.f;
		m_Params.staticParams.bFreqSweep = false;
		m_Params.staticParams.eGenSweep = AKTONEGENSWEEP_LIN;	
		m_Params.staticParams.fStopFreqRandMin = 0.f;
		m_Params.staticParams.fStopFreqRandMax = 0.f;
		m_Params.staticParams.eGenType = AKTONEGENTYPE_SINE;
		m_Params.staticParams.eGenMode = AKTONEGENMODE_FIX;
		m_Params.staticParams.fFixDur = 1.f;
		m_Params.staticParams.fAttackDur = 0.f;
		m_Params.staticParams.fDecayDur = 0.f;
		m_Params.staticParams.fReleaseDur = 0.f;
		m_Params.staticParams.fSustainDur = 0.f;
		m_Params.staticParams.fSustainVal = 0.f;
		m_Params.staticParams.uChannelMask = AK_SPEAKER_SETUP_MONO;
		
		return AK_Success;
	}

	return SetParamsBlock( in_pParamsBlock, in_uBlockSize );
}

// Parameters node termination.
AKRESULT CAkToneGenParams::Term( AK::IAkPluginMemAlloc * in_pAllocator )
{
	AK_PLUGIN_DELETE( in_pAllocator, this );
	return AK_Success;
}

// Set all shared parameters at once.
AKRESULT CAkToneGenParams::SetParamsBlock( const void * in_pParamsBlock, 
										  AkUInt32 in_ulBlockSize
										  )
{
	AKRESULT eResult = AK_Success;
	AkUInt8 * pParamsBlock = (AkUInt8 *)in_pParamsBlock;
	m_Params.fGain = READBANKDATA( AkReal32, pParamsBlock, in_ulBlockSize );
	m_Params.fStartFreq = READBANKDATA( AkReal32, pParamsBlock, in_ulBlockSize );
	m_Params.fStopFreq = READBANKDATA( AkReal32, pParamsBlock, in_ulBlockSize );
	m_Params.staticParams.fStartFreqRandMin = READBANKDATA( AkReal32, pParamsBlock, in_ulBlockSize );
	m_Params.staticParams.fStartFreqRandMax = READBANKDATA( AkReal32, pParamsBlock, in_ulBlockSize );
	m_Params.staticParams.bFreqSweep = READBANKDATA( bool, pParamsBlock, in_ulBlockSize );
	m_Params.staticParams.eGenSweep = (AkToneGenSweep) READBANKDATA( AkInt32, pParamsBlock, in_ulBlockSize );
	m_Params.staticParams.fStopFreqRandMin = READBANKDATA( AkReal32, pParamsBlock, in_ulBlockSize );
	m_Params.staticParams.fStopFreqRandMax = READBANKDATA( AkReal32, pParamsBlock, in_ulBlockSize );
	m_Params.staticParams.eGenType = (AkToneGenType) READBANKDATA( AkInt32, pParamsBlock, in_ulBlockSize );
	m_Params.staticParams.eGenMode = (AkToneGenMode) READBANKDATA( AkInt32, pParamsBlock, in_ulBlockSize );
	m_Params.staticParams.fFixDur = READBANKDATA( AkReal32, pParamsBlock, in_ulBlockSize );
	m_Params.staticParams.fAttackDur = READBANKDATA( AkReal32, pParamsBlock, in_ulBlockSize );
	m_Params.staticParams.fDecayDur = READBANKDATA( AkReal32, pParamsBlock, in_ulBlockSize );
	m_Params.staticParams.fSustainDur = READBANKDATA( AkReal32, pParamsBlock, in_ulBlockSize );
	m_Params.staticParams.fSustainVal = READBANKDATA( AkReal32, pParamsBlock, in_ulBlockSize );
	m_Params.staticParams.fReleaseDur = READBANKDATA( AkReal32, pParamsBlock, in_ulBlockSize );
	m_Params.staticParams.uChannelMask = READBANKDATA( AkUInt32, pParamsBlock, in_ulBlockSize );
	CHECKBANKDATASIZE( in_ulBlockSize, eResult );
    return eResult;
}

// Update single parameter.
AKRESULT CAkToneGenParams::SetParam(	AkPluginParamID in_ParamID,
										const void *	in_pValue, 
										AkUInt32		/*in_uParamSize*/
									)
{
	// Consistency check.
	if ( in_pValue == NULL )
	{
		return AK_InvalidParameter;
	}

	// Set parameter value.
	switch ( in_ParamID )
	{
	// This parameter is RTPCed
	case AK_TONEGEN_FXPARAM_GAIN_ID:
		m_Params.fGain = *reinterpret_cast<const AkReal32*>(in_pValue);
		m_Params.fGain = AkClamp( m_Params.fGain, -96.3f, 0.f );
		break;
	// This parameter is RTPCed
	case AK_TONEGEN_FXPARAM_STARTFREQ_ID:
		m_Params.fStartFreq = *reinterpret_cast<const AkReal32*>(in_pValue);
		break;
	// This parameter is RTPCed
	case AK_TONEGEN_FXPARAM_STOPFREQ_ID:
		m_Params.fStopFreq = *reinterpret_cast<const AkReal32*>(in_pValue);
		break;
	case AK_TONEGEN_FXPARAM_STARTFREQMIN_ID:
		m_Params.staticParams.fStartFreqRandMin = *reinterpret_cast<const AkReal32*>(in_pValue);
		break;
	case AK_TONEGEN_FXPARAM_STARTFREQMAX_ID:
		m_Params.staticParams.fStartFreqRandMax = *reinterpret_cast<const AkReal32*>(in_pValue);
		break;
	case AK_TONEGEN_FXPARAM_STOPFREQMIN_ID:
		m_Params.staticParams.fStopFreqRandMin = *reinterpret_cast<const AkReal32*>(in_pValue);
		break;
	case AK_TONEGEN_FXPARAM_STOPFREQMAX_ID:
		m_Params.staticParams.fStopFreqRandMax = *reinterpret_cast<const AkReal32*>(in_pValue);
		break;
	case AK_TONEGEN_FXPARAM_FREQSWEEP_ID:	
		m_Params.staticParams.bFreqSweep = *reinterpret_cast<const bool*>(in_pValue);
		break;
	case AK_TONEGEN_FXPARAM_SWEEPTYPE_ID:
		m_Params.staticParams.eGenSweep = (AkToneGenSweep) *reinterpret_cast<const AkInt32*>(in_pValue);
		break;
	case AK_TONEGEN_FXPARAM_WAVETYPE_ID:
		m_Params.staticParams.eGenType = (AkToneGenType) *reinterpret_cast<const AkInt32*>(in_pValue);
		break;
	case AK_TONEGEN_FXPARAM_DURMODE_ID:	
		m_Params.staticParams.eGenMode = (AkToneGenMode) *reinterpret_cast<const AkInt32*>(in_pValue);
		break;
	case AK_TONEGEN_FXPARAM_FIXDUR_ID:
		m_Params.staticParams.fFixDur = *reinterpret_cast<const AkReal32*>(in_pValue);
		break;
	case AK_TONEGEN_FXPARAM_ATTACKDUR_ID:
		m_Params.staticParams.fAttackDur = *reinterpret_cast<const AkReal32*>(in_pValue);
		break;
	case AK_TONEGEN_FXPARAM_DECAYDUR_ID:
		m_Params.staticParams.fDecayDur = *reinterpret_cast<const AkReal32*>(in_pValue);
		break;
	case AK_TONEGEN_FXPARAM_SUSTAINDUR_ID:
		m_Params.staticParams.fSustainDur = *reinterpret_cast<const AkReal32*>(in_pValue);
		break;
	case AK_TONEGEN_FXPARAM_SUSTAINVAL_ID:
		m_Params.staticParams.fSustainVal = *reinterpret_cast<const AkReal32*>(in_pValue);
		break;
	case AK_TONEGEN_FXPARAM_RELEASEDUR_ID:
		m_Params.staticParams.fReleaseDur = *reinterpret_cast<const AkReal32*>(in_pValue);
		break;
	case AK_TONEGEN_FXPARAM_CHANNELMASK_ID:
		m_Params.staticParams.uChannelMask = *reinterpret_cast<const AkUInt32*>(in_pValue);
		break;
	default:
		AKASSERT( !"Unknown parameter" );
		break;
	}

	return AK_Success;
}
