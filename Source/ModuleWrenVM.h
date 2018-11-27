#ifndef __MODULE_WREN_VM_H__
#define __MODULE_WREN_VM_H__

#include "Module.h"

class WrenVM;

class ModuleWrenVM : public Module
{
public:

	ModuleWrenVM(bool start_enabled = true);
	~ModuleWrenVM();

	bool Init(JSON_Object* jObject);
	update_status Update();
	bool CleanUp();

	bool Interpret(const char* code);

private:

	WrenVM* vm;
};

#endif