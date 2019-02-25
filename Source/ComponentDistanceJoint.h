#ifndef __COMPONENT_DISTANCE_JOINT_H__
#define __COMPONENT_DISTANCE_JOINT_H__

#include "Component.h"
#include "ComponentJoint.h"

#include "MathGeoLib\include\Math\float2.h"

class ComponentDistanceJoint : public ComponentJoint
{
public:

	ComponentDistanceJoint(GameObject* parent, ComponentTypes componentJointType);
	ComponentDistanceJoint(const ComponentDistanceJoint& componentDistanceJoint);
	~ComponentDistanceJoint();

	void OnUniqueEditor();

	void Update();

	uint GetInternalSerializationBytes();
	void OnInternalSave(char*& cursor);
	void OnInternalLoad(char*& cursor);

	// ----------------------------------------------------------------------------------------------------

	// Sets
	void SetMinDistance(float minDistance);
	void SetMaxDistance(float maxDistance);
	void SetStiffness(float stiffness);
	void SetDamping(float damping);
	void SetFlag(physx::PxDistanceJointFlag::Enum flag, bool value);

private:

	math::float2 limitDistance = math::float2::zero;

	float stiffness = 0.0f;
	float damping = 0.0f;

	uint flags = 0;
};

#endif