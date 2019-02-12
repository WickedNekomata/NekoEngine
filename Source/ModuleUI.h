#ifndef __MODULE_UI_H__
#define __MODULE_UI_H__

#include "Module.h"

class ModuleUI : public Module
{
public:
	ModuleUI(bool start_enabled = true);
	~ModuleUI();

private:

	bool Init(JSON_Object* jObject);
	bool Start();
	update_status PreUpdate();
	update_status Update();
	update_status FixedUpdate();
	update_status PostUpdate();
	bool CleanUp();

	void OnSystemEvent(System_Event event);

};

#endif