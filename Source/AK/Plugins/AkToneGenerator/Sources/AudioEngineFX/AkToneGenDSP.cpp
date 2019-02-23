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
// AkToneGenDSP.cpp
//
// Wave table DSP function for oscillators and noise
// Oscillators use table lookup for efficiency.
// 1) Sine wave.
// 2) Triangle wave.
// 3) Square wave.
// 4) Sawtooth wave.
// 5) Inverse sawtooth wave.
// Noise generators synthesize on the fly 
// otherwise memory cost would be too important.
// 1) White noise
// 2) Pink noise
//
// In future Wwise releases, a fixed audio format will be used for the processing of software FX. 
// This format will optimal for each platform supported by the sound engine. In order to minimize coding changes required, 
// the following guidelines are advised. These changes will make it easier to write AK software FX at the same time
// as enhancing quality and performance of DSP effects.
//
//	- Use normalized floating point processing (assume audio input will eventually be in floating point format *)
//	- Can assume a single sampling rate for all incoming audio signals
//	- Support both mono and stereo processing
//	- Can assume the output will be clipped for them before hardware output
//
//* Single precision, normalized in range (-1.f, 1.f)
//
//////////////////////////////////////////////////////////////////////

// Non-linear control of frequency sweeping envelope are implemented as a general
// equation of the form x(t) = a*t^b where b is an exponential/logarithmic slope
// control factor. (The equation provided is centered at the origin for simplicity.
// It obviously needs to be translated and scaled appropriately to provide necessary
// frequency controls. So in practice the instantaneous frequency is given by:

// InstFreq( CurSampPos ) =	(FreqStop-FreqStart)			
// 							-------------------- * CurSampPos^b + FreqStart
// 							    NumSampIter^b
// 
// Where	InstFreq -> Instantaneous frequency used to control wavetable sampling increment
//			CurSampPos -> Is the current sample position within the current loop iteration
//			FreqStart -> Is the starting frequency
//			FreqStop -> Is the target frequency
//			NumSampIter -> Is the total number of samples in a loop iteration
//			b -> is a slope control factor detailed below
//
// Values of b below 1 yield a logarithmic type curve with base 1/b
// Values of b above 1 yield an exponential type curve with base b

// Important notes regarding logarithmic frequency sweeping.
// As we go up in frequency (upwards sweep) a constant difference in frequency represents
// a smaller musical interval, put the other way around we need to increase the rate at which
// frequency goes up progressively to preserve the perception that we are going up at a constant
// rate in terms of musical intervals (based on log base 2 scale)
// This can be achieved with the above exponential ramp by setting the b slope control factor as follows

// As we go down in frequency (downward sweep) a constant difference in frequency represents
// a larger musical interval, put the other way around we need to decrease the rate at which
// frequency goes down progressively to preserve the perception that we are going down at a constant
// rate in terms of musical intervals (based on log base 2 scale)
// This can be achieved with the above logarithmic ramp by setting the b slope control factor as follows

#include "AkToneGen.h"
#include <math.h>
#include <AK/Tools/Common/AkAssert.h>

static const AkReal32 FLOATMIN = -1.f;
static const AkReal32 FLOATMAX = 1.f;

#define CONVERT_VALUE( _floatval_ ) ( _floatval_ )

// Truncating cast optimization
inline AkUInt32 FloorIntCast(AkReal32 flt) 
{  
#if defined(AK_CPU_X86) && defined(_MSC_VER)
	// Can assume that this procesing threas is in Round-to-nearest FPU rounding mode
	// Positive numbers only
    static const AkReal32 fHalf = 0.5f;
    int intgr;
    _asm
	{
		fld flt
		fsub fHalf
		fistp intgr
	};
    return intgr;
#else
	// Cast will do, intrinsics could be used
	return static_cast<AkUInt32>( flt );
#endif
}

//////////////////////////////// Tone generator functions ///////////////////////////////////////

