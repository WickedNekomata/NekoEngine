#ifndef __PANEL_LAYERS_H__
#define __PANEL_LAYERS_H__

#include "GameMode.h"

#ifndef GAMEMODE

#include "Panel.h"

class PanelLayers : public Panel
{
public:

	PanelLayers(const char* name);
	~PanelLayers();

	bool Draw();
};

#endif

#endif