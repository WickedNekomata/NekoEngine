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
// AkToneGenParams.h
//
// Shared parameter implementation for tone generator.
//
//////////////////////////////////////////////////////////////////////

#ifndef _AK_TONEGENPARAMS_H_
#define _AK_TONEGENPARAMS_H_

#include <AK/Tools/Common/AkAssert.h>
#include <math.h>
#include <AK/SoundEngine/Common/IAkPlugin.h>

// Parameters IDs for the Wwise or RTPC.
static const AkPluginParamID AK_TONEGEN_FXPARAM_GAIN_ID				= 0;	// RTPCable
static const AkPluginParamID AK_TONEGEN_FXPARAM_STARTFREQ_ID		= 1;	// RTPCable
static const AkPluginParamID AK_TONEGEN_FXPARAM_STARTFREQMIN_ID		= 2;
static const AkPluginParamID AK_TONEGEN_FXPARAM_STARTFREQMAX_ID		= 3;
static const AkPluginParamID AK_TONEGEN_FXPARAM_FREQSWEEP_ID		= 4;
static const AkPluginParamID AK_TONEGEN_FXPARAM_SWEEPTYPE_ID		= 5;
static const AkPluginParamID AK_TONEGEN_FXPARAM_STOPFREQ_ID			= 6;	// RTPCable
static const AkPluginParamID AK_TONEGEN_FXPARAM_STOPFREQMIN_ID		= 7;
static const AkPluginParamID AK_TONEGEN_FXPARAM_STOPFREQMAX_ID		= 8;
static const AkPluginParamID AK_TONEGEN_FXPARAM_WAVETYPE_ID			= 9;
static const AkPluginParamID AK_TONEGEN_FXPARAM_DURMODE_ID			= 10;
static const AkPluginParamID AK_TONEGEN_FXPARAM_FIXDUR_ID			= 11;
static const AkPluginParamID AK_TONEGEN_FXPARAM_ATTACKDUR_ID		= 12;
static const AkPluginParamID AK_TONEGEN_FXPARAM_DECAYDUR_ID			= 13;
static const AkPluginParamID AK_TONEGEN_FXPARAM_SUSTAINDUR_ID		= 14;
static const AkPluginParamID AK_TONEGEN_FXPARAM_SUSTAINVAL_ID		= 15;
static const AkPluginParamID AK_TONEGEN_FXPARAM_RELEASEDUR_ID		= 16;
static const AkPluginParamID AK_TONEGEN_FXPARAM_CHANNELMASK_ID		= 17;

static const AkPluginParamID AK_NUM_TONEGEN_FXPARAM					= 18;

// Tone generator waveform type
enum AkToneGenType
{
	AKTONEGENTYPE_SINE =		0,
	AKTONEGENTYPE_TRIANGLE =	1,
	AKTONEGENTYPE_SQUARE =		2,
	AKTONEGENTYPE_SAWTOOTH =	3,
	AKTONEGENTYPE_WHITENOISE =	4,
	AKTONEGENTYPE_PINKNOISE =	5
};

// Envelope control mode
enum AkToneGenMode
{
	AKTONEGENMODE_FIX = 0,	// Fixed duration	
	AKTONEGENMODE_ENV = 1	// Envelope controls
};

// Frequency sweeping mode
enum AkToneGenSweep
{
	AKTONEGENSWEEP_LIN =  0,	// Linear frequency sweep
	AKTONEGENSWEEP_LOG =  1		// Logarithmic frequency sweep
};

// Valid parameter ranges
#define TONEGEN_LEVEL_MIN			(-96.3f)
#define TONEGEN_LEVEL_MAX			(0.f)
#define TONEGEN_FREQUENCY_MIN		(0.001f)
#define TONEGEN_FREQUENCY_MAX		(20000.f)
#define TONEGEN_RANDMINFREQ_MIN		(-TONEGEN_FREQUENCY_MAX)
#define TONEGEN_RANDMINFREQ_MAX		(0.f)
#define TONEGEN_RANDMAXFREQ_MIN		(0.f)
#define TONEGEN_RANDMAXFREQ_MAX		(TONEGEN_FREQUENCY_MAX)
#define TONEGEN_SWEEPMODE_MIN		(AKTONEGENSWEEP_LIN)
#define TONEGEN_SWEEPMODE_MAX		(AKTONEGENSWEEP_LOG)
#define TONEGEN_ENVMODE_MIN			(AKTONEGENMODE_FIX)
#define TONEGEN_ENVMODE_MAX			(AKTONEGENMODE_ENV)
#define TONEGEN_GENTYPE_MIN			(AKTONEGENTYPE_SINE)
#define TONEGEN_GENTYPE_MAX			(AKTONEGENTYPE_PINKNOISE)
#define TONEGEN_FIXEDDURATION_MIN	(0.001f)
#define TONEGEN_DURATION_MIN		(0.0f)
#define TONEGEN_DURATION_MAX		(3600.f)


