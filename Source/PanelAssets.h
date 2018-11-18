#ifndef __PANEL_ASSETS_H__
#define __PANEL_ASSETS_H__

#include "GameMode.h"

#ifndef GAMEMODE

#include "Panel.h"

#include "ResourceTypes.h"

#include <string>

class PanelAssets : public Panel
{
public:

	PanelAssets(char* name);
	~PanelAssets();

	bool Draw();

private:

	void RecursiveDrawDir(const char* dir, std::string& path) const;
	void SetDragAndDropSource(const char* file) const;
};

#endif

#endif // GAME
