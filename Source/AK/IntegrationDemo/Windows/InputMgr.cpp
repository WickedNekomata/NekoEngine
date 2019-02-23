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

// InputMgr.cpp
/// \file 
/// Defines the methods declared in InputMgr.h

#include "stdafx.h"

#include <algorithm>			// For Sort()
#include <wbemidl.h>
#include <oleauto.h>
#include "InputMgr.h"
#include "Platform.h"
#include <AK/Tools/Common/AkFNVHash.h>


// Macro used by IsXInputDevice()
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }

bool IsXInputDevice( const GUID* pGuidProductFromDirectInput );

InputMgr::InputMgr()
	: m_pUInput( NULL )
	, m_pDI( NULL )
{
}

InputMgr::~InputMgr()
{
}

bool InputMgr::AddDevice( GUID in_guidInstance )
{
	// If we already have keyboard + 4 gamepads, don't add any more
	if ( m_InputObjects.size() >= 5 )
	{
		return false;
	}

	InputObject newInputObject;
	DIDEVCAPS   diCaps;
	HRESULT     hr;

	
	// Create the DIDevice Object
	newInputObject.pDevice = NULL;
	hr = m_pDI->CreateDevice( in_guidInstance, &newInputObject.pDevice, NULL );
	if ( FAILED( hr ) )
	{
		// Failed to properly add the new device
		// Return true to indiciate that there is still room in the inputmgr
		return true;
	}
	hr = newInputObject.pDevice->SetDataFormat( &c_dfDIJoystick2 );
	if ( FAILED( hr ) )
	{
		// Failed to properly add the new device
		// Return true to indiciate that there is still room in the inputmgr
		SAFE_RELEASE( newInputObject.pDevice );
		return true;
	}
	hr = newInputObject.pDevice->SetCooperativeLevel( m_hWnd, DISCL_EXCLUSIVE | DISCL_FOREGROUND );
	if ( FAILED( hr ) )
	{
		// Failed to properly add the new device
		// Return true to indiciate that there is still room in the inputmgr
		SAFE_RELEASE( newInputObject.pDevice );
		return true;
	}



	// Figure out the first available playerindex
	std::vector<InputObject>::iterator it = m_InputObjects.begin();
	int iNextIndex;
	for ( iNextIndex = 0; it != m_InputObjects.end() && iNextIndex == it->iPlayerIndex; iNextIndex++, it++ )
	{
		continue;
	}
	newInputObject.iPlayerIndex = iNextIndex;


	// Check if the device is rumble capable
	diCaps.dwSize = sizeof(DIDEVCAPS);
	hr = newInputObject.pDevice->GetCapabilities( &diCaps );
	if ( FAILED( hr ) )
	{
		// Failed to properly add the new device
		// Return true to indiciate that there is still room in the inputmgr
		SAFE_RELEASE( newInputObject.pDevice );
		return true;
	}


	m_InputObjects.push_back( newInputObject );
	sort( m_InputObjects.begin(), m_InputObjects.end(), CompareInputObjects );
	AK::FNVHash32 hash;
	m_pUInput->AddDevice((AkUInt16)newInputObject.iPlayerIndex, UGDeviceType_GAMEPAD, hash.Compute(&in_guidInstance, sizeof(GUID)));

	return true;
}

