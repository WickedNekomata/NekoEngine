#ifndef __COMPONENT_D6_JOINT_H__
#define __COMPONENT_D6_JOINT_H__

#include "Component.h"
#include "ComponentJoint.h"

class ComponentD6Joint : public ComponentJoint
{
public:

	ComponentD6Joint(GameObject* parent, ComponentTypes componentJointType);
	ComponentD6Joint(const ComponentD6Joint& componentD6Joint);
	~ComponentD6Joint();

	void OnUniqueEditor();

	void Update();

	uint GetInternalSerializationBytes();
	void OnInternalSave(char*& cursor);
	void OnInternalLoad(char*& cursor);
};

#endif