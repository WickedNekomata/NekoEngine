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

	void Update();

	void OnUniqueEditor();

	//void OnInternalSave(JSON_Object* file);
	//void OnLoad(JSON_Object* file);

private:

};

#endif