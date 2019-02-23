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

// DemoSpatialAudio.cpp
/// \file
/// Defines all methods declared in DemoSpatialAudio.h

#include "stdafx.h"

#include <math.h>
#include <float.h>
#include "Menu.h"
#include "MovableChip.h"
#include "DemoSpatialAudio.h"
#include <AK/SoundEngine/Common/AkSoundEngine.h>    // Sound engine
#include <AK/SpatialAudio/Common/AkSpatialAudio.h>

//If you get a compiling error here, it means the file wasn't generated with the banks.  Did you generate the soundbanks before compiling?
#include "../WwiseProject/GeneratedSoundBanks/Wwise_IDs.h"		

// Our game object ID.  Completely arbitrary.
// IMPORTANT: As stated in AK::SpatialAudio::AddRoom and AddPortal, room and portal IDs exist in the same ID-space than game objects.
const AkGameObjectID EMITTER = 100;
const AkGameObjectID LISTENER = 103;
const AkRoomID ROOM = 200;
const AkPortalID PORTAL0 = 300;
const AkPortalID PORTAL1 = 301;

#define REPEAT_TIME 20

/////////////////////////////////////////////////////////////////////
// DemoSpatialAudio Public Methods
/////////////////////////////////////////////////////////////////////

DemoSpatialAudio::DemoSpatialAudio(Menu& in_ParentMenu, DemoSpatialAudio_Scenario in_scenario)
	: Page( in_ParentMenu, "Spatial Audio")
	, m_pEmitterChip( NULL )
	, m_pListenerChip(NULL)
	, m_pRoom(NULL)
	, m_pPortal0(NULL)
	, m_pPortal1(NULL)
	, m_aLines(NULL)
	, m_numLines(0)
	, m_aRaycast0toL(NULL)
	, m_aRaycast1toL(NULL)
	, m_aRaycastEtoL(NULL)
	, m_fGameObjectX( 0 )
	, m_fGameObjectZ( 0 )
	, m_dryDiffractionAngle( 0 )
	, m_wetDiffractionAngle( 0 )
	, m_fWidth( 0.0f )
	, m_fHeight(0.0f)
	, m_room0_width(0.0f)
	, m_room0_height(0.0f)
	, m_roomCornerx(0.0f)
	, m_roomCornery(0.0f)
	, m_portalsOpen(0x3)	// both portals open
	, m_uRepeat(REPEAT_TIME)
	, m_uLastTick(0)
	, m_bMoved(false)
	, m_eScenario(in_scenario)
{
	m_obsPortal[0] = 0.0f;
	m_obsPortal[1] = 0.0f;
}

bool DemoSpatialAudio::Init_PortalsDemo()
{
	// Register the emitter game object
	AK::SoundEngine::RegisterGameObj(EMITTER, "Emitter E");
	
	// Let's register a different listener than the default one (registered in IntegrationDemo.cpp) just for this demo, because we are going to move it around.
	AK::SoundEngine::RegisterGameObj(LISTENER, "Listener L");
	
	// Associate our emitters with our listener. This will set up direct path from EMITTER -> LISTENER
	// The listener object will also be emitting so we need to set LISTENER -> LISTENER too.
	static const int kNumLstnrsForEm = 1;
	static const AkGameObjectID aLstnrsForEmitter[kNumLstnrsForEm] = { LISTENER };
	AK::SoundEngine::SetListeners(EMITTER, aLstnrsForEmitter, kNumLstnrsForEm);
	AK::SoundEngine::SetListeners(LISTENER, aLstnrsForEmitter, kNumLstnrsForEm);

	// Register these two objects as spatial audio emitters.
	AK::SpatialAudio::RegisterEmitter(EMITTER, AkEmitterSettings());
	AK::SpatialAudio::RegisterEmitter(LISTENER, AkEmitterSettings());

	// And register our listener as the one and only spatial audio listener.
	AK::SpatialAudio::RegisterListener(LISTENER);

	m_szHelp =
		"*** Please connect Wwise and open the Advanced Profiler to the Voices Graph.\n"
		"In this demo we have 2 rooms, ROOM (box in the upper left) and Outside, 2 portals (PORTAL0 in the upper right, PORTAL1 in the lower left), and\n"
		"* 'Emitter/[E]' - the position of an emitter.\n"
		"* 'Listener/[L]' - the position of the listener (namely the main character or camera in a game), which also emits a footstep sound when moving.\n"
		"The sound propagation path(s) are displayed, with the resulting diffraction angles in the lower left corner.\n"
		"Spatial Audio is set up so that diffraction controls both project-wide obstruction and the Diffraction built-in parameter, although only the former is used in the project.\n"
		"In addition to portal-driven propagation, a native game-side obstruction algorithm is used for same-room obstruction (emitter-listener and portals-listener).\n"
		"Refer to Rooms and Portals in the SDK documentation for more details.\n"
		"* <<UG_RIGHT_STICK>> + <<DIRECTIONAL_TYPE>> - Move the listener game object.\n"
		"* <<UG_BUTTON5>> + <<UG_RIGHT_STICK>> + <<DIRECTIONAL_TYPE>> - Move the emitter game object.\n"
		"* <<UG_BUTTON1>> - Open/close portals in sequence.";

	return true;
}

