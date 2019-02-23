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
// Demo3dBus.h
/// \file
/// Contains the declaration for the Demo3dBus class.

#pragma once

#include "Page.h"
#include "MovableChip.h"

class Box : public Control
{
public:
	Box(Page& in_rParentPage) : Control(in_rParentPage), m_iWidth(0), m_iHeight(0) {}
	virtual ~Box() {}

	/// Updates the Control based on the input received.
	/// \sa UGBtnState
	virtual void Update(const UniversalGamepad& ) {}

	/// Draws the Control to the screen.
	/// \sa DrawStyle
	virtual void Draw(DrawStyle in_eDrawStyle = DrawStyle_Control)
	{
#ifdef CAN_DRAW_LINES
		if (m_iWidth > 0 && m_iHeight > 0)
		{
			DrawLineOnScreen(m_iXPos, m_iYPos, m_iXPos + m_iWidth, m_iYPos, in_eDrawStyle);
			DrawLineOnScreen(m_iXPos, m_iYPos + m_iHeight, m_iXPos + m_iWidth, m_iYPos + m_iHeight, in_eDrawStyle);
			DrawLineOnScreen(m_iXPos, m_iYPos, m_iXPos, m_iYPos + m_iHeight, in_eDrawStyle);
			DrawLineOnScreen(m_iXPos + m_iWidth, m_iYPos, m_iXPos + m_iWidth, m_iYPos + m_iHeight, in_eDrawStyle);
		}
#endif
	}
	
	void SetDimensions(
		int in_iWidth,				///< - Width
		int in_iHeight				///< - Height
		) {
		m_iWidth = in_iWidth;
		m_iHeight = in_iHeight;
	}

	void GetRect(float &out_x, float &out_y, float &out_w, float &out_h)
	{
		out_x = (float)m_iXPos;
		out_y = (float)m_iYPos;
		out_w = (float)m_iWidth;
		out_h = (float)m_iHeight;
	}
	
	void GetCenter(float &out_x, float &out_y)
	{
		out_x = (float)m_iXPos + m_iWidth / 2.f;
		out_y = (float)m_iYPos + m_iHeight / 2.f;
	}

private:
	/// Box width.
	int m_iWidth;

	/// Box height.
	int m_iHeight;
};

class Line : public Control
{
public:
	Line(Page& in_rParentPage) : Control(in_rParentPage), m_iXPos2(-1), m_iYPos2(-1) {}
	virtual ~Line() {}

	/// Updates the Control based on the input received.
	/// \sa UGBtnState
	virtual void Update(const UniversalGamepad&) {}

	/// Draws the Control to the screen.
	/// \sa DrawStyle
	virtual void Draw(DrawStyle in_eDrawStyle = DrawStyle_Control)
	{
#ifdef CAN_DRAW_LINES
		if (m_iXPos2 >= 0)
		{
			DrawLineOnScreen(m_iXPos, m_iYPos, m_iXPos2, m_iYPos2, in_eDrawStyle);
		}
#endif
	}

	void SetPoints(
		int in_iX1,
		int in_iY1,
		int in_iX2,
		int in_iY2
		) {
		m_iXPos = in_iX1;
		m_iYPos = in_iY1;
		m_iXPos2 = in_iX2;
		m_iYPos2 = in_iY2;
	}

private:
	/// The x-coordinate of the end position of the line.
	int m_iXPos2;

	/// The y-coordinate of the end position of the line.
	int m_iYPos2;
};

enum DemoSpatialAudio_Scenario
{
	Scenario_Portals
};

class DemoSpatialAudio : public Page
{
public:

	/// Demo3dBus class constructor.
	DemoSpatialAudio(
		Menu& in_pParentMenu,	///< - Pointer to the Menu that the page belongs to
		DemoSpatialAudio_Scenario in_scenario
		);

	/// Override of the Parent's Init() method.
	virtual bool Init();

	/// Override of the Parent's Release() method.
	virtual void Release();

	/// Override of the Parent's Update() method.
	virtual bool Update();

	/// Override of the Parent's Draw() method.
	virtual void Draw();

	virtual bool OnPointerEvent( PointerEventType in_eType, int in_x, int in_y );

private:

	/// Initializes the controls on the page.
	virtual void InitControls();
	bool Init_PortalsDemo();
	void InitControls_Portals(); 
	void UpdateGameObjPos(MovableChip* in_pChip, AkGameObjectID in_GameObjectId);

	void SetPortals();
	void SetPortalsObstruction();

	// Helpers

	float PixelsToAKPos_X(float in_X);
	float PixelsToAKPos_Y(float in_y);
	float PixelsToAKLen_X(float in_X);
	float PixelsToAKLen_Y(float in_y);
	int AKPosToPixels_X(float in_X);
	int AKPosToPixels_Y(float in_y);

	struct AkRoomID IsInRoom(float in_x, float in_z);

	// Members
	
	// UI controls/widgets
	MovableChip *m_pEmitterChip;
	MovableChip *m_pListenerChip;
	Box * m_pRoom;
	Box * m_pPortal0;
	Box * m_pPortal1;
	Line ** m_aLines;
	int m_numLines;
	Line * m_aRaycast0toL;
	Line * m_aRaycast1toL;
	Line * m_aRaycastEtoL;


	float m_fGameObjectX;
	float m_fGameObjectZ;
	float m_dryDiffractionAngle;
	float m_wetDiffractionAngle;
	float m_fWidth;
	float m_fHeight;
	float m_obsPortal[2];
	float m_room0_width;
	float m_room0_height;
	float m_roomCornerx;
	float m_roomCornery;
	AkUInt8 m_portalsOpen;	// bit field: bit0 -> portal0, bit1 -> portal1

	AkUInt8 m_uRepeat;
	AkUInt32 m_uLastTick;
	bool m_bMoved;
	
	DemoSpatialAudio_Scenario m_eScenario;
};
