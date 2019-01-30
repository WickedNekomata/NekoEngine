#ifndef __PANEL_LIBRARY_H__
#define __PANEL_LIBRARY_H__

#include "GameMode.h"

#ifndef GAMEMODE

#include "Panel.h"

#include <string>

struct LibraryFile;

class PanelLibrary : public Panel
{
public:

	PanelLibrary(const char* name);
	~PanelLibrary();

	bool Draw();

private:

	void RecursiveDrawLibraryDir(LibraryFile* libraryFile) const;
};

#endif

#endif // GAME