// Structure of parameters that remain true for the whole lifespan of the tone generator (1 playback).
struct AkToneGenStaticParams
{
	// Randomized minimum start frequency difference parameter (in Hertz) (negative).
	AkReal32		fStartFreqRandMin;
	// Randomized maximum start frequency difference parameter (in Hertz).
	AkReal32		fStartFreqRandMax;

	// The target frequency values are only defined for frequency sweeps (in Hertz).
	bool			bFreqSweep;
	// Sweep frequency type (linear sweep, or logarithmic sweep)
	AkToneGenSweep	eGenSweep;
	// Randomized minimum target frequency difference parameter (in Hertz) (negative).
	AkReal32		fStopFreqRandMin;
	// Randomized maximum target frequency difference parameter (in Hertz).
	AkReal32		fStopFreqRandMax;

	// Waveform or noise type
	AkToneGenType eGenType;

	// Fixed duration or envelope controls
	AkToneGenMode eGenMode;

	// In fixed duration mode only
	AkReal32		fFixDur;		// Duration (in secs)

	// The remaining parameters are only used with envelope controls
	AkReal32		fAttackDur;		// Attack time (in secs)
    AkReal32		fDecayDur;		// Decay time (in secs)
    AkReal32		fSustainDur;    // Sustain time (in secs)
	AkReal32		fSustainVal;	// Sustain value (in dB FS)
	AkReal32		fReleaseDur;	// Release time (in secs)

	// Channel out config
	AkChannelMask	uChannelMask;	// Output channel mask.
};

// Parameters struture for this effect.
struct AkToneGenParams
{
	// Tone generator gain. This parameter can be RTPCed.
	AkReal32	fGain;
	// Start frequency of the tone (in Hertz). This paramerter is used if a constant frequency is used. RTPCed.
    AkReal32     fStartFreq;  
	// Desired target frequency(in Hertz). RTPCed.
	AkReal32     fStopFreq; 

	// Parameters that remain true for the whole lifespan of the tone generator (not  RTPCed).
	AkToneGenStaticParams staticParams;
};


//-----------------------------------------------------------------------------
// Name: class CAkToneGenParam
// Desc: Implementation of tone generator's shared parameters.
//-----------------------------------------------------------------------------
class CAkToneGenParams : public AK::IAkPluginParam
{
public:

	// Allow effect to call accessor functions for retrieving parameter values.
	friend class CAkToneGen;
	
    // Constructor.
    CAkToneGenParams();

	// Copy constructor.
    CAkToneGenParams( const CAkToneGenParams & in_rCopy );

	// Destructor.
    virtual ~CAkToneGenParams();

    // Create duplicate.
    virtual IAkPluginParam * Clone( AK::IAkPluginMemAlloc * in_pAllocator );

    // Initialize.
    virtual AKRESULT Init(	AK::IAkPluginMemAlloc *	in_pAllocator,						   
					const void *			in_pParamsBlock, 
                    AkUInt32				in_uBlockSize 
					);
	// Terminate.
    virtual AKRESULT Term( AK::IAkPluginMemAlloc * in_pAllocator );

    // Set all parameters at once.
    virtual AKRESULT SetParamsBlock( const void * in_pParamsBlock,
                             AkUInt32 in_uBlockSize
                             );

    // Update one parameter.
    virtual AKRESULT SetParam( AkPluginParamID	in_ParamID,
                       const void *			in_pValue,
                       AkUInt32			in_uParamSize
                       );

private: 

	////// Tone generator internal API ///// 
	// Get a copy of static parameters (non RTPCed)
	void GetstaticParams( AkToneGenStaticParams * out_pstaticParams );

	// Safely retrieve current gain RTPC value (linear value).
	AkReal32 GetGain( );

	// Safely retrieve current start frequency RTPC value.
	AkReal32 GetStartFreq( );

	// Safely retrieve current stop frequency RTPC value.
	AkReal32 GetStopFreq( );

