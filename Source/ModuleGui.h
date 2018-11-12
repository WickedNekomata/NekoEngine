#ifndef __MODULE_GUI_H__
#define __MODULE_GUI_H__

#include "GameMode.h"

#ifndef GAMEMODE

#include "Module.h"

#include "ResourceTexture.h"

#include <vector>

class Panel;
class PanelInspector;
class PanelAbout;
class PanelConsole;
class PanelSettings;
class PanelHierarchy;
class PanelAssets;
class PanelResources;
class PanelDebugDraw;
class PanelEdit;

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
	void AddInput(uint key, uint state) const;

	bool IsMouseHoveringAnyWindow();
	bool IsAnyItemFocused();

public:

	PanelInspector*		panelInspector = nullptr;
	PanelAbout*			panelAbout = nullptr;
	PanelConsole*		panelConsole = nullptr;
	PanelSettings*		panelSettings = nullptr;
	PanelHierarchy*		panelHierarchy = nullptr;
	PanelAssets*		panelAssets = nullptr;
	PanelResources*		panelResources = nullptr;
	PanelDebugDraw*		panelDebugDraw = nullptr;
	PanelEdit*			panelEdit = nullptr;

	ResourceTexture* atlas = nullptr;
	uint atlaas = 0;
private:

	std::vector<Panel*> panels;

	bool showSaveScenePopUp = false;
	bool showLoadScenePopUp = false;

	
};

#endif

#endif // GAME