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
// AkToneGen.cpp
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

#include "AkToneGen.h"
#include "AkWaveTables.h"
#include <AK/Tools/Common/AkAssert.h>
#include <AK/AkWwiseSDKVersion.h>

#include <math.h>
#include <stdlib.h>
#include <string.h>

//-----------------------------------------------------------------------------
// Defines.
//-----------------------------------------------------------------------------

// Duration in sample of fade in/out used in fixed duration mode
static const AkReal32 FADEDUR  = 0.005f;	// 5 ms
static const AkReal32 FILTERCUTFREQ = 18000.f;	// Oversampling low pass filter cutoff frequency
static const AkReal32 RAMPMAXTIME	= 0.1f;		// 100 ms ramps, worst-case

// Wavetable synthesis overview:
// In wavetable synthesis, we store a single period of the necessary waveform type in a table of values
// computed once at effect initialization time. After that we just need to sequentially go through the
// table and retrieve the necessary (precomputed) amplitude value (table lookup synthesis). The rate at 
// which the table is scan determines the fundamental frequency of the oscillator. To achieve higher
// frequencies we just skip over some values (effectively downsampling the signal) and the opposite for
// lower frequencies. The rate at which we scan the wavetable will change if we perform a frequency sweep
// and thus we need to be able to convert an instantaneous frequency desired into a wave table sampling
// increment. This relation is given below.

// Sampling increment relation to oscillator frequency:
// SI = N * f0
//			--
//			fs
// where SI is the sampling increment (how many samples to jump each time in the wave table)
// f0 is the desired oscillator fundamental frequency
// fs is the sampling frequency
// N is the size of the wave table used
// Obviously SI will not necessarily lend on an integer value and we must thus extrapolate the
// imaginary sample at that floating point location. In this implementation this is done through
// linear interpolation. This technique (along with the table size) governs the overall oscillator
// SNR, documented elsewhere.

// Waveform generation recipes:
// Because frequency may change at any time we cannot band-limit the wave. We thus choose to add a fixed number
// of harmonics. An oversampled intermediary representation that is filtered prior to decimation is used.
// Because of Fourier's theory stating that all signals may be
// constructed as a sum of sinusoids at proper amplitude, frequency, and phase we use some common recipes
// to build the necessary waveforms

// Square wave -> All odd harmonics with amplitude 1/k (k is the harmonic number)
// Triangle wave -> All odd harmonics with amplitude 1/k^2 (k is the harmonic number)
// Sawtooth wave -> All harmonics with amplitude 1/k (k is the harmonic number)

// Noise generators overview:
// Due to the non-periodic nature of noise, filling a wavetable with random values and repeatedly playing
// it does not give very "random" sounding noise. Noise values are thus generated on the fly. The
// basic principle to synthesize any noise is to select a random value according to a certain probability
// density function and apply some constraints on the output values for noise colors other than white.
// 
// White noise:
// White noise is simply generated by using a uniform probability and picking an unconstrained 
// sample amplitude value within the proper audio range. The power spectral density of such
// a sound will be completely flat (long time averaging).

// Pink noise or 1/f noise:
// Pink noise is generated using Gardner's algorithm (see "White and Brown Music, Fractal Curves and
// 1/f fluctuations."). A short overview of the algorithm is provided here.
// The pink noise is obtained by summing the output of N uniform random number generators 
// in a particular way. Each random generator triggers a new value when its bit changes while
// counting the output value index in binary (N-bit number). Each bit in the number is associated
// with one of the random numbers. Whenever the the bit changes from 0 to 1 or 1 to 0, the associated 
// random generator computes a new value and the sum is recomputed. Since not all random generators
// fire a new value at each new samples this constraints the possible values the new output can jump
// to in a similar fashion than a lowpass filter would constrain them. Controlling the firing order
// this way result in a -3dB/octave slope (more accurately -10dB per decade) that is characteristic 
// to pink noise. Below is a 4-bit simplification of random generator firing as a function of the 
// sample index.
/*
-------------------------------------------
Bit					Generator
3	2	1	0		1	2	3	4
-------------------------------------------
1	1	1	1		(init time)	
0	0	0	0		X	X	X	X
0	0	0	1					X
0	0	1	0				X	X	
0	0	1	1					X
0	1	0	0			X	X	X
0	1	0	1					X
0	1	1	0				X	X
ETC.

A revised implementation by McCartney generates higher quality pink noise more efficiently by reordering the
firing of the random number generator as showed in the figure below.

xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx 
x x x x x x x x x x x x x x x x 
 x   x   x   x   x   x   x   x 
   x       x       x       x 
       x               x 
               x 

This ensures that only 2 generators are firing for every sample. This is accomplished by counting the
number of trailing zeroes in an increasing index.
*/


