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

// UniversalInput.h
/// \file 
/// Contains the declaration of the input abstraction layer used by the Menu system.

#pragma once

#include <vector>
#include <AK/SoundEngine/Common/AkTypes.h>

enum PointerEventType
{
	PointerEventType_Pressed,
	PointerEventType_Released,
	PointerEventType_Moved,
	PointerEventType_Tapped,
	PointerEventType_DoubleTapped
};

/// Defines a gamepad's button state. Each bit represents the state of a single button.
typedef AkUInt16 UGBtnState;

/// Defines a single button which masks a particular bit of the button state.
typedef AkUInt16 UGButton;

// Individual button mask declarations
static const UGButton UG_DPAD_UP    = 1 << 0;
static const UGButton UG_DPAD_DOWN  = 1 << 1;
static const UGButton UG_DPAD_LEFT  = 1 << 2;
static const UGButton UG_DPAD_RIGHT = 1 << 3;
static const UGButton UG_BUTTON1    = 1 << 4;
static const UGButton UG_BUTTON2    = 1 << 5;
static const UGButton UG_BUTTON3    = 1 << 6;
static const UGButton UG_BUTTON4	= 1 << 7;
static const UGButton UG_BUTTON5	= 1 << 8;
static const UGButton UG_BUTTON6	= 1 << 9;
static const UGButton UG_BUTTON7	= 1 << 10;
static const UGButton UG_BUTTON8	= 1 << 11;
static const UGButton UG_BUTTON9	= 1 << 12;
static const UGButton UG_BUTTON10	= 1 << 13;
static const UGButton UG_BUTTON11	= 1 << 14;
static const UGButton UG_OS_BUTTON	= 1 << 15;

enum UGThumbstick { UG_STICKLEFT = 0, UG_STICKRIGHT = 1 };

struct UGStickState
{
	AkReal32 x;
	AkReal32 y;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                  //
//                               BUTTON MAPPINGS TABLE                                              //
//                                                                                                  //
//////////////////////////////////////////////////////////////////////////////////////////////////////
//+------------------------------------------------------------------------------------------------+//
//|   BUTTON   |     KeyBoard     |    X360    |    PS3     | Wii Remote |  GameCube  |  iOS	   |//
//+------------------------------------------------------------------------------------------------+//
//| DPAD_UP    | Up Arrow / W     | DPad Up    | DPad Up    | DPad Up    | DPad Up    | Up		   |//
//|	DPAD_DOWN  | Down Arrow / S   | Dpad Down  | Dpad Down  | Dpad Down  | Dpad Down  | Down	   |//
//|	DPAD_LEFT  | Left Arrow / A   | Dpad Left  | Dpad Left  | Dpad Left  | Dpad Left  | Left	   |//
//|	DPAD_RIGHT | Right Arrow / D  | Dpad Right | Dpad Right | Dpad Right | Dpad Right | Right	   |//
//|	BUTTON1    | Enter / NumEnter | A          | Cross      | A          | A          | A          |//
//|	BUTTON2    | Escape           | B          | Circle     | B          | B          | B          |//
//|	BUTTON3    | Q                | X          | Square     | 1          | X          | X          |//
//|	BUTTON4	   | E                | Y          | Triangle   | 2          | Y          | Y          |//
//|	BUTTON5	   | Left/Right Shift | LB         | L1         | C (Nunchuk)| Left Trig  |			   |//
//|	BUTTON6	   | R                | RB         | R1         | Z (Nunchuk)| Right Trig |			   |//
//|	BUTTON7	   | F1               | Start      | Start      | +          | Start      |	Start	   |//
//|	BUTTON8	   |                  | Back       | Select     | -          | Z          | Select     |//
//|	BUTTON9	   |                  | Left Thumb | L3         |            |            |            |//
//|	BUTTON10   |                  | Right Thumb| R3         |            |            |            |//
//|	BUTTON11   |                  |            |            |            |            |            |//
//| OS_BUTTON  | Left/Right Alt   |            |            | HOME       |            |            |//
//+------------------------------------------------------------------------------------------------|//
//////////////////////////////////////////////////////////////////////////////////////////////////////



/// Enumeration that describes the input device's type
enum UGDeviceType
{
	UGDeviceType_KEYBOARD,		///< Keyboard device (Windows or Mac)
	UGDeviceType_GAMEPAD		///< GamePad device
};


/// The UniversalGamepad class represents fictitious input devices that the Menu system can understand.
class UniversalGamepad
{
public:

	/// UniversalGamepad class constructor.
	UniversalGamepad(
		int in_iPlayerIndex,								///< - The 1-based player index of the device
		UGDeviceType in_eDeviceType,						///< - The type of the device
		AkDeviceID in_deviceID = AK_INVALID_DEVICE_ID		///< - Device ID
	);

	/// Tests if the button on the gamepad is pressed.
	/// \note This method takes into account and filters out buttons that were previously pressed and not released.
	/// \return True if the button is pressed and false otherwise.
	/// \sa IsButtonDown
	bool IsButtonTriggered( 
		UGButton in_Button		///< - Specifies the button to test for
		) const;

	/// Tests if the button on the gamepad is currently down.
	/// \note Unlike IsButtonTriggered, this method only checks the current raw input to see if the button is down.
	/// \return True if the button is down and false otherwise.
	/// \sa IsButtonTriggered
	bool IsButtonDown(
		UGButton in_Button		///< - Specifies the button to test for
		) const;

	/// Tests if the gamepad is connected.
	/// \return True if the gamepad is connected and false otherwise.
	bool IsConnected() const;

