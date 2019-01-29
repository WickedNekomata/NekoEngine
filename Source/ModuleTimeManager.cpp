#include "ModuleTimeManager.h"

#include "Application.h"

ModuleTimeManager::ModuleTimeManager(bool start_enabled) : Module(start_enabled)
{
	name = "TimeManager";
}

ModuleTimeManager::~ModuleTimeManager() {}

void ModuleTimeManager::PrepareUpdate()
{
	frameCount++;

	// Dt
	realDt = App->GetDt();

	// Time
	realTime += realDt;

	switch (App->GetEngineState())
	{
	case engine_states::ENGINE_PLAY:
	case engine_states::ENGINE_WANTS_PAUSE:
		time += realDt;
		dt = realDt * timeScale;
		break;

	case engine_states::ENGINE_EDITOR:
		time = 0.0f;
		break;

	case engine_states::ENGINE_PAUSE:
		dt = 0.0f;
		break;
	}
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

float ModuleTimeManager::GetRealTime() const
{
	return realTime;
}

float ModuleTimeManager::GetRealDt() const
{
	return realDt;
}

std::list<GameTimer*> ModuleTimeManager::GetGameTimerList() const
{
	return std::list<GameTimer*>();
}
