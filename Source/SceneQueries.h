#ifndef __SCENE_QUERIES_H__
#define __SCENE_QUERIES_H__

#include "Globals.h"

#include "physx\include\PxPhysicsAPI.h"

#include "MathGeoLib\include\Math\float3.h"
#include "MathGeoLib\include\Math\float2.h"

class ComponentRigidActor;
class ComponentCollider;
class GameObject;

class RaycastHit
{
public:

	RaycastHit();
	RaycastHit(GameObject* gameObject, ComponentCollider* collider, ComponentRigidActor* actor, math::float3 point, math::float3 normal, math::float2 texCoord, float distance, uint faceIndex);
	~RaycastHit();

	void SetGameObject(GameObject* gameObject);
	void SetCollider(ComponentCollider* collider);
	void SetActor(ComponentRigidActor* actor);
	void SetPoint(math::float3& point);
	void SetNormal(math::float3& normal);
	void SetTexCoord(math::float2& texCoord);
	void SetDistance(float distance);
	void SetFaceIndex(uint faceIndex);

	GameObject* GetGameObject() const;
	ComponentCollider* GetCollider() const;
	ComponentRigidActor* GetActor() const;
	math::float3 GetPoint() const;
	math::float3 GetNormal() const;
	math::float2 GetTexCoord() const;
	float GetDistance() const;
	uint GetFaceIndex() const;

private:

	GameObject* gameObject = nullptr; // the game object that was hit
	ComponentCollider* collider = nullptr; // the collider that was hit
	ComponentRigidActor* actor = nullptr; // the actor that was hit
	math::float3 point = math::float3::zero;
	math::float3 normal = math::float3::zero;
	math::float2 texCoord = math::float2::zero;
	float distance = 0.0f; // the distance from the origin to the impact point
	uint faceIndex = 0; // only for triangle meshes
};

// ----------------------------------------------------------------------------------------------------

class SweepHit
{
public:

	SweepHit();
	SweepHit(GameObject* gameObject, ComponentCollider* collider, ComponentRigidActor* actor, math::float3 point, math::float3 normal, float distance, uint faceIndex);
	~SweepHit();

	void SetGameObject(GameObject* gameObject);
	void SetCollider(ComponentCollider* collider);
	void SetActor(ComponentRigidActor* actor);
	void SetPoint(math::float3& point);
	void SetNormal(math::float3& normal);
	void SetDistance(float distance);
	void SetFaceIndex(uint faceIndex);

	GameObject* GetGameObject() const;
	ComponentCollider* GetCollider() const;
	ComponentRigidActor* GetActor() const;
	math::float3 GetPoint() const;
	math::float3 GetNormal() const;
	float GetDistance() const;
	uint GetFaceIndex() const;

private:

	GameObject* gameObject = nullptr; // the game object that was hit
	ComponentCollider* collider = nullptr; // the collider that was hit
	ComponentRigidActor* actor = nullptr; // the actor that was hit
	math::float3 point = math::float3::zero;
	math::float3 normal = math::float3::zero;
	float distance = 0.0f; // the distance from the origin to the impact point
	uint faceIndex = 0; // only for triangle meshes
};

// ----------------------------------------------------------------------------------------------------

class OverlapHit
{
public:

	OverlapHit();
	OverlapHit(GameObject* gameObject, ComponentCollider* collider, ComponentRigidActor* actor, uint faceIndex);
	~OverlapHit();

	void SetGameObject(GameObject* gameObject);
	void SetCollider(ComponentCollider* collider);
	void SetActor(ComponentRigidActor* actor);
	void SetFaceIndex(uint faceIndex);

	GameObject* GetGameObject() const;
	ComponentCollider* GetCollider() const;
	ComponentRigidActor* GetActor() const;
	uint GetFaceIndex() const;

private:

	GameObject* gameObject = nullptr; // the game object that was hit
	ComponentCollider* collider = nullptr; // the collider that was hit
	ComponentRigidActor* actor = nullptr; // the actor that was hit
	uint faceIndex = 0; // only for triangle meshes
};

// ----------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------

class QueryFilterCallback : public physx::PxQueryFilterCallback
{
public:

	QueryFilterCallback();
	~QueryFilterCallback();

	physx::PxQueryHitType::Enum preFilter(const physx::PxFilterData& filterData, const physx::PxShape* shape, const physx::PxRigidActor* actor, physx::PxHitFlags& queryFlags);
	physx::PxQueryHitType::Enum postFilter(const physx::PxFilterData& filterData, const physx::PxQueryHit& hit);
};

#endif