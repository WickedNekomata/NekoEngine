#include "ModulePhysics.h"

#include "Application.h"
#include "ModuleTimeManager.h"

#include "ComponentCollider.h"
#include "ComponentBoxCollider.h"
#include "ComponentSphereCollider.h"
#include "ComponentCapsuleCollider.h"
#include "ComponentPlaneCollider.h"
#include "ComponentRigidActor.h"
#include "ComponentRigidStatic.h"
#include "ComponentRigidDynamic.h"

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

DefaultErrorCallback::DefaultErrorCallback() {}

DefaultErrorCallback::~DefaultErrorCallback() {}

void DefaultErrorCallback::reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line)
{
	std::string errorCode;
	switch (code)
	{
	case physx::PxErrorCode::Enum::eNO_ERROR:
		errorCode = "eNO_ERROR";
		break;
	case physx::PxErrorCode::Enum::eDEBUG_INFO: //! \brief An informational message
		errorCode = "eDEBUG_INFO";
		break;
	case physx::PxErrorCode::Enum::eDEBUG_WARNING: //! \brief a warning message for the user to help with debugging
		errorCode = "eDEBUG_WARNING";
		break;
	case physx::PxErrorCode::Enum::eINVALID_PARAMETER: //! \brief method called with invalid parameter(s)
		errorCode = "eINVALID_PARAMETER";
		break;
	case physx::PxErrorCode::Enum::eINVALID_OPERATION: //! \brief method was called at a time when an operation is not possible
		errorCode = "eINVALID_OPERATION";
		break;
	case physx::PxErrorCode::Enum::eOUT_OF_MEMORY: //! \brief method failed to allocate some memory
		errorCode = "eOUT_OF_MEMORY";
		break;
	case physx::PxErrorCode::Enum::eINTERNAL_ERROR: //! \brief The library failed for some reason. Possibly you have passed invalid values like NaNs, which are not checked for
		errorCode = "eINTERNAL_ERROR";
		break;
	case physx::PxErrorCode::Enum::eABORT: //! \brief An unrecoverable error, execution should be halted and log output flushed
		errorCode = "eABORT";
		break;
	case physx::PxErrorCode::Enum::ePERF_WARNING: //! \brief The SDK has determined that an operation may result in poor performance
		errorCode = "ePERF_WARNING";
		break;
	case physx::PxErrorCode::Enum::eMASK_ALL: //! \brief A bit mask for including all errors
		errorCode = "eMASK_ALL";
		break;
	}
	errorCode.append(": ");

	CONSOLE_LOG("%s""%s", errorCode.data(), message);
}

// ----------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------

SimulationEventCallback::SimulationEventCallback(ModulePhysics* callback) : callback(callback) 
{
	assert(callback != nullptr);
}

SimulationEventCallback::~SimulationEventCallback() {}

void SimulationEventCallback::onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs)
{
	for (physx::PxU32 i = 0; i < nbPairs; ++i)
	{
		const physx::PxContactPair& cp = pairs[i];

		if (cp.events
			& physx::PxPairFlag::eCONTACT_DEFAULT
			& physx::PxPairFlag::eNOTIFY_TOUCH_FOUND
			& physx::PxPairFlag::eSOLVE_CONTACT
			& physx::PxPairFlag::eDETECT_DISCRETE_CONTACT
			& physx::PxPairFlag::eNOTIFY_CONTACT_POINTS)
		{
			// TODO
		}
	}
}

void SimulationEventCallback::onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count)
{

}

void SimulationEventCallback::onWake(physx::PxActor** actors, physx::PxU32 count)
{
	for (physx::PxActor** actor = actors; *actor != nullptr; ++actor)
		callback->OnSimulationEvent(*actor, *actor, SimulationEventTypes::SimulationEventOnWake);
}

void SimulationEventCallback::onSleep(physx::PxActor** actors, physx::PxU32 count)
{
	for (physx::PxActor** actor = actors; *actor != nullptr; ++actor)
		callback->OnSimulationEvent(*actor, *actor, SimulationEventTypes::SimulationEventOnSleep);
}

