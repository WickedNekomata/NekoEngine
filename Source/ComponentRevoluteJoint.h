#ifndef __COMPONENT_REVOLUTE_JOINT_H__
#define __COMPONENT_REVOLUTE_JOINT_H__

#include "Component.h"
#include "ComponentJoint.h"

#include "MathGeoLib\include\Math\float2.h"

// Hinge

class ComponentRevoluteJoint : public ComponentJoint
{
public:

	ComponentRevoluteJoint(GameObject* parent, ComponentTypes componentJointType);
	ComponentRevoluteJoint(const ComponentRevoluteJoint& componentRevoluteJoint);
	~ComponentRevoluteJoint();

	void OnUniqueEditor();

	void Update();

	uint GetInternalSerializationBytes();
	void OnInternalSave(char*& cursor);
	void OnInternalLoad(char*& cursor);

	// ----------------------------------------------------------------------------------------------------

	// Sets
	void SetHardLimit(math::float2& limitAngle, float contactDistance);
	void SetSoftLimit(math::float2& limitAngle, float stiffness, float damping);
	void SetTargetVelocity(float targetVelocity);
	void SetForce(float force);
	void SetFlag(physx::PxRevoluteJointFlag::Enum flag, bool value);

private:

	// Limits
	math::float2 limitAngle = math::float2::zero;
	float contactDistance = -1.0f;
	bool useSpring = false;
	float stiffness = 0.0f;
	float damping = 0.0f;

	// Motor
	float targetVelocity = 0.0f;
	float force = 0.0f;

	uint flags = 0;
};

#endif