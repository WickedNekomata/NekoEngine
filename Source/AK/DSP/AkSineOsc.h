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

#ifndef _AK_SINEOSCDSP_H_
#define _AK_SINEOSCDSP_H_

#include <AK/SoundEngine/Common/AkTypes.h>
#include <AK/SoundEngine/Common/AkCommonDefs.h>
#include <AK/Tools/Common/AkAssert.h>

namespace AK
{
	namespace DSP
	{

		static const AkReal32 TWOPI	= 6.2831853f;
		static const AkReal32 PI	= 3.1415926f;

		/// Vectorized mono sine oscillator DSP class (can be used for LFO or audio rate).
		class AkSineOsc
		{
		public:

			/// Setup DSP processing for sine oscillator.
			inline void Setup( AkReal32 in_fFrequency, AkReal32 in_fInitialPhase, AkUInt32 in_uSampleRate )
			{
				AKASSERT( in_fInitialPhase >= -PI && in_fInitialPhase < PI );
				m_fPhaseOffset = in_fInitialPhase;
				m_fSampleRate = (AkReal32)in_uSampleRate;
				SetFrequency( in_fFrequency );
			}

			/// Change sine oscillator frequency.
			inline void SetFrequency( AkReal32 in_fFrequency )
			{
				AKASSERT( in_fFrequency >= 0.f && in_fFrequency <= m_fSampleRate*0.5f );
				m_fPhaseIncrement = TWOPI * in_fFrequency / m_fSampleRate; 
			}

			/// Fill provided buffer (for in_uNumFrames) of sine waveform.
			void Process( 
				AkSampleType * out_pfBuffer, 
				AkUInt32 in_uNumFrames, 
				AkReal32 in_fPrevGain = 1.f, 
				AkReal32 in_fTargetGain = 1.f );

		private:

			AkReal32	m_fPhaseOffset;		// Sine phase offset (0-2Pi)
			AkReal32	m_fPhaseIncrement;	// Phase increment (function of frequency)
			AkReal32	m_fSampleRate;		// Sample rate

		} AK_ALIGN_DMA;

	} // namespace DSP
} // namespace AK

#endif // _AK_SINEOSCDSP_H_
