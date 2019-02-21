#ifndef __COMPONENT_RIGID_STATIC_H__
#define __COMPONENT_RIGID_STATIC_H__

#include "Component.h"
#include "ComponentRigidActor.h"

class ComponentRigidStatic : public ComponentRigidActor
{
public:

	ComponentRigidStatic(GameObject* parent);
	//ComponentRigidBody(const ComponentRigidBody& componentRigidBody);
	~ComponentRigidStatic();

	void OnUniqueEditor();

	void Update();

	uint GetInternalSerializationBytes();
	void OnInternalLoad(char*& cursor) {}
	void OnInternalSave(char*& cursor) {}
	//void OnInternalSave(JSON_Object* file);
	//void OnLoad(JSON_Object* file);
};

#endif