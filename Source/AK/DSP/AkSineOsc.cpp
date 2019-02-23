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

#include "AkSineOsc.h"
#include <AK/SoundEngine/Common/AkSimd.h>
#include <AK/SoundEngine/Common/IAkPlugin.h>
#include <AK/SoundEngine/Common/AkFPUtilities.h>
#include <AK/DSP/AkApplyGain.h>
#include <math.h>

/// Vectorized sine oscillator DSP class (can be used for LFO or audio rate).

namespace AK
{
	namespace DSP
	{

		/// Fill AkAudioBuffer (for uValidFrames) of sine waveform.
		void AkSineOsc::Process( 
			AkSampleType * out_pfBuffer, 
			AkUInt32 in_uNumFrames, 
			AkReal32 in_fPrevGain /* = 1.f */, 
			AkReal32 in_fTargetGain /* = 1.f */)
		{
#ifdef AKSIMD_V4F32_SUPPORTED
			AKASSERT( in_uNumFrames % 4 == 0 );		
			AkSampleType * AK_RESTRICT pBufOut = out_pfBuffer;
			const AkSampleType * const pBufEnd = pBufOut + in_uNumFrames;

			// 7th order Taylor series expansion sin(x) = x - x^3 / 3! + x^5 / 5! - x^7 / 7! 
			// This already gives pretty good convergence for range (-PI,PI)
			// Using Series economization techniques and chebyshevs polynomials we can compute coefficients 
			// that maximize precision for the above range
			const AkReal32 fEconomizedTaylorSinCoefficients[3] = {-1.66521856991541e-1f, 8.199913018755e-3f, -1.61475937228e-4f};
			const AKSIMD_V4F32 vCoefOrd3 = AKSIMD_LOAD1_V4F32(fEconomizedTaylorSinCoefficients[0]);
			const AKSIMD_V4F32 vCoefOrd5 = AKSIMD_LOAD1_V4F32(fEconomizedTaylorSinCoefficients[1]);
			const AKSIMD_V4F32 vCoefOrd7 = AKSIMD_LOAD1_V4F32(fEconomizedTaylorSinCoefficients[2]);
			// Phase wrap constants
			const AKSIMD_V4F32 vPI = AKSIMD_SET_V4F32( PI );
			const AKSIMD_V4F32 vTWOPI = AKSIMD_SET_V4F32( TWOPI );

			AkReal32 fPhaseIncrement = m_fPhaseIncrement;
			AK_ALIGN_SIMD( AkReal32 fInPhase[4] );
			fInPhase[0] = m_fPhaseOffset;
			fInPhase[1] = fInPhase[0] + fPhaseIncrement;
			AK_FPSetValGTE( fInPhase[1], PI, fInPhase[1], fInPhase[1] - TWOPI );
			fInPhase[2] = fInPhase[1] + fPhaseIncrement;
			AK_FPSetValGTE( fInPhase[2], PI, fInPhase[2], fInPhase[2] - TWOPI );
			fInPhase[3] = fInPhase[2] + fPhaseIncrement;
			AK_FPSetValGTE( fInPhase[3], PI, fInPhase[3], fInPhase[3] - TWOPI );
			fPhaseIncrement *= 4.f;
			// Support for frequency higher than Sr/4
			AK_FPSetValGTE( fPhaseIncrement, TWOPI, fPhaseIncrement, fPhaseIncrement - TWOPI ); 
			const AKSIMD_V4F32 vPhaseInc = AKSIMD_LOAD1_V4F32( fPhaseIncrement );
			AKSIMD_V4F32 vInPhase = AKSIMD_LOAD_V4F32( fInPhase );		

			while ( pBufOut < pBufEnd )
			{
				AKSIMD_V4F32 vX2 = AKSIMD_MUL_V4F32(vInPhase, vInPhase);
				AKSIMD_V4F32 vOut = AKSIMD_MUL_V4F32(vX2, vInPhase);
				AKSIMD_V4F32 vX5 = AKSIMD_MUL_V4F32(vOut, vX2);
				vOut = AKSIMD_MADD_V4F32(vCoefOrd3, vOut, vInPhase);
				AKSIMD_V4F32 vX7 = AKSIMD_MUL_V4F32(vX5, vX2);
				vOut = AKSIMD_MADD_V4F32(vCoefOrd5, vX5, vOut);
				vOut = AKSIMD_MADD_V4F32(vCoefOrd7, vX7, vOut);
				AKSIMD_STORE_V4F32((AKSIMD_F32*)pBufOut,vOut);
				pBufOut += 4;
				vInPhase = AKSIMD_ADD_V4F32(vInPhase,vPhaseInc);
				vInPhase = AKSIMD_SEL_GTEQ_V4F32( vInPhase, AKSIMD_SUB_V4F32(vInPhase,vTWOPI), vInPhase, vPI );
			}

			// Tear-down vector processing
			m_fPhaseOffset = AKSIMD_GETELEMENT_V4F32( vInPhase, 0 );

			AK::DSP::ApplyGain( out_pfBuffer, in_fPrevGain, in_fTargetGain, in_uNumFrames );

#elif defined (AKSIMD_V2F32_SUPPORTED)
			AKASSERT( in_uNumFrames % 4 == 0 );		
			AkSampleType * AK_RESTRICT pBufOut = out_pfBuffer;
			const AkSampleType * const pBufEnd = pBufOut + in_uNumFrames;

			// 7th order Taylor series expansion sin(x) = x - x^3 / 3! + x^5 / 5! - x^7 / 7! 
			// This already gives pretty good convergence for range (-PI,PI)
			// Using Series economization techniques and chebyshevs polynomials we can compute coefficients 
			// that maximize precision for the above range
			const AkReal32 fEconomizedTaylorSinCoefficients[3] = {-1.66521856991541e-1f, 8.199913018755e-3f, -1.61475937228e-4f};
			const AKSIMD_V2F32 vCoefOrd3 = AKSIMD_SET_V2F32(fEconomizedTaylorSinCoefficients[0]);
			const AKSIMD_V2F32 vCoefOrd5 = AKSIMD_SET_V2F32(fEconomizedTaylorSinCoefficients[1]);
			const AKSIMD_V2F32 vCoefOrd7 = AKSIMD_SET_V2F32(fEconomizedTaylorSinCoefficients[2]);
			// Phase wrap constants
			AKSIMD_BUILD_V2F32( const AKSIMD_V2F32 vPI, PI, PI );
			AKSIMD_BUILD_V2F32( const AKSIMD_V2F32 vTWOPI, TWOPI, TWOPI );

			AkReal32 fPhaseIncrement = m_fPhaseIncrement;
			AkReal32 fInPhase0 = m_fPhaseOffset;
			AkReal32 fInPhase1 = fInPhase0 + fPhaseIncrement;
			AK_FPSetValGTE( fInPhase1, PI, fInPhase1, fInPhase1 - TWOPI );
			fPhaseIncrement *= 2.f;
			// Support for frequency higher than Sr/4
			AK_FPSetValGTE( fPhaseIncrement, TWOPI, fPhaseIncrement, fPhaseIncrement - TWOPI ); 
			const AKSIMD_V2F32 vPhaseInc = AKSIMD_SET_V2F32( fPhaseIncrement );
			AKSIMD_BUILD_V2F32( AKSIMD_V2F32 vInPhase, fInPhase0, fInPhase1 );

			while ( pBufOut < pBufEnd )
			{
				AKSIMD_V2F32 vX2 = AKSIMD_MUL_V2F32(vInPhase, vInPhase);
				AKSIMD_V2F32 vOut = AKSIMD_MUL_V2F32(vX2, vInPhase);
				AKSIMD_V2F32 vX5 = AKSIMD_MUL_V2F32(vOut, vX2);
				vOut = AKSIMD_MADD_V2F32(vCoefOrd3, vOut, vInPhase);
				AKSIMD_V2F32 vX7 = AKSIMD_MUL_V2F32(vX5, vX2);
				vOut = AKSIMD_MADD_V2F32(vCoefOrd5, vX5, vOut);
				vOut = AKSIMD_MADD_V2F32(vCoefOrd7, vX7, vOut);
				AKSIMD_STORE_V2F32_OFFSET((AKSIMD_F32*)pBufOut,0,vOut);
				pBufOut += 2;
				vInPhase = AKSIMD_ADD_V2F32(vInPhase,vPhaseInc);
				vInPhase = AKSIMD_SEL_GTEQ_V2F32( vInPhase, AKSIMD_SUB_V2F32(vInPhase,vTWOPI), vInPhase, vPI );
			}

			// Tear-down vector processing
			m_fPhaseOffset = AKSIMD_GETELEMENT_V2F32( vInPhase, 0 );

			AK::DSP::ApplyGain( out_pfBuffer, in_fPrevGain, in_fTargetGain, in_uNumFrames );
#else

			const AkReal32 fPhaseIncrement = m_fPhaseIncrement;
			AkReal32 fPhase = m_fPhaseOffset; 
			AkSampleType * AK_RESTRICT pBufOut = (AkSampleType * AK_RESTRICT)out_pfBuffer;
			const AkSampleType * const pBufEnd = pBufOut + in_uNumFrames;
			AkReal32 fCurGain = in_fPrevGain;
			const AkReal32 fGainInc =	(in_fTargetGain - in_fPrevGain) / in_uNumFrames;

			while ( pBufOut < pBufEnd )
			{
				AkReal32 fSampleValue = fCurGain * sinf( fPhase );
				fCurGain += fGainInc;
				*pBufOut++ = AK_FLOAT_TO_SAMPLETYPE_NOCLIP(fSampleValue);
				fPhase += fPhaseIncrement;
				if ( fPhase >= TWOPI )
					fPhase -= TWOPI;
			}
			
			m_fPhaseOffset = fPhase;
#endif
		}

	} // Namespace DSP
} // Namespace AK

