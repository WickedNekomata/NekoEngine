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
	update_status PreUpdate(float dt);
	update_status Update(float dt);
	update_status PostUpdate(float dt);
	bool CleanUp();

	void Draw() const;
	void DockSpace() const;
	void SaveScenePopUp();
	void LoadScenePopUp();

	void SaveStatus(JSON_Object*) const;
	void LoadStatus(const JSON_Object*);

	void LogConsole(const char* log) const;
	void AddInput(uint key, uint state) const;

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