bool DemoSpatialAudio::Init()
{
	switch (m_eScenario)
	{
	case Scenario_Portals:
		if (!Init_PortalsDemo())
			return false;
		break;
	}

	// Load the sound bank
	AkBankID bankID; // Not used
	if ( AK::SoundEngine::LoadBank( "Bus3d_Demo.bnk", AK_DEFAULT_POOL_ID, bankID ) != AK_Success )
	{
		SetLoadFileErrorMessage( "Bus3d_Demo.bnk" );
		return false;
	}

	m_uLastTick = m_pParentMenu->GetTickCount();

	return Page::Init();
}

void DemoSpatialAudio::Release()
{
	AK::SoundEngine::StopAll();
	AK::SpatialAudio::RemovePortal(PORTAL0);
	AK::SpatialAudio::RemovePortal(PORTAL1);
	AK::SpatialAudio::RemoveRoom(ROOM);
	AK::SpatialAudio::RemoveRoom(AkRoomID::OutdoorsGameObjID);
	AK::SoundEngine::UnregisterGameObj(EMITTER);
	AK::SoundEngine::UnregisterGameObj(LISTENER);
	AK::SoundEngine::UnloadBank( "Bus3d_Demo.bnk", NULL );

	Page::Release();
}

#define POSITION_RANGE (200.0f)

float DemoSpatialAudio::PixelsToAKPos_X(float in_X)
{
	return ((in_X / m_fWidth) - 0.5f) * POSITION_RANGE;
}

float DemoSpatialAudio::PixelsToAKPos_Y(float in_y)
{
	return -((in_y / m_fHeight) - 0.5f) * POSITION_RANGE;
}

float DemoSpatialAudio::PixelsToAKLen_X(float in_X)
{
	return (in_X / m_fWidth) * POSITION_RANGE;
}

float DemoSpatialAudio::PixelsToAKLen_Y(float in_y)
{
	return (in_y / m_fHeight) * POSITION_RANGE;
}

int DemoSpatialAudio::AKPosToPixels_X(float in_X)
{
	return (int)(((in_X / POSITION_RANGE) + 0.5f) * m_fWidth);
}

int DemoSpatialAudio::AKPosToPixels_Y(float in_y)
{
	return (int)(((-in_y / POSITION_RANGE) + 0.5f) * m_fHeight);
}

// Naive containment test.
// Returns the room index.
AkRoomID DemoSpatialAudio::IsInRoom(float in_x, float in_y)
{
	if (in_x <= m_roomCornerx && in_y <= m_roomCornery)
		return ROOM;
	return AkRoomID::OutdoorsGameObjID;
}

static int PortalIDToIndex(AkPortalID in_portalID)
{
	if (in_portalID == PORTAL0)
		return 0;
	else
		return 1;
}

void DemoSpatialAudio::UpdateGameObjPos(MovableChip* in_pChip, AkGameObjectID in_GameObjectId)
{
	if (in_pChip)
	{
		float x, y;
		in_pChip->GetPos(x, y);

		// Converting X-Y UI into X-Z world plan.
		AkVector position;
		m_fGameObjectX = position.X = PixelsToAKPos_X(x);
		position.Y = 0;
		m_fGameObjectZ = position.Z = PixelsToAKPos_Y(y);
		AkVector orientationFront;
		orientationFront.Z = 1;
		orientationFront.Y = orientationFront.X = 0;
		AkVector orientationTop;
		orientationTop.X = orientationTop.Z = 0;
		orientationTop.Y = 1;

		AkSoundPosition soundPos;
		soundPos.Set(position, orientationFront, orientationTop);
		AK::SpatialAudio::SetPosition(in_GameObjectId, soundPos);

		// For SpatialAudio
		AkRoomID roomID = IsInRoom(x, y);
		
		// Set emitter aux send values if we are updating the emitter.
		AK::SpatialAudio::SetGameObjectInRoom(in_GameObjectId, roomID);
	}
	
}

