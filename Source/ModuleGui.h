#ifndef __MODULE_GUI_H__
#define __MODULE_GUI_H__

#include "Module.h"

#include <vector>

struct Panel;
struct PanelInspector;
struct PanelTestPCG;
struct PanelAbout;
struct PanelConsole;
struct PanelSettings;

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

	void SaveStatus(JSON_Object*);
	void LoadStatus(JSON_Object*);

	void LogConsole(const char* log) const;

public:

	PanelInspector* panelInspector = nullptr;
	PanelTestPCG* panelRandomNumber = nullptr;
	PanelAbout* panelAbout = nullptr;
	PanelConsole* panelConsole = nullptr;
	PanelSettings* panelSettings = nullptr;

private:

	std::vector<Panel*> panels;
};

#endif