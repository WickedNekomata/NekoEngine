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

private:

	void ShowGameObjectInspector() const;
	void DragnDropSeparatorTarget(Component* target) const;

	void ShowSceneInspector() const;
	void ShowMeshResourceInspector() const;
	void ShowTextureResourceInspector() const;
	void ShowMeshImportSettingsInspector() const;
	void ShowTextureImportSettingsInspector() const;
};

#endif

#endif // GAME
