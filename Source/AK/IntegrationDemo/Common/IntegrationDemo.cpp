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

// IntegrationDemo.cpp
/// \file 
/// Implements all functions declared in IntegrationDemo.h

#include "stdafx.h"
#include "Platform.h"
#include "AkFilePackageLowLevelIOBlocking.h"		// Low level io

#include "IntegrationDemo.h"
#include "Drawing.h"
#include "BaseMenuPage.h"
#include "InputMgr.h"
#include "Menu.h"
#include "../WwiseProject/GeneratedSoundBanks/Wwise_IDs.h"
#include <AK/Plugin/AllPluginsFactories.h>

using namespace AKPLATFORM;

/////////////////////////////
// IntegrationDemo Members
/////////////////////////////

IntegrationDemo& IntegrationDemo::Instance()
{
	static IntegrationDemo demo;
	return demo;
}

IntegrationDemo::IntegrationDemo():
m_pInput(NULL),
m_pMenu(NULL),
m_pLowLevelIO(NULL),
m_bGamePaused(false)
{
	m_pInput = new InputMgr();
	m_pMenu = new Menu();
	m_pLowLevelIO = new CAkFilePackageLowLevelIOBlocking();
}

IntegrationDemo::~IntegrationDemo()
{
	delete m_pLowLevelIO;
	m_pLowLevelIO = NULL;
	
	delete m_pMenu;
	m_pMenu = NULL;
	
	delete m_pInput;
	m_pInput = NULL;
}

CAkFilePackageLowLevelIOBlocking& IntegrationDemo::IOManager()
{
	return *m_pLowLevelIO;
}

bool IntegrationDemo::Init(
	AkMemSettings &in_memSettings, 
	AkStreamMgrSettings &in_stmSettings, 
	AkDeviceSettings &in_deviceSettings,
	AkInitSettings &in_initSettings,
	AkPlatformInitSettings &in_platformInitSettings,
	AkMusicSettings &in_musicInit,
	void* in_pParam,
	AkOSChar* in_szErrorBuffer,
	unsigned int in_unErrorBufferCharCount,
	AkUInt32 in_windowWidth,
	AkUInt32 in_windowHeight
)
{	
	BaseMenuPage* basePage = NULL;
	bool bSuccess;

	// Initialize the drawing engine
	bSuccess = InitDrawing( in_pParam, in_szErrorBuffer, in_unErrorBufferCharCount, in_windowWidth, in_windowHeight );
	if ( !bSuccess )
	{
		goto cleanup;
	}
	
	// Initialize the Input Manager object
	bSuccess = m_pInput->Init( in_pParam, in_szErrorBuffer, in_unErrorBufferCharCount );
	if ( !bSuccess )
	{
		goto cleanup;
	}

	// Initialize Wwise
	bSuccess = InitWwise( in_memSettings, in_stmSettings, in_deviceSettings, in_initSettings, in_platformInitSettings, in_musicInit, in_szErrorBuffer, in_unErrorBufferCharCount );
	if ( !bSuccess )
	{
		goto cleanup;
	}

	// Set the path to the SoundBank Files.
	m_pLowLevelIO->SetBasePath( SOUND_BANK_PATH );
#if defined(AK_IOS)
	m_pLowLevelIO->AddBasePath( DOCUMENTS_PATH );
#endif
	
	// Set global language. Low-level I/O devices can use this string to find language-specific assets.
	if ( AK::StreamMgr::SetCurrentLanguage( AKTEXT( "English(US)" ) ) != AK_Success )
	{
		goto cleanup;
	}	
	
	// Initialize the Menu System object
	basePage = new BaseMenuPage( *m_pMenu );
	
	m_pMenu->Init( m_pInput->UniversalInputAdapter(), 
				  IntegrationDemo::Instance().GetWindowWidth(), 
				  IntegrationDemo::Instance().GetWindowHeight(), 
				  DESIRED_FPS, 
				  *basePage );

	// Initialize the timing variables
	PerformanceFrequency( &m_PerfFreq );

	return true;

cleanup:
	Term();
	return false;
}

void IntegrationDemo::StartFrame()
{
	PerformanceCounter( &m_FrameStartPerf );
}

bool IntegrationDemo::Update()
{
	if (m_bGamePaused)
	{
		return true;
	}
	
	m_pInput->Update();
	return m_pMenu->Update();
}

void IntegrationDemo::OnPointerEvent( PointerEventType in_eType, int in_x, int in_y )
{
	m_pMenu->OnPointerEvent( in_eType, in_x, in_y );
}

void IntegrationDemo::OnBack()
{
	m_pMenu->Back();
}

