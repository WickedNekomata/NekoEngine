#include "ModuleTimeManager.h"
#include "GameTimer.h"

#include "Application.h"

ModuleTimeManager::ModuleTimeManager(bool start_enabled) : Module(start_enabled)
{
	name = "TimeManager";
}

ModuleTimeManager::~ModuleTimeManager() {}

// Called before quitting
bool ModuleTimeManager::CleanUp()
{
	gameTimerList.clear();
	return true;
}

void ModuleTimeManager::PrepareUpdate()
{
	// Frames
	frameCount++;

	// Dt
	realDt = App->GetDt();
	dt = 0.0f;

	// Time
	realTime += realDt;

	switch (App->GetEngineState())
	{
	case engine_states::ENGINE_PLAY:
		dt = realDt * timeScale;
		time += realDt;
		gameTime += dt;
		break;

	case engine_states::ENGINE_EDITOR:
		time = 0.0f;
		gameTime = 0.0f;
		break;
	}

	for (std::list<GameTimer*>::iterator it = gameTimerList.begin(); it != gameTimerList.end(); ++it)
	{
		if (App->IsEditor())
			(*it)->Update(realDt);
		else
			(*it)->Update(dt);
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

float ModuleTimeManager::GetGameTime() const
{
	return gameTime;
}

//Game Timer List
bool ModuleTimeManager::TimerInGameList(GameTimer* timer)
{
	if (timer != nullptr)
		if (std::find(gameTimerList.begin(), gameTimerList.end(), timer) == gameTimerList.end())
			gameTimerList.push_back(timer);
	return true;
}

bool ModuleTimeManager::RemoveGameTimer(GameTimer* timer)
{
	if(timer != nullptr)
		gameTimerList.remove(timer);
	return true;
}