bool DemoSpatialAudio::Update()
{
	UniversalInput::Iterator it;
	for ( it = m_pParentMenu->Input()->Begin(); it != m_pParentMenu->Input()->End(); it++ )
	{
		// Skip this input device if it's not connected
		if ( ! it->IsConnected() )
			continue;

		if ((*it).IsButtonDown(UG_BUTTON5))
		{
			m_pEmitterChip->Update(*it);
			m_bMoved = true;
		}
		else
		{
			float xprev, yprev;
			m_pListenerChip->GetPos(xprev, yprev);
			m_pListenerChip->Update(*it);
			float x, y; 
			m_pListenerChip->GetPos(x, y);
			if ((m_uLastTick + REPEAT_TIME) <= (AkUInt32)m_pParentMenu->GetTickCount()
				&& (((x - xprev)*(x - xprev) + (y - yprev)*(y - yprev)) > 0.1f))
			{
				AK::SoundEngine::PostEvent("Play_Footstep", LISTENER);
				m_uLastTick = m_pParentMenu->GetTickCount();
				
			}

			m_bMoved = true;
		}

		if (m_uRepeat == 0)
		{
			if ((*it).IsButtonDown(UG_BUTTON1))
			{
				// toggle portals open state.
				m_portalsOpen = (m_portalsOpen + 1) & 0x3;
				SetPortals();

				m_uRepeat = REPEAT_TIME;
			}
		}
		else
		{
			m_uRepeat--;
		}
	}

	if ( m_bMoved )
	{
		// Update room dimensions for proper updating of emitters' room and "ray-cast" obstruction.
		float roomx, roomy, roomw, roomh;
		m_pRoom->GetRect(roomx, roomy, roomw, roomh);
		m_roomCornerx = roomx + roomw;
		m_roomCornery = roomy + roomh;

		// Update emitters, set their room.
		UpdateGameObjPos(m_pEmitterChip, EMITTER);
		UpdateGameObjPos(m_pListenerChip, LISTENER);

		// Perform naive ray-casting for same-room obstruction.
		// We use game-side ray-casting to compute the obstruction between the listener and each portal.
		// We also use it to compute obstruction between the emitter and listener *only when they are in the same room*. When they aren't, obstruction flows from more diffraction/transmission modeling of SpatialAudio's rooms and portals.
		const float k_obstructed = 0.7f;

		float listx, listy;
		m_pListenerChip->GetPos(listx, listy);
		float emitx, emity;
		m_pEmitterChip->GetPos(emitx, emity);

		// Compute coordinates relative to the room's corner.
		// Quadrants:
		// 2 | 1
		// -----
		// 3 | 4
		float relListx = (listx - m_roomCornerx);
		float relListy = (listy - m_roomCornery);

		// Compute obstruction between portals and the listener. Portal 0 is obstructed when the listener is in quadrant 3, and portal 1 is obstructed when the listener is in quadrant 1.
		m_obsPortal[0] = (relListx < 0 && relListy > 0) ? k_obstructed : 0.f;
		m_obsPortal[1] = (relListx > 0 && relListy < 0) ? k_obstructed : 0.f;
		SetPortalsObstruction();

		// Emitter: if it is in the same room as the listener, compute a ray-cast based obstruction value.
		// This is done by the game, with various levels of complexity, and is set directly to the sound engine via AK::SoundEngine::SetObjectObstructionAndOcclusion.
		// If the emitter is in a different room, diffraction, obstruction and/or occlusion is handled by AK::SpatialAudio.
		float obsEmitter = 0.f;
		if (IsInRoom(listx, listy) == IsInRoom(emitx, emity))
		{
			float relEmitx = (emitx - m_roomCornerx);
			float relEmity = (emity - m_roomCornery);
			// Obstruction is non-zero only if emitter and listener are not in the same quadrant
			if (((relEmity >= 0) ^ (relListy >= 0)) && relEmitx * relListx < 0)
			{
				float crossprod = relListx * relEmity - relEmitx * relListy;
				obsEmitter = (relEmitx * crossprod > 0.f) ? k_obstructed : 0.f;
			}
			AK::SpatialAudio::SetEmitterObstruction(EMITTER, obsEmitter);
		}

		//
		// Query propagation for UI display.
		//
		
		for (int i = 0; i < m_numLines; i++)
		{
			if (m_aLines[i])
				delete m_aLines[i];
		}
		delete[] m_aLines;
		m_aLines = NULL;
		m_numLines = 0;

		AkVector emitterPos, listenerPos;
		AkPropagationPathInfo paths[8];
		AkUInt32 uNumPaths = 8;	// 8 arbitrary
		AK::SpatialAudio::QuerySoundPropagationPaths(
			EMITTER,	///< The ID of the game object that the client wishes to query.
			listenerPos,	///< Returns the position of the listener game object that is associated with the game object \c in_gameObjectID.
			emitterPos,		///< Returns the position of the emitter game object \c in_gameObjectID.
			paths,			///< Pointer to an array of \c AkPropagationPathInfo's which will be filled after returning.
			uNumPaths		///< The number of slots in \c out_aPaths, after returning the number of valid elements written.
			);

		// Count lines to draw.
		for (AkUInt32 path = 0; path < uNumPaths; path++)
		{
			int numNodes = paths[path].numNodes;
			int numSegments = numNodes + 1;
			m_numLines += numSegments;
		}

		m_dryDiffractionAngle = 0;
		m_wetDiffractionAngle = 0;

		float dryDiffractionAngle = FLT_MAX;
		float wetDiffractionAngle = FLT_MAX;

		if (m_numLines > 0)
		{
			m_aLines = new Line*[m_numLines];

			int line = 0;
			for (AkUInt32 path = 0; path < uNumPaths; path++)
			{
				int numNodes = paths[path].numNodes;
				int numSegments = numNodes + 1;

				// First (emitter to portal).
				m_aLines[line] = new Line(*this);
				m_aLines[line]->SetPoints(AKPosToPixels_X(emitterPos.X), AKPosToPixels_Y(emitterPos.Z), AKPosToPixels_X(paths[path].nodePoint[0].X), AKPosToPixels_Y(paths[path].nodePoint[0].Z));
				AKASSERT(numSegments <= 2);	// Given the simple geometry of this demo, we know that no propagation path will ever have more than two segments.
				/*for (int i = 1; i < numSegments - 1; i++)
				{
				}*/
				// Last (portal to listener).
				// Do not draw line if this portal is obstructed (per our raycast obstruction algorithm above).
				int uLast = line + numSegments - 1;
				m_aLines[uLast] = NULL;
				if (m_obsPortal[PortalIDToIndex(paths[path].portals[0])] == 0.f)
				{
					m_aLines[uLast] = new Line(*this);
					m_aLines[uLast]->SetPoints(AKPosToPixels_X(paths[path].nodePoint[numNodes - 1].X), AKPosToPixels_Y(paths[path].nodePoint[numNodes - 1].Z), AKPosToPixels_X(listenerPos.X), AKPosToPixels_Y(listenerPos.Z));
				}
				line = uLast + 1;

				// Display the minimum diffraction angle of the (possibly) two propagation paths. Since we only have Room (i.e. game object) scoped RTPCs in Wwise at the moment, this is the actual value that will be pushed
				// to our built-in game parameter "Diffraction".
				dryDiffractionAngle = AkMin(dryDiffractionAngle, paths[path].dryDiffractionAngle);
				wetDiffractionAngle = AkMin(wetDiffractionAngle, paths[path].wetDiffractionAngle);
			}
			m_dryDiffractionAngle = dryDiffractionAngle;
			m_wetDiffractionAngle = wetDiffractionAngle;
		}
		else
		{
			m_dryDiffractionAngle = 0;
			m_wetDiffractionAngle = 0;
		}
		
		// Ray-cast line between emitter and listener. Show it when they are in the same room and obstruction is 0.
		delete m_aRaycastEtoL;
		m_aRaycastEtoL = NULL;
		if (obsEmitter == 0.f && (IsInRoom(listx, listy) == IsInRoom(emitx, emity)))
		{
			Line * pLine = new Line(*this);
			emitx, emity;
			pLine->SetPoints((int)emitx, (int)emity, (int)listx, (int)listy);
			m_aRaycastEtoL = pLine;
		}
	}

	// Reset m_bMoved.
	m_bMoved = false;

	return Page::Update();
}

