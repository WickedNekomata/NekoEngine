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
// AkFXSrcAudioInputParams.h
//
// Allows for audio source to come from an external input.
// 
// Note: Target output format is currently determined by the source itself.
//
//////////////////////////////////////////////////////////////////////

#ifndef _AKFXSRC_AUDIOINPUTPARAMS_H_
#define _AKFXSRC_AUDIOINPUTPARAMS_H_

#include <AK/Tools/Common/AkAssert.h>
#include <math.h>
#include <AK/SoundEngine/Common/IAkPlugin.h>

// Parameters IDs.
const AkPluginParamID AK_SRCAUDIOINPUT_FXPARAM_GAIN_ID		= 1;

// Parameter range values
const AkReal32 INPUT_GAIN_MIN			= -96.3f;	// dB FS
const AkReal32 INPUT_GAIN_MAX			= 0.f;		// db FS

// Parameters struture for this effect.
struct AkFXSrcAudioInputParams
{
	AkReal32     fGain;         // Gain (in dBFS).
};


//-----------------------------------------------------------------------------
// Name: class CAkFXSrcAudioInputParams
// Desc: Sample implementation the audio input source shared parameters.
//-----------------------------------------------------------------------------
class CAkFxSrcAudioInputParams : public AK::IAkPluginParam
{
public:

	// Allow effect to call accessor functions for retrieving parameter values.
	friend class CAkFXSrcAudioInput;

    // Constructor.
    CAkFxSrcAudioInputParams();
	
	// Copy constructor.
    CAkFxSrcAudioInputParams( const CAkFxSrcAudioInputParams & in_rCopy );

	// Destructor.
    virtual ~CAkFxSrcAudioInputParams();

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

private:

    AkUInt32	GetInput();
    AkReal32	GetGain();

private:

    // Parameter structure.
    AkFXSrcAudioInputParams	m_Params;
};

// Safely retrieve gain.
inline AkReal32 CAkFxSrcAudioInputParams::GetGain( )
{
    AkReal32 fGain = m_Params.fGain;
	AKASSERT( fGain >= INPUT_GAIN_MIN && fGain <= INPUT_GAIN_MAX );
	fGain = powf( 10.f, ( fGain / 20.f ) ); // Make it a linear value	
    return fGain;
}

#endif // _AKFXSRC_AUDIOINPUTPARAMS_H_