// No sweeping
void CAkToneGen::ProcessWaveTableNoSweep( AkSampleType * AK_RESTRICT out_pBuffer, AkUInt32 in_uSampToProduce )
{
	AKASSERT( m_pOscWaveTable != NULL );

	AkReal32 fLeftY;
	AkReal32 fRightY;
	AkReal32 fOutY;
	AkUInt32 uLeftSamp;
	AkUInt32 uRightSamp;

	// Get RTPC frequency values
	AkReal32 fStartFreq = m_pSharedParams->GetStartFreq( );
	AkReal32 fStopFreq = m_pSharedParams->GetStopFreq( );

	// Change target gain if necessary (RTPC value)
	m_GainRamp.SetTarget( m_pSharedParams->GetGain( ) );

	// Start and stop frequencies needs to be offset by their respective modifier value
	AkReal32 fStartModFreq = fStartFreq + m_fStartFreqModifier;
	AkReal32 fStopModFreq = fStopFreq + m_fStopFreqModifier;

	// Ensure start frequency is in valid range
	ClipFrequencyToValidRange( &fStartModFreq );
	// Ensure stop frequency is in valid range
	ClipFrequencyToValidRange( &fStopModFreq );
	
	// Will keep the same sample increment for the whole block

	// Sampling increment relation to oscillator frequency:
	// SI = N * f0
	//			--
	//			fs
	m_fSampleInc = ( OSC_WAVETABLE_SIZE  * fStartModFreq ) / static_cast<AkReal32>( m_uSampleRate );

	// Avoid costly floor function
	AkUInt32 uSampleIncInt = FloorIntCast( m_fSampleInc );
	AKASSERT( uSampleIncInt <= m_fSampleInc );
	AkReal32 fSampleIncFrac = m_fSampleInc - uSampleIncInt;

	AkUInt32 uSampleToProduce = in_uSampToProduce;
	while ( uSampleToProduce-- )
	{
		// Linear interpolation between 2 samples
		AKASSERT( m_uPhaseIndex < OSC_WAVETABLE_SIZE );
		uLeftSamp = m_uPhaseIndex;
		uLeftSamp &= OSC_WAVETABLESIZEMASK;
		uRightSamp = uLeftSamp + 1;
		uRightSamp &= OSC_WAVETABLESIZEMASK;
		fLeftY = m_pOscWaveTable[uLeftSamp];
		fRightY = m_pOscWaveTable[uRightSamp];
		AKASSERT( m_fPhaseIndexFrac >= 0.f && m_fPhaseIndexFrac <= 1.f );
		fOutY = NormalizedInterpolate( fLeftY, fRightY, m_fPhaseIndexFrac );

		// In parts now to avoid floor
		m_uPhaseIndex += uSampleIncInt;
		m_fPhaseIndexFrac += fSampleIncFrac;
		if ( m_fPhaseIndexFrac > 1.f )
		{
			m_uPhaseIndex++;
			m_fPhaseIndexFrac -= 1.f;
		}
		m_uPhaseIndex &= OSC_WAVETABLESIZEMASK;

		// Lookup envelope value
		EnvelopeLookup( );

		// Apply appropriate envelope gain
		fOutY *= m_fEnvCurGain;

		// Tick gain interpolation ramp
		AkReal32 fCurGain = m_GainRamp.Tick( );

		// Write output
		*out_pBuffer++ = CONVERT_VALUE( fCurGain * fOutY );

		// Update production
		m_uIterOutSampCount++;

		// Check for the end of current iteration (for finite sources)
		if ( m_uIterOutSampCount >= m_uIterNumSamples )
		{		
			m_uIterOutSampCount = 0;
			m_uCurEnvSegment = 0;
			m_uEnvSegmentCount = 0;	
			if ( (m_staticParams.eGenMode == AKTONEGENMODE_ENV) || (m_iNumLoops == 1) )
				m_fEnvCurGain = 0.f;
			else
				m_fEnvCurGain = 1.f;
		}
	}
	// Update production
	m_uTotalOutSampCount += in_uSampToProduce;
}