static float Rad2Deg(float in_rad)
{
	const float k_pi = 3.14159265358979f;
	return 180.f * in_rad / k_pi;
}

void DemoSpatialAudio::Draw()
{
	Page::Draw();

	if (m_pEmitterChip)
		m_pEmitterChip->Draw();
	if (m_pListenerChip)
		m_pListenerChip->Draw();

	if (m_pRoom)
		m_pRoom->Draw();
	if (m_pPortal0)
		m_pPortal0->Draw(DrawStyle_Selected);
	if (m_pPortal1)
		m_pPortal1->Draw(DrawStyle_Selected);

	for (int i = 0; i < m_numLines; i++)
	{
		if (m_aLines[i])
			m_aLines[i]->Draw();
	}

	if (m_aRaycastEtoL)
		m_aRaycastEtoL->Draw(DrawStyle_Selected);

	char strBuf[128];
	if (m_numLines)
		snprintf(strBuf, 128, "X: %.2f\nZ: %.2f\nDiffraction dry: %.2f\nDiffraction wet: %.2f", m_fGameObjectX, m_fGameObjectZ, Rad2Deg(m_dryDiffractionAngle), Rad2Deg(m_wetDiffractionAngle));
	else
		snprintf(strBuf, 128, "X: %.2f\nZ: %.2f\nTransmission", m_fGameObjectX, m_fGameObjectZ);

	static int s_nOffset = 4 * GetLineHeight( DrawStyle_Text );

	DrawTextOnScreen( strBuf, 5, m_pParentMenu->GetHeight() - s_nOffset, DrawStyle_Text );
	
	// Display instructions at the bottom of the page
	int iInstructionsY = m_pParentMenu->GetHeight() - 3 * GetLineHeight( DrawStyle_Text );
	DrawTextOnScreen( "(Press <<UG_BUTTON2>> To Go Back...)", m_pParentMenu->GetWidth() / 4, iInstructionsY, DrawStyle_Text );
}