// ----------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------

ModulePhysics::ModulePhysics(bool start_enabled) : Module(start_enabled)
{
	name = "Physics";
}

ModulePhysics::~ModulePhysics() {}

#define DEFAULT_GRAVITY_X 0.0f
#define DEFAULT_GRAVITY_Y -9.81f
#define DEFAULT_GRAVITY_Z 0.0f

#define DEFAULT_MATERIAL_STATIC_FRICTION 0.5f
#define DEFAULT_MATERIAL_DYNAMIC_FRICTION 0.5f
#define DEFAULT_MATERIAL_RESTITUTION 0.6f

bool ModulePhysics::Init(JSON_Object* jObject)
{
	gravity = math::float3(DEFAULT_GRAVITY_X, DEFAULT_GRAVITY_Y, DEFAULT_GRAVITY_Z);

	return true;
}

physx::PxDefaultAllocator	gAllocator;
DefaultErrorCallback		gErrorCallback;

bool ModulePhysics::Start()
{
	// Foundation
	gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
	assert(gFoundation != nullptr && "MODULE PHYSICS: PxCreateFoundation failed!");

	// Physics
	bool recordMemoryAllocations = true; // whether to perform memory profiling
	//physx::PxTolerancesScale scale;
	//scale.length = 100.0f; // typical length of an object
	//scale.speed = 981.0f; // typical speed of an object
	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, physx::PxTolerancesScale(), recordMemoryAllocations);
	assert(gPhysics != nullptr && "MODULE PHYSICS: PxCreatePhysics failed!");

	// Scene
	physx::PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
	sceneDesc.gravity = physx::PxVec3(gravity.x, gravity.y, gravity.z);
	gDispatcher = physx::PxDefaultCpuDispatcherCreate(1);
	sceneDesc.cpuDispatcher = gDispatcher;
	sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
	gScene = gPhysics->createScene(sceneDesc);
	assert(gScene != nullptr && "MODULE PHYSICS: createScene failed!");
	gScene->setSimulationEventCallback(new SimulationEventCallback(this));

	// Default material
	defaultMaterial = gPhysics->createMaterial(DEFAULT_MATERIAL_STATIC_FRICTION, DEFAULT_MATERIAL_DYNAMIC_FRICTION, DEFAULT_MATERIAL_RESTITUTION);

	// Ground
	physx::PxRigidStatic* groundPlane = PxCreatePlane(*gPhysics, physx::PxPlane(0.0f, 1.0f, 0.0f, 0.0f), *defaultMaterial);
	gScene->addActor(*groundPlane);
	CONSOLE_LOG("gScene actors: %i", gScene->getNbActors(physx::PxActorTypeFlag::Enum::eRIGID_STATIC | physx::PxActorTypeFlag::Enum::eRIGID_DYNAMIC));

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
	for (std::vector<ComponentRigidActor*>::const_iterator it = rigidActorComponents.begin(); it != rigidActorComponents.end(); ++it)
	{
		if ((*it)->GetType() == ComponentTypes::RigidDynamicComponent && !(*it)->GetActor()->is<physx::PxRigidDynamic>()->isSleeping())
			(*it)->UpdateGameObjectTransform();
	}

	return UPDATE_CONTINUE;
}

bool ModulePhysics::CleanUp()
{
	colliderComponents.clear();

	gScene->release();
	gScene = nullptr;

	gPhysics->release();
	gPhysics = nullptr;

	gFoundation->release();
	gFoundation = nullptr;

	return true;
}

// ----------------------------------------------------------------------------------------------------

physx::PxRigidStatic* ModulePhysics::CreateRigidStatic(const physx::PxTransform& transform, physx::PxShape& shape) const
{
	physx::PxRigidStatic* rigidStatic = physx::PxCreateStatic(*gPhysics, transform, shape);

	gScene->addActor(*rigidStatic);
	CONSOLE_LOG("gScene actors: %i", gScene->getNbActors(physx::PxActorTypeFlag::Enum::eRIGID_STATIC | physx::PxActorTypeFlag::Enum::eRIGID_DYNAMIC));

	return rigidStatic;
}

