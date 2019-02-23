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

// InputMgr.h
/// \file 
/// Contains the declaration of the Windows Input Manager class.
/// This class handles the relaying of input (Keyboard, DirectInput, XInput) to the Menu System.

#pragma once

#include <vector>
#include "UniversalInput.h"

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>


/// Windows-Specific Input Manager class that handles all input devices and relays input
/// data to the Menu system.
class InputMgr
{
public:

	/// InputMgr class constructor
	InputMgr();

	/// InputMgr class destructor
	~InputMgr();

	/// Initializes the Input Manager object.
	/// \return True if the Input Manager has been properly initialized and false otherwise.
	bool Init( 
		void* in_pParam,						///< - Extra parameter, in this case for the application's main window handle
		AkOSChar* in_szErrorBuffer,				///< - Buffer where error details will be written (if the function returns false)
		unsigned int in_unErrorBufferCharCount	///< - Number of characters available in in_szErrorBuffer, including terminating NULL character
		);

	/// Accessor method for the UniversalInput object.
	/// \return A pointer to the UniversalInput object being used by the Input Manager.
	UniversalInput* UniversalInputAdapter() const;

	/// Updates the input from all devices and relays the new input states to the Menu system.
	/// \remark This method should be called every frame before the Menu system's Update call.
	void Update();

	/// Ends the Input Manager class and releases all used resources.
	void Release();

private:

	/// Represents an input device being tracked by the Input Manager.
	struct InputObject
	{
		LPDIRECTINPUTDEVICE8 pDevice; ///< - Pointer to the device's DInput device object (NULL if Xinput).
		int iPlayerIndex;			  ///< - The 1-based player index associated with the device.
	};

	/// Iterator to the Input Objects container.
	typedef std::vector<InputObject>::iterator Iterator;

	/// Adds a directinput device to the list of devices being tracked by the Input Manager.
	/// \note This function is normally called by the EnumJoysticksCallback function.
	/// \return True if there is still room to add more devices to the manager and false otherwise.
	bool AddDevice( 
		GUID in_guidInstance		///< - Instance GUID of the device to try to add
		);

	/// Retrieves input state from a DirectInput device then translates it to a
	/// Button State usable by the Universal Input class and sets it.
	void TranslateDInput(
		int in_iPlayerIndex,				///< - 1-Based Player Index of the DInput device to be polled
		LPDIRECTINPUTDEVICE8 in_pDevice		///< - DInput Device Object pointer to the device to be polled
		);

	/// Retrieves input state from a keyboard then translates it to a Button State 
	/// usable by the Universal Input class and sets it.
	void TranslateKeyboard( 
		int in_iPlayerIndex,				///< - 1-Based Player Index of the keyboard (should be 0)
		LPDIRECTINPUTDEVICE8 in_pKeyboard	///< - DInput Device Object pointer to the keyboard device
		);

	/// Retrieves input state from an XInput device (eg. Xbox 360 controller) then
	/// translates it to a Button State usable by the Universal Input class and sets it.
	void TranslateXInput( 
		int in_iPlayerIndex					///< - 1-Based Player Index of the XInput device to be polled
		);

	/// Comparison function to compare InputObjects for the c++ sort function from the algorithm library.
	/// \return True if the first parameter preceeds the second when in ascending order and false otherwise.
	static bool CompareInputObjects( 
		InputObject in_sFirst,			///< - The first Input Object
		InputObject in_sSecond			///< - The second Input Object
		);
	
	/// Callback function used by the DirectInput EnumDevices method.
	static BOOL CALLBACK EnumJoysticksCallback( 
		const DIDEVICEINSTANCE* pdidInstance,	///< - Instance of a Dinput device
		VOID* pContext							///< - Context the Callback was called in
	);

	/// The application's main window handle
	HWND m_hWnd;

	/// Pointer to the DirectInput object
	LPDIRECTINPUT8 m_pDI;

	/// Pointer to the Universal Input object to interface with the Menu System
	/// \sa UniversalInput
	UniversalInput* m_pUInput;

	/// Container to hold the Input Objects tracked by the Input Manager
	std::vector<InputObject> m_InputObjects;

	/// The maximum number of XInput controllers
	static const int MAX_XINPUT = 4;

	/// Thumbstick Deadzone value
	static const int DEADZONE = 15000;
};