	// Hide (not implemented) assignment operator
	CAkToneGenParams & operator=( const CAkToneGenParams & in_rCopy );

private:

    // Parameter structure. (Includes static and RTPC parameters).
    AkToneGenParams m_Params;
};

// Retrieve a copy of parameters that cannot change during playback instance.
inline void CAkToneGenParams::GetstaticParams( AkToneGenStaticParams * out_pstaticParams )
{
	*out_pstaticParams = m_Params.staticParams;

	// Parameter check
	AKASSERT( out_pstaticParams->fStartFreqRandMin >= TONEGEN_RANDMINFREQ_MIN &&
			out_pstaticParams->fStartFreqRandMin <= TONEGEN_RANDMINFREQ_MAX );
	AKASSERT( out_pstaticParams->fStartFreqRandMax >= TONEGEN_RANDMAXFREQ_MIN &&
			out_pstaticParams->fStartFreqRandMax <= TONEGEN_RANDMAXFREQ_MAX );
	AKASSERT( out_pstaticParams->fStopFreqRandMin >= TONEGEN_RANDMINFREQ_MIN &&
			out_pstaticParams->fStopFreqRandMin <= TONEGEN_RANDMINFREQ_MAX );
	AKASSERT( out_pstaticParams->fStopFreqRandMax >= TONEGEN_RANDMAXFREQ_MIN &&
			out_pstaticParams->fStopFreqRandMax <= TONEGEN_RANDMAXFREQ_MAX );
	AKASSERT( out_pstaticParams->eGenSweep >= TONEGEN_SWEEPMODE_MIN &&
			out_pstaticParams->eGenSweep <= TONEGEN_SWEEPMODE_MAX );
	AKASSERT( out_pstaticParams->eGenType >= TONEGEN_GENTYPE_MIN &&
			out_pstaticParams->eGenType <= TONEGEN_GENTYPE_MAX );
	AKASSERT( out_pstaticParams->eGenMode >= TONEGEN_ENVMODE_MIN &&
			out_pstaticParams->eGenMode <= TONEGEN_ENVMODE_MAX );
	AKASSERT( out_pstaticParams->fFixDur >= TONEGEN_FIXEDDURATION_MIN &&
			out_pstaticParams->fFixDur <= TONEGEN_DURATION_MAX );
	AKASSERT( out_pstaticParams->fAttackDur >= TONEGEN_DURATION_MIN &&
			out_pstaticParams->fAttackDur <= TONEGEN_DURATION_MAX );
	AKASSERT( out_pstaticParams->fDecayDur >= TONEGEN_DURATION_MIN &&
			out_pstaticParams->fDecayDur <= TONEGEN_DURATION_MAX );
	AKASSERT( out_pstaticParams->fSustainDur >= TONEGEN_DURATION_MIN &&
			out_pstaticParams->fSustainDur <= TONEGEN_DURATION_MAX );
	AKASSERT( out_pstaticParams->fReleaseDur >= TONEGEN_DURATION_MIN &&
			out_pstaticParams->fReleaseDur <= TONEGEN_DURATION_MAX );
	AKASSERT( out_pstaticParams->fSustainVal >= TONEGEN_LEVEL_MIN &&
			out_pstaticParams->fSustainVal <= TONEGEN_LEVEL_MAX );
}

// Retrieve current gain RTPC value (linear value).
inline AkReal32 CAkToneGenParams::GetGain( )
{
	AkReal32 fGain = m_Params.fGain;
	AKASSERT( fGain >= TONEGEN_LEVEL_MIN && fGain <= TONEGEN_LEVEL_MAX );
	// Make it a linear value
	fGain = powf( 10.f, ( fGain / 20.f ) );
	return fGain;
}

// Retrieve current start frequency RTPC value.
inline AkReal32 CAkToneGenParams::GetStartFreq( )
{
	AkReal32 fFrequency = m_Params.fStartFreq;
	AKASSERT( fFrequency >= TONEGEN_FREQUENCY_MIN && fFrequency <= TONEGEN_FREQUENCY_MAX );
	return fFrequency;
}

// Retrieve current stop frequency RTPC value.
inline AkReal32 CAkToneGenParams::GetStopFreq( )
{
	AkReal32 fFrequency = m_Params.fStopFreq;
	AKASSERT( fFrequency >= TONEGEN_FREQUENCY_MIN && fFrequency <= TONEGEN_FREQUENCY_MAX );
	return fFrequency;
}

#endif
