#ifndef __MODULE_PHYSICS_H__
#define __MODULE_PHYSICS_H__

#include "Module.h"

#include "physx/include/PxPhysicsAPI.h"
#include "physx/include/extensions/PxDefaultAllocator.h"
#include "physx/include/extensions/PxDefaultCpuDispatcher.h"

#include <vector>

using namespace physx;

class DefaultErrorCallback : public PxErrorCallback
{
public:
	DefaultErrorCallback();
	~DefaultErrorCallback();

	void reportError(PxErrorCode::Enum code, const char* message, const char* file, int line);
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

	PxRigidStatic* CreateRigidStatic(const PxTransform& transform, PxShape& shape) const;
	PxRigidDynamic* CreateRigidDynamic(const PxTransform& transform, PxShape& shape, float density, bool isKinematic = false) const;

	PxShape* CreateShape(const PxGeometry& geometry, const PxMaterial& material, bool isExclusive = true) const;

	void RemoveActor(PxActor& actor) const;

	PxMaterial* GetDefaultMaterial() const;

	std::vector<PxRigidActor*> GetRigidStatics() const;
	std::vector<PxRigidActor*> GetRigidDynamics() const;

private:

	PxFoundation* gFoundation = nullptr;
	PxPhysics* gPhysics = nullptr;
	PxScene* gScene = nullptr;
	PxDefaultCpuDispatcher*	gDispatcher = nullptr;
	PxMaterial*	gMaterial = nullptr;

	float gAccumulator = 0.0f;
};

#endif