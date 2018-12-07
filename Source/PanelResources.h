#ifndef __PANEL_RESOURCES_H__
#define __PANEL_RESOURCES_H__

#include "GameMode.h"

#ifndef GAMEMODE

#include "Panel.h"

#include <string>

struct LibraryFile;

class PanelResources : public Panel
{
public:

	PanelResources(char* name);
	~PanelResources();

	bool Draw();

private:

	void RecursiveDrawLibraryDir(LibraryFile* libraryFile) const;
	void OpenSettingsAtClick(const char** iterator, std::string currentFile) const;
};

#endif

#endif // GAME