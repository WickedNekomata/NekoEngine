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

// IntegrationDemo.h
/// \file 
/// Declares a wrapper class for the integration demo application.

#pragma once

#include <AK/SoundEngine/Common/AkTypes.h>

#include <AK/SoundEngine/Common/AkMemoryMgr.h>		// Memory Manager
#include <AK/SoundEngine/Common/AkModule.h>			// Default memory and stream managers
#include <AK/SoundEngine/Common/IAkStreamMgr.h>		// Streaming Manager
#include <AK/SoundEngine/Common/AkSoundEngine.h>    // Sound engine
#include <AK/MusicEngine/Common/AkMusicEngine.h>	// Music Engine
#include <AK/SoundEngine/Common/AkStreamMgrModule.h>	// AkStreamMgrModule
#include <AK/SpatialAudio/Common/AkSpatialAudio.h>	// Spatial Audio module

#ifndef AK_OPTIMIZED
#include <AK/Comm/AkCommunication.h>	// Communication between Wwise and the game (excluded in release build)
#endif

#include "Drawing.h"
#include "UniversalInput.h"

//#include "AkFilePackageLowLevelIOBlocking.h"

/////////////////////////
//  GLOBAL CONSTANTS
/////////////////////////

static const AkUInt16 DESIRED_FPS   = 60;
static const AkReal32 MS_PER_FRAME  = ( 1000 / (AkReal32)DESIRED_FPS );

static const AkGameObjectID LISTENER_ID = 10000;

// Please use foward declaration to keep to reduce include size
class InputMgr;
class Menu;
class CAkFilePackageLowLevelIOBlocking;

//////////////////////////////
//  IntegrationDemo Class
//////////////////////////////

/// Wrapper class for the Integration Demo application. Implements a Singleton design pattern.
class IntegrationDemo
{
public:

	/// Returns a reference to the instance of the Integration Demo application.
	/// \return Reference to the instance of the IntegrationDemo object.
	static IntegrationDemo& Instance();
	

	/// Accessor to the low-level IO manager.
	/// \return Reference to the Low-level IO manager used by the demo application.
	CAkFilePackageLowLevelIOBlocking& IOManager();

	/// Gets the default settings for the various sub components of the sound engine.  Modify these settings to suit your application better.
	void GetDefaultSettings(AkMemSettings& out_memSettings, AkStreamMgrSettings& out_stmSettings, AkDeviceSettings& out_deviceSettings, AkInitSettings& out_initSettings, AkPlatformInitSettings& out_platformInitSettings, AkMusicSettings& out_musicInit);

	/// Initializes the Integration Demo application.
	/// \return True if the application's modules initialized properly and false otherwise.
	bool Init(
		AkMemSettings &in_memSettings, 
		AkStreamMgrSettings &in_stmSettings, 
		AkDeviceSettings &in_deviceSettings,
		AkInitSettings &in_initSettings,
		AkPlatformInitSettings &in_platformInitSettings,
		AkMusicSettings &in_musicInit,
		void* in_pParam,						///< - Extra parameter that might be necessary to initialize the platform specific components of the application
		AkOSChar* in_szErrorBuffer,				///< - Buffer where error details will be written (if the function returns false)
		unsigned int in_unErrorBufferCharCount,	///< - Number of characters available in in_szErrorBuffer, including terminating NULL character
		AkUInt32 in_windowWidht = 640,
		AkUInt32 in_windowHeight = 480
			  );

	/// Call this method at the start of every game loop itteration to signal the 
	/// application to start a new frame.
	void StartFrame();

	/// Processes a single frame of the application.
	/// \return False when the user wants to quit the Integration Demo, otherwise True.
	bool Update();

	void OnPointerEvent( PointerEventType in_eType, int in_x, int in_y );

	/// Back button pressed.
	void OnBack();

	/// Renders a single frame of the application.
	void Render();

	/// Call this method at the end of every game loop itteration to signal the
	/// application to end the current frame and regulate framerate.
	void EndFrame();

	/// Uninitializes the application's components and releases any used resources.
	void Term();
	
	/// Get the application input mgr
	InputMgr* GetInputMgr()  { return m_pInput; }
	
	/// Get the application window height
	AkUInt32 GetWindowWidth() { return ::GetWindowWidth(); }
	
	/// Get the application window width
	AkUInt32 GetWindowHeight() { return ::GetWindowHeight(); }

	CAkFilePackageLowLevelIOBlocking* GetLowLevelIOHandler() { return m_pLowLevelIO; }


	void PauseAllSounds();
	void ResumeAllSounds();
	
	inline void SetGamePaused(bool in_bPaused) { m_bGamePaused = in_bPaused; }
	inline bool IsGamePaused() { return m_bGamePaused; }

private:

	/// Hidden default constructor.
	IntegrationDemo();

	/// Hidden copy constructor.
	IntegrationDemo( IntegrationDemo const& ){};

	/// Hidden assignment operator.
	IntegrationDemo& operator=( IntegrationDemo const& );
	
	/// Hidden default desctructor.
	~IntegrationDemo();
	

	/// Initializes the Wwise sound engine.
	bool InitWwise( 
		AkMemSettings &in_memSettings, 
		AkStreamMgrSettings &in_stmSettings, 
		AkDeviceSettings &in_deviceSettings,
		AkInitSettings &in_initSettings,
		AkPlatformInitSettings &in_platformInitSettings,
		AkMusicSettings &in_musicInit,		
		AkOSChar* in_szErrorBuffer,				///< - Buffer where error details will be written (if the function returns false)
		unsigned int in_unErrorBufferCharCount	///< - Number of characters available in in_szErrorBuffer, including terminating NULL character
	);

	/// Terminates the Wwise sound engine
	void TermWwise();

	/// Calculates the amount of time to sleep at the end of each frame.
	AkInt32 CalculateSleepTime();
	// Menu System Objects
	InputMgr* m_pInput;	///< The application's input manager pointer.
	Menu*     m_pMenu;	///< The menu system pointer.

	/// We're using the default Low-Level I/O implementation that's part
	/// of the SDK's sample code, with the file package extension
	CAkFilePackageLowLevelIOBlocking* m_pLowLevelIO;

	// Timing Variables
	AkInt64  m_PerfFreq;			///< The system's performance frequency.
	AkInt64  m_FrameStartPerf;		///< Performance counter value at the start of a frame.
	
	bool m_bGamePaused;
};
