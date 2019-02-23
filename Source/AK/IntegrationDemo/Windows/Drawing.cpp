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

// Drawing.cpp
/// \file 
// Implements the Drawing.h functions for the Windows platform


/////////////////////////
//  INCLUDES
/////////////////////////

#include "stdafx.h"
#include "Platform.h"
#include "Drawing.h"
#include <string>
using std::string;


#define DEFAULT_DRAW_COLOUR  0xFFFFFFFF	 // White
#define SELECTED_DRAW_COLOUR 0xFFFFFF00  // Yellow
#define TITLE_DRAW_COLOUR    0xFFFFFFFF	 // White
#define ERROR_DRAW_COLOUR	 0xFFFF0000  // Red

/////////////////////////
//  GLOBAL OBJECTS
/////////////////////////

LPDIRECT3D9       g_d3d       = NULL;  // The Direct3D9 Object
LPDIRECT3DDEVICE9 g_d3dDevice = NULL;  // The Direct3D9 Device Object
ID3DXFont        *g_RegFont   = NULL;  // Direct3D Font Object for regular text
ID3DXFont        *g_TitleFont = NULL;  // Direct3D Font Object for page titles
ID3DXFont		 *g_TextFont  = NULL;  // Direct3D Font Object for smaller text
HWND			 g_hWnd;
       
/////////////////////////
//  FUNCTIONS
/////////////////////////

static AkUInt32 g_iWidth;
static AkUInt32 g_iHeight;

AkUInt32 GetWindowWidth()
{
	return g_iWidth;
}

AkUInt32 GetWindowHeight()
{
	return g_iHeight;
}

void BeginDrawing()
{
	while (g_d3dDevice == NULL || g_d3dDevice->TestCooperativeLevel() != D3D_OK)
	{
		TermDrawing();
		InitDrawing(g_hWnd, NULL, 0);
		Sleep(250);
	}

	g_d3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB( 0, 40, 100 ), 1.0f, 0 );
	g_d3dDevice->BeginScene(); 
}

void DoneDrawing()
{
	g_d3dDevice->EndScene();
	g_d3dDevice->Present( NULL, NULL, NULL, NULL );
}

bool InitDrawing(
	void* in_pParam,
	AkOSChar* in_szErrorBuffer,
	unsigned int in_unErrorBufferCharCount,
	AkUInt32 in_windowWidth,
	AkUInt32 in_windowHeight
)
{
	g_hWnd = (HWND)(in_pParam);
	D3DPRESENT_PARAMETERS d3dpp;
	HRESULT hr;

	g_iWidth = in_windowWidth;
	g_iHeight = in_windowHeight;

	g_d3d = Direct3DCreate9( D3D_SDK_VERSION );
	ZeroMemory( &d3dpp, sizeof( d3dpp ) );
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.hDeviceWindow = g_hWnd;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

    hr = g_d3d->CreateDevice( D3DADAPTER_DEFAULT,
					          D3DDEVTYPE_HAL,
                              g_hWnd,
                              D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                              &d3dpp,
                              &g_d3dDevice );

	if ( FAILED( hr ) )
	{
		if ( in_szErrorBuffer )
			__AK_OSCHAR_SNPRINTF( in_szErrorBuffer, in_unErrorBufferCharCount, AKTEXT("IDirect3D9::CreateDevice() returned HRESULT %d"), hr );
		goto cleanup;
	}


	// Create the regular text font
	// NOTE: If the font size is changed, adjust GetLineHeight() to be suitable for the new value
	hr = D3DXCreateFont( g_d3dDevice,
                         22,                  //Font height
                         0,                   //Font width (default)
                         FW_NORMAL,           //Font Weight
                         1,                   //MipLevels
                         false,               //Italic
                         DEFAULT_CHARSET,     //CharSet
                         OUT_DEFAULT_PRECIS,  //OutputPrecision
                         ANTIALIASED_QUALITY, //Quality
                         DEFAULT_PITCH | FF_DONTCARE, //PitchAndFamily
                         L"Arial",            //pFacename,
                         &g_RegFont );
	if ( FAILED( hr ) )
	{
		if ( in_szErrorBuffer )
			__AK_OSCHAR_SNPRINTF( in_szErrorBuffer, in_unErrorBufferCharCount, AKTEXT("D3DXCreateFont() returned HRESULT %d"), hr );
		goto cleanup;
	}


	// Create the title text font
	// NOTE: If the font size is changed, adjust GetLineHeight() to be suitable for the new value
	hr = D3DXCreateFont( g_d3dDevice,
                         28,                  //Font height
                         0,                   //Font width (default)
                         FW_NORMAL,           //Font Weight
                         1,                   //MipLevels
                         false,               //Italic
                         DEFAULT_CHARSET,     //CharSet
                         OUT_DEFAULT_PRECIS,  //OutputPrecision
                         ANTIALIASED_QUALITY, //Quality
                         DEFAULT_PITCH | FF_DONTCARE, //PitchAndFamily
                         L"Arial",            //pFacename,
                         &g_TitleFont );
	if ( FAILED( hr ) )
	{
		if ( in_szErrorBuffer )
			__AK_OSCHAR_SNPRINTF( in_szErrorBuffer, in_unErrorBufferCharCount, AKTEXT("D3DXCreateFont() returned HRESULT %d"), hr );
		goto cleanup;
	}

	// Create the smaller text font
	// NOTE: If the font size is changed, adjust GetLineHeight() to be suitable for the new value
	hr = D3DXCreateFont( g_d3dDevice,
                         17,                  //Font height
                         0,                   //Font width (default)
                         FW_NORMAL,           //Font Weight
                         1,                   //MipLevels
                         false,               //Italic
                         DEFAULT_CHARSET,     //CharSet
                         OUT_DEFAULT_PRECIS,  //OutputPrecision
                         ANTIALIASED_QUALITY, //Quality
                         DEFAULT_PITCH | FF_DONTCARE, //PitchAndFamily
                         L"Arial",            //pFacename,
                         &g_TextFont );
	if ( FAILED( hr ) )
	{
		if ( in_szErrorBuffer )
			__AK_OSCHAR_SNPRINTF( in_szErrorBuffer, in_unErrorBufferCharCount, AKTEXT("D3DXCreateFont() returned HRESULT %d"), hr );
		goto cleanup;
	}

	return true;

cleanup:
	TermDrawing();
	return false;
}


