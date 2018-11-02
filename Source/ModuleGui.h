#ifndef __MODULE_GUI_H__
#define __MODULE_GUI_H__

#include "Module.h"

#include <vector>

struct Panel;
struct PanelInspector;
struct PanelAbout;
struct PanelConsole;
struct PanelSettings;
struct PanelHierarchy;
struct PanelAssets;

struct Texture;

class ModuleGui : public Module
{
public:

	ModuleGui(bool start_enabled = true);
	~ModuleGui();

	bool Init(JSON_Object* jObject);
	bool Start();
	update_status PreUpdate();
	update_status Update();
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

	PanelInspector* panelInspector = nullptr;
	PanelAbout* panelAbout = nullptr;
	PanelConsole* panelConsole = nullptr;
	PanelSettings* panelSettings = nullptr;
	PanelHierarchy* panelHierarchy = nullptr;
	PanelAssets* panelAssets = nullptr;

private:

	std::vector<Panel*> panels;

	bool showSaveScenePopUp = false;
	bool showLoadScenePopUp = false;

	Texture* timeButtonTex = nullptr;
};

#endif