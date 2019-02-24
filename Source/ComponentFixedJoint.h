#ifndef __COMPONENT_FIXED_JOINT_H__
#define __COMPONENT_FIXED_JOINT_H__

#include "Component.h"
#include "ComponentJoint.h"

class ComponentFixedJoint : public ComponentJoint
{
public:

	ComponentFixedJoint(GameObject* parent, ComponentTypes componentJointType);
	ComponentFixedJoint(const ComponentFixedJoint& componentFixedJoint);
	~ComponentFixedJoint();

	void OnUniqueEditor();

	void Update();

	uint GetInternalSerializationBytes();
	void OnInternalSave(char*& cursor);
	void OnInternalLoad(char*& cursor);
};

#endif