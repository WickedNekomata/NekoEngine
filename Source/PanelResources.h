#ifndef __PANEL_RESOURCES_H__
#define __PANEL_RESOURCES_H__

#include "GameMode.h"

#ifndef GAMEMODE

#include "Panel.h"

#include <string>

class PanelResources : public Panel
{
public:

	PanelResources(char* name);
	~PanelResources();

	bool Draw();

private:

	void RecursiveDrawDir(const char* dir, std::string& path) const;
	void OpenSettingsAtClick(const char** iterator, std::string currentFile) const;
};

#endif

#endif // GAME