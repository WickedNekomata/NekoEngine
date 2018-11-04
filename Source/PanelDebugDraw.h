#ifndef __PANEL_DEBUG_DRAW_H__
#define __PANEL_DEBUG_DRAW_H__

#include "Panel.h"

class PanelDebugDraw : public Panel
{
public:

	PanelDebugDraw(char* name);
	~PanelDebugDraw();

	bool Draw();
};

#endif