bool InputMgr::Init(
	void* in_pParam,
	AkOSChar* in_szErrorBuffer,
	unsigned int in_unErrorBufferCharCount
)
{
	HRESULT hr;
	InputObject newInputObject;

	m_hWnd = (HWND)in_pParam;
	m_pUInput = new UniversalInput;

	// Create the DirectInput Object
	hr = DirectInput8Create( GetModuleHandle(NULL), DIRECTINPUT_VERSION, 
                             IID_IDirectInput8, (VOID**)&m_pDI, NULL );
	if ( FAILED( hr ) )
	{
		__AK_OSCHAR_SNPRINTF( in_szErrorBuffer, in_unErrorBufferCharCount, AKTEXT("DirectInput8Create() returned HRESULT %d"), hr );
		goto cleanup;
	}

	// Find the system keyboard and set it to player 0.
	newInputObject.iPlayerIndex = 0;
	hr = m_pDI->CreateDevice( GUID_SysKeyboard, &newInputObject.pDevice, NULL );
	if ( FAILED( hr ) )
	{
		__AK_OSCHAR_SNPRINTF( in_szErrorBuffer, in_unErrorBufferCharCount, AKTEXT("IDirectInput8::CreateDevice() returned HRESULT %d"), hr );
		goto cleanup;
	}
	hr = newInputObject.pDevice->SetCooperativeLevel( m_hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND );
	if ( FAILED( hr ) )
	{
		__AK_OSCHAR_SNPRINTF( in_szErrorBuffer, in_unErrorBufferCharCount, AKTEXT("IDirectInputDevice8::SetCooperativeLevel() returned HRESULT %d"), hr );
		goto cleanup;
	}
	hr = newInputObject.pDevice->SetDataFormat( &c_dfDIKeyboard );
	if ( FAILED( hr ) )
	{
		__AK_OSCHAR_SNPRINTF( in_szErrorBuffer, in_unErrorBufferCharCount, AKTEXT("IDirectInputDevice8::SetDataFormat() returned HRESULT %d"), hr );
		goto cleanup;
	}
	m_InputObjects.push_back( newInputObject );
	sort( m_InputObjects.begin(), m_InputObjects.end(), CompareInputObjects );
	m_pUInput->AddDevice( 0, UGDeviceType_KEYBOARD);


	// Figure out what XInput devices are connected and add them
	XINPUT_STATE xState;
	newInputObject.pDevice = NULL;		// Set pDevice to NULL, this is how the InputMgr distinguishes XInput devices
	for ( int i = 0; i < MAX_XINPUT; i++ )
	{
		if ( XInputGetState( i, &xState ) == ERROR_SUCCESS )
		{
			// Controller connected, add it to the list
			newInputObject.iPlayerIndex = i + 1;		// Adjust XInput index to be 1-based
			m_InputObjects.push_back( newInputObject );
			sort( m_InputObjects.begin(), m_InputObjects.end(), CompareInputObjects );
			m_pUInput->AddDevice( (AkUInt16) newInputObject.iPlayerIndex, UGDeviceType_GAMEPAD );
		}
	}


	// Bind any dinput devices to fill up the rest of players 1-4
	m_pDI->EnumDevices( DI8DEVCLASS_GAMECTRL, EnumJoysticksCallback, this, DIEDFL_ATTACHEDONLY );
	

	return true;

cleanup:
	Release();
	return false;
}


UniversalInput* InputMgr::UniversalInputAdapter() const
{
	return m_pUInput;
}


void InputMgr::Release()
{
	if ( m_pUInput )
	{
		delete m_pUInput;
		m_pUInput = NULL;
	}
	
	// Release the input devices
	for ( Iterator it = m_InputObjects.begin(); it != m_InputObjects.end(); it++ )
	{
		if ( it->pDevice )
		{
			it->pDevice->Release();
			it->pDevice = NULL;
		}
	}
	m_InputObjects.clear();

	// Release the directinput object
	if ( m_pDI )
	{
		m_pDI->Release();
		m_pDI = NULL;
	}
}


void InputMgr::Update()
{
	Iterator it;

	for ( it = m_InputObjects.begin(); it != m_InputObjects.end(); it++ )
	{
		if ( it->iPlayerIndex == 0 )
		{
			// 'it' is pointing to the keyboard device
			TranslateKeyboard( it->iPlayerIndex, it->pDevice );
		}
		else if ( it->pDevice )
		{
			// 'it' is pointing to a directinput device
			TranslateDInput( it->iPlayerIndex, it->pDevice );
		}
		else
		{
			// 'it' is pointing to an xinput device
			TranslateXInput( it->iPlayerIndex );
		}
	}
}


