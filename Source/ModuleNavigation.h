#ifndef __MODULE_NAVIGATION_H__
#define __MODULE_NAVIGATION_H__

#include "Module.h"
#include "Globals.h"

class ModuleNavigation : public Module
{
public:

	ModuleNavigation(bool start_enabled = true);
	~ModuleNavigation();

	bool Init(JSON_Object* jObject);
	update_status Update();
	bool CleanUp();

	void HandleDebugDraw();
};

#endif