#ifndef __PANEL_INSPECTOR_H__
#define __PANEL_INSPECTOR_H__

#include "GameMode.h"

#ifndef GAMEMODE

#include "Panel.h"

class Component;

class PanelInspector : public Panel
{
public:

	PanelInspector(char* name);
	~PanelInspector();

	bool Draw();

	void ShowGameObjectInspector();
	void DragnDropSeparatorTarget(Component* target);

	void ShowSceneInspector() const;
	void ShowMeshResourceInspector() const;
	void ShowTextureResourceInspector() const;
	void ShowMeshImportSettingsInspector();
	void ShowTextureImportSettingsInspector();
};

#endif

#endif // GAME