//-----------------------------------------------------------------------------
// Name: CreateToneEffect
// Desc: Plugin mechanism. Dynamic create function whose address must be 
//       registered to the FX manager.
//-----------------------------------------------------------------------------
AK::IAkPlugin* AkCreateToneSource( AK::IAkPluginMemAlloc * in_pAllocator )
{
	return AK_PLUGIN_NEW( in_pAllocator, CAkToneGen() );
}

// Plugin mechanism. Parameters node creation function to be registered to the FX manager.
AK::IAkPluginParam * AkCreateToneSourceParams(AK::IAkPluginMemAlloc * in_pAllocator)
{
	return AK_PLUGIN_NEW(in_pAllocator, CAkToneGenParams());
}

AK::PluginRegistration AkToneSourceRegistration(AkPluginTypeSource, 0, 102, AkCreateToneSource, AkCreateToneSourceParams);

// Initialize the seed only once
AkUInt32 CAkToneGen::s_uSeedVal = 22222;

//-----------------------------------------------------------------------------
// Name: CAkToneGen
// Desc: Constructor.
//-----------------------------------------------------------------------------
CAkToneGen::CAkToneGen()
{
	// Initialize members.
	m_pSharedParams = NULL;
	m_pSourceFXContext = NULL;
	m_uSampleRate = 0;
	memset(&m_staticParams,0,sizeof(m_staticParams));
	memset(&m_LpFilters,0,sizeof(m_LpFilters[0])*NUMFILTERSECTIONS);
	m_pOscWaveTable = NULL;
	m_pPinkNoiseRandGen = NULL;
	m_fpPerformDSP = NULL;
	m_iNumLoops = 1;
	m_uTotalOutSampCount = 0;
	m_uIterOutSampCount = 0;
	m_uTotalNumSamples = 0;
	m_uIterNumSamples = 0;
	m_uPhaseIndex = 0;
	m_fSampleInc = 1.f;
	m_fStartFreqModifier = 0.f;	
	m_fStopFreqModifier = 0.f;	
	m_fOscMaxFreq = 0.f;
	m_fFreqSweepScaleFactor = 0.f;
	m_bFinite = true;
	m_fOverSamplingFrequency = 0.f;
	memset(m_fEnvInc,0,sizeof(AkReal32)*4);
	m_uCurEnvSegment = 0;
	memset(m_uEnvSegmentDur,0,sizeof(AkUInt32)*4);
	m_uEnvSegmentCount = 0;
	m_fEnvCurGain = 0.f;
}

//-----------------------------------------------------------------------------
// Name: ~CAkToneGen
// Desc: Destructor.
//-----------------------------------------------------------------------------
CAkToneGen::~CAkToneGen()
{

}

