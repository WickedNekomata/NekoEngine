#ifndef __MODULE_PHYSICS_H__
#define __MODULE_PHYSICS_H__

#include "Module.h"

#include "physx/include/PxPhysicsAPI.h"
#include "physx/include/extensions/PxDefaultAllocator.h"
#include "physx/include/extensions/PxDefaultCpuDispatcher.h"

#include "MathGeoLib/include/Math/float3.h"

#include <vector>

class ComponentRigidActor;
class ComponentCollider;
class GameObject;
class Collision;
enum ComponentTypes;
enum SimulationEventTypes;
enum CollisionTypes;

class DefaultErrorCallback : public physx::PxErrorCallback
{
public:

	DefaultErrorCallback();
	~DefaultErrorCallback();

	void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line);
};

// ----------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------

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

	void OnSystemEvent(System_Event event);

	bool OnGameMode();
	bool OnEditorMode();

	// ----------------------------------------------------------------------------------------------------

	// Physic elements
	/// Rigid actors
	physx::PxRigidStatic* CreateRigidStatic(const physx::PxTransform& transform, physx::PxShape& shape) const;
	physx::PxRigidDynamic* CreateRigidDynamic(const physx::PxTransform& transform, physx::PxShape& shape, float density, bool isKinematic = false) const;	
	void AddActor(physx::PxActor& actor) const;
	void RemoveActor(physx::PxActor& actor) const;

	/// Shapes
	physx::PxShape* CreateShape(const physx::PxGeometry& geometry, const physx::PxMaterial& material, bool isExclusive = true) const;

	// ----------------------------------------------------------------------------------------------------

	// Components
	ComponentRigidActor* CreateRigidActorComponent(GameObject* parent, ComponentTypes componentRigidActorType);
	bool AddRigidActorComponent(ComponentRigidActor* toAdd);
	bool EraseRigidActorComponent(ComponentRigidActor* toErase);
	std::vector<ComponentRigidActor*> GetRigidActorComponents() const;
	ComponentRigidActor* FindRigidActorComponentByActor(physx::PxActor* actor) const;

	ComponentCollider* CreateColliderComponent(GameObject* parent, ComponentTypes componentColliderType);
	bool AddColliderComponent(ComponentCollider* toAdd);
	bool EraseColliderComponent(ComponentCollider* toErase);
	std::vector<ComponentCollider*> GetColliderComponents() const;
	ComponentCollider* FindColliderComponentByShape(physx::PxShape* shape) const;

	// ----------------------------------------------------------------------------------------------------

	// Callbacks
	void OnSimulationEvent(ComponentRigidActor* actor, SimulationEventTypes simulationEventType) const;
	void OnCollision(ComponentCollider* collider, Collision& collision, CollisionTypes collisionType) const;

	// ----------------------------------------------------------------------------------------------------

	// General configuration values
	void SetGravity(math::float3 gravity);
	math::float3 GetGravity() const;

	void SetDefaultMaterial(physx::PxMaterial* material);
	physx::PxMaterial* GetDefaultMaterial() const;

private:

	physx::PxFoundation* gFoundation = nullptr;
	physx::PxPhysics* gPhysics = nullptr;
	physx::PxScene* gScene = nullptr;
	physx::PxDefaultCpuDispatcher* gDispatcher = nullptr;

	float gAccumulator = 0.0f;

	// -----

	// Components
	std::vector<ComponentRigidActor*> rigidActorComponents;
	std::vector<ComponentCollider*> colliderComponents;

	// General configuration values
	math::float3 gravity = math::float3::zero;
	physx::PxMaterial* defaultMaterial = nullptr;
};

#endif