// Linear sweep
void CAkToneGen::ProcessWaveTableLinSweep( AkSampleType * AK_RESTRICT out_pBuffer, AkUInt32 in_uSampToProduce )
{
	AKASSERT( m_pOscWaveTable != NULL );

	AkReal32 fLeftY;
	AkReal32 fRightY;
	AkReal32 fOutY;
	AkUInt32 uLeftSamp;
	AkUInt32 uRightSamp;

	// Get RTPC frequency values
	AkReal32 fStartFreq = m_pSharedParams->GetStartFreq( );
	AkReal32 fStopFreq = m_pSharedParams->GetStopFreq( );

	// Change target gain if necessary (RTPC value)
	m_GainRamp.SetTarget( m_pSharedParams->GetGain( ) );

	// Start and stop frequencies needs to be offset by their respective modifier value
	AkReal32 fStartModFreq = fStartFreq + m_fStartFreqModifier;
	AkReal32 fStopModFreq = fStopFreq + m_fStopFreqModifier;

	// Ensure start frequency is in valid range
	ClipFrequencyToValidRange( &fStartModFreq );
	// Ensure stop frequency is in valid range
	ClipFrequencyToValidRange( &fStopModFreq );

	// Compute frequency scaling factor (part of instantaneous frequency computation
	//  =	(FreqStop-FreqStart)			
	// 		-------------------- 
	// 		   NumSampIter^b

	m_fFreqSweepScaleFactor = ( fStopModFreq - fStartModFreq ) / static_cast<AkReal32>( m_uIterNumSamples );

	AkUInt32 uSampleToProduce = in_uSampToProduce;
	while ( uSampleToProduce-- )
	{
		// Compute instantaneous frequency from frequency control ramp function
		// InstFreq( CurSampPos ) =	(FreqStop-FreqStart)			
		// 							-------------------- * CurSampPos^b + StartFreq
		// 							    NumSampIter^1

		AkReal32 fInstantFreq = fStartModFreq + ( m_fFreqSweepScaleFactor * m_uIterOutSampCount );
		// Sampling increment relation to oscillator frequency:
		// SI = N * f0
		//			--
		//			fs

		m_fSampleInc = ( OSC_WAVETABLE_SIZE  * fInstantFreq ) / static_cast<AkReal32>( m_uSampleRate );
		AKASSERT( m_fSampleInc >= 0.f );

		// Limit occurence of costly floor function
		AkUInt32 uSampleIncInt = FloorIntCast( m_fSampleInc );
		AKASSERT ( uSampleIncInt <= m_fSampleInc );
		AkReal32 fSampleIncFrac = m_fSampleInc - uSampleIncInt;

		// Linear interpolation between 2 samples
		AKASSERT( m_uPhaseIndex < OSC_WAVETABLE_SIZE );
		uLeftSamp = m_uPhaseIndex;
		uLeftSamp &= OSC_WAVETABLESIZEMASK;
		uRightSamp = uLeftSamp + 1;
		uRightSamp &= OSC_WAVETABLESIZEMASK;
		fLeftY = m_pOscWaveTable[uLeftSamp];
		fRightY = m_pOscWaveTable[uRightSamp];
		AKASSERT( m_fPhaseIndexFrac >= 0.f && m_fPhaseIndexFrac <= 1.f );
		fOutY = NormalizedInterpolate( fLeftY, fRightY, m_fPhaseIndexFrac );

		// In parts now to avoid floor
		m_uPhaseIndex += uSampleIncInt;
		m_fPhaseIndexFrac += fSampleIncFrac;
		if ( m_fPhaseIndexFrac > 1.f )
		{
			m_uPhaseIndex++;
			m_fPhaseIndexFrac -= 1.f;
		}
		m_uPhaseIndex &= OSC_WAVETABLESIZEMASK;

		// Lookup envelope value
		EnvelopeLookup( );

		// Apply appropriate envelope gain
		fOutY *= m_fEnvCurGain;

		// Tick gain interpolation ramp
		AkReal32 fCurGain = m_GainRamp.Tick( );

		// Write output
		*out_pBuffer++ = CONVERT_VALUE( fCurGain * fOutY );

		// Update production
		m_uIterOutSampCount++;

		// Check for the end of current iteration (for finite sources)
		if ( m_uIterOutSampCount >= m_uIterNumSamples )
		{		
			m_uIterOutSampCount = 0;
			m_uCurEnvSegment = 0;
			m_uEnvSegmentCount = 0;
			if ( (m_staticParams.eGenMode == AKTONEGENMODE_ENV) || (m_iNumLoops == 1) )
				m_fEnvCurGain = 0.f;
			else
				m_fEnvCurGain = 1.f;
		}
	}
	// Update production
	m_uTotalOutSampCount += in_uSampToProduce;
}