void IntegrationDemo::Render()
{
	if(m_bGamePaused)
		return;
#ifdef __EMSCRIPTEN__
	// With EMSCRIPTEN, our frame rate is higher and it brings video rendering performances issues, Make it skip some frames so it is fluid and more like others platforms
	static int iFrameCounter = -1; //init -1 so first frame is 0.
	++iFrameCounter;
	if (iFrameCounter % 3 == 0)
#endif
	{
	m_pMenu->Draw();
	}
	
	AK::SoundEngine::RenderAudio();
}

void IntegrationDemo::EndFrame()
{
	AkInt32 iSleepTime = CalculateSleepTime();
	AkSleep( iSleepTime );
}

void IntegrationDemo::Term()
{
	TermDrawing();
	m_pInput->Release();
	m_pMenu->Release();

	// Terminate Wwise
	TermWwise();
}

void IntegrationDemo::GetDefaultSettings(AkMemSettings &out_memSettings, 
										 AkStreamMgrSettings &out_stmSettings, 
										 AkDeviceSettings &out_deviceSettings,
										 AkInitSettings &out_initSettings,
										 AkPlatformInitSettings &out_platformInitSettings,
										 AkMusicSettings &out_musicInit)
{
	out_memSettings.uMaxNumPools = 20;
	AK::StreamMgr::GetDefaultSettings( out_stmSettings );
	
	AK::StreamMgr::GetDefaultDeviceSettings( out_deviceSettings );
	
	AK::SoundEngine::GetDefaultInitSettings( out_initSettings );
	out_initSettings.uDefaultPoolSize = DEMO_DEFAULT_POOL_SIZE;
#if defined( INTEGRATIONDEMO_ASSERT_HOOK )
	out_initSettings.pfnAssertHook = INTEGRATIONDEMO_ASSERT_HOOK;
#endif // defined( INTEGRATIONDEMO_ASSERT_HOOK )
	
	AK::SoundEngine::GetDefaultPlatformInitSettings( out_platformInitSettings );
	out_platformInitSettings.uLEngineDefaultPoolSize = DEMO_LENGINE_DEFAULT_POOL_SIZE;

	AK::MusicEngine::GetDefaultInitSettings( out_musicInit );
}

bool IntegrationDemo::InitWwise(
	AkMemSettings &in_memSettings, 
	AkStreamMgrSettings &in_stmSettings, 
	AkDeviceSettings &in_deviceSettings,
	AkInitSettings &in_initSettings,
	AkPlatformInitSettings &in_platformInitSettings,
	AkMusicSettings &in_musicInit,
	AkOSChar* in_szErrorBuffer,
	unsigned int in_unErrorBufferCharCount
)
{
    //
    // Create and initialize an instance of the default memory manager. Note
    // that you can override the default memory manager with your own. Refer
    // to the SDK documentation for more information.
    //

	AKRESULT res = AK::MemoryMgr::Init( &in_memSettings );
    if ( res != AK_Success )
    {
		__AK_OSCHAR_SNPRINTF( in_szErrorBuffer, in_unErrorBufferCharCount, AKTEXT("AK::MemoryMgr::Init() returned AKRESULT %d"), res );
        return false;
    }

	//
    // Create and initialize an instance of the default streaming manager. Note
    // that you can override the default streaming manager with your own. Refer
    // to the SDK documentation for more information.
    //

    // Customize the Stream Manager settings here.
    
    if ( !AK::StreamMgr::Create( in_stmSettings ) )
    {
		AKPLATFORM::SafeStrCpy( in_szErrorBuffer, AKTEXT( "AK::StreamMgr::Create() failed" ), in_unErrorBufferCharCount );
        return false;
    }
    
	// 
    // Create a streaming device with blocking low-level I/O handshaking.
    // Note that you can override the default low-level I/O module with your own. Refer
    // to the SDK documentation for more information.        
	//
    
    // CAkFilePackageLowLevelIOBlocking::Init() creates a streaming device
    // in the Stream Manager, and registers itself as the File Location Resolver.
	in_deviceSettings.bUseStreamCache = true;
	res = m_pLowLevelIO->Init( in_deviceSettings );
	if ( res != AK_Success )
	{
		__AK_OSCHAR_SNPRINTF( in_szErrorBuffer, in_unErrorBufferCharCount, AKTEXT("m_lowLevelIO.Init() returned AKRESULT %d"), res );
        return false;
    }

    //
    // Create the Sound Engine
    // Using default initialization parameters
    //
    
	res = AK::SoundEngine::Init( &in_initSettings, &in_platformInitSettings );
    if ( res != AK_Success )
    {
		__AK_OSCHAR_SNPRINTF( in_szErrorBuffer, in_unErrorBufferCharCount, AKTEXT("AK::SoundEngine::Init() returned AKRESULT %d"), res );
        return false;
    }

    //
    // Initialize the music engine
    // Using default initialization parameters
    //

	res = AK::MusicEngine::Init( &in_musicInit );
    if ( res != AK_Success )
    {
		__AK_OSCHAR_SNPRINTF( in_szErrorBuffer, in_unErrorBufferCharCount, AKTEXT("AK::MusicEngine::Init() returned AKRESULT %d"), res );
        return false;
    }

#if !defined AK_OPTIMIZED && !defined INTEGRATIONDEMO_DISABLECOMM
    //
    // Initialize communications (not in release build!)
    //
	AkCommSettings commSettings;
	AK::Comm::GetDefaultInitSettings( commSettings );
	AKPLATFORM::SafeStrCpy(commSettings.szAppNetworkName, "Integration Demo", AK_COMM_SETTINGS_MAX_STRING_SIZE);
	res = AK::Comm::Init( commSettings );
	if ( res != AK_Success )
	{
		__AK_OSCHAR_SNPRINTF( in_szErrorBuffer, in_unErrorBufferCharCount, AKTEXT("AK::Comm::Init() returned AKRESULT %d. Communication between the Wwise authoring application and the game will not be possible."), res );
	}
#endif // AK_OPTIMIZED	
   
	AkSpatialAudioInitSettings settings;
	settings.uDiffractionFlags = DiffractionFlags_UseBuiltInParam | DiffractionFlags_UseObstruction | DiffractionFlags_CalcEmitterVirtualPosition;
	res = AK::SpatialAudio::Init(settings);
	if (res != AK_Success)
	{
		__AK_OSCHAR_SNPRINTF(in_szErrorBuffer, in_unErrorBufferCharCount, AKTEXT("AK::SpatialAudio::Init() returned AKRESULT %d"), res);
		return false;
	}

	AK::SoundEngine::RegisterGameObj(LISTENER_ID, "Listener (Default)");
	AK::SoundEngine::SetDefaultListeners(&LISTENER_ID, 1);

    return true;
}

