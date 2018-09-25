#ifndef __MODULE_GUI_H__
#define __MODULE_GUI_H__

#include "Module.h"

#include <vector>

struct Panel;
struct PanelInspector;
struct PanelRandomNumber;
struct PanelAbout;
struct PanelConsole;
struct PanelPreferences;

class ModuleGui : public Module
{
public:

	ModuleGui(Application* app, bool start_enabled = true);
	~ModuleGui();

	bool Start();
	update_status PreUpdate(float dt);
	update_status Update(float dt);
	update_status PostUpdate(float dt);
	bool CleanUp();

public:
	PanelInspector* pInspector = nullptr;
	PanelRandomNumber* pRandomNumber = nullptr;
	PanelAbout* pAbout = nullptr;
	PanelConsole* pConsole = nullptr;
	PanelPreferences* pPreferences = nullptr;

private:
	std::vector<Panel*> panels;
};

#endif