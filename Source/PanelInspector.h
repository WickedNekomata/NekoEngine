#ifndef __PANEL_INSPECTOR_H__
#define __PANEL_INSPECTOR_H__

#include "GameMode.h"

#include "ResourceMesh.h"
#include "ResourceTexture.h"

#ifndef GAMEMODE

#include "Panel.h"

class Component;

class PanelInspector : public Panel
{
public:

	PanelInspector(const char* name);
	~PanelInspector();

	bool Draw();
	
	void SetMeshImportSettings(ResourceMeshImportSettings& is) { m_is = is; }
	void SetTextureImportSettings(ResourceTextureImportSettings& is) { t_is = is; }

private:

	void ShowGameObjectInspector() const;
	void DragnDropSeparatorTarget(Component* target) const;

	void ShowMeshResourceInspector() const;
	void ShowTextureResourceInspector() const;
	void ShowMeshImportSettingsInspector();
	void ShowTextureImportSettingsInspector() const;
	void ShowShaderObjectInspector() const;
	void ShowShaderProgramInspector() const;
	void ShowMaterialInspector() const;
	void ShowAnimationInspector() const;

	ResourceMeshImportSettings m_is;
	ResourceTextureImportSettings t_is;
};

#endif

#endif // GAME
