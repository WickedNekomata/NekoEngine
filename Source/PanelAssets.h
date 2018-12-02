#ifndef __PANEL_ASSETS_H__
#define __PANEL_ASSETS_H__

#include "GameMode.h"

#ifndef GAMEMODE

#include "Panel.h"

#include "ResourceTypes.h"

#include <string>

struct FileInAssets;

class PanelAssets : public Panel
{
public:

	PanelAssets(char* name);
	~PanelAssets();

	bool Draw();

private:

	void RecursiveDrawDir(FileInAssets* fileInAssets) const;
	void SetDragAndDropSource(const char* file) const;
};

#endif

#endif // GAME
