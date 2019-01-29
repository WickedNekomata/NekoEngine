#include "ModulePhysics.h"

#include "Application.h"
#include "ModuleTimeManager.h"

#include <assert.h>

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

#define STEP_SIZE 1.0f / 60.0f

// TODO: Physics Manager like Unity (custom options)

DefaultErrorCallback::DefaultErrorCallback() {}

DefaultErrorCallback::~DefaultErrorCallback() {}

void DefaultErrorCallback::reportError(PxErrorCode::Enum code, const char* message, const char* file, int line)
{
	std::string errorCode;
	switch (code)
	{
	case PxErrorCode::Enum::eNO_ERROR:
		errorCode = "eNO_ERROR";
		break;
	case PxErrorCode::Enum::eDEBUG_INFO: //! \brief An informational message
		errorCode = "eDEBUG_INFO";
		break;
	case PxErrorCode::Enum::eDEBUG_WARNING: //! \brief a warning message for the user to help with debugging
		errorCode = "eDEBUG_WARNING";
		break;
	case PxErrorCode::Enum::eINVALID_PARAMETER: //! \brief method called with invalid parameter(s)
		errorCode = "eINVALID_PARAMETER";
		break;
	case PxErrorCode::Enum::eINVALID_OPERATION: //! \brief method was called at a time when an operation is not possible
		errorCode = "eINVALID_OPERATION";
		break;
	case PxErrorCode::Enum::eOUT_OF_MEMORY: //! \brief method failed to allocate some memory
		errorCode = "eOUT_OF_MEMORY";
		break;
	case PxErrorCode::Enum::eINTERNAL_ERROR: //! \brief The library failed for some reason. Possibly you have passed invalid values like NaNs, which are not checked for
		errorCode = "eINTERNAL_ERROR";
		break;
	case PxErrorCode::Enum::eABORT: //! \brief An unrecoverable error, execution should be halted and log output flushed
		errorCode = "eABORT";
		break;
	case PxErrorCode::Enum::ePERF_WARNING: //! \brief The SDK has determined that an operation may result in poor performance
		errorCode = "ePERF_WARNING";
		break;
	case PxErrorCode::Enum::eMASK_ALL: //! \brief A bit mask for including all errors
		errorCode = "eMASK_ALL";
		break;
	}
	errorCode.append(": ");

	CONSOLE_LOG("%s""%s", errorCode.data(), message);
}

// ----------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------

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
DefaultErrorCallback	gErrorCallback;

bool ModulePhysics::Start()
{
	// Foundation
	gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
	assert(gFoundation != nullptr && "MODULE PHYSICS: PxCreateFoundation failed!");

	// Physics
	bool recordMemoryAllocations = true; // whether to perform memory profiling
	PxTolerancesScale scale;
	scale.length = 100; // typical length of an object
	scale.speed = 981; // typical speed of an object
	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, scale, recordMemoryAllocations);
	assert(gPhysics != nullptr && "MODULE PHYSICS: PxCreatePhysics failed!");

	// Scene
	PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	gDispatcher = PxDefaultCpuDispatcherCreate(1);
	sceneDesc.cpuDispatcher = gDispatcher;
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;
	gScene = gPhysics->createScene(sceneDesc);
	assert(gScene != nullptr && "MODULE PHYSICS: createScene failed!");

	gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);

	// Ground
	PxRigidStatic* groundPlane = PxCreatePlane(*gPhysics, PxPlane(0, 1, 0, 0), *gMaterial);
	gScene->addActor(*groundPlane);

	return true;
}

update_status ModulePhysics::PreUpdate()
{
	return UPDATE_CONTINUE;
}

update_status ModulePhysics::Update()
{
	// Step physics
	gAccumulator += App->timeManager->GetDt();
	if (gAccumulator >= STEP_SIZE)
	{
		gAccumulator = 0.0f;

		gScene->simulate(STEP_SIZE); // moves all objects in the scene forward by STEP_SIZE
		gScene->fetchResults(true); // allows the simulation to finish and return the results
	}

	return UPDATE_CONTINUE;
}

update_status ModulePhysics::PostUpdate()
{
	return UPDATE_CONTINUE;
}

bool ModulePhysics::CleanUp()
{
	gScene->release();
	gScene = nullptr;

	gPhysics->release();
	gPhysics = nullptr;

	gFoundation->release();
	gFoundation = nullptr;

	return true;
}

// ----------------------------------------------------------------------------------------------------

PxRigidStatic* ModulePhysics::CreateRigidStatic(const PxTransform& transform, PxShape& shape) const
{
	PxRigidStatic* rigidStatic = PxCreateStatic(*gPhysics, transform, shape);

	gScene->addActor(*rigidStatic);

	return rigidStatic;
}

PxRigidDynamic* ModulePhysics::CreateRigidDynamic(const PxTransform& transform, PxShape& shape, float density, bool isKinematic) const
{
	PxRigidDynamic* rigidDynamic = nullptr;
	if (isKinematic)
		rigidDynamic = PxCreateKinematic(*gPhysics, transform, shape, density);
	else
		rigidDynamic = PxCreateDynamic(*gPhysics, transform, shape, density); // gPhysics->createRigidDynamic(transform);

	gScene->addActor(*rigidDynamic);

	return rigidDynamic;
}

PxShape* ModulePhysics::CreateShape(const PxGeometry& geometry, const PxMaterial& material, bool isExclusive) const
{
	return gPhysics->createShape(geometry, material, isExclusive);
}

void ModulePhysics::RemoveActor(PxActor& actor) const
{
	gScene->removeActor(actor);
}

PxMaterial* ModulePhysics::GetDefaultMaterial() const
{
	return gMaterial;
}

std::vector<PxRigidActor*> ModulePhysics::GetRigidStatics() const
{
	uint nbStaticActors = gScene->getNbActors(PxActorTypeFlag::Enum::eRIGID_STATIC);
	std::vector<PxRigidActor*> staticActors(nbStaticActors);
	
	if (nbStaticActors > 0)
		gScene->getActors(PxActorTypeFlag::Enum::eRIGID_STATIC, reinterpret_cast<PxActor**>(&staticActors[0]), nbStaticActors);

	return staticActors;
}

std::vector<PxRigidActor*> ModulePhysics::GetRigidDynamics() const
{
	uint nbDynamicActors = gScene->getNbActors(PxActorTypeFlag::Enum::eRIGID_DYNAMIC);
	std::vector<PxRigidActor*> dynamicActors(nbDynamicActors);
	
	if (nbDynamicActors > 0)
		gScene->getActors(PxActorTypeFlag::Enum::eRIGID_DYNAMIC, reinterpret_cast<PxActor**>(&dynamicActors[0]), nbDynamicActors);

	return dynamicActors;
}