void IntegrationDemo::TermWwise()
{
#if !defined AK_OPTIMIZED && !defined INTEGRATIONDEMO_DISABLECOMM
	// Terminate communications between Wwise and the game
	AK::Comm::Term();
#endif // AK_OPTIMIZED
	// Terminate the music engine
	AK::MusicEngine::Term();

	// Terminate the sound engine
	if ( AK::SoundEngine::IsInitialized() )
	{
		AK::SoundEngine::Term();
	}

	// Terminate the streaming device and streaming manager
	// CAkFilePackageLowLevelIOBlocking::Term() destroys its associated streaming device 
	// that lives in the Stream Manager, and unregisters itself as the File Location Resolver.
    if ( AK::IAkStreamMgr::Get() )
	{
		m_pLowLevelIO->Term();
		AK::IAkStreamMgr::Get()->Destroy();
	}

	// Terminate the Memory Manager
	if ( AK::MemoryMgr::IsInitialized() )
	{
		AK::MemoryMgr::Term();
	}
}

AkInt32 IntegrationDemo::CalculateSleepTime()
{
	AkInt64  iFrameEndPerf;	  // Performance counter at the end of the current frame
	PerformanceCounter( &iFrameEndPerf );

	// Calculate how long the current frame has lasted so far
	AkReal64 dblCurrFrameDur = ( (AkReal64)( iFrameEndPerf - m_FrameStartPerf ) * 1000 / (AkReal64)m_PerfFreq );

	// We want sleep the remainder to get to a duration of MS_PER_FRAME
	AkInt32 iSleepTime = AkMax( 0, (AkInt32)(MS_PER_FRAME - dblCurrFrameDur));
	
	return iSleepTime;	 
}


void IntegrationDemo::PauseAllSounds()
{
	if ( AK::SoundEngine::IsInitialized() )
	{
		AK::SoundEngine::PostEvent(AK::EVENTS::PAUSE_ALL_GLOBAL, AK_INVALID_GAME_OBJECT);
		AK::SoundEngine::RenderAudio();

		AK::SoundEngine::Suspend(false);
		IntegrationDemo::Instance().SetGamePaused(true);
	}
}

void IntegrationDemo::ResumeAllSounds()
{
	if ( AK::SoundEngine::IsInitialized() )
	{
		AK::SoundEngine::WakeupFromSuspend();
		AK::SoundEngine::PostEvent(AK::EVENTS::RESUME_ALL_GLOBAL, AK_INVALID_GAME_OBJECT);
		AK::SoundEngine::RenderAudio();
		
		IntegrationDemo::Instance().SetGamePaused(false);
	}
}
