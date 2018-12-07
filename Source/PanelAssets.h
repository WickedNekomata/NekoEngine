#ifndef __PANEL_ASSETS_H__
#define __PANEL_ASSETS_H__

#include "GameMode.h"

#ifndef GAMEMODE

#include "Panel.h"

#include "ResourceTypes.h"
#include "Globals.h"

struct AssetsFile;
struct Resource;

class PanelAssets : public Panel
{
public:

	PanelAssets(char* name);
	~PanelAssets();

	bool Draw();

private:

	void RecursiveDrawAssetsDir(AssetsFile* assetsFile) const;
	void SetResourceDragAndDropSource(ResourceType type, uint UUID = 0, const Resource* resource = nullptr, const char* file = nullptr) const;
};

#endif

#endif // GAME
