#include "ModulePhysics.h"

#include "Application.h"
#include "ModuleTimeManager.h"
#include "GameObject.h"
#include "Layers.h"

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

ContactPoint::ContactPoint() {}

ContactPoint::ContactPoint(math::float3 point, math::float3 normal, float separation) :point(point), normal(normal), separation(separation) {}

ContactPoint::~ContactPoint() {}

math::float3 ContactPoint::GetPoint() const
{
	return point;
}

math::float3 ContactPoint::GetNormal() const
{
	return normal;
}

float ContactPoint::GetSeparation() const
{
	return separation;
}

// ----------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------

Collision::Collision() {}

Collision::Collision(GameObject* gameObject, ComponentCollider* collider, ComponentRigidActor* actor, math::float3 impulse, std::vector<ContactPoint> contactPoints) :
	gameObject(gameObject), collider(collider), actor(actor), impulse(impulse), contactPoints(contactPoints) {}

Collision::~Collision() {}

GameObject* Collision::GetGameObject() const
{
	return gameObject;
}

ComponentCollider* Collision::GetCollider() const
{
	return collider;
}

ComponentRigidActor* Collision::GetActor() const
{
	return actor;
}

math::float3 Collision::GetImpulse() const
{
	return impulse;
}

std::vector<ContactPoint> Collision::GetContactPoints() const
{
	return contactPoints;
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
		const physx::PxContactPair& contactPair = pairs[i];

		if (contactPair.flags
			& (physx::PxContactPairFlag::eREMOVED_SHAPE_0
				| physx::PxContactPairFlag::eREMOVED_SHAPE_1))
			continue;
		else
		{
			math::float3 totalImpulse = math::float3::zero;
			std::vector<ContactPoint> contactPoints;

			if (contactPair.contactCount > 0)
			{
				std::vector<physx::PxContactPairPoint> contactPairPoints;
				contactPairPoints.resize(contactPair.contactCount);
				contactPair.extractContacts(&contactPairPoints[0], contactPair.contactCount);

				for (physx::PxU32 j = 0; j < contactPair.contactCount; ++j)
				{
					math::float3 point = math::float3(contactPairPoints[i].position.x, contactPairPoints[i].position.y, contactPairPoints[i].position.z);
					math::float3 normal = math::float3(contactPairPoints[i].normal.x, contactPairPoints[i].normal.y, contactPairPoints[i].normal.z);
					math::float3 impulse = math::float3(contactPairPoints[i].impulse.x, contactPairPoints[i].impulse.y, contactPairPoints[i].impulse.z);
					totalImpulse += impulse;
					
					ContactPoint contactPoint(point, normal, contactPairPoints[i].separation);
					contactPoints.push_back(contactPoint);
				}
			}

			// Collision A
			ComponentCollider* colliderA = callback->FindColliderComponentByShape(contactPair.shapes[1]);
			ComponentRigidActor* actorA = callback->FindRigidActorComponentByActor(pairHeader.actors[1]);
			GameObject* gameObjectA = actorA->GetParent();
			Collision collisionA(gameObjectA, colliderA, actorA, totalImpulse, contactPoints);
			ComponentCollider* thisColliderA = callback->FindColliderComponentByShape(contactPair.shapes[0]);

			// Collision B
			ComponentCollider* colliderB = callback->FindColliderComponentByShape(contactPair.shapes[0]);
			ComponentRigidActor* actorB = callback->FindRigidActorComponentByActor(pairHeader.actors[0]);
			GameObject* gameObjectB = actorB->GetParent();
			Collision collisionB(gameObjectB, colliderB, actorB, totalImpulse, contactPoints);
			ComponentCollider* thisColliderB = callback->FindColliderComponentByShape(contactPair.shapes[1]);

			if (contactPair.events & physx::PxPairFlag::eNOTIFY_TOUCH_FOUND)
			{			
				callback->OnCollision(thisColliderA, collisionA, CollisionTypes::OnCollisionEnter);
				callback->OnCollision(thisColliderB, collisionB, CollisionTypes::OnCollisionEnter);
			}
			else if (contactPair.events & physx::PxPairFlag::eNOTIFY_TOUCH_PERSISTS)
			{
				callback->OnCollision(thisColliderA, collisionA, CollisionTypes::OnCollisionStay);
				callback->OnCollision(thisColliderB, collisionB, CollisionTypes::OnCollisionStay);
			}
			else if (contactPair.events & physx::PxPairFlag::eNOTIFY_TOUCH_LOST)
			{
				callback->OnCollision(thisColliderA, collisionA, CollisionTypes::OnCollisionExit);
				callback->OnCollision(thisColliderB, collisionB, CollisionTypes::OnCollisionExit);
			}
		}
	}
}

