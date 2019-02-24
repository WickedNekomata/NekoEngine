#ifndef __COMPONENT_SPHERICAL_JOINT_H__
#define __COMPONENT_SPHERICAL_JOINT_H__

#include "Component.h"
#include "ComponentJoint.h"

#include "MathGeoLib\include\Math\float2.h"

// Ball-and-socket

class ComponentSphericalJoint : public ComponentJoint
{
public:

	ComponentSphericalJoint(GameObject* parent, ComponentTypes componentJointType);
	ComponentSphericalJoint(const ComponentSphericalJoint& componentSphericalJoint);
	~ComponentSphericalJoint();

	void OnUniqueEditor();

	void Update();

	uint GetInternalSerializationBytes();
	void OnInternalSave(char*& cursor);
	void OnInternalLoad(char*& cursor);

	// ----------------------------------------------------------------------------------------------------
	
	// Sets
	void SetHardLimit(math::float2& limitAngle, float contactDistance);
	void SetSoftLimit(math::float2& limitAngle, float stiffness, float damping);
	void SetFlag(physx::PxSphericalJointFlag::Enum flag, bool value);

private:

	// Limits
	math::float2 limitAngle = math::float2::zero;
	float contactDistance = -1.0f;
	bool useSpring = false;
	float stiffness = 0.0f;
	float damping = 0.0f;

	uint flags = 0;
};

#endif