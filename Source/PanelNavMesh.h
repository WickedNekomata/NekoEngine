#ifndef __PANEL_NAV_MESH_H__
#define __PANEL_NAV_MESH_H__

#include "GameMode.h"

#ifndef GAMEMODE

#include "Panel.h"

class PanelNavMesh : public Panel
{
public:

	PanelNavMesh(char* name);
	~PanelNavMesh();

	bool Draw();
};

#endif

#endif // GAME