	/// Sets the state of the gamepad.
	void SetState( 
		bool in_bConnected,				///< - If the gamepad is connected
		UGBtnState in_BtnState,			///< - The button state to set
		UGStickState * in_StickStates	///< - The thumbstick states.
		);

	/// Accessor to the current button state, which takes into account and filters out buttons that were previously
	/// pressed and not yet released.
	UGBtnState GetCurrentState() const;

	/// Accessor to the current "raw" button state, which only takes into account whatever buttons are currently
	/// pressed down.
	UGBtnState GetCurrentRawState() const;
	
	/// Accessor to get the Universal Gamepad's player index.
	AkUInt16 GetPlayerIndex() const;

	/// Accessor to the Gamepad's device type
	UGDeviceType GetDeviceType() const;

	/// Accessor to the Gamepad's device ID
	AkDeviceID GetDeviceID() const;

    /// Accessor to the Gamepad's extra parameter
	void* GetExtraParam() const;

	/// Get the analog stick position
	void GetStickPosition(
		UGThumbstick in_Stick,	///< The stick to query
		float &out_x,			///< The horizontal position (normalized -1.0 to 1.0)
		float &out_y			///< The vertical position (normalized -1.0 to 1.0)
		) const;

private:

	/// The current state of the buttons exactly as they are on the gamepad.
	/// \sa UGBtnState
	UGBtnState m_CurRawBtnState;

	/// The current state of the buttons on the gamepad which filters out what was previously already down.
	/// \sa UGBtnState
	UGBtnState m_CurBtnState;

	/// The previous state of the buttons on the gamepad.
	/// \sa UGBtnState
	UGBtnState m_PrevBtnState;

	/// The current analog sticks states.
	UGStickState m_StickStates[2];

	/// The type of the device.
	/// \sa UGDeviceType
	UGDeviceType m_DeviceType;

	/// The 1-based player index associated with the gamepad.
	AkUInt16 m_iPlayerIndex;

	/// The gamepad device ID.
	AkDeviceID m_iDeviceID;

	/// Whether the device is currently connected or not.
	bool m_bConnected;

	/// Pointer to an extra parameter that may be needed (eg. DInput device object pointer).
	void* m_pExtraParam;
};




/// This class serves as an input abstraction layer between the Menu system and the platform 
/// it is running on. It exposes the concept of a "Universal Gamepad" that is used to control
/// the application. Platform specific code should translate input that it reads into a Universal
/// Input format in order to interface with the Menu system.
class UniversalInput
{
public:

	/// UniversalInput class destructor.
	~UniversalInput();

	/// Iterator to the Universal Gamepads container.
	typedef std::vector<UniversalGamepad>::iterator Iterator;

	/// Provides an Iterator to the first object in Universal Gamepads container.
	Iterator Begin();

	/// Provides an Iterator just past the last object in the Universal Gamepads container.
	Iterator End();

	/// Adds a UniversalGamepad device to be tracked by the UniversalInput object.
	/// \return True if the device was added and false if a gamepad with the specified player index already exists.
	bool AddDevice(
		AkUInt16 in_nPlayerIndex,							///< - The 1-based player index of the device to be added
		UGDeviceType in_eDeviceType,						///< - The type of the device to be added
		AkDeviceID in_deviceID = AK_INVALID_DEVICE_ID		///< - Device ID
	);

	/// Returns a pointer to the UniversalGamepad with the specified player index.
	/// \return Pointer to the UniversalGamepad with the specified player index if one exists, otherwise NULL.
	const UniversalGamepad* GetGamepad( 
		AkUInt16 in_nPlayerIndex		///< - The player index of the gamepad to retrieve
		) const;

	/// Returns the number of devices that are being tracked by the Input class.
	/// \return The number of devices tracked.
	AkUInt16 NumDevices() const;

	/// Returns the number of devices of a specific type that are being tracked by the Input class.
	/// \return The number of devices tracked of the specified type.
	AkUInt16 NumDevices( 
		UGDeviceType in_eDeviceType		///< - Type of the devices to count
		) const;

	/// Returns whether an input device with the specified player index exists.
	/// \return True if the device is connected and false if it is not connected or does not exist.
	bool IsPlayerConnected(
		AkUInt16 in_nPlayerIndex		///< - The player index to look for
		) const;

	/// Removes the device with the specified player index from the list of tracked devices.
	/// \return True if the device was removed and false if no gamepad exists with the specified index.
	bool RemoveDevice( 
		AkUInt16 in_nPlayerIndex		///< - The player index of the gamepad to be removed
		);

	/// Sets the State of the gamepad with the specified player index.
	/// \return True if the new state has been set and false if no gamepad exists with the specified index.
	bool SetState( 
		AkUInt16   in_nPlayerIndex,		///< - The player index of the gamepad to set the state of
		bool       in_bConected,		///< - Whether the gamepad is connected or not
		UGBtnState in_BtnState,			///< - The button state to set
		UGStickState *in_StickStates	///< - The thumbstick positions
		);

private:

	/// Comparison function to compare UniversalGamepads for the c++ sort function from the algorithm library.
	/// \return True if the first parameter preceeds the second when in ascending order and false otherwise.
	static bool CompareGamepads( 
		UniversalGamepad in_sFirst,		///< - The first Gamepad
		UniversalGamepad in_sSecond		///< - The second Gamepad
		);

	/// Container to hold the Universal Gamepad objects tracked by the Universal Input class.
	std::vector<UniversalGamepad> m_GamePads;
};
