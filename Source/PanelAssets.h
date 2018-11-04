#ifndef __PANEL_ASSETS_H__
#define __PANEL_ASSETS_H__

#include "GameMode.h"

#ifndef GAMEMODE

#include "Panel.h"

class PanelAssets : public Panel
{
public:

	PanelAssets(char* name);
	~PanelAssets();

	bool Draw();

	void RecursiveDrawDir(const char* dir);
};

#endif

#endif // GAME
