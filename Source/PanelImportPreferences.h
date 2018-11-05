#ifndef __PANEL_IMPORT_PREFERENCES_H__
#define __PANEL_IMPORT_PREFERENCES_H__

#include "GameMode.h"

#ifndef GAMEMODE

#include "Panel.h"

class PanelImportPreferences : public Panel
{
public:

	PanelImportPreferences(char* name);
	~PanelImportPreferences();

	bool Draw();

private:

	void ModelNode() const;
	void TextureNode() const;
};

#endif

#endif // GAME