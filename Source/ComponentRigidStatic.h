#ifndef __COMPONENT_RIGID_STATIC_H__
#define __COMPONENT_RIGID_STATIC_H__

#include "Component.h"
#include "ComponentRigidActor.h"

class ComponentRigidStatic : public ComponentRigidActor
{
public:

	ComponentRigidStatic(GameObject* parent);
	ComponentRigidStatic(const ComponentRigidStatic& componentRigidStatic);
	~ComponentRigidStatic();

	void OnUniqueEditor();

	void Update();

	uint GetInternalSerializationBytes();
	void OnInternalSave(char*& cursor);
	void OnInternalLoad(char*& cursor);
};

#endif