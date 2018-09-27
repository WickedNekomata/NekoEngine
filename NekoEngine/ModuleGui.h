#ifndef __MODULE_GUI_H__
#define __MODULE_GUI_H__

#include "Module.h"

#include <vector>

struct Panel;
struct PanelInspector;
struct PCGtest;
struct PanelAbout;
struct PanelConsole;
struct PanelPreferences;

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

public:
	PanelInspector* pInspector = nullptr;
	PCGtest* pRandomNumber = nullptr;
	PanelAbout* pAbout = nullptr;
	PanelConsole* pConsole = nullptr;
	PanelPreferences* pPreferences = nullptr;

private:
	std::vector<Panel*> panels;
};

#endif