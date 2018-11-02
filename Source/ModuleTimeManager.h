#ifndef __MODULE_TIME_MANAGER_H__
#define __MODULE_TIME_MANAGER_H__

#include "Module.h"

#define MAX_TIME_SCALE 2.0f

class ModuleTimeManager : public Module
{
public:

	ModuleTimeManager(bool start_enabled = true);
	~ModuleTimeManager();

	update_status Update(float dt);

	void SetTimeScale(float timeScale);
	float GetTimeScale() const;

	float GetTime() const;
	float GetDt() const;
	int GetFrameCount() const;
	float GetRealTimeSinceStartup() const;
	float GetRealTimeDeltaTime() const;

private:

	float timeScale = 1.0f; // Scale at which time is passing (Game Clock)
	float time = 0.0f; // Seconds since game start (Game Clock)
	float dt = 0.0f; // Last frame time expressed in seconds (Game Clock)
	
	int frameCount = 0; // App graphics frames since game start (Real Time Clock)
	float realTimeSinceStartup = 0.0f; // Seconds since game start (Real Time Clock)
	float realTimeDeltaTime = 0.0f; // Last frame time expressed in seconds (Real Time Clock)
};

#endif