//-----------------------------------------------------------------------------
// Name: Init
// Desc: Initialize tone generator 
//-----------------------------------------------------------------------------
AKRESULT CAkToneGen::Init(	AK::IAkPluginMemAlloc *			in_pAllocator,			// Memory allocator interface.
							AK::IAkSourcePluginContext *	in_pSourceFXContext,	// Source FX context
							AK::IAkPluginParam *			in_pParams,				// Effect parameters.
							AkAudioFormat &					io_rFormat				// Supported audio output format.
						  )
{

	m_pSourceFXContext = in_pSourceFXContext;

	// Output format set to Mono native by default.
	// Output to native format STRONGLY recommended but other formats are supported.

	// Save audio format internally
	m_uSampleRate = io_rFormat.uSampleRate;

	// Nyquist frequency
	m_fOscMaxFreq = AkMin( m_uSampleRate/2.f, TONEGEN_FREQUENCY_MAX );

	// Set parameters access.
	AKASSERT( NULL != in_pParams );
	m_pSharedParams = reinterpret_cast<CAkToneGenParams*>(in_pParams);

	// Keep a local copy of all parameters that are not RTPCed.
	// The value of these parameters will remain true for the lifespan of the effect,
	// regardless of changes in the UI.
	reinterpret_cast<CAkToneGenParams*>(in_pParams)->GetstaticParams( &m_staticParams );

	// Notify pipeline of chosen output format change.
	// You must also change other format field when changing number of output channel or sample type.
	io_rFormat.channelConfig.SetStandard( m_staticParams.uChannelMask );
	if ( !io_rFormat.IsChannelConfigSupported() )
	{
		// LFE channel is not supported on the some platforms. 
		// Output just mono channel in these case.
		io_rFormat.channelConfig.SetStandard( AK_SPEAKER_SETUP_MONO );
	}

	// Save number of loop iteration to perform
	// Looping info.
	m_iNumLoops = m_pSourceFXContext->GetNumLoops( );
	AKASSERT( m_iNumLoops >= 0 );
	if ( m_iNumLoops == 0 )
		m_bFinite = false;
	else
		m_bFinite = true;

	if ( m_staticParams.eGenMode == AKTONEGENMODE_ENV )
	{
		m_uEnvSegmentDur[0] = static_cast<AkUInt32>( m_staticParams.fAttackDur * m_uSampleRate );
		m_uEnvSegmentDur[1] = static_cast<AkUInt32>( m_staticParams.fDecayDur * m_uSampleRate );
		m_uEnvSegmentDur[2] = static_cast<AkUInt32>( m_staticParams.fSustainDur * m_uSampleRate );
		m_uEnvSegmentDur[3] = static_cast<AkUInt32>( m_staticParams.fReleaseDur * m_uSampleRate );

		// Compute iteration duration
		m_uIterNumSamples = m_uEnvSegmentDur[0] + m_uEnvSegmentDur[1] + m_uEnvSegmentDur[2] + m_uEnvSegmentDur[3];

		if ( m_uEnvSegmentDur[0] == 0 )
			m_uEnvSegmentDur[0] = 1;
		m_fEnvInc[0] = 1.f / m_uEnvSegmentDur[0];
		// Compute linear change
		AkReal32 fLinSustainVal = powf( 10.f, ( m_staticParams.fSustainVal / 20.f ) );
		AKASSERT( fLinSustainVal  >= 0.f && fLinSustainVal <= 1.f );
		AkReal32 fDecayLinDist = 1.f - fLinSustainVal;
		if ( m_uEnvSegmentDur[1] == 0)
			m_uEnvSegmentDur[1] = 1;
		m_fEnvInc[1] = -fDecayLinDist / m_uEnvSegmentDur[1];
		m_fEnvInc[2] = 0.f;
		if ( m_uEnvSegmentDur[3] == 0 )
			m_uEnvSegmentDur[3] = 1;
		m_fEnvInc[3] = -fLinSustainVal / m_uEnvSegmentDur[3];

		m_fEnvCurGain = 0.f;
	}
	else
	{
		// Fixed duration
		if ( m_iNumLoops == 1 )
		{
			// Not looping we can force fade in fade out
			// Note force fade-in and fade out over FADEDUR to avoid clicks
			m_uEnvSegmentDur[0] = static_cast<AkUInt32>(FADEDUR*io_rFormat.uSampleRate);
			m_uEnvSegmentDur[1] = 0;
			m_uEnvSegmentDur[2] = static_cast<AkUInt32>( m_staticParams.fFixDur * m_uSampleRate );
			m_uEnvSegmentDur[3] = static_cast<AkUInt32>(FADEDUR*io_rFormat.uSampleRate);		
			m_fEnvInc[0] = 1.f / m_uEnvSegmentDur[0];
			m_fEnvInc[1] = 0.f;
			m_fEnvInc[2] = 0.f;
			m_fEnvInc[3] = -1.f / m_uEnvSegmentDur[3];
			m_fEnvCurGain = 0.f;
		}
		else
		{
			// Disable envelope
			m_uEnvSegmentDur[0] = 0;
			m_uEnvSegmentDur[1] = 0;
			m_uEnvSegmentDur[2] = static_cast<AkUInt32>( m_staticParams.fFixDur * m_uSampleRate );
			m_uEnvSegmentDur[3] = 0;
			m_fEnvInc[0] = 0.f;
			m_fEnvInc[1] = 0.f;
			m_fEnvInc[2] = 0.f;
			m_fEnvInc[3] = 0.f;
			m_fEnvCurGain = 1.f;
		}

		// Compute iteration duration
		m_uIterNumSamples = m_uEnvSegmentDur[0] + m_uEnvSegmentDur[1] + m_uEnvSegmentDur[2] + m_uEnvSegmentDur[3];
	}	

	// In case it is infinite, this value simply gets ignored ( == 0)
	m_uTotalNumSamples = m_uIterNumSamples * m_iNumLoops;

	/////// Oscillators initialization ////////////
	if (	m_staticParams.eGenType == AKTONEGENTYPE_SINE ||
			m_staticParams.eGenType == AKTONEGENTYPE_TRIANGLE ||
			m_staticParams.eGenType == AKTONEGENTYPE_SQUARE ||
			m_staticParams.eGenType == AKTONEGENTYPE_SAWTOOTH )
	{
		AKASSERT( m_staticParams.fStartFreqRandMin <= 0.f );
		AKASSERT( m_staticParams.fStartFreqRandMax >= 0.f );

		// Compute start frequency modifier value (m_staticParams.fStartFreqRandMin,m_staticParams.fStartFreqRandMax)
		m_fStartFreqModifier = RandRange( m_staticParams.fStartFreqRandMin, m_staticParams.fStartFreqRandMax );	

		///////// Frequency sweeping initialization /////////
		if ( m_staticParams.bFreqSweep )
		{
			// Frequency sweeping enabled

			AKASSERT( m_staticParams.fStopFreqRandMin <= 0.f );
			AKASSERT( m_staticParams.fStopFreqRandMax >= 0.f );

			// Compute start frequency modifier value (m_staticParams.fStopFreqRandMin,m_staticParams.fStopFreqRandMax)
			m_fStopFreqModifier = RandRange( m_staticParams.fStopFreqRandMin, m_staticParams.fStopFreqRandMax );			
		}

	}
	// Pink noise initialization
	else if ( m_staticParams.eGenType == AKTONEGENTYPE_PINKNOISE )
	{
		// Allocate table of random number generators for pink noise
		m_pPinkNoiseRandGen = (AkReal32*)AK_PLUGIN_ALLOC( in_pAllocator, sizeof(AkReal32)*PINKNOISENUMGEN );	
		if ( m_pPinkNoiseRandGen == NULL )
			return AK_InsufficientMemory;

		m_uPinkNoiseInd = 0;
		m_uPinkNoiseIndMask = (1<<PINKNOISENUMGEN) - 1;

		m_fNoiseScale = PINKNOISEAMPCOMP / (PINKNOISENUMGEN + 1);

		// Initialize generators 
		for( unsigned int i = 0; i < PINKNOISENUMGEN; i++ ) 
		{
			m_pPinkNoiseRandGen[i] = 0.f;
		}
		m_fPinkNoiseRunSum = 0.f;

	}	
	else if ( m_staticParams.eGenType == AKTONEGENTYPE_WHITENOISE )
	{
		// Note: No initializations necessary for white noise synthesis	
	}
	else
	{
		AKASSERT( !"Invalid waveform type." );
	}

	// Compute oversampled frequency
	m_fOverSamplingFrequency = static_cast<AkReal32>( m_uSampleRate * OSCOVERSAMPLING );

	// Reset filter memory and set coefficients
	for ( unsigned int i = 0; i < NUMFILTERSECTIONS; i++ )
	{
		//The cutoff frequency was computed for 48kHz.  Transform it for the current sample rate.
		m_LpFilters[i].SetCoefs( FILTERCUTFREQ * m_uSampleRate/48000.f, m_fOverSamplingFrequency );
	}

	// Fill wavetable depending on waveform type and set proper execute function
	switch ( m_staticParams.eGenType )
	{
	case AKTONEGENTYPE_SINE:
		m_pOscWaveTable = fSineTable;
		if ( m_staticParams.bFreqSweep )
		{
			if ( m_staticParams.eGenSweep == AKTONEGENSWEEP_LOG)
				m_fpPerformDSP = &CAkToneGen::ProcessWaveTableLogSweep;
			else
				m_fpPerformDSP = &CAkToneGen::ProcessWaveTableLinSweep;
		}
		else
			m_fpPerformDSP = &CAkToneGen::ProcessWaveTableNoSweep;
		break;
	case AKTONEGENTYPE_TRIANGLE:			
		m_pOscWaveTable = fTriangleTable;
		if ( m_staticParams.bFreqSweep )
		{
			if ( m_staticParams.eGenSweep == AKTONEGENSWEEP_LOG)
				m_fpPerformDSP = &CAkToneGen::ProcessOversampledWaveTableLogSweep;
			else
				m_fpPerformDSP = &CAkToneGen::ProcessOversampledWaveTableLinSweep;
		}
		else
			m_fpPerformDSP = &CAkToneGen::ProcessOversampledWaveTableNoSweep;
		break;
	case AKTONEGENTYPE_SQUARE: 
		m_pOscWaveTable = fSquareTable;
		if ( m_staticParams.bFreqSweep )
		{
			if ( m_staticParams.eGenSweep == AKTONEGENSWEEP_LOG)
				m_fpPerformDSP = &CAkToneGen::ProcessOversampledWaveTableLogSweep;
			else
				m_fpPerformDSP = &CAkToneGen::ProcessOversampledWaveTableLinSweep;
		}
		else
			m_fpPerformDSP = &CAkToneGen::ProcessOversampledWaveTableNoSweep;
		break;
	case AKTONEGENTYPE_SAWTOOTH:
		m_pOscWaveTable = fSawtoothTable;
		if ( m_staticParams.bFreqSweep )
		{
			if ( m_staticParams.eGenSweep == AKTONEGENSWEEP_LOG)
				m_fpPerformDSP = &CAkToneGen::ProcessOversampledWaveTableLogSweep;
			else
				m_fpPerformDSP = &CAkToneGen::ProcessOversampledWaveTableLinSweep;
		}
		else
			m_fpPerformDSP = &CAkToneGen::ProcessOversampledWaveTableNoSweep;
		break;
	case AKTONEGENTYPE_WHITENOISE:
		m_fpPerformDSP = &CAkToneGen::ProcessWhiteNoise;
		break;
	case AKTONEGENTYPE_PINKNOISE:
		m_fpPerformDSP = &CAkToneGen::ProcessPinkNoise;
		break;
	default:
		AKASSERT(!"Unknown oscillator wave type");
	}

	AK_PERF_RECORDING_RESET();

	return AK_Success;
}

