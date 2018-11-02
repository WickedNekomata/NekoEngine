#include "ModuleTimeManager.h"

#include "Application.h"

ModuleTimeManager::ModuleTimeManager(bool start_enabled) : Module(start_enabled)
{
	name = "TimeManager";
}

ModuleTimeManager::~ModuleTimeManager() {}

update_status ModuleTimeManager::Update(float dt)
{
	frameCount++;
	realTimeSinceStartup += dt * MSTOSECONDS;
	realTimeDeltaTime = dt * MSTOSECONDS;
	this->dt = dt * timeScale * MSTOSECONDS;

	switch (App->GetEngineState())
	{
	case engine_states::ENGINE_EDITOR:

		time = 0.0f;

		break;

	case engine_states::ENGINE_PLAY:

		time += dt * MSTOSECONDS;

		break;

	case engine_states::ENGINE_PAUSE:
	default:
		break;
	}

	return UPDATE_CONTINUE;
}

void ModuleTimeManager::SetTimeScale(float timeScale)
{
	this->timeScale = timeScale;

	if (this->timeScale > MAX_TIME_SCALE)
		this->timeScale = MAX_TIME_SCALE;
}

float ModuleTimeManager::GetTimeScale() const
{
	return timeScale;
}

float ModuleTimeManager::GetTime() const
{
	return time;
}

float ModuleTimeManager::GetDt() const
{
	return dt;
}

int ModuleTimeManager::GetFrameCount() const
{
	return frameCount;
}

float ModuleTimeManager::GetRealTimeSinceStartup() const
{
	return realTimeSinceStartup;
}

float ModuleTimeManager::GetRealTimeDeltaTime() const
{
	return realTimeDeltaTime;
}