/*******************************************************************************
The content of this file includes portions of the AUDIOKINETIC Wwise Technology
released in source code form as part of the SDK installer package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use this file in accordance with the end user license agreement provided 
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

  Version: v2017.2.6  Build: 6581
  Copyright (c) 2006-2018 Audiokinetic Inc.
*******************************************************************************/

// InputMgr.cpp
/// \file 
/// Defines the methods declared in InputMgr.h

#include "stdafx.h"

#include "InputMgr.h"


InputMgr::InputMgr()
	: m_pUInput( NULL )
{
}

InputMgr::~InputMgr()
{
}

bool InputMgr::Init(
	void* in_pParam,
	AkOSChar* in_szErrorBuffer,
	unsigned int in_unErrorBufferCharCount
)
{
	
	m_pUInput = new UniversalInput;

	m_pUInput->AddDevice(0, UGDeviceType_KEYBOARD);

	// 3 Xbox controller inputs max because player 0 is the keyboard.
	for ( int i = 1; i < MAX_INPUT; i++ )
	{
		m_pUInput->AddDevice( i , UGDeviceType_GAMEPAD );	
	}

	return true;
}

UniversalInput* InputMgr::UniversalInputAdapter() const
{
	return m_pUInput;
}

void InputMgr::Update()
{
	// Keyboard index always 0.
	TranslateKeyboard( 0 );

	for ( int i = 0; i < MAX_INPUT; i++ )
	{
		TranslateInput( NULL, i );
	}
}

void InputMgr::Release()
{
	if ( m_pUInput )
	{
		delete m_pUInput;
		m_pUInput = NULL;
	}
}

void InputMgr::TranslateKeyboard( int in_iPlayerIndex )
{
	UGBtnState iState = 0;	// Returned button state value
	bool bConnected = true;
	UGStickState joysticks[2];
	memset(joysticks, 0, sizeof(joysticks));


	Windows::Devices::Input::KeyboardCapabilities ^keyboardCapabilities = ref new Windows::Devices::Input::KeyboardCapabilities();
	if (keyboardCapabilities->KeyboardPresent) // Checks if keyboard is connected.
	{
		if (Windows::UI::Core::CoreVirtualKeyStates::Down == Windows::UI::Core::CoreWindow::GetForCurrentThread()->GetAsyncKeyState(Windows::System::VirtualKey::Up)
			|| Windows::UI::Core::CoreVirtualKeyStates::Down == Windows::UI::Core::CoreWindow::GetForCurrentThread()->GetAsyncKeyState(Windows::System::VirtualKey::W))
		{
			iState |= UG_DPAD_UP;
		}

		if (Windows::UI::Core::CoreVirtualKeyStates::Down == Windows::UI::Core::CoreWindow::GetForCurrentThread()->GetAsyncKeyState(Windows::System::VirtualKey::Down)
			|| Windows::UI::Core::CoreVirtualKeyStates::Down == Windows::UI::Core::CoreWindow::GetForCurrentThread()->GetAsyncKeyState(Windows::System::VirtualKey::S))
		{
			iState |= UG_DPAD_DOWN;
		}

		if (Windows::UI::Core::CoreVirtualKeyStates::Down == Windows::UI::Core::CoreWindow::GetForCurrentThread()->GetAsyncKeyState(Windows::System::VirtualKey::Left)
			|| Windows::UI::Core::CoreVirtualKeyStates::Down == Windows::UI::Core::CoreWindow::GetForCurrentThread()->GetAsyncKeyState(Windows::System::VirtualKey::A))
		{
			iState |= UG_DPAD_LEFT;
		}

		if (Windows::UI::Core::CoreVirtualKeyStates::Down == Windows::UI::Core::CoreWindow::GetForCurrentThread()->GetAsyncKeyState(Windows::System::VirtualKey::Right)
			|| Windows::UI::Core::CoreVirtualKeyStates::Down == Windows::UI::Core::CoreWindow::GetForCurrentThread()->GetAsyncKeyState(Windows::System::VirtualKey::D))
		{
			iState |= UG_DPAD_RIGHT;
		}

		if (Windows::UI::Core::CoreVirtualKeyStates::Down == Windows::UI::Core::CoreWindow::GetForCurrentThread()->GetAsyncKeyState(Windows::System::VirtualKey::Enter))
		{
			iState |= UG_BUTTON1;
		}

		if (Windows::UI::Core::CoreVirtualKeyStates::Down == Windows::UI::Core::CoreWindow::GetForCurrentThread()->GetAsyncKeyState(Windows::System::VirtualKey::Escape))
		{
			iState |= UG_BUTTON2;
		}

		if (Windows::UI::Core::CoreVirtualKeyStates::Down == Windows::UI::Core::CoreWindow::GetForCurrentThread()->GetAsyncKeyState(Windows::System::VirtualKey::Q))
		{
			iState |= UG_BUTTON3;
		}

		if (Windows::UI::Core::CoreVirtualKeyStates::Down == Windows::UI::Core::CoreWindow::GetForCurrentThread()->GetAsyncKeyState(Windows::System::VirtualKey::E))
		{
			iState |= UG_BUTTON4;
		}

		if (Windows::UI::Core::CoreVirtualKeyStates::Down == Windows::UI::Core::CoreWindow::GetForCurrentThread()->GetAsyncKeyState(Windows::System::VirtualKey::LeftShift))
		{
			iState |= UG_BUTTON5;
		}

		if (Windows::UI::Core::CoreVirtualKeyStates::Down == Windows::UI::Core::CoreWindow::GetForCurrentThread()->GetAsyncKeyState(Windows::System::VirtualKey::R))
		{
			iState |= UG_BUTTON6;
		}

		if (Windows::UI::Core::CoreVirtualKeyStates::Down == Windows::UI::Core::CoreWindow::GetForCurrentThread()->GetAsyncKeyState(Windows::System::VirtualKey::F1))
		{
			iState |= UG_BUTTON7;
		}

		if (Windows::UI::Core::CoreVirtualKeyStates::Down == Windows::UI::Core::CoreWindow::GetForCurrentThread()->GetAsyncKeyState(Windows::System::VirtualKey::Menu))
		{
			// If Alt is pressed.
			iState |= UG_OS_BUTTON;
		}

		m_pUInput->SetState( (AkUInt16) in_iPlayerIndex, bConnected, iState, joysticks );
	}
}

void InputMgr::TranslateInput( void * in_pad, int in_iPlayerIndex )
{
	UGBtnState iState = 0;
	bool bConnected = false;

	UGStickState joysticks[2];
	memset( joysticks, 0, sizeof( joysticks ) );

	XINPUT_STATE state;
	ZeroMemory( &state, sizeof( XINPUT_STATE ) );

	DWORD dwResult;
	dwResult = XInputGetState( in_iPlayerIndex, &state );

	if ( dwResult == ERROR_SUCCESS ) // XBox Controller is connected.
	{
		bConnected = true;
		WORD wButtons = state.Gamepad.wButtons;
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

		joysticks[UG_STICKLEFT].x = state.Gamepad.sThumbLX / 32768.f;
		joysticks[UG_STICKLEFT].y = state.Gamepad.sThumbLY / 32768.f;
		joysticks[UG_STICKRIGHT].x = state.Gamepad.sThumbRX / 32768.f;
		joysticks[UG_STICKRIGHT].y = state.Gamepad.sThumbRY / 32768.f;

	}

	m_pUInput->SetState( in_iPlayerIndex + 1, bConnected, iState, joysticks );
}
