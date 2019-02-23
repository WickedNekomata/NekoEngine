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

#include "AkSinkXAudio.h"
#include <AK/SoundEngine/Common/AkSimd.h>
#include "SinkPluginParams.h"
#include <AK/Tools/Common/AkBankReadHelpers.h>


// Constructor/destructor.
SinkPluginParams::SinkPluginParams()
{
}

SinkPluginParams::~SinkPluginParams()
{
}

// Copy constructor.
SinkPluginParams::SinkPluginParams(const SinkPluginParams & in_rCopy)
{
	m_params = in_rCopy.m_params;
}

// Create duplicate.
AK::IAkPluginParam * SinkPluginParams::Clone(AK::IAkPluginMemAlloc * in_pAllocator)
{
	AKASSERT(in_pAllocator != NULL);
	return AK_PLUGIN_NEW(in_pAllocator, SinkPluginParams(*this));
}

// Init/Term.
AKRESULT SinkPluginParams::Init(AK::IAkPluginMemAlloc *	in_pAllocator,
	const void *			in_pParamsBlock,
	AkUInt32				in_ulBlockSize)
{
	if (in_ulBlockSize == 0)
	{
		// Init default parameters.
		m_params.fDirectGain = 1.f;
		return AK_Success;
	}

	return SetParamsBlock(in_pParamsBlock, in_ulBlockSize);
}

AKRESULT SinkPluginParams::Term(AK::IAkPluginMemAlloc * in_pAllocator)
{
	AKASSERT(in_pAllocator != NULL);

	AK_PLUGIN_DELETE(in_pAllocator, this);
	return AK_Success;
}

// Blob set.
AKRESULT SinkPluginParams::SetParamsBlock(const void * in_pParamsBlock,
	AkUInt32 in_ulBlockSize)
{
	AKRESULT eResult = AK_Success;

	AkUInt8 * pParamsBlock = (AkUInt8 *)in_pParamsBlock;

	m_params.fDirectGain = READBANKDATA(AkReal32, pParamsBlock, in_ulBlockSize);

	CHECKBANKDATASIZE(in_ulBlockSize, eResult);

	return eResult;
}

// Update one parameter.
AKRESULT SinkPluginParams::SetParam(AkPluginParamID in_ParamID,
	const void * in_pValue,
	AkUInt32 in_ulParamSize)
{
	AKASSERT(in_pValue != NULL);
	if (in_pValue == NULL)
	{
		return AK_InvalidParameter;
	}
	AKRESULT eResult = AK_Success;

	switch (in_ParamID)
	{
	case SINK_DIRECT_GAIN_PROP:
		m_params.fDirectGain = (*(AkReal32*)(in_pValue));
		m_params.fDirectGain = AkClamp( m_params.fDirectGain, 0.f, 1.f );
		break;

	default:
		AKASSERT(!"Invalid parameter.");
		eResult = AK_InvalidParameter;
	}

	return eResult;
}
