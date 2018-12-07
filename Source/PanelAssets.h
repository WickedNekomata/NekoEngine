#ifndef __PANEL_ASSETS_H__
#define __PANEL_ASSETS_H__

#include "GameMode.h"

#ifndef GAMEMODE

#include "Panel.h"

#include "ResourceTypes.h"
#include "Globals.h"

struct AssetsFile;

class PanelAssets : public Panel
{
public:

	PanelAssets(char* name);
	~PanelAssets();

	bool Draw();

private:

	void RecursiveDrawAssetsDir(AssetsFile* assetsFile) const;
	void SetResourceDragAndDropSource(ResourceType type, const char* file = nullptr, uint UUID = 0) const;
};

#endif

#endif // GAME
