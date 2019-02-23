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
// AkToneGen.h
//
// Sample source effect. Implements a variety of tone generators. 
// Oscillators use table lookup for efficiency.
// 1) Sine wave.
// 2) Triangle wave.
// 3) Square wave.
// 4) Sawtooth wave.
// 5) Inverse sawtooth wave.
// Noise generators.
// 1) White noise
// 2) Pink noise
//
//////////////////////////////////////////////////////////////////////

#ifndef _AK_TONEGEN_H_
#define _AK_TONEGEN_H_

#include <AK/Plugin/PluginServices/AkValueRamp.h>
#include "AkToneGenParams.h"
#include "AkLpFilter.h"

// Oscillator SNR is governed by table size and sampling increment rounding technique.
// Since we use linear interpolation on the sampling increment the SNR is given by
// 12 * (log2(TABLESIZE) - 1) thus 96dB for a size of 512
static const AkUInt32 OSC_WAVETABLE_SIZE = 512;				// 512 samples used in oscillator wavetable
static const AkUInt32 OSC_WAVETABLESIZEMASK =  0x000001FF;

// Random number generation defines
static const AkReal32 ONEOVERMAXRANDVAL	= ( 1.f / 0x80000000 ); // 2^31

// Pink noise algorithm definitions
static const AkUInt32 PINKNOISENUMGEN = 30;		// Number of random generators used in Gardner's algorithm

// Compensation gain for pink noise (keeping in mind its low pass filtered white noise which takes some gain away)
static const AkReal32 PINKNOISEAMPCOMP = 4.f;	

// Fixed number of harmonics used by oscillators
static const AkUInt32 NUMOSCHARMONICS  = 15;

// At 48 kHz output -> maximum fundamental frequency allowed is 12000
// With 15 harmonics we must able to represent signal up to 15 * 12000 = 180000 to avoid aliased signal
// 180000 / 48 000 corresponds to a 3.75 x oversampling requirement
// Overshoot to avoid critical sampling
static const AkUInt32 OSCOVERSAMPLING = 4;

// Using an array of 2nd order Butterworth filter to get steeper slope
// Each section is -12 dB/octave
static const AkUInt32 NUMFILTERSECTIONS = 3;

// The tone generator will produce samples based on the platform.

//-----------------------------------------------------------------------------
// Name: class CAkToneGen
// Desc: Tone generator implementation (source effect).
//-----------------------------------------------------------------------------
class CAkToneGen : public AK::IAkSourcePlugin
{
public:

    // Constructor.
    CAkToneGen();

	// Destructor.
    virtual ~CAkToneGen();

    // Effect initialization.
    virtual AKRESULT Init(	AK::IAkPluginMemAlloc *			in_pAllocator,    	// Memory allocator interface.
                    AK::IAkSourcePluginContext *	in_pSourceFXContext,// Source FX context
                    AK::IAkPluginParam *			in_pParams,         // Effect parameters.
                    AkAudioFormat &					io_rFormat			// Supported audio output format.
                    );

    // Effect termination.
    virtual AKRESULT Term( AK::IAkPluginMemAlloc * in_pAllocator );

	// Reset.
	virtual AKRESULT Reset( );

    // Effect information query.
    virtual AKRESULT GetPluginInfo( AkPluginInfo & out_rPluginInfo );

    // Execute effect processing.
	virtual void Execute(	AkAudioBuffer *		io_pBuffer		// Output buffer interface.
					);

	// Get the duration of the source in mSec.
	virtual AkReal32 GetDuration() const;
	// Get normalized envelope.
	virtual AkReal32 GetEnvelope() const;
	virtual AKRESULT StopLooping();

private:

	// Performs table lookup synthesis
	void ProcessWaveTableNoSweep( AkSampleType * AK_RESTRICT out_pBuffer, AkUInt32 in_uSampToProduce );
	void ProcessWaveTableLinSweep( AkSampleType * AK_RESTRICT out_pBuffer, AkUInt32 in_uSampToProduce );
	void ProcessWaveTableLogSweep( AkSampleType * AK_RESTRICT out_pBuffer, AkUInt32 in_uSampToProduce );
	// Performs over sampled table lookup synthesis
	void ProcessOversampledWaveTableLogSweep( AkSampleType * AK_RESTRICT out_pBuffer, AkUInt32 in_uSampToProduce );
	void ProcessOversampledWaveTableLinSweep( AkSampleType * AK_RESTRICT out_pBuffer, AkUInt32 in_uSampToProduce );
	void ProcessOversampledWaveTableNoSweep( AkSampleType * AK_RESTRICT out_pBuffer, AkUInt32 in_uSampToProduce );
	// Performs white noise synthesis
	void ProcessWhiteNoise( AkSampleType * AK_RESTRICT out_pBuffer, AkUInt32 in_uSampToProduce );
	// Performs pink noise synthesis
	void ProcessPinkNoise( AkSampleType * AK_RESTRICT out_pBuffer, AkUInt32 in_uSampToProduce );

	// Lookup ADSR envelope value
	void EnvelopeLookup( );
	// Linear interpolation between (0.,1.)
	inline AkReal32 NormalizedInterpolate( AkReal32 in_fLowerY, AkReal32 in_fUpperY, AkReal32 in_fX );
	// Returns a random float value between in_fMin and in_fMax
	inline AkReal32 RandRange( AkReal32 in_fMin, AkReal32 in_fMax );
	// Fast pseudo random number generator
	inline AkReal32 PseudoRandomNumber( );
	// Clip frequency value to valid range
	inline void ClipFrequencyToValidRange( AkReal32 * io_fFreqValue );


