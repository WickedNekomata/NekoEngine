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
// AkFXSrcSineParams.h
//
// Sample physical modelling source. Implements a simple sine tone source.
// 
// Note: Target output format is currently determined by the source itself.
//
//////////////////////////////////////////////////////////////////////

#ifndef _AKFXSRC_SINEPARAMS_H_
#define _AKFXSRC_SINEPARAMS_H_

#include <AK/Tools/Common/AkAssert.h>
#include <AK/Plugin/PluginServices/AkFXParameterChangeHandler.h>
#include <math.h>
#include <AK/SoundEngine/Common/IAkPlugin.h>

// Parameters IDs.
static const AkPluginParamID AK_SINE_FXPARAM_FREQ_ID			= 0;
static const AkPluginParamID AK_SINE_FXPARAM_GAIN_ID			= 1;
static const AkPluginParamID AK_SINE_FXPARAM_DURATION_ID		= 2;
static const AkPluginParamID AK_SINE_FXPARAM_CHANNELMASK_ID		= 3;
static const AkUInt32		 AK_SINE_NUMFXPARAMS				= 4;

// Parameters struture for this effect.
struct AkSineFXParams
{
    AkReal32		fFrequency;    // Frequency (in Hertz).
    AkReal32		fGain;         // Gain (in dBFS).
    AkReal32		fDuration;		// Sustain duration (only valid if finite).
	AkChannelMask	uChannelMask;	// Output channel mask.
} AK_ALIGN_DMA;

class CAkFxSrcSineParams 
	: public AK::IAkPluginParam
	, public AkSineFXParams
{
public:

    // Constructor.
    CAkFxSrcSineParams();
	
	// Copy constructor.
    CAkFxSrcSineParams( const CAkFxSrcSineParams & in_rCopy );

	// Destructor.
    virtual ~CAkFxSrcSineParams();

    // Create parameter object duplicate.
	virtual AK::IAkPluginParam * Clone( AK::IAkPluginMemAlloc * in_pAllocator );

    // Initialization.
    virtual AKRESULT Init( AK::IAkPluginMemAlloc *	in_pAllocator,		// Memory allocator.						    
                   const void *				in_pParamsBlock,	// Pointer to param block.
                   AkUInt32					in_ulBlockSize		// Sise of the parm block.
                   );
   
	// Termination.
	virtual AKRESULT Term( AK::IAkPluginMemAlloc * in_pAllocator );

    // Set all parameters at once.
    virtual AKRESULT SetParamsBlock( const void * in_pParamsBlock, 
                             AkUInt32 in_ulBlockSize
                             );

    // Update one parameter.
    virtual AKRESULT SetParam( AkPluginParamID in_ParamID,
                       const void * in_pValue, 
                       AkUInt32 in_ulParamSize
                       );
public:

	AK::AkFXParameterChangeHandler<AK_SINE_NUMFXPARAMS> m_ParamChangeHandler;
};

#endif // _AKFXSRC_SINEPARAMS_H_
