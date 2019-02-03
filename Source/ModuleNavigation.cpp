#include "ModuleNavigation.h"

ModuleNavigation::ModuleNavigation(bool start_enabled)
{
}

ModuleNavigation::~ModuleNavigation()
{
}

bool ModuleNavigation::Init(JSON_Object* jObject)
{
	return true;
}

update_status ModuleNavigation::Update()
{
	return update_status();
}

bool ModuleNavigation::CleanUp()
{
	return true;
}

void ModuleNavigation::HandleDebugDraw()
{
}
