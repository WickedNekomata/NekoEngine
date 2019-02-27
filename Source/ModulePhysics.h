#ifndef __MODULE_PHYSICS_H__
#define __MODULE_PHYSICS_H__

#include "Module.h"

#include "ModuleLayers.h"
#include "SceneQueries.h"

#include "physx\include\PxPhysicsAPI.h"
#include "physx\include\extensions\PxDefaultAllocator.h"
#include "physx\include\extensions\PxDefaultCpuDispatcher.h"

#include "MathGeoLib\include\Math\float3.h"
#include "MathGeoLib\include\Math\MathConstants.h"

// *****Debug*****
#include "MathGeoLib\include\Geometry\Ray.h"
#include "MathGeoLib\include\Math\float4x4.h"
//_*****Debug*****

#include <vector>

class ComponentRigidActor;
class ComponentCollider;
class ComponentJoint;
class GameObject;
class Collision;
class RaycastHit;
class SweepHit;
class OverlapHit;
class SimulationEventCallback;
enum ComponentTypes;
enum SimulationEventTypes;
enum CollisionTypes;
enum JointTypes;

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
	update_status FixedUpdate();
	update_status PostUpdate();
	bool CleanUp();

	void OnSystemEvent(System_Event event);

	void Debug();

	// ----------------------------------------------------------------------------------------------------

	// Physic elements
	/// Rigid Actors
	physx::PxRigidStatic* CreateRigidStatic(const physx::PxTransform& transform, physx::PxShape& shape) const;
	physx::PxRigidDynamic* CreateRigidDynamic(const physx::PxTransform& transform, physx::PxShape& shape, float density, bool isKinematic = false) const;
	void AddActor(physx::PxActor& actor) const;
	void RemoveActor(physx::PxActor& actor) const;

	/// Colliders
	physx::PxShape* CreateShape(const physx::PxGeometry& geometry, const physx::PxMaterial& material, bool isExclusive = true) const;

	/// Joints
	physx::PxJoint* CreateJoint(JointTypes jointType, physx::PxRigidActor* actor0, const physx::PxTransform& localFrame0, physx::PxRigidActor* actor1, physx::PxTransform& localFrame1) const;

	// ----------------------------------------------------------------------------------------------------

	// Components
	/// Rigid Actors
	ComponentRigidActor* CreateRigidActorComponent(GameObject* parent, ComponentTypes componentRigidActorType);
	bool AddRigidActorComponent(ComponentRigidActor* toAdd);
	bool EraseRigidActorComponent(ComponentRigidActor* toErase);
	std::vector<ComponentRigidActor*> GetRigidActorComponents() const;
	ComponentRigidActor* FindRigidActorComponentByActor(physx::PxActor* actor) const;

	/// Colliders
	ComponentCollider* CreateColliderComponent(GameObject* parent, ComponentTypes componentColliderType);
	bool AddColliderComponent(ComponentCollider* toAdd);
	bool EraseColliderComponent(ComponentCollider* toErase);
	std::vector<ComponentCollider*> GetColliderComponents() const;
	ComponentCollider* FindColliderComponentByShape(physx::PxShape* shape) const;

	/// Joints
	ComponentJoint* CreateJointComponent(GameObject* parent, ComponentTypes componentJointType);
	bool AddJointComponent(ComponentJoint* toAdd);
	bool EraseJointComponent(ComponentJoint* toErase);
	std::vector<ComponentJoint*> GetJointComponents() const;
	ComponentJoint* FindJointComponentByJoint(physx::PxJoint* joint) const;

	// ----------------------------------------------------------------------------------------------------

	// Simulation events
	void OnSimulationEvent(ComponentRigidActor* actor, SimulationEventTypes simulationEventType) const;
	void OnCollision(ComponentCollider* collider, Collision& collision, CollisionTypes collisionType) const;

	// ----------------------------------------------------------------------------------------------------

	// Scene queries
	/// They require a Rigid Actor component and a Collider component
	bool Raycast(math::float3& origin, math::float3& direction, RaycastHit& hitInfo, std::vector<RaycastHit>& touchesInfo, float maxDistance = FLT_MAX, uint filterMask = DEFAULT_FILTER_MASK, SceneQueryFlags sceneQueryFlags = (SceneQueryFlags)(SceneQueryFlags::Static | SceneQueryFlags::Dynamic)) const;
	bool Raycast(math::float3& origin, math::float3& direction, RaycastHit& hitInfo, float maxDistance = FLT_MAX, uint filterMask = DEFAULT_FILTER_MASK, SceneQueryFlags sceneQueryFlags = (SceneQueryFlags)(SceneQueryFlags::Static | SceneQueryFlags::Dynamic)) const;
	bool Raycast(math::float3& origin, math::float3& direction, std::vector<RaycastHit>& touchesInfo, float maxDistance = FLT_MAX, uint filterMask = DEFAULT_FILTER_MASK, SceneQueryFlags sceneQueryFlags = (SceneQueryFlags)(SceneQueryFlags::Static | SceneQueryFlags::Dynamic)) const;

	bool Sweep(physx::PxGeometry& geometry, physx::PxTransform& transform, math::float3& direction, SweepHit& hitInfo, float maxDistance = FLT_MAX, float inflation = 0.0f, uint filterMask = DEFAULT_FILTER_MASK, SceneQueryFlags sceneQueryFlags = (SceneQueryFlags)(SceneQueryFlags::Static | SceneQueryFlags::Dynamic)) const;

	bool Overlap(physx::PxGeometry& geometry, physx::PxTransform& transform, std::vector<OverlapHit>& touchesInfo, uint filterMask = DEFAULT_FILTER_MASK, SceneQueryFlags sceneQueryFlags = (SceneQueryFlags)(SceneQueryFlags::Static | SceneQueryFlags::Dynamic)) const;
	bool OverlapSphere(float radius, math::float3 center, std::vector<OverlapHit>& touchesInfo, uint filterMask = DEFAULT_FILTER_MASK, SceneQueryFlags sceneQueryFlags = (SceneQueryFlags)(SceneQueryFlags::Static | SceneQueryFlags::Dynamic)) const;

	// ----------------------------------------------------------------------------------------------------

	// General configuration values
	void SetGravity(math::float3& gravity);
	math::float3 GetGravity() const;

	void SetDefaultMaterial(physx::PxMaterial* material);
	physx::PxMaterial* GetDefaultMaterial() const;

	physx::PxTolerancesScale GetTolerancesScale() const;

private:

	physx::PxFoundation* gFoundation = nullptr;
	physx::PxPhysics* gPhysics = nullptr;
	physx::PxScene* gScene = nullptr;
	physx::PxDefaultCpuDispatcher* gDispatcher = nullptr;

	SimulationEventCallback* simulationEventCallback = nullptr;

	float gAccumulator = 0.0f;

	// -----

	// Components
	std::vector<ComponentRigidActor*> rigidActorComponents;
	std::vector<ComponentCollider*> colliderComponents;
	std::vector<ComponentJoint*> jointComponents;

	// General configuration values
	math::float3 gravity = math::float3::zero;
	physx::PxMaterial* defaultMaterial = nullptr;

	// *****Debug*****
	math::Ray debugRay;
	float debugRadius = 1.0f;
	math::float4x4 debugTransform = math::float4x4::identity;
	//_*****Debug*****
};

#endif