//-----------------------------------------------------------------------------
// Name: Term.
// Desc: Term. The effect must destroy itself herein
//-----------------------------------------------------------------------------
AKRESULT CAkToneGen::Term( AK::IAkPluginMemAlloc * in_pAllocator )
{
	if ( m_pPinkNoiseRandGen != NULL )
	{
		AK_PLUGIN_FREE( in_pAllocator, m_pPinkNoiseRandGen );
		m_pPinkNoiseRandGen = NULL;
	}

	AK_PLUGIN_DELETE( in_pAllocator, this );
	return AK_Success;
}

//-----------------------------------------------------------------------------
// Name: Reset
// Desc: Reset or seek to start (looping).
//-----------------------------------------------------------------------------
AKRESULT CAkToneGen::Reset( void )
{
	// Reset counters
	m_uTotalOutSampCount = 0;
	m_uIterOutSampCount = 0;
	m_uPhaseIndex = 0;
	m_fPhaseIndexFrac = 0.f;	
	// For envelope controls set break points as a function of the envelope parameters
	m_uCurEnvSegment = 0;
	m_uEnvSegmentCount = 0;
	if ( m_staticParams.eGenMode == AKTONEGENMODE_ENV || m_iNumLoops == 1 )
	{
		m_fEnvCurGain = 0.f;
	}
	else
	{
		m_fEnvCurGain = 1.f;
	}
	// Gain ramp initialization
	AkReal32 fGainIncrement = 1.f/(RAMPMAXTIME*m_uSampleRate);
	m_GainRamp.RampSetup( fGainIncrement, m_pSharedParams->GetGain( ) );
	
	// Reset filter memory and set coefficients
	for ( unsigned int i = 0; i < NUMFILTERSECTIONS; i++ )
	{
		m_LpFilters[i].Reset();
	}
	return AK_Success;
}