void SimulationEventCallback::onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count)
{
	for (physx::PxU32 i = 0; i < count; ++i)
	{
		const physx::PxTriggerPair& triggerPair = pairs[i];

		if (triggerPair.flags &
			(physx::PxTriggerPairFlag::eREMOVED_SHAPE_TRIGGER
				| physx::PxTriggerPairFlag::eREMOVED_SHAPE_OTHER))
			continue;
		else
		{
			// Collision A
			ComponentCollider* colliderA = callback->FindColliderComponentByShape(triggerPair.triggerShape);
			ComponentRigidActor* actorA = callback->FindRigidActorComponentByActor(triggerPair.triggerActor);
			GameObject* gameObjectA = actorA->GetParent();
			Collision collisionA(gameObjectA, colliderA, actorA, math::float3::zero, std::vector<ContactPoint>());
			ComponentCollider* thisColliderA = callback->FindColliderComponentByShape(triggerPair.otherShape);

			// Collision B
			ComponentCollider* colliderB = callback->FindColliderComponentByShape(triggerPair.otherShape);
			ComponentRigidActor* actorB = callback->FindRigidActorComponentByActor(triggerPair.otherActor);
			GameObject* gameObjectB = actorB->GetParent();
			Collision collisionB(gameObjectB, colliderB, actorB, math::float3::zero, std::vector<ContactPoint>());
			ComponentCollider* thisColliderB = callback->FindColliderComponentByShape(triggerPair.triggerShape);

			if (triggerPair.status & physx::PxPairFlag::eNOTIFY_TOUCH_FOUND)
			{
				callback->OnCollision(thisColliderA, collisionA, CollisionTypes::OnTriggerEnter);
				callback->OnCollision(thisColliderB, collisionB, CollisionTypes::OnTriggerEnter);
			}
			else if (triggerPair.status & physx::PxPairFlag::eNOTIFY_TOUCH_LOST)
			{
				callback->OnCollision(thisColliderA, collisionA, CollisionTypes::OnTriggerExit);
				callback->OnCollision(thisColliderB, collisionB, CollisionTypes::OnTriggerExit);
			}
		}
	}
}

void SimulationEventCallback::onWake(physx::PxActor** actors, physx::PxU32 count)
{
	for (physx::PxActor** actor = actors; *actor != nullptr; ++actor)
		callback->OnSimulationEvent(callback->FindRigidActorComponentByActor(*actor), SimulationEventTypes::OnWake);
}

void SimulationEventCallback::onSleep(physx::PxActor** actors, physx::PxU32 count)
{
	for (physx::PxActor** actor = actors; *actor != nullptr; ++actor)
		callback->OnSimulationEvent(callback->FindRigidActorComponentByActor(*actor), SimulationEventTypes::OnSleep);
}

// ----------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------