// Log sweep
void CAkToneGen::ProcessWaveTableLogSweep( AkSampleType * AK_RESTRICT out_pBuffer, AkUInt32 in_uSampToProduce )
{
	AKASSERT( m_pOscWaveTable != NULL );

	AkReal32 fLeftY;
	AkReal32 fRightY;
	AkReal32 fOutY;
	AkUInt32 uLeftSamp;
	AkUInt32 uRightSamp;

	// Get RTPC frequency values
	AkReal32 fStartFreq = m_pSharedParams->GetStartFreq( );
	AkReal32 fStopFreq = m_pSharedParams->GetStopFreq( );

	// Change target gain if necessary (RTPC value)
	m_GainRamp.SetTarget( m_pSharedParams->GetGain( ) );

	// Start and stop frequencies needs to be offset by their respective modifier value
	AkReal32 fStartModFreq = fStartFreq + m_fStartFreqModifier;
	AkReal32 fStopModFreq = fStopFreq + m_fStopFreqModifier;

	// Ensure start frequency is in valid range
	ClipFrequencyToValidRange( &fStartModFreq );
	// Ensure stop frequency is in valid range
	ClipFrequencyToValidRange( &fStopModFreq );

	// Logarithmic sweep initialization
	// Compute frequency scaling factor (part of instantaneous frequency computation
	//  =	(FreqStop-FreqStart)			
	// 		-------------------- 
	// 		   NumSampIter^b
	bool bDownSweep = fStartModFreq >= fStopModFreq;
	if ( bDownSweep )
	{
		// Downward sweep
		m_fFreqSweepScaleFactor = ( fStopModFreq - fStartModFreq ) / sqrtf( static_cast<AkReal32>( m_uIterNumSamples ) );
	}
	else
	{
		// Upward sweep
		AkReal32 fTmp = static_cast<AkReal32>( m_uIterNumSamples );
		m_fFreqSweepScaleFactor = ( fStopModFreq - fStartModFreq ) / ( fTmp * fTmp );
	}

	AkUInt32 uSampleToProduce = in_uSampToProduce;
	while ( uSampleToProduce-- )
	{

		// Compute instantaneous frequency from frequency control ramp function
		// InstFreq( CurSampPos ) =	(FreqStop-FreqStart)			
		// 							-------------------- * CurSampPos^b + StartFreq
		// 							    NumSampIter^b

		AkReal32 fInstantFreq;
		if ( bDownSweep )
		{
			fInstantFreq = fStartModFreq + ( m_fFreqSweepScaleFactor * sqrtf( static_cast<AkReal32>( m_uIterOutSampCount ) ) );
		}
		else
		{
			AkReal32 fTmpVal = static_cast<AkReal32>( m_uIterOutSampCount ); 
			fInstantFreq = fStartModFreq + ( m_fFreqSweepScaleFactor * ( fTmpVal * fTmpVal ) );
		}

		// Sampling increment relation to oscillator frequency:
		// SI = N * f0
		//			--
		//			fs

		// Compute new step increment based on current instantaneous frequency
		m_fSampleInc = ( OSC_WAVETABLE_SIZE  * fInstantFreq ) / static_cast<AkReal32>( m_uSampleRate );

		// Limit occurence of costly floor function
		AkUInt32 uSampleIncInt = FloorIntCast( m_fSampleInc );
		AKASSERT ( uSampleIncInt <= m_fSampleInc );
		AkReal32 fSampleIncFrac = m_fSampleInc - uSampleIncInt;

		// Linear interpolation between 2 samples
		AKASSERT( m_uPhaseIndex < OSC_WAVETABLE_SIZE );
		uLeftSamp = m_uPhaseIndex;
		uLeftSamp &= OSC_WAVETABLESIZEMASK;
		uRightSamp = uLeftSamp + 1;
		uRightSamp &= OSC_WAVETABLESIZEMASK;
		fLeftY = m_pOscWaveTable[uLeftSamp];
		fRightY = m_pOscWaveTable[uRightSamp];
		AKASSERT( m_fPhaseIndexFrac >= 0.f && m_fPhaseIndexFrac <= 1.f );
		fOutY = NormalizedInterpolate( fLeftY, fRightY, m_fPhaseIndexFrac );

		// In parts now to avoid floor
		m_uPhaseIndex += uSampleIncInt;
		m_fPhaseIndexFrac += fSampleIncFrac;
		if ( m_fPhaseIndexFrac > 1.f )
		{
			m_uPhaseIndex++;
			m_fPhaseIndexFrac -= 1.f;
		}
		m_uPhaseIndex &= OSC_WAVETABLESIZEMASK;

		// Lookup envelope value
		EnvelopeLookup( );

		// Apply appropriate envelope gain
		fOutY *= m_fEnvCurGain;

		// Tick gain interpolation ramp
		AkReal32 fCurGain = m_GainRamp.Tick( );

		// Write output
		*out_pBuffer++ = CONVERT_VALUE( fCurGain * fOutY );

		// Update production
		m_uIterOutSampCount++;

		// Check for the end of current iteration (for finite sources)
		if ( m_uIterOutSampCount >= m_uIterNumSamples )
		{		
			m_uIterOutSampCount = 0;
			m_uCurEnvSegment = 0;
			m_uEnvSegmentCount = 0;
			if ( (m_staticParams.eGenMode == AKTONEGENMODE_ENV) || (m_iNumLoops == 1) )
				m_fEnvCurGain = 0.f;
			else
				m_fEnvCurGain = 1.f;
		}
	}
	// Update production
	m_uTotalOutSampCount += in_uSampToProduce;
}