//-----------------------------------------------------------------------------
// Name: GetEffectType
// Desc: Effect information query.
//-----------------------------------------------------------------------------
AKRESULT CAkToneGen::GetPluginInfo( AkPluginInfo & out_rPluginInfo )
{
	out_rPluginInfo.eType = AkPluginTypeSource;
	out_rPluginInfo.bIsInPlace = true;
	out_rPluginInfo.uBuildVersion = AK_WWISESDK_VERSION_COMBINED;
	return AK_Success;
}

//-----------------------------------------------------------------------------
// Name: Execute
// Desc: Source effect processing.
//-----------------------------------------------------------------------------
void CAkToneGen::Execute(	AkAudioBuffer *	io_pBufferOut )
{
	AK_PERF_RECORDING_START( "ToneGenerator", 25, 30 );

	// Figure out if we can produce a full buffer before the end of the sound
	AkUInt32 uSampToProduce;
	if ( m_bFinite )
	{
		// Only produce what is necessary to finish the sound
		uSampToProduce = AkMin( (AkUInt32)io_pBufferOut->MaxFrames(), m_uTotalNumSamples - m_uTotalOutSampCount );
	}
	else
		uSampToProduce = io_pBufferOut->MaxFrames();	// Infinite, always produce maximum


	if ( uSampToProduce > 0 )
	{
		// Call appropriate DSP function
		(this->*m_fpPerformDSP)( io_pBufferOut->GetChannel(0), uSampToProduce );

		// Notify buffers of updated production
		io_pBufferOut->uValidFrames = (AkUInt16)uSampToProduce;
	}
	if ( (m_uTotalOutSampCount < m_uTotalNumSamples) || !m_bFinite )
		io_pBufferOut->eState = AK_DataReady;	// Still more to go
	else
		io_pBufferOut->eState = AK_NoMoreData;	// Last buffer was just processed

	AK_PERF_RECORDING_STOP( "ToneGenerator", 25, 30 );
}


