#ifndef __COMPONENT_PRISMATIC_JOINT_H__
#define __COMPONENT_PRISMATIC_JOINT_H__

#include "Component.h"
#include "ComponentJoint.h"

#include "MathGeoLib\include\Math\float2.h"

// Slider

class ComponentPrismaticJoint : public ComponentJoint
{
public:

	ComponentPrismaticJoint(GameObject* parent, ComponentTypes componentJointType);
	ComponentPrismaticJoint(const ComponentPrismaticJoint& componentPrismaticJoint);
	~ComponentPrismaticJoint();

	void OnUniqueEditor();

	void Update();

	uint GetInternalSerializationBytes();
	void OnInternalSave(char*& cursor);
	void OnInternalLoad(char*& cursor);

	// ----------------------------------------------------------------------------------------------------

	// Sets
	void SetHardLimit(math::float2& limitAngle, float contactDistance);
	void SetSoftLimit(math::float2& limitAngle, float stiffness, float damping);
	void SetFlag(physx::PxPrismaticJointFlag::Enum flag, bool value);

private:

	// Limits
	math::float2 limitDistance = math::float2::zero;
	float contactDistance = -1.0f;
	bool useSpring = false;
	float stiffness = 0.0f;
	float damping = 0.0f;

	uint flags = 0;
};

#endif