    // Internal state variables.
    AkUInt32			m_uSampleRate;			// Sample rate
    AkInt16             m_iNumLoops;			// Total number of loops to execute source
	AkUInt32			m_uTotalOutSampCount;	// Total samples outputted so far
	AkUInt32			m_uIterOutSampCount;	// Samples outputted so far in this loop iteration
	AkUInt32			m_uTotalNumSamples;		// Total number of samples to output
	AkUInt32			m_uIterNumSamples;		// Number of samples to output in each loop iteration
	AkUInt32			m_uPhaseIndex;			// Current oscillator phase index
	AkReal32			m_fPhaseIndexFrac;		// Fractional part
	AkReal32			m_fSampleInc;			// Table lookup sample increment
	AkReal32			m_fStartFreqModifier;	// Modifier value for the frequency
	AkReal32			m_fOscMaxFreq;			// Oscillator maximum frequency (sampling rate dependent)
	AkReal32			m_fStopFreqModifier;	// Modifier value for the frequency
	AkReal32			m_fFreqSweepScaleFactor;// Frequency scaling used for frequency sweeping
	bool				m_bFinite;				// Source is finite (true) or infinite (false)
	AkUInt32			m_uPinkNoiseIndMask;	// Mask for index wrap around
	AkUInt32			m_uPinkNoiseInd;		// Pink noise index value
	AkReal32			m_fPinkNoiseRunSum;		// Pink noise running sum
	AkReal32			m_fNoiseScale;			// Scale factor for pink noise
	AkReal32			m_fOverSamplingFrequency;// Oversampled representation sample frequency
	AkReal32			m_fEnvInc[4];			// Envelope increment value for each segment
	AkUInt16			m_uCurEnvSegment;		// Current envelope segment
	AkUInt32			m_uEnvSegmentDur[4];	// Duration of each envelope segment
	AkUInt32			m_uEnvSegmentCount;		// Position within current envelope segment
	AkReal32			m_fEnvCurGain;			// Current envelope gain

	static AkUInt32		s_uSeedVal;				// Fast pseudo random number generator seed

	// Function ptr to the appropriate DSP execution routine
	void (CAkToneGen::*m_fpPerformDSP) ( AkSampleType * AK_RESTRICT out_psBuffer, AkUInt32 in_uSampToProduce );

	AkReal32 * m_pOscWaveTable;				// Oscillator wavetable

	AkReal32 * m_pPinkNoiseRandGen;			// Table of random generator output for Gardner's pink noise algo

	// Static parameters (remain valid for the lifespan of the effect)
	AkToneGenStaticParams m_staticParams;

	// Butterworth low pass filters
	CAkLpFilter m_LpFilters[NUMFILTERSECTIONS];

	// Shared parameters structure (modified by Wwise or RTPC)
    CAkToneGenParams * m_pSharedParams;

	// Source FX context interface
	AK::IAkSourcePluginContext * m_pSourceFXContext;

	// Gain ramp interpolator
	AK::CAkValueRamp m_GainRamp;
};

// Linear congruential method for generating pseudo-random number (float version)
inline AkReal32 CAkToneGen::PseudoRandomNumber( )
{
	// Generate a (pseudo) random number (32 bits range)
	s_uSeedVal = (s_uSeedVal * 196314165) + 907633515;
	// Generate rectangular PDF
	AkInt32 iRandVal = ((AkInt32) s_uSeedVal);
	// Scale to floating point range
	AkReal32 fRandVal = iRandVal * ONEOVERMAXRANDVAL;
	AKASSERT( fRandVal >= -1.f && fRandVal <= 1.f );
	// Output
	return fRandVal;
}

// Lookup ADSR envelope value
inline void CAkToneGen::EnvelopeLookup( )
{
	if ( m_uEnvSegmentCount >= m_uEnvSegmentDur[m_uCurEnvSegment] )
	{
		// Enterring new segment
		m_uEnvSegmentCount = 0;
		m_uCurEnvSegment++;
		AKASSERT( m_uCurEnvSegment <= 3 );
	}
	m_fEnvCurGain += m_fEnvInc[m_uCurEnvSegment];

	// Produced one sample
	m_uEnvSegmentCount++;
}

// Interpolates between |0.0f       and |1.0f       at position in_fX
//                      |in_fLowerY     |in_fUpperY
inline AkReal32 CAkToneGen::NormalizedInterpolate( AkReal32 in_fLowerY, AkReal32 in_fUpperY, AkReal32 in_fX )
{
	AkReal32 fInterpolated;
	fInterpolated = in_fLowerY + ( in_fX * (in_fUpperY - in_fLowerY) );
	return fInterpolated;
}

// Clip frequency value to valid range
inline void CAkToneGen::ClipFrequencyToValidRange( AkReal32 * io_fFreqValue )
{
	// Ensure start frequency is in valid range
	if ( *io_fFreqValue < TONEGEN_FREQUENCY_MIN )
	{
		*io_fFreqValue = TONEGEN_FREQUENCY_MIN;
	}
	else if ( *io_fFreqValue >= m_fOscMaxFreq )
	{
		// Just below Nyquist
		*io_fFreqValue = m_fOscMaxFreq - 1;
	}
}

#endif // _AK_TONEGEN_H_