void InputMgr::TranslateDInput( int in_iPlayerIndex, LPDIRECTINPUTDEVICE8 in_pDevice )
{
	UGBtnState  iState = 0;	// Returned button state value
	UGStickState joysticks[2];
	memset(joysticks, 0, sizeof(joysticks));

	DIJOYSTATE2 js;			// DInput joystick state
	bool bConnected = false;

	in_pDevice->Acquire();
	if ( in_pDevice->GetDeviceState( sizeof(DIJOYSTATE2), &js ) == DI_OK )
	{
		bConnected = true;

		if ( js.rgdwPOV[0] == 0 )
		{
			// Up on DPAD is pressed
			iState |= UG_DPAD_UP;
		}
		if ( js.rgdwPOV[0] == 18000 )
		{
			// Down on DPAD is pressed
			iState |= UG_DPAD_DOWN;
		}
		if ( js.rgdwPOV[0] == 27000 )
		{
			// Left on DPAD is pressed
			iState |= UG_DPAD_LEFT;
		}
		if ( js.rgdwPOV[0] == 9000 )
		{
			// Right on DPAD is pressed
			iState |= UG_DPAD_RIGHT;
		}
		if ( js.rgbButtons[0] & 0x80 )
		{
			iState |= UG_BUTTON1;
		}
		if ( js.rgbButtons[1] & 0x80 )
		{
			iState |= UG_BUTTON2;
		}
		if ( js.rgbButtons[2] & 0x80 )
		{
			iState |= UG_BUTTON3;
		}
		if ( js.rgbButtons[3] & 0x80 )
		{
			iState |= UG_BUTTON4;
		}
		if ( js.rgbButtons[4] & 0x80 )
		{
			iState |= UG_BUTTON5;
		}
		if ( js.rgbButtons[5] & 0x80 )
		{
			iState |= UG_BUTTON6;
		}
		if ( js.rgbButtons[6] & 0x80 )
		{
			iState |= UG_BUTTON7;
		}
		if ( js.rgbButtons[7] & 0x80 )
		{
			iState |= UG_BUTTON8;
		}
		if ( js.rgbButtons[8] & 0x80 )
		{
			iState |= UG_BUTTON9;
		}
		if ( js.rgbButtons[9] & 0x80 )
		{
			iState |= UG_BUTTON10;
		}
		if ( js.rgbButtons[10] & 0x80 )
		{
			iState |= UG_BUTTON11;
		}
		if ( js.rgbButtons[11] & 0x80 )
		{
			iState |= UG_OS_BUTTON;
		}

		joysticks[UG_STICKLEFT].x = js.lX / 32768.f;
		joysticks[UG_STICKLEFT].y = js.lY / 32768.f;

		joysticks[UG_STICKRIGHT].x = js.rglSlider[0] / 32768.f;
		joysticks[UG_STICKRIGHT].y = js.rglSlider[1] / 32768.f;
	}
	
	m_pUInput->SetState( (AkUInt16) in_iPlayerIndex, bConnected, iState, joysticks );
}


void InputMgr::TranslateKeyboard( int in_iPlayerIndex, LPDIRECTINPUTDEVICE8 in_pKeyboard )
{
	UGBtnState iState = 0;	// Returned button state value
	UGStickState joysticks[2];
	memset(joysticks, 0, sizeof(joysticks));

	BYTE keys[256];			// Buffer to hold keyboard state
	bool bConnected = false;

	in_pKeyboard->Acquire();
	if ( in_pKeyboard->GetDeviceState( 256, keys ) == DI_OK ) 
	{ 
		bConnected = true;

		if ( ( keys[DIK_UP] & 0x80 ) || ( keys[DIK_W] & 0x80 ) )
		{
			// Up arrow or 'W' is pressed
			iState |= UG_DPAD_UP;
		}
		if ( ( keys[DIK_DOWN] & 0x80 ) || ( keys[DIK_S] & 0x80 ) )
		{
			// Down arrow or 'S' is pressed
			iState |= UG_DPAD_DOWN;
		}
		if ( ( keys[DIK_LEFT] & 0x80 ) || ( keys[DIK_A] & 0x80 ) )
		{
			// Left arrow or 'A' is pressed
			iState |= UG_DPAD_LEFT;
		}
		if ( ( keys[DIK_RIGHT] & 0x80 ) || ( keys[DIK_D] & 0x80 ) )
		{
			// Right arrow 'D' is pressed
			iState |= UG_DPAD_RIGHT;
		}
		if ( ( keys[DIK_RETURN] & 0x80 ) || ( keys[DIK_NUMPADENTER] & 0x80 ) )
		{
			// Enter button is pressed
			iState |= UG_BUTTON1;
		}
		if ( keys[DIK_ESCAPE] & 0x80 )
		{
			// Escape button is pressed
			iState |= UG_BUTTON2;
		}
		if ( keys[DIK_Q] & 0x80 )
		{
			// 'Q' is pressed
			iState |= UG_BUTTON3;
		}
		if ( keys[DIK_E] & 0x80 )
		{
			// 'E' is pressed
			iState |= UG_BUTTON4;
		}
		if ( ( keys[DIK_LSHIFT] & 0x80 ) || ( keys[DIK_RSHIFT] & 0x80 ) )
		{
			// Left Shift is pressed
			iState |= UG_BUTTON5;
		}
		if ( keys[DIK_R] & 0x80 )
		{
			// 'R' is pressed
			iState |= UG_BUTTON6;
		}
		if ( keys[DIK_F1] & 0x80 )
		{
			// 'F1' is pressed
			iState |= UG_BUTTON7;
		}
		if ( ( keys[DIK_LALT] & 0x80 ) || ( keys[DIK_RALT] & 0x80 ) )
		{
			iState |= UG_OS_BUTTON;
		}
	}

	m_pUInput->SetState( (AkUInt16) in_iPlayerIndex, bConnected, iState, joysticks );
}


