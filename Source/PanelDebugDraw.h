#ifndef __PANEL_DEBUG_DRAW_H__
#define __PANEL_DEBUG_DRAW_H__

#include "Panel.h"

#ifndef GAMEMODE

class PanelDebugDraw : public Panel
{
public:

	PanelDebugDraw(const char* name);
	~PanelDebugDraw();

	bool Draw();
};

#endif

#endif