//-----------------------------------------------------------------------------
// Name: GetDuration
// Desc: Determine approximative source duration.
//-----------------------------------------------------------------------------
AkReal32 CAkToneGen::GetDuration() const
{
	// Should return total duration, accounting for looping info
	// Note here that the calcul is done using float to avoid integer overflow
	return (m_uTotalNumSamples * 1000.f) / m_uSampleRate;
}

// Get normalized envelope.
AkReal32 CAkToneGen::GetEnvelope() const
{
	return m_fEnvCurGain;
}

AKRESULT CAkToneGen::StopLooping()
{
	m_bFinite = true;
	m_iNumLoops = 1;
	m_uTotalOutSampCount = m_uTotalOutSampCount % m_uIterNumSamples;
	m_uTotalNumSamples = m_uIterNumSamples;
	return AK_Success;
}

////////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// RandRange returns a random float value between in_fMin and in_fMax
//-----------------------------------------------------------------------------
AkReal32 CAkToneGen::RandRange( AkReal32 in_fMin, AkReal32 in_fMax )
{
	// Get an integer in range (0,1.)
	if (in_fMin != in_fMax)
	{
		AkReal32 fRandVal = rand() / static_cast<AkReal32>(RAND_MAX);
		return ( fRandVal * (in_fMax - in_fMin) + in_fMin );
	}
	return in_fMin;
}
