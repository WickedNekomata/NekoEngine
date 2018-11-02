#ifndef __MODULE_TIME_MANAGER_H__
#define __MODULE_TIME_MANAGER_H__

#include "Module.h"

class ModuleTimeManager : public Module
{
public:

	ModuleTimeManager(bool start_enabled = true);
	~ModuleTimeManager();

	update_status PreUpdate(float dt);
	update_status Update(float dt);
	update_status PostUpdate(float dt);

private:

	int frameCount = 0; // App graphics frames since game start
	float time = 0.0f; // Seconds since game start (Game Clock)
	float timeScale = 0.0f; // Scale at which time is passing (Game Clock)
	float dt = 0.0f; // Last frame time expressed in seconds (Real Time Clock)
	float realTimeSinceStartup = 0.0f; // Seconds since game start (Real Time Clock)
	float realTimeDeltaTime = 0.0f; // Last frame time expressed in seconds (Real Time Clock)
};

#endif