physx::PxFilterFlags FilterShader(
	physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0,
	physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1,
	physx::PxPairFlags& pairFlags, const void* constantBlock, physx::PxU32 constantBlockSize)
{
	if ((filterData0.word0 != 0 || filterData1.word0 != 0) &&
		!(filterData0.word0 & filterData1.word1 || filterData1.word0 & filterData0.word1))
		return physx::PxFilterFlag::eSUPPRESS;

	// Let triggers through
	if (physx::PxFilterObjectIsTrigger(attributes0) || physx::PxFilterObjectIsTrigger(attributes1))
		pairFlags = physx::PxPairFlag::eTRIGGER_DEFAULT;
	else
	{
		// Generate contacts for all that were not filtered above
		pairFlags = physx::PxPairFlag::eCONTACT_DEFAULT;
		pairFlags |= physx::PxPairFlag::eNOTIFY_CONTACT_POINTS;
		pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_FOUND;
		pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_PERSISTS;
		pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_LOST;
	}

	return physx::PxFilterFlags();
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
	sceneDesc.filterShader = FilterShader;
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

		// Update colliders
		for (std::vector<ComponentCollider*>::const_iterator it = colliderComponents.begin(); it != colliderComponents.end(); ++it)
			(*it)->Update();

		// Update rigid actors
		for (std::vector<ComponentRigidActor*>::const_iterator it = rigidActorComponents.begin(); it != rigidActorComponents.end(); ++it)
			(*it)->Update();
	}

	return UPDATE_CONTINUE;
}

update_status ModulePhysics::PostUpdate()
{
	return UPDATE_CONTINUE;
}

bool ModulePhysics::CleanUp()
{
	colliderComponents.clear();
	rigidActorComponents.clear();

	gScene->release();
	gScene = nullptr;

	gPhysics->release();
	gPhysics = nullptr;

	gFoundation->release();
	gFoundation = nullptr;

	return true;
}

bool ModulePhysics::OnGameMode()
{
	// Set filtering
	for (uint i = 0; i < colliderComponents.size(); ++i)
	{
		Layer* layer = App->layers->GetLayer(colliderComponents[i]->GetParent()->layer);
		colliderComponents[i]->SetFiltering(layer->GetFilterGroup(), layer->filterMask);
	}

	// -----

	return true;
}

bool ModulePhysics::OnEditorMode()
{
	// Reset filtering
	for (uint i = 0; i < colliderComponents.size(); ++i)
		colliderComponents[i]->SetFiltering(0, 0);

	// -----

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

void ModulePhysics::OnSimulationEvent(ComponentRigidActor* actor, SimulationEventTypes simulationEventType) const
{
	if (actor == nullptr)
		return;

	switch (simulationEventType)
	{
	case SimulationEventTypes::OnWake:
		actor->OnWake();
		break;
	case SimulationEventTypes::OnSleep:
		actor->OnSleep();
		break;
	}
}

void ModulePhysics::OnCollision(ComponentCollider* collider, Collision& collision, CollisionTypes collisionType) const
{
	assert(collider != nullptr);

	switch (collisionType)
	{
	case OnCollisionEnter:
		collider->OnCollisionEnter(collision);
		break;
	case OnCollisionStay:
		collider->OnCollisionStay(collision);
		break;
	case OnCollisionExit:
		collider->OnCollisionExit(collision);
		break;
	case OnTriggerEnter:
		collider->OnTriggerEnter(collision);
		break;
	case OnTriggerExit:
		collider->OnTriggerExit(collision);
		break;
	}
}

// ----------------------------------------------------------------------------------------------------

std::vector<ComponentRigidActor*> ModulePhysics::GetRigidActorComponents() const
{
	return rigidActorComponents;
}

ComponentRigidActor* ModulePhysics::FindRigidActorComponentByActor(physx::PxActor* actor) const
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

ComponentCollider* ModulePhysics::FindColliderComponentByShape(physx::PxShape* shape) const
{
	if (shape == nullptr)
		return nullptr;

	for (std::vector<ComponentCollider*>::const_iterator it = colliderComponents.begin(); it != colliderComponents.end(); ++it)
	{
		if ((*it)->GetShape() == shape)
			return *it;
	}

	return nullptr;
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