#ifndef __COMPONENT_COLLIDER_H__
#define __COMPONENT_COLLIDER_H__

#include "Component.h"

#include "SimulationEvents.h"

#include "physx\include\PxPhysicsAPI.h"

#include "MathGeoLib\include\Math\float3.h"
#include "MathGeoLib\include\Math\Quat.h"

enum ColliderTypes
{
	NoCollider,
	BoxCollider,
	CapsuleCollider,
	PlaneCollider,
	SphereCollider
};

class ComponentCollider : public Component
{
public:

	ComponentCollider(GameObject* parent, ComponentTypes componentColliderType);
	ComponentCollider(const ComponentCollider& componentCollider, GameObject* parent, ComponentTypes componentColliderType);
	virtual ~ComponentCollider();

	virtual void OnUniqueEditor();

	virtual void Update();

	virtual uint GetInternalSerializationBytes();
	virtual void OnInternalSave(char*& cursor);
	virtual void OnInternalLoad(char*& cursor);

	// ----------------------------------------------------------------------------------------------------

	virtual void EncloseGeometry() = 0;
	virtual void RecalculateShape() = 0;
	void ClearShape();
	void SetFiltering(physx::PxU32 filterGroup, physx::PxU32 filterMask);

	// Sets
	void SetIsTrigger(bool isTrigger);
	void SetParticipateInContactTests(bool participateInContactTests);
	void SetParticipateInSceneQueries(bool participateInSceneQueries);
	virtual void SetCenter(const math::float3& center);

	// Gets
	physx::PxShape* GetShape() const;
	ColliderTypes GetColliderType() const;

	// Callbacks
	void OnCollisionEnter(Collision& collision);
	void OnCollisionStay(Collision& collision);
	void OnCollisionExit(Collision& collision);
	void OnTriggerEnter(Collision& collision);
	void OnTriggerStay(Collision& collision);
	void OnTriggerExit(Collision& collision);

	// Utils
	static float GetPointToGeometryObjectDistance(const math::float3& point, const physx::PxGeometry& geometry, const physx::PxTransform& pose);
	static float GetPointToGeometryObjectDistance(const math::float3& point, const physx::PxGeometry& geometry, const physx::PxTransform& pose, math::float3& closestPoint);
	static physx::PxBounds3 GetGeometryObjectAABB(const physx::PxGeometry& geometry, const physx::PxTransform& pose, float inflation = 1.01f);

protected:

	bool isTrigger = false;
	bool participateInContactTests = true;
	bool participateInSceneQueries = true;
	physx::PxMaterial* gMaterial = nullptr; // TODO
	math::float3 center = math::float3::zero;

	uint filterGroup = 0;
	uint filterMask = 0;

	// -----

	physx::PxShape* gShape = nullptr;
	ColliderTypes colliderType = ColliderTypes::NoCollider;

private:

	bool triggerEnter = false;
	bool triggerExit = false;
	Collision collision;
};

#endif