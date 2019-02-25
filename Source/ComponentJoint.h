#ifndef __COMPONENT_JOINT_H__
#define __COMPONENT_JOINT_H__

#include "Component.h"

#include "physx\include\PxPhysicsAPI.h"

#include "MathGeoLib\include\Math\float3.h"

class ComponentRigidActor;

enum JointTypes
{
	NoJoint,
	FixedJoint,
	DistanceJoint,
	SphericalJoint,
	RevoluteJoint,
	PrismaticJoint,
	D6Joint
};

/*
Joints:
- One of the actors must be a PxRigidDynamic or a PxArticulationLink
*/

class ComponentJoint : public Component
{
public:

	ComponentJoint(GameObject* parent, ComponentTypes componentJointType);
	ComponentJoint(const ComponentJoint& componentJoint, ComponentTypes componentJointType);
	virtual ~ComponentJoint();

	virtual void OnUniqueEditor();

	virtual void Update();

	virtual uint GetInternalSerializationBytes() = 0;
	virtual void OnInternalSave(char*& cursor) = 0;
	virtual void OnInternalLoad(char*& cursor) = 0;

	// ----------------------------------------------------------------------------------------------------

	void RecalculateJoint();

	// Sets
	void SetCenter(physx::PxJointActorIndex::Enum actor, math::float3& center);
	void SetBreakForce(float breakForce);
	void SetBreakTorque(float breakTorque);
	void SetMassScale(physx::PxJointActorIndex::Enum actor, float massScale);

	// Gets
	physx::PxJoint* GetJoint() const;
	JointTypes GetJointType() const;
	bool IsBroken() const;

protected:

	ComponentRigidActor* connectedBody = nullptr;
	math::float3 center = math::float3::zero;
	math::float3 connectedCenter = math::float3::zero;

	// Breakage
	float breakForce = FLT_MAX;
	float breakTorque = FLT_MAX;

	// Mass scaling
	float massScale = 0.0f;
	float connectedMassScale = 0.0f;

	// -----

	physx::PxJoint* gJoint = nullptr;
	JointTypes jointType = JointTypes::NoJoint;
};

#endif