#ifndef __PANEL_LAYER_TAGS_H__
#define __PANEL_LAYER_TAGS_H__

#include "Panel.h"

#ifndef GAMEMODE

#include "Globals.h"

class PanelLayersTags : public Panel
{
public:

	PanelLayersTags(char* name);
	~PanelLayersTags();

	bool Draw();
};

#endif

#endif