bool DemoSpatialAudio::OnPointerEvent(PointerEventType in_eType, int in_x, int in_y)
{
	if ( in_eType == PointerEventType_Moved )
	{
		bool bMoveListener = true;
		for (UniversalInput::Iterator it = m_pParentMenu->Input()->Begin(); it != m_pParentMenu->Input()->End(); it++)
		{
			if ((*it).IsButtonDown(UG_BUTTON5))
			{
				bMoveListener = false;
				break;
			}
		}

		if (bMoveListener)
			m_pListenerChip->SetPos((float)in_x, (float)in_y);
		else
			m_pEmitterChip->SetPos((float)in_x, (float)in_y);

		m_bMoved = true;	// Force update of game objects and GUI of sound propagation.
	}

	return Page::OnPointerEvent( in_eType, in_x, in_y );
}

// Called at init but also every time we open or close (all) doors.
void DemoSpatialAudio::InitControls_Portals()
{
	m_pRoom = new Box(*this);
	m_pPortal0 = new Box(*this);
	m_pPortal1 = new Box(*this);

	// Register rooms to Spatial Audio.
	AkRoomParams paramsRoom;
	// Let's orient our rooms towards the top of the screen. 
	paramsRoom.Front.X = 0.f;
	paramsRoom.Front.Y = 0.f;
	paramsRoom.Front.Z = 1.f;
	paramsRoom.Up.X = paramsRoom.Up.Z = 0.f;
	paramsRoom.Up.Y = 1.f;
	paramsRoom.WallOcclusion = 0.9f;	// Let's have a bit of sound transmitted through walls when all portals are closed.
	paramsRoom.strName = "Room Object";
	paramsRoom.ReverbAuxBus = AK::SoundEngine::GetIDFromString("Room");
	
	AK::SpatialAudio::SetRoom(ROOM, paramsRoom);
	
	// Also register "outside". Outside is already created automatically, but we want to give it a name, and set its aux send (reverb)
	paramsRoom.WallOcclusion = 0.f;
	paramsRoom.strName = "Outside Object";
	paramsRoom.ReverbAuxBus = AK::SoundEngine::GetIDFromString("Outside");
	AK::SpatialAudio::SetRoom(AkRoomID::OutdoorsGameObjID, paramsRoom);

	SetPortals();
}

