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

// main.cpp
/// \file 
/// Contains the entry point for the application.

/////////////////////////
//  INCLUDES
/////////////////////////

#include "stdafx.h"

#include <malloc.h>
#include "IntegrationDemo.h"
#include "Helpers.h"
#include "Platform.h"

/////////////////////////
//  PROTOTYPES
/////////////////////////

LRESULT CALLBACK WindowProc( HWND, UINT, WPARAM, LPARAM );



/////////////////////////////////////////////////////////////////////////////////
//                              MEMORY HOOKS SETUP
//
//                             ##### IMPORTANT #####
//
// These custom alloc/free functions are declared as "extern" in AkMemoryMgr.h
// and MUST be defined by the game developer.
/////////////////////////////////////////////////////////////////////////////////

namespace AK
{
	void * AllocHook( size_t in_size )
	{
		return malloc( in_size );
	}
	void FreeHook( void * in_ptr )
	{
		free( in_ptr );
	}
	void * VirtualAllocHook(
		void * in_pMemAddress,
		size_t in_size,
		DWORD in_dwAllocationType,
		DWORD in_dwProtect
		)
	{
		return VirtualAlloc( in_pMemAddress, in_size, in_dwAllocationType, in_dwProtect );
	}
	void VirtualFreeHook( 
		void * in_pMemAddress,
		size_t in_size,
		DWORD in_dwFreeType
		)
	{
		VirtualFree( in_pMemAddress, in_size, in_dwFreeType );
	}
}


/////////////////////////
//  MAIN
/////////////////////////

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int nCmdShow )
{
	HWND hWnd;
	WNDCLASSEX wc;
	MSG  msg;

	AkUInt32 width = 640;
	AkUInt32 height = 480;

	// Setup the application's window
    ZeroMemory( &wc, sizeof( WNDCLASSEX ) );
	wc.cbSize = sizeof( WNDCLASSEX );
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpfnWndProc = WindowProc;
    wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.hInstance = hInstance;
	wc.lpszClassName = L"DemoWindowClass";
	RegisterClassEx( &wc );
	hWnd = CreateWindowEx( NULL,
                           L"DemoWindowClass",    
                           L"Wwise Integration Demo",
                           ( WS_OVERLAPPEDWINDOW ^ WS_MAXIMIZEBOX ),
                           300,    // x-position of window
                           300,    // y-position of window
                           width,
                           height + 30,  // Compensate for the top bar
                           NULL,
                           NULL,
                           hInstance,
                           NULL );

	AkMemSettings memSettings;
	AkStreamMgrSettings stmSettings;
	AkDeviceSettings deviceSettings;
	AkInitSettings initSettings;
	AkPlatformInitSettings platformInitSettings;
	AkMusicSettings musicInit;
	IntegrationDemo::Instance().GetDefaultSettings(memSettings, stmSettings, deviceSettings, initSettings, platformInitSettings, musicInit);
	//initSettings.settingsMainOutput.audioDeviceShareset = AK::SoundEngine::GetIDFromString("New_AK_Rumble_Sink_Plugin");
	
	platformInitSettings.hWnd = hWnd;
	
	// Initialize the various components of the application and show the window
	AkOSChar szError[500];
	if ( !IntegrationDemo::Instance().Init( memSettings, stmSettings, deviceSettings, initSettings, platformInitSettings, musicInit, hWnd, szError, (unsigned int) IntegrationDemoHelpers::AK_ARRAYSIZE(szError), width, height ) )
	{
		AkOSChar szMsg[550];
		__AK_OSCHAR_SNPRINTF( szMsg, IntegrationDemoHelpers::AK_ARRAYSIZE(szMsg), AKTEXT("Failed to initialize the Integration Demo.\r\n%s"), szError );
		MessageBox( NULL, szMsg, L"Error", MB_ICONERROR | MB_OK );
		return 1;
	}
	ShowWindow( hWnd, nCmdShow );


	// The application's main loop
	// Each iteration represents 1 frame
	for(;;) 
	{
		// Records the starting time of the current frame
		IntegrationDemo::Instance().StartFrame();

		// Empty the Windows message queue
		while ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
		if ( msg.message == WM_QUIT )
		{
			break;
		}

		// Process the current frame, quit if Update() is false.
		if ( ! IntegrationDemo::Instance().Update() )
		{
			break;
		}
		IntegrationDemo::Instance().Render();
		
		// Ends the frame and regulates the application's framerate
		IntegrationDemo::Instance().EndFrame();
	}

	// Terminate the various components of the application
	IntegrationDemo::Instance().Term();

	return 0;
}



LRESULT CALLBACK WindowProc( HWND in_hWnd, UINT in_unMsg, WPARAM in_wParam, LPARAM in_lParam )
{
	switch ( in_unMsg )
	{
	case WM_SIZE:
		if(in_wParam == SIZE_MINIMIZED)
			IntegrationDemo::Instance().PauseAllSounds();
		if(in_wParam == SIZE_RESTORED || in_wParam == SIZE_MAXIMIZED)
			IntegrationDemo::Instance().ResumeAllSounds();
		break;
	case WM_DESTROY:
		PostQuitMessage( 0 );
		return TRUE;
	}
	return ( DefWindowProc( in_hWnd, in_unMsg, in_wParam, in_lParam ) );
}