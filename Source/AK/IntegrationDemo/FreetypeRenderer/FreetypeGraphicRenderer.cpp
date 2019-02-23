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

// FreetypeGraphicRenderer.cpp
/// \file 
/// Print text in a RGB656 buffer
/// Freetype errors are logged but there is no fallback code to handle errors.

/////////////////////////
//  INCLUDES
/////////////////////////

#include "stdafx.h"
#include <AK/Tools/Common/AkPlatformFuncs.h>
#include "Platform.h"
#include "FreetypeGraphicRenderer.h"

#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#ifdef AK_ANDROID
	#include <android/log.h>
	#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "IntegrationDemo", __VA_ARGS__))
	#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "IntegrationDemo", __VA_ARGS__))

	#define TTFFONT_LOCATION "/system/fonts/DroidSans.ttf"
#else
	#define LOGI(...) printf( __VA_ARGS__)
	#define LOGW(...) printf( __VA_ARGS__)

    #ifdef AK_EMSCRIPTEN	
        #define TTFFONT_LOCATION "Assets/GentiumPlusAm-R.ttf"
    #else
        #define TTFFONT_LOCATION "../../../samples/IntegrationDemo/FreetypeRenderer/Assets/GentiumPlusAm-R.ttf"		
    #endif
#endif

using std::string;

#define DEFAULT_DRAW_COLOUR		0xFFFFFFFF	// White
#define SELECTED_DRAW_COLOUR	0xFFFFFF00  // Yellow 0xFBE0/
#define TITLE_DRAW_COLOUR   	0xFFFFFFFF	// White  0xFFFF
#define ERROR_DRAW_COLOUR	 	0xFFFF0000  // Red	0xf800

/////////////////////////
//  FUNCTIONS
/////////////////////////

const PixelType* FreetypeGraphicRenderer::GetWindowBuffer()
{
	return m_pWindowBuffer;
}

void FreetypeGraphicRenderer::BeginDrawing()
{
	// Wipe out the buffer
	memset(m_pWindowBuffer,0,m_width*m_height*sizeof(PixelType));
}

void FreetypeGraphicRenderer::DoneDrawing()
{
	// Nothing to do
}


#define NUM_CACHED_GLYPHS 256
static unsigned char* CachedPixels[DrawStyle_Num][NUM_CACHED_GLYPHS];

void FreetypeGraphicRenderer::printString( const char* pStr, int in_x, int in_y, DrawStyle in_eDrawStyle)
{
	// Set colour based on the desired DrawStyle
	uint32_t color;
	stbtt_fontinfo* pFace = &m_font;
	float scalex = 1.0f;
	
	switch ( in_eDrawStyle )
	{
	case DrawStyle_Title:
		color = TITLE_DRAW_COLOUR;
		break;
	case DrawStyle_Selected:
		color = SELECTED_DRAW_COLOUR;
		scalex = 1.1f; // fake bold
		break;
	case DrawStyle_Error:
		color = ERROR_DRAW_COLOUR;
		break;
	case DrawStyle_Text:
		color = DEFAULT_DRAW_COLOUR;
		break;
	case DrawStyle_Control:
	default:
		color = DEFAULT_DRAW_COLOUR;
		break;
	}

	int b_w = 32; /* bitmap width */
    int b_h = GetLineHeight(in_eDrawStyle); /* bitmap height */    

	int ascent, descent, lineGap;
	float scale = stbtt_ScaleForPixelHeight(pFace, b_h);
	
	stbtt_GetFontVMetrics(pFace, &ascent, &descent, &lineGap);

	ascent *= scale;
	descent *= scale;
	scalex *= scale;

	/* create a bitmap for the phrase */
	unsigned char* bitmap = NULL;

	int i;
	int x = 0;
	int height = 0;
	for (i = 0; i < strlen(pStr); ++i)
	{
		int glyph = pStr[i];
		/* get bounding box for character (may be offset to account for chars that dip above or below the line */
		int c_x1, c_y1, c_x2, c_y2;
		stbtt_GetCodepointBitmapBox(pFace, glyph, scalex, scale, &c_x1, &c_y1, &c_x2, &c_y2);

		/* compute y (different characters have different heights */
		int y = ascent + c_y1;

		unsigned char ** cachedBitmap = &CachedPixels[in_eDrawStyle][glyph];

		if (*cachedBitmap)
			bitmap = *cachedBitmap;
		else
		{
			bitmap = (unsigned char*)malloc(b_w * b_h);

			memset(bitmap, 0, b_w * b_h);
			stbtt_MakeCodepointBitmap(pFace, bitmap, c_x2 - c_x1, c_y2 - c_y1, b_w, scalex, scale, glyph);

			*cachedBitmap = bitmap;
		}
		
		if (c_y2 - c_y1 > height)
			height = c_y2 - c_y1;

		/* how wide is this character */
		int ax;
		stbtt_GetCodepointHMetrics(pFace, glyph, &ax, 0);

		// now, draw to our target surface
		draw_bitmap( bitmap, in_x+x, in_y+y, b_w, height, color );

		x += ax * scalex + 2/*kerning doesn't work, so 2 pixel between each*/;
	}
}