void DrawTextOnScreen( const char* in_szText, int in_iXPos, int in_iYPos, DrawStyle in_eDrawStyle )
{
	string buffer = in_szText;

	// Replace tags in the text
	ReplaceTags( buffer );

	// Make sure the fonts have been initialized
	if ( !g_TitleFont || !g_RegFont || !g_TextFont )
	{
		return;
	}

	RECT       rect;
	ID3DXFont* font;
	UINT       colour;

	SetRect( &rect, in_iXPos, in_iYPos, 100, 100 );

	// Draw using the font and colour based on the desired DrawStyle
	switch ( in_eDrawStyle )
	{
	case DrawStyle_Title:
		font   = g_TitleFont;
		colour = TITLE_DRAW_COLOUR;
		break;
	case DrawStyle_Selected:
		font   = g_RegFont;
		colour = SELECTED_DRAW_COLOUR;
		break;
	case DrawStyle_Error:
		font   = g_RegFont;
		colour = ERROR_DRAW_COLOUR;
		break;
	case DrawStyle_Text:
	default:
		font = g_TextFont;
		colour = DEFAULT_DRAW_COLOUR;
		break;
	case DrawStyle_Control:
		font   = g_RegFont;
		colour = DEFAULT_DRAW_COLOUR;
		break;
	}

	font->DrawTextA( NULL,                    //pSprite
                    buffer.c_str(), //pString
                    -1,                      //Count
                    &rect,                   //pRect
                    DT_LEFT | DT_NOCLIP,     //Format
                    colour );                //Color
}

#ifdef CAN_DRAW_LINES
void DrawLineOnScreen(
	int in_iXPosStart,		    ///< - The X value of the drawing position
	int in_iYPosStart,		    ///< - The Y value of the drawing position
	int in_iXPosEnd,		    ///< - The X value of the drawing position
	int in_iYPosEnd,		    ///< - The Y value of the drawing position
	DrawStyle in_eDrawStyle ///< - The style with which to draw the text
	)
{
	if (!g_d3dDevice)
		return;

	UINT       colour;

	// Draw using the font and colour based on the desired DrawStyle
	switch (in_eDrawStyle)
	{
	case DrawStyle_Title:
		colour = TITLE_DRAW_COLOUR;
		break;
	case DrawStyle_Selected:
		colour = SELECTED_DRAW_COLOUR;
		break;
	case DrawStyle_Error:
		colour = ERROR_DRAW_COLOUR;
		break;
	case DrawStyle_Text:
	default:
		colour = DEFAULT_DRAW_COLOUR;
		break;
	case DrawStyle_Control:
		colour = DEFAULT_DRAW_COLOUR;
		break;
	}

	LPD3DXLINE line;
	D3DXCreateLine(g_d3dDevice, &line);
	D3DXVECTOR2 lines[] = { D3DXVECTOR2((float)in_iXPosStart, (float)in_iYPosStart), D3DXVECTOR2((float)in_iXPosEnd, (float)in_iYPosEnd) };
	line->Begin();
	line->Draw(lines, 2, colour);
	line->End();
	line->Release();
}
#endif

void TermDrawing()
{
	if ( g_RegFont )
	{
		g_RegFont->Release();
		g_RegFont = NULL;
	}
	if ( g_TitleFont )
	{
		g_TitleFont->Release();
		g_TitleFont = NULL;
	}
	if ( g_TextFont )
	{
		g_TextFont->Release();
		g_TextFont = NULL;
	}
	if ( g_d3dDevice )
	{
		g_d3dDevice->Release();
		g_d3dDevice = NULL;
	}
	if ( g_d3d )
	{
		g_d3d->Release();
		g_d3d = NULL;
	}
}


int GetLineHeight( DrawStyle in_eDrawStyle )
{
	if ( in_eDrawStyle == DrawStyle_Title )
	{
		return 30;  // Change this if the font size is changed in InitDrawing()!!
	}
	else if ( in_eDrawStyle == DrawStyle_Text )
	{
		return 20;	// Change this if the font size is changed in InitDrawing()!!
	}
	else
	{
		return 25;	// Change this if the font size is changed in InitDrawing()!!
	}
}