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

	void RecursiveDrawDir(const char* dir, std::string& currentFile) const;
	void SetDragAndDropSource(ResourceType type, const char* path, const char* extension) const;
};

#endif

#endif // GAME