// Log sweeping function
void CAkToneGen::ProcessOversampledWaveTableNoSweep( AkSampleType * AK_RESTRICT out_pBuffer, AkUInt32 in_uSampToProduce )
{
	AKASSERT( m_pOscWaveTable != NULL );

	AkReal32 fLeftY;
	AkReal32 fRightY;
	AkReal32 fOutY;
	AkUInt32 uLeftSamp;
	AkUInt32 uRightSamp;

	// Get RTPC frequency values
	AkReal32 fStartFreq = m_pSharedParams->GetStartFreq( );
	AkReal32 fStopFreq = m_pSharedParams->GetStopFreq( );

	// Change target gain if necessary (RTPC value)
	m_GainRamp.SetTarget( m_pSharedParams->GetGain( ) );

	// Start and stop frequencies needs to be offset by their respective modifier value
	AkReal32 fStartModFreq = fStartFreq + m_fStartFreqModifier;
	AkReal32 fStopModFreq = fStopFreq + m_fStopFreqModifier;

	// Ensure start frequency is in valid range
	ClipFrequencyToValidRange( &fStartModFreq );
	// Ensure stop frequency is in valid range
	ClipFrequencyToValidRange( &fStopModFreq );

	// Sampling increment relation to oscillator frequency:
	// SI = N * f0
	//			--
	//			fs
	m_fSampleInc = ( OSC_WAVETABLE_SIZE * fStartModFreq ) / m_fOverSamplingFrequency;

	// Avoid costly floor function
	AkUInt32 uSampleIncInt = FloorIntCast( m_fSampleInc );
	AKASSERT( uSampleIncInt <= m_fSampleInc );
	AkReal32 fSampleIncFrac = m_fSampleInc - uSampleIncInt;

	AkUInt32 uSampleToProduce = in_uSampToProduce;
	while ( uSampleToProduce-- )
	{
		for ( unsigned int j = 0; j < OSCOVERSAMPLING; j++ )
		{
			// Linear interpolation between 2 samples
			AKASSERT( m_uPhaseIndex < OSC_WAVETABLE_SIZE );
			uLeftSamp = m_uPhaseIndex;
			uLeftSamp &= OSC_WAVETABLESIZEMASK;
			uRightSamp = uLeftSamp + 1;
			uRightSamp &= OSC_WAVETABLESIZEMASK;
			fLeftY = m_pOscWaveTable[uLeftSamp];
			fRightY = m_pOscWaveTable[uRightSamp];
			AKASSERT( m_fPhaseIndexFrac >= 0.f && m_fPhaseIndexFrac <= 1.f );
			fOutY = NormalizedInterpolate( fLeftY, fRightY, m_fPhaseIndexFrac );

			// In parts now to avoid floor
			m_uPhaseIndex += uSampleIncInt;
			m_fPhaseIndexFrac += fSampleIncFrac;
			if ( m_fPhaseIndexFrac > 1.f )
			{
				m_uPhaseIndex++;
				m_fPhaseIndexFrac -= 1.f;
			}
			m_uPhaseIndex &= OSC_WAVETABLESIZEMASK;
			
			// Reset filter memory and set coefficients
			// 3 filter sections to avoid aliasing problems before decimation (loop unroll)
			fOutY = m_LpFilters[0].ProcessSample( fOutY );
			fOutY = m_LpFilters[1].ProcessSample( fOutY );
			fOutY = m_LpFilters[2].ProcessSample( fOutY );
		}

		// Implicit decimation by taking only the output of the last oversampled value

		// Lookup envelope value
		EnvelopeLookup( );

		// Apply appropriate envelope gain
		fOutY *= m_fEnvCurGain;

		// Tick gain interpolation ramp
		AkReal32 fCurGain = m_GainRamp.Tick( );

		// Write output
		*out_pBuffer++ = CONVERT_VALUE( fCurGain * fOutY );

		// Update production
		m_uIterOutSampCount++;

		// Check for the end of current iteration (for finite sources)
		if ( m_uIterOutSampCount >= m_uIterNumSamples )
		{		
			m_uIterOutSampCount = 0;
			m_uCurEnvSegment = 0;
			m_uEnvSegmentCount = 0;
			if ( (m_staticParams.eGenMode == AKTONEGENMODE_ENV) || (m_iNumLoops == 1) )
				m_fEnvCurGain = 0.f;
			else
				m_fEnvCurGain = 1.f;
		}
	}
	// Update production
	m_uTotalOutSampCount += in_uSampToProduce;
}