void FreetypeGraphicRenderer::draw_bitmap( unsigned char* bitmap, int x, int y, int w, int h, uint32_t color)
{
	int  i, j, p, q;

	int  x_max = x + w;
	int  y_max = y + h;

	float fR = float((color & 0XFF0000) >> 16) / 255.f;
	float fG = float((color & 0XFF00) >> 8) / 255.f;
	float fB = float(color & 0XFF) / 255.f;

#ifndef FREETYPE_USE_32_BIT_RENDERER
	fR = (fR / 255.f) * 0X1F;
	fG = (fG / 255.f) * 0X1F;
	fB = (fB / 255.f) * 0X1F;
#endif

	for ( j = y, q = 0; j < y_max; j++, q++ )
	{
		// Outside range
		if( j > m_height || j < 0 )
			continue;

		int jOffset = j*m_width;
		int qOffset = q * w;
		for ( i = x, p = 0; i < x_max; i++, p++ )
		{
			// Outside range
			if ( i >= m_width  || i<0 )
				continue;

			PixelType R,G,B;
			float greyLevel = bitmap[qOffset+p];
			R = greyLevel * fR;
			G = greyLevel * fG;
			B = greyLevel * fB;
#ifdef FREETYPE_USE_32_BIT_RENDERER
			// Converting to ARGB8888
			R <<= 16;
			G <<= 8;
#else
			// Converting to RGB 565
			R <<= 11;
			G <<= 6;
#endif
		
			// Write the value to the buffer
			m_pWindowBuffer[jOffset+i] += R+B+G;
		}
	}
}

bool FreetypeGraphicRenderer::InitFreetype()
{
	const char *szFile = TTFFONT_LOCATION;
	int file=0;
	if((file=open(szFile,O_RDONLY)) <= -1)
	{
		LOGI("Could not open %s", szFile);
		return false;
	}
	
	struct stat fileStat; 
	fstat(file,&fileStat);

	m_pFontBuffer = (unsigned char*)malloc(fileStat.st_size);
		
	read(file, m_pFontBuffer, fileStat.st_size);
	close(file);

	int retFont = stbtt_InitFont(&m_font, m_pFontBuffer, 0);	

	return true;
}

bool FreetypeGraphicRenderer::InitDrawing( void* in_pParam, AkOSChar* in_szErrorBuffer, unsigned int in_unErrorBufferCharCount, int in_windowWidth, int in_windowHeight )
{
	m_width = in_windowWidth;
	m_height = in_windowHeight;

	m_pWindowBuffer = (PixelType*) malloc(m_width*m_height*sizeof(PixelType));

	memset(CachedPixels, 0, NUM_CACHED_GLYPHS*sizeof(unsigned char*) );

	return m_pWindowBuffer && InitFreetype();
}


void FreetypeGraphicRenderer::DrawTextOnScreen( const char* in_szText, int in_iXPos, int in_iYPos, DrawStyle in_eDrawStyle )
{
	string translated = in_szText;
	ReplaceTags( translated );

	char tmpStr[INTEGRATIONDEMO_MAX_MESSAGE];
	char* token;

	AKPLATFORM::SafeStrCpy(tmpStr, translated.c_str(), INTEGRATIONDEMO_MAX_MESSAGE);

	token = strtok( tmpStr, "\n" );
	while ( token )
	{
		printString( token , in_iXPos, in_iYPos, in_eDrawStyle );
		in_iYPos += GetLineHeight( in_eDrawStyle );
		token = strtok( NULL, "\n" );
	}
}

void FreetypeGraphicRenderer::TermDrawing()
{
	if (m_pFontBuffer)
		free(m_pFontBuffer);

	if( m_pWindowBuffer )
	{
		free(m_pWindowBuffer);
		m_pWindowBuffer = NULL;
	}

	for (int style = 0; style < DrawStyle_Num; ++style)
	{
		for (int i = 0; i < NUM_CACHED_GLYPHS; ++i)
		{
			if (CachedPixels[style][i])
			{
				free(CachedPixels[style][i]);
				CachedPixels[style][i] = 0;
			}
		}
	}
}

int FreetypeGraphicRenderer::GetLineHeight( DrawStyle in_eDrawStyle )
{
	if ( in_eDrawStyle == DrawStyle_Title )
	{
		return 28;  // Change this if the font size is changed in InitDrawing()!!
	}
	else if ( in_eDrawStyle == DrawStyle_Text )
	{
		return 20;	// Change this if the font size is changed in InitDrawing()!!
	}
	else
	{
		return 24;	// Change this if the font size is changed in InitDrawing()!!
	}
}
