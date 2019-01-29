#ifndef __MODULE_PHYSICS_H__
#define __MODULE_PHYSICS_H__

#include "Module.h"

#include "physx/include/PxPhysicsAPI.h"
#include "physx/include/extensions/PxDefaultAllocator.h"
#include "physx/include/extensions/PxDefaultCpuDispatcher.h"

#include <vector>

class ComponentCollider;
enum ComponentTypes;

class DefaultErrorCallback : public physx::PxErrorCallback
{
public:
	DefaultErrorCallback();
	~DefaultErrorCallback();

	void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line);
};

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

	physx::PxRigidStatic* CreateRigidStatic(const physx::PxTransform& transform, physx::PxShape& shape) const;
	physx::PxRigidDynamic* CreateRigidDynamic(const physx::PxTransform& transform, physx::PxShape& shape, float density, bool isKinematic = false) const;

	physx::PxShape* CreateShape(const physx::PxGeometry& geometry, const physx::PxMaterial& material, bool isExclusive = true) const;

	ComponentCollider* CreateColliderComponent(GameObject* parent, ComponentTypes componentColliderType);
	bool AddColliderComponent(ComponentCollider* toAdd);
	bool EraseColliderComponent(ComponentCollider* toErase);

	void RemoveActor(physx::PxActor& actor) const;

	physx::PxMaterial* GetDefaultMaterial() const;

	std::vector<physx::PxRigidActor*> GetRigidStatics() const;
	std::vector<physx::PxRigidActor*> GetRigidDynamics() const;

	std::vector<ComponentCollider*> GetColliderComponents() const;

private:

	physx::PxFoundation* gFoundation = nullptr;
	physx::PxPhysics* gPhysics = nullptr;
	physx::PxScene* gScene = nullptr;
	physx::PxDefaultCpuDispatcher*	gDispatcher = nullptr;
	physx::PxMaterial*	gMaterial = nullptr;

	float gAccumulator = 0.0f;

	std::vector<ComponentCollider*> colliderComponents;
};

#endif