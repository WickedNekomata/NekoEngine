#ifndef __COMPONENT_COLLIDER_H__
#define __COMPONENT_COLLIDER_H__

#include "Component.h"

#include "physx\include\PxPhysicsAPI.h"

#include "MathGeoLib\include\Math\float3.h"

#include "SimulationEvents.h"

class ComponentCollider : public Component
{
public:

	ComponentCollider(GameObject* parent, ComponentTypes componentColliderType);
	//ComponentCollider(const ComponentRigidActor& componentRigidActor);
	virtual ~ComponentCollider();

	virtual void OnUniqueEditor();

	virtual void Update();

	virtual void ClearShape();
	virtual void RecalculateShape() = 0;
	void SetFiltering(physx::PxU32 filterGroup, physx::PxU32 filterMask);

	void SetIsTrigger(bool isTrigger);
	void ParticipateInContactTests(bool participateInContactTests);
	void ParticipateInSceneQueries(bool participateInSceneQueries);

	physx::PxShape* GetShape() const;

	void OnCollisionEnter(Collision& collision);
	void OnCollisionStay(Collision& collision);
	void OnCollisionExit(Collision& collision);
	void OnTriggerEnter(Collision& collision);
	void OnTriggerStay(Collision& collision);
	void OnTriggerExit(Collision& collision);

	/// Transformed box, sphere, capsule or convex geometry
	static float GetPointToGeometryObjectDistance(const math::float3& point, const physx::PxGeometry& geometry, const physx::PxTransform& transform, math::float3& closestPoint = math::float3::zero);
	static physx::PxBounds3 GetGeometryObjectAABB(const physx::PxGeometry& geometry, const physx::PxTransform& pose, float inflation = 1.01f);

	//void OnInternalSave(JSON_Object* file);
	//void OnLoad(JSON_Object* file);

protected:

	bool isTrigger = false;
	bool participateInContactTests = true;
	bool participateInSceneQueries = true;
	physx::PxMaterial* gMaterial = nullptr;
	math::float3 center = math::float3::zero;

	physx::PxShape* gShape = nullptr;

private:

	bool triggerEnter = false;
	bool triggerExit = false;
	Collision collision;
};

#endif