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

#pragma once

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <wbemidl.h>
#include <oleauto.h>


#include <stdio.h>      // standard input/output
#include <vector>       // stl vector header

#include <AK/SoundEngine/Common/AkTypes.h>

namespace AkDirectInputDevice
{

struct DirectInputController
{
	GUID m_ControllerGUID;
	LPDIRECTINPUTDEVICE8 m_pController;
	wchar_t m_tszProductName[MAX_PATH];
	bool m_bIsActive;
	bool m_bIsInitialised;
};

class AkControllers
{
public:
	std::vector<DirectInputController>  m_DirectInputDevices;	// array of direct input device
	AkInt32 m_nXInputControllerCount;

	AkControllers();
	~AkControllers();
	void Reset();
};

/// Get the controllers list.
/// \sa
/// - \ref InitControllers UpdateControllers
const AkControllers& GetControllers();

/// Initialise direct input interface and update the controllers list.
/// \return true if direct input device found.
/// \sa
/// - \ref UpdateControllers
bool InitControllers(HWND in_hwnd);

/// Update the controllers list with connected device.
/// \return true if direct input device found.
/// \sa
bool UpdateControllers();

/// Get the first connected controller from the list,
/// \return IDirectInputDevice8* if found and NULL otherwise
/// \sa
/// - \ref InitControllers UpdateControllers 
IDirectInputDevice8 *GetFirstDirectInputController();

} //namespeace AkDirectInputDevice
