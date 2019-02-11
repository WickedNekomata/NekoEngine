#ifndef __MODULE_EVENTS_H__
#define __MODULE_EVENTS_H__

#include "Module.h"

class ModuleEvents : public Module
{
public:
	ModuleEvents(bool start_enabled = true) : Module(start_enabled) { name = "ModuleEvents"; }

	void OnSystemEvent(System_Event event);
};

#endif