// Linear sweeping function
void CAkToneGen::ProcessOversampledWaveTableLinSweep( AkSampleType * AK_RESTRICT out_pBuffer, AkUInt32 in_uSampToProduce )
{
	AKASSERT( m_pOscWaveTable != NULL );

	AkReal32 fLeftY;
	AkReal32 fRightY;
	AkReal32 fOutY;
	AkUInt32 uLeftSamp;
	AkUInt32 uRightSamp;

	// Get RTPC frequency values
	AkReal32 fStartFreq = m_pSharedParams->GetStartFreq( );
	AkReal32 fStopFreq = m_pSharedParams->GetStopFreq( );

	// Change target gain if necessary (RTPC value)
	m_GainRamp.SetTarget( m_pSharedParams->GetGain( ) );

	// Start and stop frequencies needs to be offset by their respective modifier value
	AkReal32 fStartModFreq = fStartFreq + m_fStartFreqModifier;
	AkReal32 fStopModFreq = fStopFreq + m_fStopFreqModifier;

	// Ensure start frequency is in valid range
	ClipFrequencyToValidRange( &fStartModFreq );
	// Ensure stop frequency is in valid range
	ClipFrequencyToValidRange( &fStopModFreq );

	// Compute frequency scaling factor (part of instantaneous frequency computation
	//  =	(FreqStop-FreqStart)			
	// 		-------------------- 
	// 		   NumSampIter^1

	m_fFreqSweepScaleFactor = ( fStopModFreq - fStartModFreq ) / ( m_uIterNumSamples * OSCOVERSAMPLING );

	AkUInt32 uSampleToProduce = in_uSampToProduce;
	while ( uSampleToProduce-- )
	{
		AkReal32 fOverSampCount = static_cast<AkReal32>(m_uIterOutSampCount * OSCOVERSAMPLING);

		// Compute instantaneous frequency from frequency control ramp function
		// InstFreq( CurSampPos ) =	(FreqStop-FreqStart)			
		// 							-------------------- * CurSampPos^b + StartFreq
		// 							    NumSampIter^1

		AkReal32 fInstantFreq = fStartModFreq + ( m_fFreqSweepScaleFactor * fOverSampCount );
		// Sampling increment relation to oscillator frequency:
		// SI = N * f0
		//			--
		//			fs

		m_fSampleInc = ( OSC_WAVETABLE_SIZE * fInstantFreq ) / m_fOverSamplingFrequency;

		// Limit occurence of costly floor function
		AkUInt32 uSampleIncInt = FloorIntCast( m_fSampleInc );
		AKASSERT ( uSampleIncInt <= m_fSampleInc );
		AkReal32 fSampleIncFrac = m_fSampleInc - uSampleIncInt;

		for ( unsigned int j = 0; j < OSCOVERSAMPLING; j++ )
		{
			// Linear interpolation between 2 samples
			AKASSERT( m_uPhaseIndex < OSC_WAVETABLE_SIZE );
			uLeftSamp = m_uPhaseIndex;
			uLeftSamp &= OSC_WAVETABLESIZEMASK;
			uRightSamp = uLeftSamp + 1;
			uRightSamp &= OSC_WAVETABLESIZEMASK;
			fLeftY = m_pOscWaveTable[uLeftSamp];
			fRightY = m_pOscWaveTable[uRightSamp];
			AKASSERT( m_fPhaseIndexFrac >= 0.f && m_fPhaseIndexFrac <= 1.f );
			fOutY = NormalizedInterpolate( fLeftY, fRightY, m_fPhaseIndexFrac );

			// In parts now to avoid floor
			m_uPhaseIndex += uSampleIncInt;
			m_fPhaseIndexFrac += fSampleIncFrac;
			if ( m_fPhaseIndexFrac > 1.f )
			{
				m_uPhaseIndex++;
				m_fPhaseIndexFrac -= 1.f;
			}
			m_uPhaseIndex &= OSC_WAVETABLESIZEMASK;

			// Reset filter memory and set coefficients
			// 3 filter sections to avoid aliasing problems before decimation (loop unroll)
			fOutY = m_LpFilters[0].ProcessSample( fOutY );
			fOutY = m_LpFilters[1].ProcessSample( fOutY );
			fOutY = m_LpFilters[2].ProcessSample( fOutY );
		}

		// Implicit decimation by taking only the output of the last oversampled value

		// Lookup envelope value
		EnvelopeLookup( );

		// Apply appropriate envelope gain
		fOutY *= m_fEnvCurGain;

		// Tick gain interpolation ramp
		AkReal32 fCurGain = m_GainRamp.Tick( );

		// Write output
		*out_pBuffer++ = CONVERT_VALUE( fCurGain * fOutY );

		// Update production
		m_uIterOutSampCount++;

		// Check for the end of current iteration (for finite sources)
		if ( m_uIterOutSampCount >= m_uIterNumSamples )
		{		
			m_uIterOutSampCount = 0;
			m_uCurEnvSegment = 0;
			m_uEnvSegmentCount = 0;
			if ( (m_staticParams.eGenMode == AKTONEGENMODE_ENV) || (m_iNumLoops == 1) )
				m_fEnvCurGain = 0.f;
			else
				m_fEnvCurGain = 1.f;
		}
	}
	// Update production
	m_uTotalOutSampCount += in_uSampToProduce;
}

