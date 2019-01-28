#ifndef __COMPONENT_RIGID_STATIC_H__
#define __COMPONENT_RIGID_STATIC_H__

#include "Component.h"
#include "ComponentRigidBody.h"

class ComponentRigidStatic : public ComponentRigidBody
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