void InputMgr::TranslateXInput( int in_iPlayerIndex )
{
	UGBtnState   iState = 0; // Returned button state value
	UGStickState joysticks[2];
	memset(joysticks, 0, sizeof(joysticks));

	XINPUT_STATE xState;	 // Holds the state of the xinput device
	bool bConnected = false;
	
	if ( XInputGetState( in_iPlayerIndex - 1, &xState ) == ERROR_SUCCESS )
	{
		bConnected = true;
		WORD wButtons = xState.Gamepad.wButtons;

		if ( wButtons & XINPUT_GAMEPAD_DPAD_UP )
		{
			iState |= UG_DPAD_UP;
		}
		if ( wButtons & XINPUT_GAMEPAD_DPAD_DOWN )
		{
			iState |= UG_DPAD_DOWN;
		}
		if ( wButtons & XINPUT_GAMEPAD_DPAD_LEFT )
		{
			iState |= UG_DPAD_LEFT;
		}
		if ( wButtons & XINPUT_GAMEPAD_DPAD_RIGHT )
		{
			iState |= UG_DPAD_RIGHT;
		}
		if ( wButtons & XINPUT_GAMEPAD_A )
		{
			iState |= UG_BUTTON1;
		}
		if ( wButtons & XINPUT_GAMEPAD_B )
		{
			iState |= UG_BUTTON2;
		}
		if ( wButtons & XINPUT_GAMEPAD_X )
		{
			iState |= UG_BUTTON3;
		}
		if ( wButtons & XINPUT_GAMEPAD_Y )
		{
			iState |= UG_BUTTON4;
		}
		if ( wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER )
		{
			iState |= UG_BUTTON5;
		}
		if ( wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER )
		{
			iState |= UG_BUTTON6;
		}
		if ( wButtons & XINPUT_GAMEPAD_START )
		{
			iState |= UG_BUTTON7;
		}
		if ( wButtons & XINPUT_GAMEPAD_BACK )
		{
			iState |= UG_BUTTON8;
		}
		if ( wButtons & XINPUT_GAMEPAD_LEFT_THUMB )
		{
			iState |= UG_BUTTON9;
		}
		if ( wButtons & XINPUT_GAMEPAD_RIGHT_THUMB )
		{
			iState |= UG_BUTTON10;
		}

		joysticks[UG_STICKLEFT].x = xState.Gamepad.sThumbLX / 32768.f;
		joysticks[UG_STICKLEFT].y = xState.Gamepad.sThumbLY / 32768.f;
		joysticks[UG_STICKRIGHT].x = xState.Gamepad.sThumbRX / 32768.f;
		joysticks[UG_STICKRIGHT].y = xState.Gamepad.sThumbRY / 32768.f;
	}

	m_pUInput->SetState( (AkUInt16) in_iPlayerIndex, bConnected, iState, joysticks );
}


bool InputMgr::CompareInputObjects( InputMgr::InputObject in_sFirst, InputMgr::InputObject in_sSecond )
{
	return ( in_sFirst.iPlayerIndex < in_sSecond.iPlayerIndex );
}


BOOL CALLBACK InputMgr::EnumJoysticksCallback( const DIDEVICEINSTANCE* pdidInstance, VOID* pContext )
{
	InputMgr* mgr = (InputMgr*)pContext;
	bool bContinue = false;

	if( IsXInputDevice( &pdidInstance->guidProduct ) )
	{
		bContinue = true;
	}
	else
	{
		bContinue = mgr->AddDevice( pdidInstance->guidInstance );
	}

	return ( bContinue ) ? DIENUM_CONTINUE : DIENUM_STOP;
}


