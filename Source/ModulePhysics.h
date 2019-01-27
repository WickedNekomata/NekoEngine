#ifndef __MODULE_PHYSICS_H__
#define __MODULE_PHYSICS_H__

#include "Module.h"

#include "physx/include/PxPhysicsAPI.h"
#include "physx/include/extensions/PxDefaultAllocator.h"
#include "physx/include/extensions/PxDefaultErrorCallback.h"
#include "physx/include/common/windows/PxWindowsDelayLoadHook.h"

using namespace physx;

class ModulePhysics : public Module
{
public:

	ModulePhysics(bool start_enabled = true);
	~ModulePhysics();

	bool Init(JSON_Object* jObject);
	bool Start();
	update_status PreUpdate();
	update_status Update();
	update_status PostUpdate();
	bool CleanUp();



private:

	PxFoundation* gFoundation = nullptr;
	PxPhysics* gPhysics = nullptr;
};

#endif