void DemoSpatialAudio::SetPortals()
{
	float room0_width = m_room0_width = m_fWidth / 2.f;
	float room0_height = m_room0_height = m_fHeight / 2.f;
	
	m_pRoom->SetPosition(0, 0);
	m_pRoom->SetDimensions((int)room0_width, (int)room0_height);

	const float PORTAL_MARGIN = 15.f;
	const float PORTAL0_WIDTH = 90.f;	// width in screen coordinates (left-right). This is actually used for the portal's depth.
	const float PORTAL0_HEIGHT = 60.f;
	float portal0TopLeftX = room0_width - PORTAL0_WIDTH / 2.f;
	float portal0TopLeftZ = PORTAL_MARGIN;

	m_pPortal0->SetPosition((int)portal0TopLeftX, (int)portal0TopLeftZ);
	m_pPortal0->SetDimensions((int)PORTAL0_WIDTH, (int)PORTAL0_HEIGHT);

	const float PORTAL1_HEIGHT = 90.f;
	
	float portal1TopLeftX = PORTAL_MARGIN;
	float portal1TopLeftZ = room0_height - PORTAL1_HEIGHT / 2.f;
	float portal1Width = room0_width - 2.f * PORTAL_MARGIN;
	m_pPortal1->SetPosition((int)portal1TopLeftX, (int)portal1TopLeftZ);
	m_pPortal1->SetDimensions((int)portal1Width, (int)PORTAL1_HEIGHT);
	
	//
	// Register portals to SpatialAudio.
	//

	AkPortalParams paramsPortal;
	
	//
	// Portal 0 (ROOM->Outside, like an horizontal tube, on the top right)
	//
	paramsPortal.Transform.SetPosition( 
		PixelsToAKPos_X(portal0TopLeftX + PORTAL0_WIDTH / 2.f),
		0,
		PixelsToAKPos_Y(portal0TopLeftZ + PORTAL0_HEIGHT / 2.f));
	// Points to the right.
	// Up vector: This is a 2D game with Y pointing towards the player, and so should the local Y.
	paramsPortal.Transform.SetOrientation(1.f, 0.f, 0.f, 0.f, 1.f, 0.f);
	// Portal extent. Defines the dimensions of the portal relative to its center; all components must be positive numbers. The local X and Y dimensions (side and top) are used in diffraction calculations, 
	// whereas the Z dimension (front) defines a depth value which is used to implement smooth transitions between rooms. It is recommended that users experiment with different portal depths to find a value 
	// that results in appropriately smooth transitions between rooms.
	// Important: divide height and width by 2, because Extent expresses dimensions relative to the center (like a radius).
	paramsPortal.Extent.X = PixelsToAKLen_Y(PORTAL0_HEIGHT / 2.f);	// Door width. Note: PORTAL0_HEIGHT is screen coordinates (up-down).
	paramsPortal.Extent.Y = PixelsToAKLen_Y(PORTAL0_HEIGHT / 2.f);	// This app is 2D. Just pick a non-zero value so that this edge is never chosen.
	paramsPortal.Extent.Z = PixelsToAKLen_X(PORTAL0_WIDTH / 2.f);	// Portal depth (transition region). Note: PORTAL0_WIDTH is screen coordinates (left-right).
	// Whether or not the portal is active/enabled. For example, this parameter may be used to simulate open/closed doors.
	paramsPortal.bEnabled = (m_portalsOpen & 0x1) > 0;	// Open if bit 0 of our portal open state m_portalsOpen is set.
	// Name used to identify portal (optional).
	paramsPortal.strName = "Portal ROOM->Outside, horizontal";
	// ID of the room that the portal connects to, in the direction of the Front vector.
	paramsPortal.FrontRoom = AkRoomID::OutdoorsGameObjID;
	// ID of room that that portal connects, in the direction opposite to the Front vector. 
	paramsPortal.BackRoom = ROOM;

	AK::SpatialAudio::SetPortal(PORTAL0, paramsPortal);
	
	
	//
	// Portal 1 (Outside->ROOM, like a wide vertical tube, bottom-left).
	//
	paramsPortal.Transform.SetPosition(
		PixelsToAKPos_X(portal1TopLeftX + portal1Width / 2.f),
		0,
		PixelsToAKPos_Y(portal1TopLeftZ + PORTAL1_HEIGHT / 2.f));
	// Points up towards ROOM.
	// Up vector: This is a 2D game with Y pointing towards the player, and so should the local Y.
	paramsPortal.Transform.SetOrientation(0.f, 0.f, 1.f, 0.f, 1.f, 0.f);
	// Portal extent. Defines the dimensions of the portal relative to its center; all components must be positive numbers. The local X and Y dimensions (side and top) are used in diffraction calculations, 
	// whereas the Z dimension (front) defines a depth value which is used to implement smooth transitions between rooms. It is recommended that users experiment with different portal depths to find a value 
	// that results in appropriately smooth transitions between rooms.
	// Important: divide width and height by 2, because Extent expresses dimensions relative to the center (like a radius).
	paramsPortal.Extent.X = PixelsToAKLen_X(portal1Width / 2.f);	// Door width. 
	paramsPortal.Extent.Y = PixelsToAKLen_X(PORTAL0_HEIGHT / 2.f);	// This app is 2D. Just pick a non-zero value so that this edge is never chosen.
	paramsPortal.Extent.Z = PixelsToAKLen_Y(PORTAL1_HEIGHT / 2.f);	// Portal depth (transition region). 
	/// Whether or not the portal is active/enabled. For example, this parameter may be used to simulate open/closed doors.
	paramsPortal.bEnabled = (m_portalsOpen & 0x2) > 0;	// Open if bit 1 of our portal open state m_portalsOpen is set.
	/// Name used to identify portal (optional).
	paramsPortal.strName = "Outside->ROOM, vertical";
	// ID of the room that the portal connects to, in the direction of the Front vector.
	paramsPortal.FrontRoom = ROOM;
	// ID of room that that portal connects, in the direction opposite to the Front vector. 
	paramsPortal.BackRoom = AkRoomID::OutdoorsGameObjID;

	AK::SpatialAudio::SetPortal(PORTAL1, paramsPortal);	

	SetPortalsObstruction();
}