// Log sweeping function
void CAkToneGen::ProcessOversampledWaveTableLogSweep( AkSampleType * AK_RESTRICT out_pBuffer, AkUInt32 in_uSampToProduce )
{
	AKASSERT( m_pOscWaveTable != NULL );

	AkReal32 fLeftY;
	AkReal32 fRightY;
	AkReal32 fOutY;
	AkUInt32 uLeftSamp;
	AkUInt32 uRightSamp;

	// Get RTPC frequency values
	AkReal32 fStartFreq = m_pSharedParams->GetStartFreq( );
	AkReal32 fStopFreq = m_pSharedParams->GetStopFreq( );

	// Change target gain if necessary (RTPC value)
	m_GainRamp.SetTarget( m_pSharedParams->GetGain( ) );

	// Start and stop frequencies needs to be offset by their respective modifier value
	AkReal32 fStartModFreq = fStartFreq + m_fStartFreqModifier;
	AkReal32 fStopModFreq = fStopFreq + m_fStopFreqModifier;

	// Ensure start frequency is in valid range
	ClipFrequencyToValidRange( &fStartModFreq );
	// Ensure stop frequency is in valid range
	ClipFrequencyToValidRange( &fStopModFreq );


	// Logarithmic sweep initialization
	// Compute frequency scaling factor (part of instantaneous frequency computation
	//  =	(FreqStop-FreqStart)			
	// 		-------------------- 
	// 		   NumSampIter^b
	bool bDownSweep = fStartModFreq >= fStopModFreq;
	if ( bDownSweep )
	{
		// Downward sweep
		m_fFreqSweepScaleFactor = ( fStopModFreq - fStartModFreq ) / sqrtf( static_cast<AkReal32>( m_uIterNumSamples ) * OSCOVERSAMPLING );
	}
	else
	{
		// Upward sweep
		AkReal32 fTmp = static_cast<AkReal32>( m_uIterNumSamples ) * OSCOVERSAMPLING;
		m_fFreqSweepScaleFactor = ( fStopModFreq - fStartModFreq ) / ( fTmp * fTmp );
	}

	AkUInt32 uSampleToProduce = in_uSampToProduce;
	while ( uSampleToProduce-- )
	{
		AkReal32 fOverSampCount = static_cast<AkReal32>(m_uIterOutSampCount * OSCOVERSAMPLING);

		// Compute instantaneous frequency from frequency control ramp function
		// InstFreq( CurSampPos ) =	(FreqStop-FreqStart)			
		// 							-------------------- * CurSampPos^b + StartFreq
		// 							    NumSampIter^b

		AkReal32 fInstantFreq;
		if ( bDownSweep )
		{
			fInstantFreq = fStartModFreq + ( m_fFreqSweepScaleFactor * sqrtf( fOverSampCount ) );
		}
		else
		{
			fInstantFreq = fStartModFreq + ( m_fFreqSweepScaleFactor * ( fOverSampCount * fOverSampCount ) );
		}

		// Sampling increment relation to oscillator frequency:
		// SI = N * f0
		//			--
		//			fs

		// Compute new step increment based on current instantaneous frequency

		m_fSampleInc = ( OSC_WAVETABLE_SIZE * fInstantFreq ) / m_fOverSamplingFrequency;

		// Limit occurence of costly floor function
		AkUInt32 uSampleIncInt = FloorIntCast( m_fSampleInc );
		AKASSERT ( uSampleIncInt <= m_fSampleInc );
		AkReal32 fSampleIncFrac = m_fSampleInc - uSampleIncInt;

		for ( unsigned int j = 0; j < OSCOVERSAMPLING; j++ )
		{
			// Linear interpolation between 2 samples
			AKASSERT( m_uPhaseIndex < OSC_WAVETABLE_SIZE );
			uLeftSamp = m_uPhaseIndex;
			uLeftSamp &= OSC_WAVETABLESIZEMASK;
			uRightSamp = uLeftSamp + 1;
			uRightSamp &= OSC_WAVETABLESIZEMASK;
			fLeftY = m_pOscWaveTable[uLeftSamp];
			fRightY = m_pOscWaveTable[uRightSamp];
			AKASSERT( m_fPhaseIndexFrac >= 0.f && m_fPhaseIndexFrac <= 1.f );
			fOutY = NormalizedInterpolate( fLeftY, fRightY, m_fPhaseIndexFrac );

			// In parts now to avoid floor
			m_uPhaseIndex += uSampleIncInt;
			m_fPhaseIndexFrac += fSampleIncFrac;
			if ( m_fPhaseIndexFrac > 1.f )
			{
				m_uPhaseIndex++;
				m_fPhaseIndexFrac -= 1.f;
			}
			m_uPhaseIndex &= OSC_WAVETABLESIZEMASK;

			// Reset filter memory and set coefficients
			// 3 filter sections to avoid aliasing problems before decimation (loop unroll)
			fOutY = m_LpFilters[0].ProcessSample( fOutY );
			fOutY = m_LpFilters[1].ProcessSample( fOutY );
			fOutY = m_LpFilters[2].ProcessSample( fOutY );
		}

		// Implicit decimation by taking only the output of the last oversampled value

		// Lookup envelope value
		EnvelopeLookup( );

		// Apply appropriate envelope gain
		fOutY *= m_fEnvCurGain;

		// Tick gain interpolation ramp
		AkReal32 fCurGain = m_GainRamp.Tick( );

		// Write output
		*out_pBuffer++ = CONVERT_VALUE( fCurGain * fOutY );

		// Update production
		m_uIterOutSampCount++;

		// Check for the end of current iteration (for finite sources)
		if ( m_uIterOutSampCount >= m_uIterNumSamples )
		{		
			m_uIterOutSampCount = 0;
			m_uCurEnvSegment = 0;
			m_uEnvSegmentCount = 0;
			if ( (m_staticParams.eGenMode == AKTONEGENMODE_ENV) || (m_iNumLoops == 1) )
				m_fEnvCurGain = 0.f;
			else
				m_fEnvCurGain = 1.f;
		}
	}
	// Update production
	m_uTotalOutSampCount += in_uSampToProduce;
}

