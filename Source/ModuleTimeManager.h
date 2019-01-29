#ifndef __MODULE_TIME_MANAGER_H__
#define __MODULE_TIME_MANAGER_H__

#include "Module.h"

#include <list>
#define MAX_TIME_SCALE 2.0f

class GameTimer;
class ModuleTimeManager : public Module
{
public:

	ModuleTimeManager(bool start_enabled = true);
	~ModuleTimeManager();

	bool CleanUp();

	void PrepareUpdate();

	void SetTimeScale(float timeScale);
	float GetTimeScale() const;

	float GetTime() const;
	float GetDt() const;

	int GetFrameCount() const;
	float GetRealTime() const;
	float GetRealDt() const;

	std::list<GameTimer*> GetGameTimerList() const;
private:

	// Game Clock
	float timeScale = 1.0f; // Scale at which time is passing
	float time = 0.0f; // Seconds since game start
	float dt = 0.0f; // Last frame time expressed in seconds
	
	// Real Time Clock
	int frameCount = 0; // App graphics frames since game start
	float realTime = 0.0f; // Seconds since game start
	float realDt = 0.0f; // Last frame time expressed in seconds

	std::list<GameTimer*> gameTimerList;
};

#endif