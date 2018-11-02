#include "ModuleTimeManager.h"

#include "Application.h"

ModuleTimeManager::ModuleTimeManager(bool start_enabled) : Module(start_enabled)
{
	name = "TimeManager";
}

ModuleTimeManager::~ModuleTimeManager() {}

update_status ModuleTimeManager::PreUpdate(float dt)
{
	return UPDATE_CONTINUE;
}

update_status ModuleTimeManager::Update(float dt)
{
	frameCount++;
	realTimeSinceStartup += dt * MSTOSECONDS;

	switch (App->GetEngineState())
	{
	case engine_states::ENGINE_EDITOR:

		break;

	case engine_states::ENGINE_PAUSE:

		break;

	case engine_states::ENGINE_PLAY:

		time++;

		break;

	default:
		break;
	}

	return UPDATE_CONTINUE;
}

update_status ModuleTimeManager::PostUpdate(float dt)
{
	return UPDATE_CONTINUE;
}