physx::PxRigidDynamic* ModulePhysics::CreateRigidDynamic(const physx::PxTransform& transform, physx::PxShape& shape, float density, bool isKinematic) const
{
	physx::PxRigidDynamic* rigidDynamic = nullptr;
	if (isKinematic)
		rigidDynamic = physx::PxCreateKinematic(*gPhysics, transform, shape, density);
	else
		rigidDynamic = physx::PxCreateDynamic(*gPhysics, transform, shape, density);

	gScene->addActor(*rigidDynamic);
	CONSOLE_LOG("gScene actors: %i", gScene->getNbActors(physx::PxActorTypeFlag::Enum::eRIGID_STATIC | physx::PxActorTypeFlag::Enum::eRIGID_DYNAMIC));

	return rigidDynamic;
}

void ModulePhysics::RemoveActor(physx::PxActor& actor) const
{
	gScene->removeActor(actor);
}

physx::PxShape* ModulePhysics::CreateShape(const physx::PxGeometry& geometry, const physx::PxMaterial& material, bool isExclusive) const
{
	return gPhysics->createShape(geometry, material, isExclusive);
}

// ----------------------------------------------------------------------------------------------------

ComponentRigidActor* ModulePhysics::CreateRigidActorComponent(GameObject* parent, ComponentTypes componentRigidActorType)
{
	ComponentRigidActor* newComponent = nullptr;
	switch (componentRigidActorType)
	{
	case ComponentTypes::RigidStaticComponent:
		newComponent = new ComponentRigidStatic(parent);
		break;
	case ComponentTypes::RigidDynamicComponent:
		newComponent = new ComponentRigidDynamic(parent);
		break;
	}
	assert(newComponent != nullptr);

	std::vector<ComponentRigidActor*>::const_iterator it = std::find(rigidActorComponents.begin(), rigidActorComponents.end(), newComponent);
	assert(it == rigidActorComponents.end());

	rigidActorComponents.push_back(newComponent);

	return newComponent;
}

bool ModulePhysics::AddRigidActorComponent(ComponentRigidActor* toAdd)
{
	bool ret = true;

	std::vector<ComponentRigidActor*>::const_iterator it = std::find(rigidActorComponents.begin(), rigidActorComponents.end(), toAdd);
	ret = it == rigidActorComponents.end();

	if (ret)
		rigidActorComponents.push_back(toAdd);

	return ret;
}

bool ModulePhysics::EraseRigidActorComponent(ComponentRigidActor* toErase)
{
	bool ret = false;

	std::vector<ComponentRigidActor*>::const_iterator it = std::find(rigidActorComponents.begin(), rigidActorComponents.end(), toErase);
	ret = it != rigidActorComponents.end();

	if (ret)
		rigidActorComponents.erase(it);

	return ret;
}

ComponentCollider* ModulePhysics::CreateColliderComponent(GameObject* parent, ComponentTypes componentColliderType)
{
	ComponentCollider* newComponent = nullptr;
	switch (componentColliderType)
	{
	case ComponentTypes::BoxColliderComponent:
		newComponent = new ComponentBoxCollider(parent);
		break;
	case ComponentTypes::SphereColliderComponent:
		newComponent = new ComponentSphereCollider(parent);
		break;
	case ComponentTypes::CapsuleColliderComponent:
		newComponent = new ComponentCapsuleCollider(parent);
		break;
	case ComponentTypes::PlaneColliderComponent:
		newComponent = new ComponentPlaneCollider(parent);
		break;
	}
	assert(newComponent != nullptr);

	std::vector<ComponentCollider*>::const_iterator it = std::find(colliderComponents.begin(), colliderComponents.end(), newComponent);
	assert(it == colliderComponents.end());

	colliderComponents.push_back(newComponent);

	return newComponent;
}

