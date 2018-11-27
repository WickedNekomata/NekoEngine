#include "ModuleWrenVM.h"
#include "wren/include/wren.hpp"

#include "Globals.h"

ModuleWrenVM::ModuleWrenVM(bool start_enabled) : Module(start_enabled)
{
}

ModuleWrenVM::~ModuleWrenVM()
{
}

void Write(WrenVM* vm, const char* log)
{
	CONSOLE_LOG(log);
}

bool ModuleWrenVM::Init(JSON_Object* jObject)
{
	WrenConfiguration config;
	wrenInitConfiguration(&config);
	config.writeFn = Write;

	vm = wrenNewVM(&config);
	
	return true;
}

update_status ModuleWrenVM::Update()
{
	//Interpret("System.print(\"Hello, world!\")");

	return UPDATE_CONTINUE;
}

bool ModuleWrenVM::CleanUp()
{
	wrenFreeVM(vm);
	return true;
}

bool ModuleWrenVM::Interpret(const char* code)
{
	WrenInterpretResult result = wrenInterpret(vm, NULL, code);

	if (result == WREN_RESULT_SUCCESS)
		return true;

	return false;
}