bool IsXInputDevice( const GUID* pGuidProductFromDirectInput )
{
	// Enum each PNP device using WMI and check each device ID to see if it contains 
	// "IG_" (ex. "VID_045E&PID_028E&IG_00").  If it does, then it's an XInput device
	
	IWbemLocator*           pIWbemLocator  = NULL;
    IEnumWbemClassObject*   pEnumDevices   = NULL;
    IWbemClassObject*       pDevices[20]   = {0};
    IWbemServices*          pIWbemServices = NULL;
    BSTR                    bstrNamespace  = NULL;
    BSTR                    bstrDeviceID   = NULL;
    BSTR                    bstrClassName  = NULL;
    DWORD                   uReturned      = 0;
    bool                    bIsXinputDevice= false;
    UINT                    iDevice        = 0;
    VARIANT                 var;
    HRESULT                 hr;

    // CoInit if needed
    hr = CoInitialize(NULL);
    bool bCleanupCOM = SUCCEEDED(hr);

    // Create WMI
    hr = CoCreateInstance( __uuidof(WbemLocator),
                           NULL,
                           CLSCTX_INPROC_SERVER,
                           __uuidof(IWbemLocator),
                           (LPVOID*) &pIWbemLocator);
    if( FAILED(hr) || pIWbemLocator == NULL )
        goto LCleanup;

    bstrNamespace = SysAllocString( L"\\\\.\\root\\cimv2" );if( bstrNamespace == NULL ) goto LCleanup;        
    bstrClassName = SysAllocString( L"Win32_PNPEntity" );   if( bstrClassName == NULL ) goto LCleanup;        
    bstrDeviceID  = SysAllocString( L"DeviceID" );          if( bstrDeviceID == NULL )  goto LCleanup;        
    
    // Connect to WMI 
    hr = pIWbemLocator->ConnectServer( bstrNamespace, NULL, NULL, 0L, 
                                       0L, NULL, NULL, &pIWbemServices );
    if( FAILED(hr) || pIWbemServices == NULL )
        goto LCleanup;

    // Switch security level to IMPERSONATE. 
    CoSetProxyBlanket( pIWbemServices, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, 
                       RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE );                    

    hr = pIWbemServices->CreateInstanceEnum( bstrClassName, 0, NULL, &pEnumDevices ); 
    if( FAILED(hr) || pEnumDevices == NULL )
        goto LCleanup;

    // Loop over all devices
    for( ;; )
    {
        // Get 20 at a time
        hr = pEnumDevices->Next( 10000, 20, pDevices, &uReturned );
        if( FAILED(hr) )
            goto LCleanup;
        if( uReturned == 0 )
            break;

        for( iDevice=0; iDevice<uReturned; iDevice++ )
        {
            // For each device, get its device ID
            hr = pDevices[iDevice]->Get( bstrDeviceID, 0L, &var, NULL, NULL );
            if( SUCCEEDED( hr ) && var.vt == VT_BSTR && var.bstrVal != NULL )
            {
                // Check if the device ID contains "IG_".  If it does, then it's an XInput device
				    // This information can not be found from DirectInput 
                if( wcsstr( var.bstrVal, L"IG_" ) )
                {
                    // If it does, then get the VID/PID from var.bstrVal
                    DWORD dwPid = 0, dwVid = 0;
                    WCHAR* strVid = wcsstr( var.bstrVal, L"VID_" );
                    if( strVid && swscanf_s( strVid, L"VID_%4X", &dwVid ) != 1 )
                        dwVid = 0;
                    WCHAR* strPid = wcsstr( var.bstrVal, L"PID_" );
                    if( strPid && swscanf_s( strPid, L"PID_%4X", &dwPid ) != 1 )
                        dwPid = 0;

                    // Compare the VID/PID to the DInput device
                    DWORD dwVidPid = MAKELONG( dwVid, dwPid );
                    if( dwVidPid == pGuidProductFromDirectInput->Data1 )
                    {
                        bIsXinputDevice = true;
                        goto LCleanup;
                    }
                }
            }   
            SAFE_RELEASE( pDevices[iDevice] );
        }
    }

LCleanup:
    if(bstrNamespace)
        SysFreeString(bstrNamespace);
    if(bstrDeviceID)
        SysFreeString(bstrDeviceID);
    if(bstrClassName)
        SysFreeString(bstrClassName);
    for( iDevice=0; iDevice<20; iDevice++ )
        SAFE_RELEASE( pDevices[iDevice] );
    SAFE_RELEASE( pEnumDevices );
    SAFE_RELEASE( pIWbemLocator );
    SAFE_RELEASE( pIWbemServices );

    if( bCleanupCOM )
        CoUninitialize();

    return bIsXinputDevice;
}