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
// AkDelayFX.h
//
// Sample delay FX implementation.
//
//////////////////////////////////////////////////////////////////////

#ifndef _AK_DELAYFX_H_
#define _AK_DELAYFX_H_

#include "AkDelayFXParams.h"
#include "AkDelayFXDSP.h"

//-----------------------------------------------------------------------------
// Name: class CAkDelayFX
//-----------------------------------------------------------------------------
class CAkDelayFX : public AK::IAkInPlaceEffectPlugin
{
public:

    /// Constructor
    CAkDelayFX();

	 /// Destructor
    ~CAkDelayFX();

	/// Effect plug-in initialization
    AKRESULT Init(	AK::IAkPluginMemAlloc *			in_pAllocator,		/// Memory allocator interface.
					AK::IAkEffectPluginContext *	in_pFXCtx,			/// Sound engine plug-in execution context.
                    AK::IAkPluginParam *			in_pParams,			/// Associated effect parameters node.
                    AkAudioFormat &					in_rFormat			/// Input/output audio format.
                    );
    
	/// Effect plug-in termination
	AKRESULT Term( AK::IAkPluginMemAlloc * in_pAllocator );

	/// Reset effect
	AKRESULT Reset( );

    /// Effect info query
    AKRESULT GetPluginInfo( AkPluginInfo & out_rPluginInfo );

    /// Effect plug-in DSP processing
	void Execute(	AkAudioBuffer * io_pBuffer		/// Input/Output audio buffer structure.		
		);

	/// Execution processing when the voice is virtual. Nothing special to do for this effect.
	AKRESULT TimeSkip(AkUInt32 in_uFrames){ return AK_DataReady; }	

private:

	/// Cached information
	CAkDelayFXDSP			m_FXState;		/// Internal effect state
    CAkDelayFXParams *		m_pParams;		/// Effect parameter node
	AK::IAkPluginMemAlloc * m_pAllocator;	/// Memory allocator interface
};

#endif // _AK_DELAYFX_H_
