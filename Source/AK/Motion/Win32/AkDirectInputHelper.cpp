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

#include "stdafx.h"
#include "AkDirectInputHelper.h"
#include <AK/Tools/Common/AkPlatformFuncs.h>

#define SAFE_RELEASE(p) { if ((p) != NULL) { (p)->Release(); (p) = NULL; } } 

namespace AkDirectInputDevice
{

HWND			g_hwnd = NULL;
LPDIRECTINPUT8	g_pDI = NULL; // Direct input interface
AkControllers	g_Controlers;


AkControllers::AkControllers()
:m_nXInputControllerCount(0)
{
}

AkControllers::~AkControllers()
{
	Reset();

	SAFE_RELEASE( g_pDI );
	g_hwnd = NULL;
}

void AkControllers::Reset()
{
	m_nXInputControllerCount = 0;
	std::vector<DirectInputController>::iterator itPos = m_DirectInputDevices.begin();
	for(; itPos < m_DirectInputDevices.end(); ++itPos)
	{
		SAFE_RELEASE((*itPos).m_pController);
	}
	m_DirectInputDevices.clear();
};

//-----------------------------------------------------------------------------
// Name: EnumFFDevicesCallback()
// Desc: Called once for each enumerated connected device. If we find
//       one, create a device interface on it so we can play with it.
//-----------------------------------------------------------------------------
BOOL CALLBACK EnumFFDevicesCallback( const DIDEVICEINSTANCE* pInst, VOID * );

// http://msdn.microsoft.com/en-us/library/bb173051.aspx
//-----------------------------------------------------------------------------
// Enum each PNP device using WMI and check each device ID to see if it contains 
// "IG_" (ex. "VID_045E&PID_028E&IG_00").  If it does, then it's an XInput device
// Unfortunately this information can not be found by just using DirectInput 
//-----------------------------------------------------------------------------
BOOL IsXInputDevice( const GUID* pGuidProductFromDirectInput );

bool InitControllers(HWND in_hwnd)
{
	AKASSERT(in_hwnd);
	g_hwnd = in_hwnd;
    HRESULT     hr;

	// Register with the DirectInput subsystem and get a pointer
    // to a IDirectInput interface we can use.
	hr = DirectInput8Create( GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (VOID**)&g_pDI, NULL );
	if( hr != DI_OK )
	{
		return false;
	}

	return UpdateControllers();	
}

bool UpdateControllers()
{
	AKASSERT(g_pDI);
	AKASSERT(g_hwnd);

	std::vector<DirectInputController>::iterator itPos = g_Controlers.m_DirectInputDevices.begin();
	while(itPos < g_Controlers.m_DirectInputDevices.end())
	{
		(*itPos++).m_bIsActive = false;
	}
	g_Controlers.m_nXInputControllerCount = 0;

    // Look for a force feedback device we can use
	HRESULT hr = g_pDI->EnumDevices( DI8DEVCLASS_GAMECTRL, EnumFFDevicesCallback, NULL, DIEDFL_ATTACHEDONLY );
    if( hr != DI_OK || !g_Controlers.m_DirectInputDevices.size())
    {
        return false;
    }

    // Set the data format to "simple joystick" - a predefined data format. A
    // data format specifies which controls on a device we are interested in,
    // and how they should be reported.
    //
    // This tells DirectInput that we will be passing a DIJOYSTATE structure to
    // IDirectInputDevice8::GetDeviceState(). Even though we won't actually do
    // it in this sample. But setting the data format is important so that the
    // DIJOFS_* values work properly.
	itPos = g_Controlers.m_DirectInputDevices.begin();
	while( !(g_Controlers.m_DirectInputDevices.empty()) && itPos != g_Controlers.m_DirectInputDevices.end() )
	{
		if( !(*itPos).m_pController || !(*itPos).m_bIsActive )
		{
			SAFE_RELEASE((*itPos).m_pController);
			g_Controlers.m_DirectInputDevices.erase(itPos++);	//remove this controler from the list
		}
		else if ((*itPos).m_bIsInitialised == true)
		{
			++itPos; // OK this one is good, let's see next.
		}
		else if( (*itPos).m_pController->SetDataFormat( &c_dfDIJoystick ) != DI_OK )	//let's initialise controller
		{
			SAFE_RELEASE((*itPos).m_pController);
			g_Controlers.m_DirectInputDevices.erase(itPos++);	//remove this controler from the list
		}
		else
		{
			// Set the cooperative level to let DInput know how this device should
			// interact with the system and with other DInput applications.
			// Exclusive access is required in order to perform force feedback.
			if( (*itPos).m_pController->SetCooperativeLevel( g_hwnd, DISCL_EXCLUSIVE | DISCL_BACKGROUND ) != DI_OK )
			{
				SAFE_RELEASE((*itPos).m_pController);
				g_Controlers.m_DirectInputDevices.erase(itPos++);	//remove this controler from the list
			}
			else
			{
				(*itPos).m_bIsInitialised = true;
				++itPos; // OK this one is good, let's see next.
			}		
		}
	}

    return g_Controlers.m_DirectInputDevices.size() != 0;
}

BOOL IsXInputDevice( const GUID* pGuidProductFromDirectInput )
{
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
                    if( strVid && swscanf( strVid, L"VID_%4X", &dwVid ) != 1 )
                        dwVid = 0;
                    WCHAR* strPid = wcsstr( var.bstrVal, L"PID_" );
                    if( strPid && swscanf( strPid, L"PID_%4X", &dwPid ) != 1 )
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


BOOL CALLBACK EnumFFDevicesCallback( const DIDEVICEINSTANCE* pInst, 
                                     VOID* /* pContext */ )
{
    LPDIRECTINPUTDEVICE8 pDevice;
    HRESULT              hr;

	if( IsXInputDevice( &pInst->guidProduct ) )
	{
		++g_Controlers.m_nXInputControllerCount;
        return DIENUM_CONTINUE;
	}

	std::vector<DirectInputController>::iterator itPos = g_Controlers.m_DirectInputDevices.begin();
	while(itPos < g_Controlers.m_DirectInputDevices.end())
	{
		if ((*itPos).m_ControllerGUID == pInst->guidInstance)
		{
			(*itPos).m_bIsActive = true;
			return DIENUM_CONTINUE;
		}
		++itPos;
	}

    // Obtain an interface to the enumerated force feedback device.
    hr = g_pDI->CreateDevice( pInst->guidInstance, &pDevice, NULL );

    // If it failed, then we can't use this device for 
    // some reason.  (Maybe the user unplugged it while we
    // were in the middle of enumerating it.)  So continue enumerating
    if( FAILED(hr) ) 
        return DIENUM_CONTINUE;

	DIDEVCAPS deviceCapabilities;
	deviceCapabilities.dwSize = sizeof(DIDEVCAPS);
	hr = pDevice->GetCapabilities(&deviceCapabilities);
	if (hr != DI_OK || !(deviceCapabilities.dwFlags & DIDC_FORCEFEEDBACK ))
	{
		SAFE_RELEASE(pDevice);
		return DIENUM_CONTINUE;
	}

    // We successfully created an IDirectInputDevice8. Add it to the list.
	DirectInputController newController;
	newController.m_ControllerGUID = pInst->guidInstance;
	newController.m_pController = pDevice;
	newController.m_bIsActive = true;
	newController.m_bIsInitialised = false;
	AKPLATFORM::SafeStrCpy(newController.m_tszProductName, pInst->tszProductName, MAX_PATH);
	g_Controlers.m_DirectInputDevices.push_back(newController);

    return DIENUM_CONTINUE;
}

IDirectInputDevice8 * GetFirstDirectInputController()
{
	IDirectInputDevice8 * pDevice(NULL);

	std::vector<DirectInputController>::iterator itPos = g_Controlers.m_DirectInputDevices.begin();
	while(pDevice == NULL && itPos < g_Controlers.m_DirectInputDevices.end())
	{
		if ((*itPos).m_bIsActive )
		{
			pDevice = (*itPos).m_pController;
			break;
		}
		++itPos;
	}
	return pDevice;
}

const AkControllers& GetControllers()
{
	return g_Controlers;
}

}