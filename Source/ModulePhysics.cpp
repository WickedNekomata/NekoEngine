#include "ModulePhysics.h"

#include "Application.h"

#ifdef _DEBUG
	#pragma comment(lib, "physx/libx86/debugx86/PhysX_32.lib")
	#pragma comment(lib, "physx/libx86/debugx86/PhysXCommon_32.lib")
	#pragma comment(lib, "physx/libx86/debugx86/PhysXFoundation_32.lib")
	#pragma comment(lib, "physx/libx86/debugx86/PhysXExtensions_static_32.lib")
#endif

#ifdef NDEBUG
	#pragma comment(lib, "physx/libx86/releasex86/PhysX_32.lib")
	#pragma comment(lib, "physx/libx86/releasex86/PhysXCommon_32.lib")
	#pragma comment(lib, "physx/libx86/releasex86/PhysXFoundation_32.lib")
	#pragma comment(lib, "physx/libx86/releasex86/PhysXExtensions_static_32.lib")
#endif

ModulePhysics::ModulePhysics(bool start_enabled) : Module(start_enabled)
{
	name = "Physics";
}

ModulePhysics::~ModulePhysics() {}

bool ModulePhysics::Init(JSON_Object* jObject)
{
	return true;
}

PxDefaultAllocator		gAllocator;
PxDefaultErrorCallback	gErrorCallback;

bool ModulePhysics::Start()
{
	// Foundation
	gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
	if (gFoundation == nullptr)
		CONSOLE_LOG("MODULE PHYSICS: PxCreateFoundation failed!");

	// Physics
	bool recordMemoryAllocations = true; // whether to perform memory profiling
	PxTolerancesScale scale;
	scale.length = 100; // typical length of an object
	scale.speed = 981; // typical speed of an object
	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, scale, recordMemoryAllocations);
	if (gPhysics == nullptr)
		CONSOLE_LOG("MODULE PHYSICS: PxCreatePhysics failed!");

	return true;
}

update_status ModulePhysics::PreUpdate()
{
	return UPDATE_CONTINUE;
}

update_status ModulePhysics::Update()
{
	return UPDATE_CONTINUE;
}

update_status ModulePhysics::PostUpdate()
{
	return UPDATE_CONTINUE;
}

bool ModulePhysics::CleanUp()
{
	gPhysics->release();
	gFoundation->release();

	return true;
}