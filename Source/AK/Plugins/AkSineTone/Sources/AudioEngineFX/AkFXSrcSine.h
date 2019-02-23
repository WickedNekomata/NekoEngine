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
// AkFXSrcSine.h
//
// Sample sine tone source. Implements a very basic oscillator.
//
//////////////////////////////////////////////////////////////////////

#ifndef _AKFXSRC_SINE_H_
#define _AKFXSRC_SINE_H_

#include "AkFXSrcSineParams.h"
#include "AkSineOsc.h"
#include <AK/Plugin/PluginServices/AkFXDurationHandler.h>

struct AkSineFXState
{
	AkFXDurationHandler DurationHandler;	// Helper to manage looping and how many frames left to produce based on changing duration
	AK::DSP::AkSineOsc	Oscillator;			// Sine oscillator
	AkReal32			m_fPreviousGain;	// Previous gain value for interpolation
}  AK_ALIGN_DMA;

class CAkFXSrcSine : public AK::IAkSourcePlugin
{
public:

    /// Constructor.
    CAkFXSrcSine();

	/// Destructor.
    virtual ~CAkFXSrcSine();

    /// Source plug-in initialization.
    virtual AKRESULT Init(	AK::IAkPluginMemAlloc *			in_pAllocator,		/// Memory allocator interface.
					AK::IAkSourcePluginContext *	in_pSourceFXContext,/// Sound engine plug-in execution context.
                    AK::IAkPluginParam *			in_pParams,			/// Associated effect parameters node.
                    AkAudioFormat &					io_rFormat			/// Output audio format.
                    );

    /// Source plug-in termination.
    virtual AKRESULT Term( AK::IAkPluginMemAlloc * in_pAllocator );

	/// Reset effect.
	virtual AKRESULT Reset( );

    /// Plug-in info query.
    virtual AKRESULT GetPluginInfo( AkPluginInfo & out_rPluginInfo );

    /// Effect plug-in DSP processing.
	virtual void Execute(	AkAudioBuffer *							io_pBuffer	/// Output audio buffer structure.	
					);

	/// Get the total duration of the source in milliseconds.
	virtual AkReal32 GetDuration( ) const;

	/// Looping stop signal.
	virtual AKRESULT StopLooping();

	/// Skip processing of some frames when voice is virtual from elapsed time.
	virtual AKRESULT TimeSkip(AkUInt32 &io_uFrames );

private:

	AkSineFXState		 m_SineState;		// Internal effect state
	CAkFxSrcSineParams * m_pParams;			// Parameters node
};

#endif // _AKFXSRC_SINE_H_