bool ModulePhysics::AddColliderComponent(ComponentCollider* toAdd)
{
	bool ret = true;

	std::vector<ComponentCollider*>::const_iterator it = std::find(colliderComponents.begin(), colliderComponents.end(), toAdd);
	ret = it == colliderComponents.end();

	if (ret)
		colliderComponents.push_back(toAdd);

	return ret;
}

bool ModulePhysics::EraseColliderComponent(ComponentCollider* toErase)
{
	bool ret = false;

	std::vector<ComponentCollider*>::const_iterator it = std::find(colliderComponents.begin(), colliderComponents.end(), toErase);
	ret = it != colliderComponents.end();

	if (ret)
		colliderComponents.erase(it);

	return ret;
}

// ----------------------------------------------------------------------------------------------------

physx::PxFilterFlags FilterShader(
	physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0,
	physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1,
	physx::PxPairFlags& pairFlags, const void* constantBlock, physx::PxU32 constantBlockSize)
{
	if ((filterData0.word0 & filterData1.word1) && (filterData1.word0 & filterData0.word1))
	{
		// Let triggers through
		if (physx::PxFilterObjectIsTrigger(attributes0) || physx::PxFilterObjectIsTrigger(attributes1))
			pairFlags = physx::PxPairFlag::eTRIGGER_DEFAULT;
		else
		{
			// Generate contacts for all that were not filtered above
			pairFlags = physx::PxPairFlag::eCONTACT_DEFAULT;
			pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_FOUND;
			pairFlags |= physx::PxPairFlag::eSOLVE_CONTACT;
			pairFlags |= physx::PxPairFlag::eDETECT_DISCRETE_CONTACT;
			pairFlags |= physx::PxPairFlag::eNOTIFY_CONTACT_POINTS;
		}

		return physx::PxFilterFlag::eDEFAULT;
	}
	else
		return physx::PxFilterFlag::eKILL;
}

void ModulePhysics::OnSimulationEvent(physx::PxActor* actorA, physx::PxActor* actorB, SimulationEventTypes simulationEventType) const
{
	ComponentRigidActor* componentRigidActorA = GetRigidActorComponentFromActor(actorA);
	ComponentRigidActor* componentRigidActorB = GetRigidActorComponentFromActor(actorB);

	switch (simulationEventType)
	{
	case SimulationEventTypes::SimulationEventOnWake:
		if (componentRigidActorA != nullptr)
			componentRigidActorA->OnWake();
		break;
	case SimulationEventTypes::SimulationEventOnSleep:
		if (componentRigidActorA != nullptr)
			componentRigidActorA->OnSleep();
		break;
	case SimulationEventTypes::SimulationEventOnContact:
		break;
	case SimulationEventTypes::SimulationEventOnTrigger:
		break;
	}
}

// ----------------------------------------------------------------------------------------------------

std::vector<ComponentRigidActor*> ModulePhysics::GetRigidActorComponents() const
{
	return rigidActorComponents;
}

ComponentRigidActor* ModulePhysics::GetRigidActorComponentFromActor(physx::PxActor* actor) const
{
	if (actor == nullptr)
		return nullptr;

	for (std::vector<ComponentRigidActor*>::const_iterator it = rigidActorComponents.begin(); it != rigidActorComponents.end(); ++it)
	{
		if ((*it)->GetActor() == actor)
			return *it;
	}

	return nullptr;
}

std::vector<ComponentCollider*> ModulePhysics::GetColliderComponents() const
{
	return colliderComponents;
}

// ----------------------------------------------------------------------------------------------------

void ModulePhysics::SetGravity(math::float3 gravity)
{
	this->gravity = gravity;

	gScene->setGravity(physx::PxVec3(gravity.x, gravity.y, gravity.z));
}

math::float3 ModulePhysics::GetGravity() const
{
	return gravity;
}

void ModulePhysics::SetDefaultMaterial(physx::PxMaterial* material)
{
	defaultMaterial = material;
}

physx::PxMaterial* ModulePhysics::GetDefaultMaterial() const
{
	return defaultMaterial;
}