// Performs white noise synthesis
void CAkToneGen::ProcessWhiteNoise( AkSampleType * AK_RESTRICT out_pBuffer, AkUInt32 in_uSampToProduce )
{
	// Change target gain if necessary (RTPC value)
	m_GainRamp.SetTarget( m_pSharedParams->GetGain( ) );

	// Uses a member saved offset 
	AkUInt32 uSampleToProduce = in_uSampToProduce;
	while ( uSampleToProduce-- )
	{
		// Get a new random value in range (-1.,1.) 
		AkReal32 fOutY = PseudoRandomNumber( );

		// Lookup envelope value
		EnvelopeLookup( );
		
		// Apply appropriate envelope gain
		fOutY *= m_fEnvCurGain;

		// Tick gain interpolation ramp
		AkReal32 fCurGain = m_GainRamp.Tick( );

		// Write output
		*out_pBuffer++ = CONVERT_VALUE( fCurGain * fOutY );

		// Update production
		m_uIterOutSampCount++;

		// Check for the end of current iteration
		if ( m_uIterOutSampCount >= m_uIterNumSamples )
		{
			m_uIterOutSampCount = 0;
			m_uCurEnvSegment = 0;
			m_uEnvSegmentCount = 0;
			if ( (m_staticParams.eGenMode == AKTONEGENMODE_ENV) || (m_iNumLoops == 1) )
				m_fEnvCurGain = 0.f;
			else
				m_fEnvCurGain = 1.f;
		}
	}
	// Update production
	m_uTotalOutSampCount += in_uSampToProduce;
}

// Implements McCartney's enhancements of Gardner's algorithm to generate pink noise
void CAkToneGen::ProcessPinkNoise( AkSampleType * AK_RESTRICT out_pBuffer, AkUInt32 in_uSampToProduce )
{
	AKASSERT ( m_pPinkNoiseRandGen != NULL );

	AkReal32 fPinkOut;	
	AkReal32 fRandVal;

	// Change target gain if necessary (RTPC value)
	m_GainRamp.SetTarget( m_pSharedParams->GetGain( ) );

	AkUInt32 uSampleToProduce = in_uSampToProduce;
	while ( uSampleToProduce-- )
	{	
		// Increment and mask index 
		m_uPinkNoiseInd = (m_uPinkNoiseInd + 1) & m_uPinkNoiseIndMask;

		// If index is zero don't fire any new random value
		if ( m_uPinkNoiseInd != 0 )
		{
			// Count number of trailing 0 in current index
			AkUInt32 uNumZeros = 0;
			AkUInt32 uTempVal = m_uPinkNoiseInd;
			while ( ( uTempVal & 1) == 0 )
			{
				uTempVal = uTempVal >> 1;
				uNumZeros++;
			}

			// Replace the indexed random value generator
			// Subtract and add back to the running sum
			m_fPinkNoiseRunSum -= m_pPinkNoiseRandGen[uNumZeros];
			fRandVal = PseudoRandomNumber( );
			m_fPinkNoiseRunSum += fRandVal;
			m_pPinkNoiseRandGen[uNumZeros] = fRandVal;
		}

		// Add extra white noise value
		fRandVal = PseudoRandomNumber( );
		
		// Theoretically there is a possibility to clip since effectively mixing many signals
		// Using a NoiseScaleFactor bigger than 1/numinputs that will be statitiscally representative
		// To obtain noise power similar to white noise
		fPinkOut = m_fNoiseScale * ( m_fPinkNoiseRunSum + fRandVal );
		
		// Lookup envelope value
		EnvelopeLookup( );

		// Apply appropriate envelope gain
		fPinkOut *= m_fEnvCurGain;

		// Tick gain interpolation ramp
		AkReal32 fCurGain = m_GainRamp.Tick( );

		fPinkOut *= fCurGain;

		// Note: On some platforms this could be optimized with __fsel
		fPinkOut = AkMax( fPinkOut, FLOATMIN );
		fPinkOut = AkMin( fPinkOut, FLOATMAX );
		AKASSERT( fPinkOut >= FLOATMIN && fPinkOut <= FLOATMAX );

		// Write output
		*out_pBuffer++ = CONVERT_VALUE( fPinkOut );

		// Update production
		m_uIterOutSampCount++;	

		// Check for the end of current iteration
		if ( m_uIterOutSampCount >= m_uIterNumSamples )
		{
			m_uIterOutSampCount = 0;
			m_uCurEnvSegment = 0;
			m_uEnvSegmentCount = 0;
			if ( (m_staticParams.eGenMode == AKTONEGENMODE_ENV) || (m_iNumLoops == 1) )
				m_fEnvCurGain = 0.f;
			else
				m_fEnvCurGain = 1.f;
		}
	}
	// Update production
	m_uTotalOutSampCount += in_uSampToProduce;
}
