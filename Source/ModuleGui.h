#ifndef __MODULE_GUI_H__
#define __MODULE_GUI_H__

#include "GameMode.h"

#ifndef GAMEMODE

#include "Module.h"

#include "ResourceTexture.h"

#include <vector>

class ModuleGui : public Module
{
public:

	ModuleGui(bool start_enabled = true);
	~ModuleGui();

	bool Init(JSON_Object* jObject);
	bool Start();
	update_status PreUpdate();
	update_status Update();
	update_status PostUpdate();
	bool CleanUp();

	void SaveStatus(JSON_Object*) const;
	void LoadStatus(const JSON_Object*);

	void Draw() const;
	void DockSpace() const;
	void SaveScenePopUp();
	void LoadScenePopUp();

	void ShowAllWindows();
	void HideAllWindows();

	void LogConsole(const char* log) const;
	void ClearConsole() const;

	void AddInput(uint key, uint state) const;

	bool IsMouseHoveringAnyWindow();
	bool IsAnyItemFocused();
	bool WantTextInput();

public:

	class PanelInspector*		panelInspector = nullptr;
	class PanelAbout*			panelAbout = nullptr;
	class PanelConsole*			panelConsole = nullptr;
	class PanelSettings*		panelSettings = nullptr;
	class PanelHierarchy*		panelHierarchy = nullptr;
	class PanelAssets*			panelAssets = nullptr;
	class PanelDebugDraw*		panelDebugDraw = nullptr;
	class PanelEdit*			panelEdit = nullptr;
	class PanelCodeEditor*		panelCodeEditor = nullptr;
	class PanelShaderEditor*	panelShaderEditor = nullptr;
	class PanelSkybox*			panelSkybox = nullptr;
	class PanelNavigation*		panelNavigation = nullptr;
	class PanelSimulatedTime*	panelSimulatedTime = nullptr;
	class PanelLayers*			panelLayers = nullptr;
	class PanelPhysics*			panelPhysics = nullptr;

	ResourceTexture* atlas = nullptr;

private:

	std::vector<class Panel*> panels;

	bool showSaveScenePopUp = false;
	bool showLoadScenePopUp = false;
};

#endif

#endif // GAME