void DemoSpatialAudio::SetPortalsObstruction()
{
	/// Portal obstruction value.  Set a value greater than 0 to indicate that an object resides between the listener and the portal. Valid range: (0.f-1.f)
	AK::SpatialAudio::SetPortalObstruction(PORTAL0, m_obsPortal[0]);
	AK::SpatialAudio::SetPortalObstruction(PORTAL1, m_obsPortal[1]);
}

void DemoSpatialAudio::InitControls()
{
	float fMargin = (float)m_pEmitterChip->GetRightBottomMargin();
	float x = (m_pParentMenu->GetWidth() - fMargin) * 1.f / 2.0f;
	float y = (m_pParentMenu->GetHeight() - fMargin) * 1.f / 2.0f;

	m_pEmitterChip = new MovableChip(*this);
	m_pEmitterChip->SetLabel( "<E>" );
	m_pEmitterChip->UseJoystick(UG_STICKRIGHT);
	m_pEmitterChip->SetPos(x - 100, y - 100);
	m_pEmitterChip->SetNonLinear();
	m_pEmitterChip->SetMaxSpeed(3.f);

	m_pListenerChip = new MovableChip(*this);
	m_pListenerChip->SetLabel("<L>");
	m_pListenerChip->UseJoystick(UG_STICKRIGHT);
	m_pListenerChip->SetNonLinear();
	m_pListenerChip->SetPos(x + 30, y + 30);
	m_pListenerChip->SetMaxSpeed(3.f);
	m_pListenerChip->Update(*(m_pParentMenu->Input()->Begin()));

	m_fWidth = (float)m_pParentMenu->GetWidth() - fMargin;
	m_fHeight = (float)m_pParentMenu->GetHeight() - fMargin;

	switch (m_eScenario)
	{
	case Scenario_Portals:
		InitControls_Portals();
		AK::SoundEngine::PostEvent("Play_Room_Emitter", EMITTER);
		break;
	}

	// Update movable chips the first time.
	UniversalInput::Iterator it;
	for (it = m_pParentMenu->Input()->Begin(); it != m_pParentMenu->Input()->End(); it++)
	{
		// Skip this input device if it's not connected
		if (!it->IsConnected())
			continue;

		m_pEmitterChip->Update(*it);
		m_pListenerChip->Update(*it);
		UpdateGameObjPos(m_pEmitterChip, EMITTER);
		UpdateGameObjPos(m_pListenerChip, LISTENER);
	}
}
